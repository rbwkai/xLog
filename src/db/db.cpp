#include "xlog/db.hpp"
#include "xlog/math/xp.hpp"
#include "xlog/math/scoring.hpp"
#include "xlog/math/calibration.hpp"
#include "xlog/task_engine.hpp"
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <chrono>
#include <iomanip>
#include <cmath>

namespace xlog {

static std::string getCurrentLocalDate() {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d");
    return ss.str();
}

static int64_t getCurrentEpoch() {
    return std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
}

Database::Database(const std::string& db_path) {
    if (sqlite3_open(db_path.c_str(), &db_) != SQLITE_OK) {
        std::string err = sqlite3_errmsg(db_);
        throw std::runtime_error("Failed to open SQLite database: " + err);
    }
    
    execSql("PRAGMA journal_mode = WAL;");
    execSql("PRAGMA synchronous = NORMAL;");
    execSql("PRAGMA foreign_keys = ON;");
    execSql("PRAGMA busy_timeout = 5000;");
}

Database::~Database() {
    if (db_) {
        sqlite3_close(db_);
        db_ = nullptr;
    }
}

void Database::execSql(const std::string& sql) {
    char* err_msg = nullptr;
    if (sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &err_msg) != SQLITE_OK) {
        std::string err = err_msg ? err_msg : "Unknown error";
        sqlite3_free(err_msg);
        throw std::runtime_error("SQL execution error: " + err + "\nSQL: " + sql);
    }
}

void Database::initializeSchema() {
    std::string schema = R"(
        CREATE TABLE IF NOT EXISTS users (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            username TEXT NOT NULL UNIQUE,
            timezone TEXT NOT NULL DEFAULT 'UTC',
            day_boundary_hour INTEGER NOT NULL DEFAULT 4,
            rating_current REAL NOT NULL DEFAULT 1.0,
            rank_current TEXT NOT NULL DEFAULT 'Gray',
            debt_current REAL NOT NULL DEFAULT 0,
            grace_tokens INTEGER NOT NULL DEFAULT 0,
            streak_days INTEGER NOT NULL DEFAULT 0,
            longest_streak INTEGER NOT NULL DEFAULT 0,
            daily_budget_ema REAL NOT NULL DEFAULT 0,
            crit_pity_k INTEGER NOT NULL DEFAULT 0,
            last_active_date TEXT
        );

        CREATE TABLE IF NOT EXISTS domains (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            user_id INTEGER NOT NULL REFERENCES users(id) ON DELETE CASCADE,
            name TEXT NOT NULL,
            order_index INTEGER NOT NULL,
            color_code TEXT,
            score_cached REAL NOT NULL DEFAULT 1.0,
            UNIQUE(user_id, order_index),
            UNIQUE(user_id, name)
        );

        CREATE TABLE IF NOT EXISTS subdomains (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            domain_id INTEGER NOT NULL REFERENCES domains(id) ON DELETE CASCADE,
            name TEXT NOT NULL,
            order_index INTEGER NOT NULL,
            xp_raw_total REAL NOT NULL DEFAULT 0,
            xp_eff_cached REAL NOT NULL DEFAULT 0,
            score_cached REAL NOT NULL DEFAULT 0,
            last_activity_date TEXT,
            UNIQUE(domain_id, order_index),
            UNIQUE(domain_id, name)
        );

        CREATE TABLE IF NOT EXISTS tasks (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            user_id INTEGER NOT NULL REFERENCES users(id) ON DELETE CASCADE,
            name TEXT NOT NULL,
            type TEXT NOT NULL,
            major_subdomain_id INTEGER NOT NULL REFERENCES subdomains(id),
            minor_subdomain_id INTEGER NOT NULL REFERENCES subdomains(id),
            difficulty_current REAL NOT NULL,
            difficulty_original REAL NOT NULL,
            cr_ema REAL NOT NULL DEFAULT 0.775,
            priority_base REAL NOT NULL,
            priority_current REAL NOT NULL DEFAULT 0.5,
            started_at_date TEXT,
            period_days INTEGER,
            recurrence_mask INTEGER,
            due_today INTEGER NOT NULL DEFAULT 0,
            first_appeared_date TEXT,
            reward_unlock TEXT,
            status TEXT NOT NULL DEFAULT 'active',
            last_completed_date TEXT,
            total_completions INTEGER NOT NULL DEFAULT 0,
            total_xp_earned REAL NOT NULL DEFAULT 0
        );

        CREATE TABLE IF NOT EXISTS task_completions (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            task_id INTEGER NOT NULL REFERENCES tasks(id) ON DELETE CASCADE,
            completed_at INTEGER NOT NULL,
            local_date TEXT NOT NULL,
            t_actual REAL NOT NULL,
            difficulty_at_completion REAL NOT NULL,
            xp_raw REAL NOT NULL,
            m_frog REAL NOT NULL DEFAULT 1,
            m_crit REAL NOT NULL DEFAULT 1,
            m_streak REAL NOT NULL DEFAULT 1,
            m_balance REAL NOT NULL DEFAULT 1,
            xp_final REAL NOT NULL,
            xp_major REAL NOT NULL,
            xp_minor REAL NOT NULL,
            was_frog INTEGER NOT NULL DEFAULT 0,
            was_crit INTEGER NOT NULL DEFAULT 0,
            debt_redeemed REAL NOT NULL DEFAULT 0,
            cr_after REAL
        );

        CREATE TABLE IF NOT EXISTS daily_user_stats (
            user_id INTEGER NOT NULL REFERENCES users(id) ON DELETE CASCADE,
            local_date TEXT NOT NULL,
            xp_earned REAL NOT NULL DEFAULT 0,
            tasks_completed INTEGER NOT NULL DEFAULT 0,
            rating_snapshot REAL NOT NULL,
            rank_snapshot TEXT NOT NULL,
            PRIMARY KEY (user_id, local_date)
        );

        CREATE TABLE IF NOT EXISTS quotes (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            user_id INTEGER NOT NULL REFERENCES users(id) ON DELETE CASCADE,
            text TEXT NOT NULL,
            author TEXT,
            active INTEGER NOT NULL DEFAULT 1,
            times_shown INTEGER NOT NULL DEFAULT 0,
            last_shown_at INTEGER
        );
    )";
    execSql(schema);
}

