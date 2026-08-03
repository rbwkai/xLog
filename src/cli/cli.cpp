#include "xlog/cli.hpp"
#include "xlog/cli/fuzzy.hpp"
#include "xlog/ui.hpp"
#include "xlog/math/xp.hpp"
#include "xlog/task_engine.hpp"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <limits>
#include <chrono>
#include <iomanip>

namespace xlog {
namespace cli {

static int parseWeekdayMask(const std::string& input) {
    if (input.empty()) {
        return (1 << 2) | (1 << 6); // Default Wed & Sun (bit 2 and bit 6)
    }
    std::stringstream ss(input);
    std::string token;
    int mask = 0;
    while (ss >> token) {
        std::transform(token.begin(), token.end(), token.begin(), ::tolower);
        if (token == "mon") mask |= (1 << 0);
        else if (token == "tue") mask |= (1 << 1);
        else if (token == "wed") mask |= (1 << 2);
        else if (token == "thu") mask |= (1 << 3);
        else if (token == "fri") mask |= (1 << 4);
        else if (token == "sat") mask |= (1 << 5);
        else if (token == "sun") mask |= (1 << 6);
    }
    return (mask > 0) ? mask : ((1 << 2) | (1 << 6));
}

void runOnboarding(Database& db) {
    std::cout << ui::colorText("\n⚡ Welcome to xLog Gamification Engine Setup\n", ui::colors::LAVENDER + ui::colors::BOLD);
    std::cout << ui::colorText("Enter your username: ", ui::colors::TEXT);
    std::string username;
    std::getline(std::cin, username);
    if (username.empty()) username = "Hero";

    std::cout << ui::colorText("\nxLog structures life into 4 Domains, each with 4 Subdomains.\n", ui::colors::SUBTEXT);

    std::vector<std::pair<std::string, std::vector<std::string>>> domains;
    std::vector<std::string> default_dom_names = {"Faith", "Intellect", "Physique", "Artistry"};
    std::vector<std::vector<std::string>> default_subs = {
        {"Qur'an", "Arabic", "Dhikr", "Discipline"},
        {"Heuristics", "Theory", "Exploration", "Craft"},
        {"Combat", "Strength", "Agility", "Aesthetics"},
        {"Sketching", "Articulation", "Versatility", "Character"}
    };

    for (int i = 0; i < 4; ++i) {
        std::cout << ui::colorText("\nDomain " + std::to_string(i + 1) + " name [" + default_dom_names[i] + "]: ", ui::colors::TEXT);
        std::string dom_input;
        std::getline(std::cin, dom_input);
        std::string dom_name = dom_input.empty() ? default_dom_names[i] : dom_input;

        std::vector<std::string> subs;
        for (int j = 0; j < 4; ++j) {
            std::cout << "  Subdomain " + std::to_string(j + 1) + " [" + default_subs[i][j] + "]: ";
            std::string sub_input;
            std::getline(std::cin, sub_input);
            subs.push_back(sub_input.empty() ? default_subs[i][j] : sub_input);
        }
        domains.push_back({dom_name, subs});
    }

    User u = db.onboardUser(username, domains);
    std::cout << ui::colorText("\n✓ Onboarding complete! Account created for " + u.username + ".\n\n", ui::colors::GREEN + ui::colors::BOLD);
}

void runAddTask(Database& db, int64_t user_id) {
    std::cout << ui::colorText("\n⚡ Create New Task\n", ui::colors::LAVENDER + ui::colors::BOLD);

    std::cout << "Task Name: ";
    std::string name;
    std::getline(std::cin, name);
    if (name.empty()) return;

    std::cout << "Task Type (1: One-Time, 2: Periodic, 3: Recurring, 4: Hobby) [1]: ";
    std::string type_in;
    std::getline(std::cin, type_in);
    TaskType type = TaskType::OneTime;
    if (type_in == "2") type = TaskType::Periodic;
    else if (type_in == "3") type = TaskType::Recurring;
    else if (type_in == "4") type = TaskType::Hobby;

    std::optional<int> period_days;
    std::optional<int> recurrence_mask;

    if (type == TaskType::Periodic) {
        std::cout << "Period in Days [2]: ";
        std::string p_in;
        std::getline(std::cin, p_in);
        int p_val = p_in.empty() ? 2 : std::stoi(p_in);
        period_days = std::max(1, p_val);
    } else if (type == TaskType::Recurring) {
        std::cout << "Recurring Weekdays (3-letter space-separated, e.g. sat sun) [mon fri]: ";
        std::string rec_in;
        std::getline(std::cin, rec_in);
        recurrence_mask = parseWeekdayMask(rec_in);
    }

    auto all_subs = db.getAllSubdomains(user_id);
    if (all_subs.size() < 2) {
        std::cout << ui::colorText("Error: Subdomains not properly set up.\n", ui::colors::RED);
        return;
    }

    std::cout << "\nSelect Major Subdomain:\n";
    for (size_t i = 0; i < all_subs.size(); ++i) {
        std::cout << "  " << (i + 1) << ". " << all_subs[i].name << "\n";
    }
    std::cout << "Choice [1]: ";
    std::string maj_in;
    std::getline(std::cin, maj_in);
    int maj_idx = maj_in.empty() ? 0 : (std::stoi(maj_in) - 1);
    maj_idx = std::clamp(maj_idx, 0, static_cast<int>(all_subs.size() - 1));

    std::cout << "Select Minor Subdomain:\nChoice [2]: ";
    std::string min_in;
    std::getline(std::cin, min_in);
    int min_idx = min_in.empty() ? 1 : (std::stoi(min_in) - 1);
    min_idx = std::clamp(min_idx, 0, static_cast<int>(all_subs.size() - 1));
    if (min_idx == maj_idx) min_idx = (maj_idx + 1) % all_subs.size();

    std::cout << "Estimated Intended Time in Minutes [15]: ";
    std::string diff_in;
    std::getline(std::cin, diff_in);
    double diff = diff_in.empty() ? 15.0 : std::stod(diff_in);

    std::cout << "Starting Base Priority (0.0 to 1.0) [0.5]: ";
    std::string prio_in;
    std::getline(std::cin, prio_in);
    double prio = prio_in.empty() ? 0.5 : std::stod(prio_in);
    prio = std::clamp(prio, 0.0, 1.0);

    User u = db.getUser().value();
    std::string logical_today = TaskEngine::getLogicalDate(u.day_boundary_hour);
    int logical_wday = TaskEngine::getLogicalWeekday(u.day_boundary_hour);

    Task t;
    t.user_id = user_id;
    t.name = name;
    t.type = type;
    t.major_subdomain_id = all_subs[maj_idx].id;
    t.minor_subdomain_id = all_subs[min_idx].id;
    t.difficulty_current = diff;
    t.difficulty_original = diff;
    t.period_days = period_days;
    t.recurrence_mask = recurrence_mask;
    t.priority_base = prio;
    t.priority_current = prio;
    t.first_appeared_date = logical_today;
    t.due_today = TaskEngine::isTaskDueToday(t, logical_today, logical_wday);

    int64_t id = db.createTask(t);
    std::cout << ui::colorText("\n✓ Task '" + name + "' created successfully (ID: " + std::to_string(id) + ")!\n\n", ui::colors::GREEN + ui::colors::BOLD);
}

void runDoneTask(Database& db, int64_t user_id, const std::vector<std::string>& args) {
    auto u_opt = db.getUser();
    if (!u_opt) return;
    User u = *u_opt;

    std::string task_target;
    double time_spent = -1.0;

    if (args.size() >= 2) {
        task_target = args[1];
        if (args.size() >= 3) {
            try { time_spent = std::stod(args[2]); } catch (...) {}
        }
    }

    if (task_target.empty()) {
        auto today_tasks = db.getTodayTasks(user_id);
        if (today_tasks.empty()) {
            std::cout << ui::colorText("No tasks available to mark complete.\n", ui::colors::YELLOW);
            return;
        }
        std::cout << "\nSelect task to complete:\n";
        for (size_t i = 0; i < today_tasks.size(); ++i) {
            std::cout << "  " << (i + 1) << ". " << today_tasks[i].name << " (" << today_tasks[i].difficulty_current << "m)\n";
        }
        std::cout << "Choice [1]: ";
        std::string choice_in;
        std::getline(std::cin, choice_in);
        int idx = choice_in.empty() ? 0 : (std::stoi(choice_in) - 1);
        idx = std::clamp(idx, 0, static_cast<int>(today_tasks.size() - 1));
        task_target = today_tasks[idx].name;
    }

    auto t_opt = db.getTaskByName(user_id, task_target);
    if (!t_opt) {
        std::cout << ui::colorText("Task '" + task_target + "' not found.\n", ui::colors::RED);
        return;
    }
    Task t = *t_opt;

    if (time_spent <= 0) {
        time_spent = t.difficulty_current;
    }

    TaskCompletion comp;
    comp.task_id = t.id;
    comp.t_actual = time_spent;
    comp.local_date = TaskEngine::getLogicalDate(u.day_boundary_hour);

    auto today_tasks = db.getTodayTasks(user_id);
    if (!today_tasks.empty() && today_tasks[0].id == t.id) {
        comp.was_frog = true;
        comp.m_frog = math::rollFrogMultiplier();
    } else {
        comp.was_frog = false;
        comp.m_frog = 1.0;
        comp.m_crit = math::rollCritMultiplier(u.crit_pity_k, comp.was_crit);
    }

    db.recordCompletion(comp, t, u);

    std::cout << "\n" << ui::colors::GREEN << ui::colors::BOLD << "✔ Task Completed: " << t.name << ui::colors::RESET << "\n";
    if (comp.was_frog) {
        std::cout << ui::colors::PEACH << ui::colors::BOLD << "🐸 FROG CRITICAL HIT! (" << std::fixed << std::setprecision(2) << comp.m_frog << "x Multiplier)" << ui::colors::RESET << "\n";
    } else if (comp.was_crit) {
        std::cout << ui::colors::PINK << ui::colors::BOLD << "⭐⭐ CRITICAL HIT! (" << std::fixed << std::setprecision(2) << comp.m_crit << "x Multiplier)" << ui::colors::RESET << "\n";
    }

    std::cout << ui::colors::TEXT << "  XP Gained: " << ui::colors::GREEN << ui::colors::BOLD << "+" << std::fixed << std::setprecision(0) << comp.xp_final << " XP" << ui::colors::RESET << "\n";
    std::cout << ui::colors::OVERLAY2 << "  New Global Rating: " << std::fixed << std::setprecision(1) << u.rating_current << " (" << u.rank_current << ")" << ui::colors::RESET << "\n\n";
}

void runQuoteCommand(Database& db, int64_t user_id, const std::vector<std::string>& args) {
    if (args.size() <= 1) {
        ui::renderQuote(db, user_id);
        return;
    }

    std::string subcmd = args[1];
    if (subcmd == "add") {
        if (args.size() < 3) {
            std::cout << "Usage: xlog quote add \"Quote text\" [Author]\n";
            return;
        }
        std::string text = args[2];
        std::string author = (args.size() >= 4) ? args[3] : "";
        db.addQuote(user_id, text, author);
        std::cout << ui::colorText("✓ Quote added.\n", ui::colors::GREEN);
    } else if (subcmd == "list") {
        auto quotes = db.getAllQuotes(user_id);
        std::cout << "\n" << ui::colors::LAVENDER << ui::colors::BOLD << "── Saved Quotes ────────────────────────────────────" << ui::colors::RESET << "\n";
        for (const auto& q : quotes) {
            std::cout << "  " << ui::colors::OVERLAY2 << "[" << q.id << "] " << ui::colors::TEXT << "\"" << q.text << "\""
                      << (q.author.empty() ? "" : " — " + q.author) << ui::colors::RESET << "\n";
        }
        std::cout << "\n";
    }
}

void dispatchCommand(Database& db, const std::vector<std::string>& args) {
    auto u_opt = db.getUser();
    if (!u_opt && (args.empty() || args[0] != "setup")) {
        runOnboarding(db);
        u_opt = db.getUser();
        if (!u_opt) return;
    }

    User u = u_opt.value();
    std::string logical_today = TaskEngine::getLogicalDate(u.day_boundary_hour);
    db.checkAndRunDailyRollover(u, logical_today);

    std::string cmd = args.empty() ? "prompt" : matchFuzzyCommand(args[0]);

    if (cmd == "prompt") {
        ui::renderPrompt(db, u.id);
    } else if (cmd == "today") {
        ui::renderToday(db, u.id);
    } else if (cmd == "add") {
        runAddTask(db, u.id);
    } else if (cmd == "done") {
        runDoneTask(db, u.id, args);
    } else if (cmd == "profile") {
        ui::renderProfile(db, u.id);
    } else if (cmd == "bored") {
        ui::renderBored(db, u.id);
    } else if (cmd == "quick") {
        ui::renderQuick(db, u.id);
    } else if (cmd == "why") {
        ui::renderWhy();
    } else if (cmd == "quote") {
        runQuoteCommand(db, u.id, args);
    } else if (cmd == "setup") {
        runOnboarding(db);
    } else {
        std::cout << "xLog Commands:\n"
                  << "  xlog prompt   - fastfetch style status & quote\n"
                  << "  xlog today    - view daily task list\n"
                  << "  xlog add      - create a new task\n"
                  << "  xlog done     - complete a task and earn XP\n"
                  << "  xlog profile  - detailed rank, domain progress & heatmap\n"
                  << "  xlog bored    - view hobby tasks\n"
                  << "  xlog quick    - view quick tasks (<15 min)\n"
                  << "  xlog why      - system math & mechanics overview\n"
                  << "  xlog quote    - display or manage motivational quotes\n";
    }
}

} // namespace cli
} // namespace xlog