std::optional<User> Database::getUser() {
    sqlite3_stmt* stmt = nullptr;
    std::string query = "SELECT id, username, timezone, day_boundary_hour, rating_current, rank_current, "
                        "debt_current, grace_tokens, streak_days, longest_streak, daily_budget_ema, "
                        "crit_pity_k, last_active_date FROM users LIMIT 1;";

    if (sqlite3_prepare_v2(db_, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        return std::nullopt;
    }

    std::optional<User> user;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        User u;
        u.id = sqlite3_column_int64(stmt, 0);
        u.username = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        u.timezone = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        u.day_boundary_hour = sqlite3_column_int(stmt, 3);
        u.rating_current = sqlite3_column_double(stmt, 4);
        u.rank_current = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        u.debt_current = sqlite3_column_double(stmt, 6);
        u.grace_tokens = sqlite3_column_int(stmt, 7);
        u.streak_days = sqlite3_column_int(stmt, 8);
        u.longest_streak = sqlite3_column_int(stmt, 9);
        u.daily_budget_ema = sqlite3_column_double(stmt, 10);
        u.crit_pity_k = sqlite3_column_int(stmt, 11);
        const unsigned char* last_act = sqlite3_column_text(stmt, 12);
        if (last_act) u.last_active_date = reinterpret_cast<const char*>(last_act);
        user = u;
    }
    sqlite3_finalize(stmt);
    return user;
}

User Database::onboardUser(const std::string& username, const std::vector<std::pair<std::string, std::vector<std::string>>>& domain_structure) {
    execSql("BEGIN TRANSACTION;");
    try {
        sqlite3_stmt* stmt = nullptr;
        std::string sql_user = "INSERT INTO users (username, last_active_date) VALUES (?, ?);";
        sqlite3_prepare_v2(db_, sql_user.c_str(), -1, &stmt, nullptr);
        std::string today = getCurrentLocalDate();
        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, today.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_step(stmt);
        int64_t user_id = sqlite3_last_insert_rowid(db_);
        sqlite3_finalize(stmt);

        int dom_idx = 0;
        std::vector<std::string> default_colors = {"Flamingo", "Sky", "Lavender", "Pink"};
        for (const auto& domain_pair : domain_structure) {
            std::string dom_name = domain_pair.first;
            std::string color = (dom_idx < static_cast<int>(default_colors.size())) ? default_colors[dom_idx] : "Text";

            sqlite3_stmt* dom_stmt = nullptr;
            std::string sql_dom = "INSERT INTO domains (user_id, name, order_index, color_code, score_cached) VALUES (?, ?, ?, ?, 1.0);";
            sqlite3_prepare_v2(db_, sql_dom.c_str(), -1, &dom_stmt, nullptr);
            sqlite3_bind_int64(dom_stmt, 1, user_id);
            sqlite3_bind_text(dom_stmt, 2, dom_name.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_int(dom_stmt, 3, dom_idx);
            sqlite3_bind_text(dom_stmt, 4, color.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_step(dom_stmt);
            int64_t dom_id = sqlite3_last_insert_rowid(db_);
            sqlite3_finalize(dom_stmt);

            int sub_idx = 0;
            for (const auto& sub_name : domain_pair.second) {
                sqlite3_stmt* sub_stmt = nullptr;
                std::string sql_sub = "INSERT INTO subdomains (domain_id, name, order_index, xp_raw_total, xp_eff_cached, score_cached) VALUES (?, ?, ?, 0, 0, 0);";
                sqlite3_prepare_v2(db_, sql_sub.c_str(), -1, &sub_stmt, nullptr);
                sqlite3_bind_int64(sub_stmt, 1, dom_id);
                sqlite3_bind_text(sub_stmt, 2, sub_name.c_str(), -1, SQLITE_TRANSIENT);
                sqlite3_bind_int(sub_stmt, 3, sub_idx);
                sqlite3_step(sub_stmt);
                sqlite3_finalize(sub_stmt);
                sub_idx++;
            }
            dom_idx++;
        }

        // Add default quote
        addQuote(user_id, "Nothing changes if nothing changes.", "James Clear");

        execSql("COMMIT;");
        return getUser().value();
    } catch (...) {
        execSql("ROLLBACK;");
        throw;
    }
}

void Database::updateUser(const User& user) {
    sqlite3_stmt* stmt = nullptr;
    std::string sql = "UPDATE users SET rating_current = ?, rank_current = ?, debt_current = ?, "
                      "grace_tokens = ?, streak_days = ?, longest_streak = ?, daily_budget_ema = ?, "
                      "crit_pity_k = ?, last_active_date = ? WHERE id = ?;";
    sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr);
    sqlite3_bind_double(stmt, 1, user.rating_current);
    sqlite3_bind_text(stmt, 2, user.rank_current.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 3, user.debt_current);
    sqlite3_bind_int(stmt, 4, user.grace_tokens);
    sqlite3_bind_int(stmt, 5, user.streak_days);
    sqlite3_bind_int(stmt, 6, user.longest_streak);
    sqlite3_bind_double(stmt, 7, user.daily_budget_ema);
    sqlite3_bind_int(stmt, 8, user.crit_pity_k);
    sqlite3_bind_text(stmt, 9, user.last_active_date.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int64(stmt, 10, user.id);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

std::vector<Domain> Database::getDomains(int64_t user_id) {
    std::vector<Domain> result;
    sqlite3_stmt* stmt = nullptr;
    std::string sql = "SELECT id, user_id, name, order_index, color_code, score_cached FROM domains WHERE user_id = ? ORDER BY order_index ASC;";
    if (sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int64(stmt, 1, user_id);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            Domain d;
            d.id = sqlite3_column_int64(stmt, 0);
            d.user_id = sqlite3_column_int64(stmt, 1);
            d.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
            d.order_index = sqlite3_column_int(stmt, 3);
            const unsigned char* col = sqlite3_column_text(stmt, 4);
            if (col) d.color_code = reinterpret_cast<const char*>(col);
            d.score_cached = sqlite3_column_double(stmt, 5);
            result.push_back(d);
        }
        sqlite3_finalize(stmt);
    }
    return result;
}

std::vector<Subdomain> Database::getSubdomains(int64_t domain_id) {
    std::vector<Subdomain> result;
    sqlite3_stmt* stmt = nullptr;
    std::string sql = "SELECT id, domain_id, name, order_index, xp_raw_total, xp_eff_cached, score_cached, last_activity_date FROM subdomains WHERE domain_id = ? ORDER BY order_index ASC;";
    if (sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int64(stmt, 1, domain_id);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            Subdomain s;
            s.id = sqlite3_column_int64(stmt, 0);
            s.domain_id = sqlite3_column_int64(stmt, 1);
            s.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
            s.order_index = sqlite3_column_int(stmt, 3);
            s.xp_raw_total = sqlite3_column_double(stmt, 4);
            s.xp_eff_cached = sqlite3_column_double(stmt, 5);
            s.score_cached = sqlite3_column_double(stmt, 6);
            const unsigned char* last_act = sqlite3_column_text(stmt, 7);
            if (last_act) s.last_activity_date = reinterpret_cast<const char*>(last_act);
            result.push_back(s);
        }
        sqlite3_finalize(stmt);
    }
    return result;
}

std::vector<Subdomain> Database::getAllSubdomains(int64_t user_id) {
    std::vector<Subdomain> result;
    auto domains = getDomains(user_id);
    for (const auto& d : domains) {
        auto subs = getSubdomains(d.id);
        result.insert(result.end(), subs.begin(), subs.end());
    }
    return result;
}

std::optional<Subdomain> Database::getSubdomainById(int64_t subdomain_id) {
    sqlite3_stmt* stmt = nullptr;
    std::string sql = "SELECT id, domain_id, name, order_index, xp_raw_total, xp_eff_cached, score_cached, last_activity_date FROM subdomains WHERE id = ?;";
    if (sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int64(stmt, 1, subdomain_id);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            Subdomain s;
            s.id = sqlite3_column_int64(stmt, 0);
            s.domain_id = sqlite3_column_int64(stmt, 1);
            s.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
            s.order_index = sqlite3_column_int(stmt, 3);
            s.xp_raw_total = sqlite3_column_double(stmt, 4);
            s.xp_eff_cached = sqlite3_column_double(stmt, 5);
            s.score_cached = sqlite3_column_double(stmt, 6);
            const unsigned char* last_act = sqlite3_column_text(stmt, 7);
            if (last_act) s.last_activity_date = reinterpret_cast<const char*>(last_act);
            sqlite3_finalize(stmt);
            return s;
        }
        sqlite3_finalize(stmt);
    }
    return std::nullopt;
}

std::optional<Domain> Database::getDomainById(int64_t domain_id) {
    sqlite3_stmt* stmt = nullptr;
    std::string sql = "SELECT id, user_id, name, order_index, color_code, score_cached FROM domains WHERE id = ?;";
    if (sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int64(stmt, 1, domain_id);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            Domain d;
            d.id = sqlite3_column_int64(stmt, 0);
            d.user_id = sqlite3_column_int64(stmt, 1);
            d.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
            d.order_index = sqlite3_column_int(stmt, 3);
            const unsigned char* col = sqlite3_column_text(stmt, 4);
            if (col) d.color_code = reinterpret_cast<const char*>(col);
            d.score_cached = sqlite3_column_double(stmt, 5);
            sqlite3_finalize(stmt);
            return d;
        }
        sqlite3_finalize(stmt);
    }
    return std::nullopt;
}

int64_t Database::createTask(const Task& task) {
    sqlite3_stmt* stmt = nullptr;
    std::string sql = "INSERT INTO tasks (user_id, name, type, major_subdomain_id, minor_subdomain_id, "
                      "difficulty_current, difficulty_original, cr_ema, priority_base, priority_current, "
                      "period_days, recurrence_mask, due_today, reward_unlock, status) "
                      "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);";
    sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr);
    sqlite3_bind_int64(stmt, 1, task.user_id);
    sqlite3_bind_text(stmt, 2, task.name.c_str(), -1, SQLITE_TRANSIENT);
    std::string type_str = taskTypeToString(task.type);
    sqlite3_bind_text(stmt, 3, type_str.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int64(stmt, 4, task.major_subdomain_id);
    sqlite3_bind_int64(stmt, 5, task.minor_subdomain_id);
    sqlite3_bind_double(stmt, 6, task.difficulty_current);
    sqlite3_bind_double(stmt, 7, task.difficulty_original);
    sqlite3_bind_double(stmt, 8, task.cr_ema);
    sqlite3_bind_double(stmt, 9, task.priority_base);
    sqlite3_bind_double(stmt, 10, task.priority_current);
    if (task.period_days) sqlite3_bind_int(stmt, 11, *task.period_days); else sqlite3_bind_null(stmt, 11);
    if (task.recurrence_mask) sqlite3_bind_int(stmt, 12, *task.recurrence_mask); else sqlite3_bind_null(stmt, 12);
    sqlite3_bind_int(stmt, 13, task.due_today ? 1 : 0);
    if (task.reward_unlock) sqlite3_bind_text(stmt, 14, task.reward_unlock->c_str(), -1, SQLITE_TRANSIENT); else sqlite3_bind_null(stmt, 14);
    sqlite3_bind_text(stmt, 15, task.status.c_str(), -1, SQLITE_TRANSIENT);

    sqlite3_step(stmt);
    int64_t new_id = sqlite3_last_insert_rowid(db_);
    sqlite3_finalize(stmt);
    return new_id;
}

static Task parseTaskFromStmt(sqlite3_stmt* stmt) {
    Task t;
    t.id = sqlite3_column_int64(stmt, 0);
    t.user_id = sqlite3_column_int64(stmt, 1);
    t.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
    t.type = stringToTaskType(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3)));
    t.major_subdomain_id = sqlite3_column_int64(stmt, 4);
    t.minor_subdomain_id = sqlite3_column_int64(stmt, 5);
    t.difficulty_current = sqlite3_column_double(stmt, 6);
    t.difficulty_original = sqlite3_column_double(stmt, 7);
    t.cr_ema = sqlite3_column_double(stmt, 8);
    t.priority_base = sqlite3_column_double(stmt, 9);
    t.priority_current = sqlite3_column_double(stmt, 10);
    if (sqlite3_column_type(stmt, 11) != SQLITE_NULL) t.started_at_date = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 11));
    if (sqlite3_column_type(stmt, 12) != SQLITE_NULL) t.period_days = sqlite3_column_int(stmt, 12);
    if (sqlite3_column_type(stmt, 13) != SQLITE_NULL) t.recurrence_mask = sqlite3_column_int(stmt, 13);
    t.due_today = (sqlite3_column_int(stmt, 14) == 1);
    if (sqlite3_column_type(stmt, 15) != SQLITE_NULL) t.first_appeared_date = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 15));
    if (sqlite3_column_type(stmt, 16) != SQLITE_NULL) t.reward_unlock = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 16));
    t.status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 17));
    if (sqlite3_column_type(stmt, 18) != SQLITE_NULL) t.last_completed_date = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 18));
    t.total_completions = sqlite3_column_int(stmt, 19);
    t.total_xp_earned = sqlite3_column_double(stmt, 20);
    return t;
}

static const char* TASK_FIELDS = "id, user_id, name, type, major_subdomain_id, minor_subdomain_id, "
                                 "difficulty_current, difficulty_original, cr_ema, priority_base, priority_current, "
                                 "started_at_date, period_days, recurrence_mask, due_today, first_appeared_date, "
                                 "reward_unlock, status, last_completed_date, total_completions, total_xp_earned ";

std::optional<Task> Database::getTaskById(int64_t task_id) {
    sqlite3_stmt* stmt = nullptr;
    std::string sql = std::string("SELECT ") + TASK_FIELDS + "FROM tasks WHERE id = ?;";
    if (sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int64(stmt, 1, task_id);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            Task t = parseTaskFromStmt(stmt);
            sqlite3_finalize(stmt);
            return t;
        }
        sqlite3_finalize(stmt);
    }
    return std::nullopt;
}

std::optional<Task> Database::getTaskByName(int64_t user_id, const std::string& name) {
    sqlite3_stmt* stmt = nullptr;
    std::string sql = std::string("SELECT ") + TASK_FIELDS + "FROM tasks WHERE user_id = ? AND name LIKE ? AND status != 'archived' LIMIT 1;";
    if (sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int64(stmt, 1, user_id);
        std::string pattern = "%" + name + "%";
        sqlite3_bind_text(stmt, 2, pattern.c_str(), -1, SQLITE_TRANSIENT);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            Task t = parseTaskFromStmt(stmt);
            sqlite3_finalize(stmt);
            return t;
        }
        sqlite3_finalize(stmt);
    }
    return std::nullopt;
}

std::vector<Task> Database::getAllTasks(int64_t user_id) {
    std::vector<Task> result;
    sqlite3_stmt* stmt = nullptr;
    std::string sql = std::string("SELECT ") + TASK_FIELDS + "FROM tasks WHERE user_id = ? AND status = 'active';";
    if (sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int64(stmt, 1, user_id);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            result.push_back(parseTaskFromStmt(stmt));
        }
        sqlite3_finalize(stmt);
    }
    return result;
}

std::vector<Task> Database::getAllTasksIncludingPaused(int64_t user_id) {
    std::vector<Task> result;
    sqlite3_stmt* stmt = nullptr;
    std::string sql = std::string("SELECT ") + TASK_FIELDS + "FROM tasks WHERE user_id = ? AND status != 'archived';";
    if (sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int64(stmt, 1, user_id);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            result.push_back(parseTaskFromStmt(stmt));
        }
        sqlite3_finalize(stmt);
    }
    return result;
}

std::vector<Task> Database::getTodayTasks(int64_t user_id) {
    auto u_opt = getUser();
    if (u_opt) {
        std::string logical_today = TaskEngine::getLogicalDate(u_opt->day_boundary_hour);
        TaskEngine::updateTaskPrioritiesAndSchedules(*this, *u_opt, logical_today);
    }

    std::vector<Task> result;
    sqlite3_stmt* stmt = nullptr;
    std::string sql = std::string("SELECT ") + TASK_FIELDS + "FROM tasks WHERE user_id = ? AND status = 'active' AND due_today = 1 AND type != 'hobby' ORDER BY priority_current DESC;";
    if (sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int64(stmt, 1, user_id);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            result.push_back(parseTaskFromStmt(stmt));
        }
        sqlite3_finalize(stmt);
    }
    return result;
}

std::vector<Task> Database::getBoredTasks(int64_t user_id) {
    std::vector<Task> result;
    sqlite3_stmt* stmt = nullptr;
    std::string sql = std::string("SELECT ") + TASK_FIELDS + "FROM tasks WHERE user_id = ? AND type = 'hobby' AND status = 'active';";
    if (sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int64(stmt, 1, user_id);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            result.push_back(parseTaskFromStmt(stmt));
        }
        sqlite3_finalize(stmt);
    }
    return result;
}

std::vector<Task> Database::getQuickTasks(int64_t user_id) {
    std::vector<Task> result;
    sqlite3_stmt* stmt = nullptr;
    std::string sql = std::string("SELECT ") + TASK_FIELDS + "FROM tasks WHERE user_id = ? AND difficulty_current < 15 AND status = 'active' ORDER BY priority_current DESC;";
    if (sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int64(stmt, 1, user_id);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            result.push_back(parseTaskFromStmt(stmt));
        }
        sqlite3_finalize(stmt);
    }
    return result;
}

void Database::updateTask(const Task& task) {
    sqlite3_stmt* stmt = nullptr;
    std::string sql = "UPDATE tasks SET name = ?, type = ?, major_subdomain_id = ?, minor_subdomain_id = ?, "
                      "difficulty_current = ?, difficulty_original = ?, cr_ema = ?, priority_base = ?, "
                      "priority_current = ?, period_days = ?, recurrence_mask = ?, due_today = ?, "
                      "reward_unlock = ?, status = ?, last_completed_date = ?, total_completions = ?, total_xp_earned = ?, "
                      "first_appeared_date = ?, started_at_date = ? "
                      "WHERE id = ?;";
    sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, task.name.c_str(), -1, SQLITE_TRANSIENT);
    std::string type_str = taskTypeToString(task.type);
    sqlite3_bind_text(stmt, 2, type_str.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int64(stmt, 3, task.major_subdomain_id);
    sqlite3_bind_int64(stmt, 4, task.minor_subdomain_id);
    sqlite3_bind_double(stmt, 5, task.difficulty_current);
    sqlite3_bind_double(stmt, 6, task.difficulty_original);
    sqlite3_bind_double(stmt, 7, task.cr_ema);
    sqlite3_bind_double(stmt, 8, task.priority_base);
    sqlite3_bind_double(stmt, 9, task.priority_current);
    if (task.period_days) sqlite3_bind_int(stmt, 10, *task.period_days); else sqlite3_bind_null(stmt, 10);
    if (task.recurrence_mask) sqlite3_bind_int(stmt, 11, *task.recurrence_mask); else sqlite3_bind_null(stmt, 11);
    sqlite3_bind_int(stmt, 12, task.due_today ? 1 : 0);
    if (task.reward_unlock) sqlite3_bind_text(stmt, 13, task.reward_unlock->c_str(), -1, SQLITE_TRANSIENT); else sqlite3_bind_null(stmt, 13);
    sqlite3_bind_text(stmt, 14, task.status.c_str(), -1, SQLITE_TRANSIENT);
    if (task.last_completed_date) sqlite3_bind_text(stmt, 15, task.last_completed_date->c_str(), -1, SQLITE_TRANSIENT); else sqlite3_bind_null(stmt, 15);
    sqlite3_bind_int(stmt, 16, task.total_completions);
    sqlite3_bind_double(stmt, 17, task.total_xp_earned);
    if (task.first_appeared_date) sqlite3_bind_text(stmt, 18, task.first_appeared_date->c_str(), -1, SQLITE_TRANSIENT); else sqlite3_bind_null(stmt, 18);
    if (task.started_at_date) sqlite3_bind_text(stmt, 19, task.started_at_date->c_str(), -1, SQLITE_TRANSIENT); else sqlite3_bind_null(stmt, 19);
    sqlite3_bind_int64(stmt, 20, task.id);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

void Database::setTaskStatus(int64_t task_id, const std::string& status) {
    sqlite3_stmt* stmt = nullptr;
    std::string sql = "UPDATE tasks SET status = ? WHERE id = ?;";
    if (sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, status.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int64(stmt, 2, task_id);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }
}

void Database::recordCompletion(TaskCompletion& completion, Task& task, User& user) {
    execSql("BEGIN TRANSACTION;");
    try {
        // 1. Calculate multipliers & XP using math engine
        completion.xp_raw = math::calculateRawXp(completion.t_actual, task.difficulty_current);
        completion.m_streak = math::calculateStreakMultiplier(user.streak_days);

        // Fetch Major Subdomain & Domain for Balance multiplier
        auto maj_sub = getSubdomainById(task.major_subdomain_id).value();
        auto maj_dom = getDomainById(maj_sub.domain_id).value();
        completion.m_balance = math::calculateBalanceBonus(maj_dom.score_cached, user.rating_current);

        completion.xp_final = completion.xp_raw * completion.m_frog * completion.m_crit * completion.m_streak * completion.m_balance;
        completion.xp_major = completion.xp_final * 0.70;
        completion.xp_minor = completion.xp_final * 0.30;

        // 2. Insert into task_completions ledger
        sqlite3_stmt* stmt = nullptr;
        std::string sql_comp = "INSERT INTO task_completions (task_id, completed_at, local_date, t_actual, "
                               "difficulty_at_completion, xp_raw, m_frog, m_crit, m_streak, m_balance, "
                               "xp_final, xp_major, xp_minor, was_frog, was_crit, cr_after) "
                               "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);";
        sqlite3_prepare_v2(db_, sql_comp.c_str(), -1, &stmt, nullptr);
        sqlite3_bind_int64(stmt, 1, task.id);
        sqlite3_bind_int64(stmt, 2, getCurrentEpoch());
        sqlite3_bind_text(stmt, 3, completion.local_date.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_double(stmt, 4, completion.t_actual);
        sqlite3_bind_double(stmt, 5, task.difficulty_current);
        sqlite3_bind_double(stmt, 6, completion.xp_raw);
        sqlite3_bind_double(stmt, 7, completion.m_frog);
        sqlite3_bind_double(stmt, 8, completion.m_crit);
        sqlite3_bind_double(stmt, 9, completion.m_streak);
        sqlite3_bind_double(stmt, 10, completion.m_balance);
        sqlite3_bind_double(stmt, 11, completion.xp_final);
        sqlite3_bind_double(stmt, 12, completion.xp_major);
        sqlite3_bind_double(stmt, 13, completion.xp_minor);
        sqlite3_bind_int(stmt, 14, completion.was_frog ? 1 : 0);
        sqlite3_bind_int(stmt, 15, completion.was_crit ? 1 : 0);

        // Recalibrate difficulty
        auto recal = math::recalibrateDifficulty(task.cr_ema, task.difficulty_current, task.difficulty_original, true);
        task.cr_ema = recal.cr_new;
        task.difficulty_current = recal.d_new;
        sqlite3_bind_double(stmt, 16, recal.cr_new);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);

        // 3. Update Subdomains (major & minor)
        auto min_sub = getSubdomainById(task.minor_subdomain_id).value();

        maj_sub.xp_raw_total += completion.xp_major;
        maj_sub.xp_eff_cached += completion.xp_major;
        maj_sub.score_cached = math::calculateSubdomainScore(maj_sub.xp_eff_cached);
        maj_sub.last_activity_date = completion.local_date;

        min_sub.xp_raw_total += completion.xp_minor;
        min_sub.xp_eff_cached += completion.xp_minor;
        min_sub.score_cached = math::calculateSubdomainScore(min_sub.xp_eff_cached);
        min_sub.last_activity_date = completion.local_date;

        auto update_sub = [this](const Subdomain& s) {
            sqlite3_stmt* st = nullptr;
            std::string sql = "UPDATE subdomains SET xp_raw_total = ?, xp_eff_cached = ?, score_cached = ?, last_activity_date = ? WHERE id = ?;";
            sqlite3_prepare_v2(db_, sql.c_str(), -1, &st, nullptr);
            sqlite3_bind_double(st, 1, s.xp_raw_total);
            sqlite3_bind_double(st, 2, s.xp_eff_cached);
            sqlite3_bind_double(st, 3, s.score_cached);
            sqlite3_bind_text(st, 4, s.last_activity_date.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_int64(st, 5, s.id);
            sqlite3_step(st);
            sqlite3_finalize(st);
        };

        update_sub(maj_sub);
        update_sub(min_sub);

        // 4. Update Domains & Rating
        auto domains = getDomains(user.id);
        std::vector<double> dom_scores;
        for (auto& d : domains) {
            auto subs = getSubdomains(d.id);
            std::vector<double> sub_scores;
            for (const auto& s : subs) {
                if (s.id == maj_sub.id) sub_scores.push_back(maj_sub.score_cached);
                else if (s.id == min_sub.id) sub_scores.push_back(min_sub.score_cached);
                else sub_scores.push_back(s.score_cached);
            }
            d.score_cached = math::calculateDomainScore(sub_scores);
            dom_scores.push_back(d.score_cached);

            sqlite3_stmt* st = nullptr;
            std::string sql = "UPDATE domains SET score_cached = ? WHERE id = ?;";
            sqlite3_prepare_v2(db_, sql.c_str(), -1, &st, nullptr);
            sqlite3_bind_double(st, 1, d.score_cached);
            sqlite3_bind_int64(st, 2, d.id);
            sqlite3_step(st);
            sqlite3_finalize(st);
        }

        user.rating_current = math::calculateRating(dom_scores);
        user.rank_current = rankToString(math::ratingToRank(user.rating_current));
        
        // 5. Update Task
        task.last_completed_date = completion.local_date;
        task.total_completions += 1;
        task.total_xp_earned += completion.xp_final;
        if (task.type == TaskType::OneTime) {
            task.status = "archived";
            task.due_today = false;
        } else {
            task.due_today = false; // drop from today for remainder of logical day
        }
        updateTask(task);
        updateUser(user);

        execSql("COMMIT;");
    } catch (...) {
        execSql("ROLLBACK;");
        throw;
    }
}

void Database::checkAndRunDailyRollover(User& user, const std::string& today_date) {
    if (user.last_active_date == today_date) return;

    execSql("BEGIN TRANSACTION;");
    try {
        // Roll stats and decay
        auto domains = getDomains(user.id);
        std::vector<double> dom_scores;

        for (auto& d : domains) {
            auto subs = getSubdomains(d.id);
            std::vector<double> sub_scores;
            for (auto& s : subs) {
                int days_inactive = 1; // standard daily decay tick
                s.xp_eff_cached = math::calculateSubdomainDecay(s.xp_raw_total, days_inactive);
                s.score_cached = math::calculateSubdomainScore(s.xp_eff_cached);
                sub_scores.push_back(s.score_cached);

                sqlite3_stmt* st = nullptr;
                std::string sql = "UPDATE subdomains SET xp_eff_cached = ?, score_cached = ? WHERE id = ?;";
                sqlite3_prepare_v2(db_, sql.c_str(), -1, &st, nullptr);
                sqlite3_bind_double(st, 1, s.xp_eff_cached);
                sqlite3_bind_double(st, 2, s.score_cached);
                sqlite3_bind_int64(st, 3, s.id);
                sqlite3_step(st);
                sqlite3_finalize(st);
            }
            d.score_cached = math::calculateDomainScore(sub_scores);
            dom_scores.push_back(d.score_cached);

            sqlite3_stmt* st = nullptr;
            std::string sql = "UPDATE domains SET score_cached = ? WHERE id = ?;";
            sqlite3_prepare_v2(db_, sql.c_str(), -1, &st, nullptr);
            sqlite3_bind_double(st, 1, d.score_cached);
            sqlite3_bind_int64(st, 2, d.id);
            sqlite3_step(st);
            sqlite3_finalize(st);
        }

        user.rating_current = math::calculateRating(dom_scores);
        user.rank_current = rankToString(math::ratingToRank(user.rating_current));
        user.last_active_date = today_date;

        // Run TaskEngine priority & schedule recalculation
        TaskEngine::updateTaskPrioritiesAndSchedules(*this, user, today_date);

        updateUser(user);

        execSql("COMMIT;");
    } catch (...) {
        execSql("ROLLBACK;");
        throw;
    }
}

std::optional<Quote> Database::getRandomQuote(int64_t user_id) {
    sqlite3_stmt* stmt = nullptr;
    std::string sql = "SELECT id, user_id, text, author, active, times_shown, last_shown_at FROM quotes WHERE user_id = ? AND active = 1 ORDER BY RANDOM() LIMIT 1;";
    if (sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int64(stmt, 1, user_id);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            Quote q;
            q.id = sqlite3_column_int64(stmt, 0);
            q.user_id = sqlite3_column_int64(stmt, 1);
            q.text = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
            const unsigned char* auth = sqlite3_column_text(stmt, 3);
            if (auth) q.author = reinterpret_cast<const char*>(auth);
            q.active = (sqlite3_column_int(stmt, 4) == 1);
            q.times_shown = sqlite3_column_int(stmt, 5);
            q.last_shown_at = sqlite3_column_int64(stmt, 6);
            sqlite3_finalize(stmt);
            return q;
        }
        sqlite3_finalize(stmt);
    }
    return std::nullopt;
}

int64_t Database::addQuote(int64_t user_id, const std::string& text, const std::string& author) {
    sqlite3_stmt* stmt = nullptr;
    std::string sql = "INSERT INTO quotes (user_id, text, author) VALUES (?, ?, ?);";
    sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr);
    sqlite3_bind_int64(stmt, 1, user_id);
    sqlite3_bind_text(stmt, 2, text.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, author.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_step(stmt);
    int64_t id = sqlite3_last_insert_rowid(db_);
    sqlite3_finalize(stmt);
    return id;
}

bool Database::deleteQuote(int64_t quote_id) {
    sqlite3_stmt* stmt = nullptr;
    std::string sql = "DELETE FROM quotes WHERE id = ?;";
    if (sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int64(stmt, 1, quote_id);
        int res = sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        return res == SQLITE_DONE;
    }
    return false;
}

std::vector<Quote> Database::getAllQuotes(int64_t user_id) {
    std::vector<Quote> result;
    sqlite3_stmt* stmt = nullptr;
    std::string sql = "SELECT id, user_id, text, author, active, times_shown, last_shown_at FROM quotes WHERE user_id = ?;";
    if (sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int64(stmt, 1, user_id);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            Quote q;
            q.id = sqlite3_column_int64(stmt, 0);
            q.user_id = sqlite3_column_int64(stmt, 1);
            q.text = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
            const unsigned char* auth = sqlite3_column_text(stmt, 3);
            if (auth) q.author = reinterpret_cast<const char*>(auth);
            q.active = (sqlite3_column_int(stmt, 4) == 1);
            q.times_shown = sqlite3_column_int(stmt, 5);
            q.last_shown_at = sqlite3_column_int64(stmt, 6);
            result.push_back(q);
        }
        sqlite3_finalize(stmt);
    }
    return result;
}

Database::ProfileStats Database::getProfileStats(int64_t user_id) {
    ProfileStats stats;
    auto u_opt = getUser();
    if (!u_opt) return stats;
    stats.user = *u_opt;

    auto domains = getDomains(user_id);
    for (const auto& d : domains) {
        auto subs = getSubdomains(d.id);
        stats.domains.push_back({d, subs});
    }

    sqlite3_stmt* stmt = nullptr;
    std::string sql = "SELECT COUNT(*), COALESCE(SUM(xp_final), 0) FROM task_completions tc JOIN tasks t ON tc.task_id = t.id WHERE t.user_id = ?;";
    if (sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int64(stmt, 1, user_id);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            stats.total_tasks_completed = sqlite3_column_int(stmt, 0);
            stats.total_xp_earned = sqlite3_column_double(stmt, 1);
        }
        sqlite3_finalize(stmt);
    }

    return stats;
}

} // namespace xlog