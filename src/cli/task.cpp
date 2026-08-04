#include "xlog/cli/task.hpp"
#include "xlog/ui/colors.hpp"
#include "xlog/math/xp.hpp"
#include "xlog/task_engine.hpp"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <limits>
#include <iomanip>

namespace xlog {
namespace cli {

int parseWeekdayMask(const std::string& input) {
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

void runAddTask(Database& db, int64_t user_id, const std::vector<std::string>& args) {
    auto all_subs = db.getAllSubdomains(user_id);
    if (all_subs.empty()) {
        std::cout << ui::colorText("Error: Subdomains not properly set up.\n", ui::colors::RED);
        return;
    }

    User u = db.getUser().value();
    std::string logical_today = TaskEngine::getLogicalDate(u.day_boundary_hour);
    int logical_wday = TaskEngine::getLogicalWeekday(u.day_boundary_hour);

    // Default fast add mode: `xlog add <taskname>`
    if (args.size() >= 2) {
        std::string name = args[1];
        for (size_t i = 2; i < args.size(); ++i) {
            name += " " + args[i];
        }

        Task t;
        t.user_id = user_id;
        t.name = name;
        t.type = TaskType::OneTime;
        t.major_subdomain_id = all_subs[3].id;
        t.minor_subdomain_id = all_subs[3].id;
        t.difficulty_current = 15.0;
        t.difficulty_original = 15.0;
        t.priority_base = 0.5;
        t.priority_current = 0.5;
        t.first_appeared_date = logical_today;
        t.due_today = TaskEngine::isTaskDueToday(t, logical_today, logical_wday);

        int64_t id = db.createTask(t);
        std::cout << ui::colorText("\n✓ Task '" + name + "' created successfully (ID: " + std::to_string(id) + ")!\n\n", ui::colors::GREEN + ui::colors::BOLD);
        return;
    }

    // Interactive prompt mode: `xlog add` without params
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

void runEditTask(Database& db, int64_t user_id, const std::vector<std::string>& args) {
    std::cout << ui::colorText("\n⚡ Edit Task\n", ui::colors::LAVENDER + ui::colors::BOLD);

    std::optional<Task> selected_task;
    if (args.size() >= 2) {
        selected_task = db.getTaskByName(user_id, args[1]);
    }

    if (!selected_task.has_value()) {
        auto tasks = db.getAllTasksIncludingPaused(user_id);
        if (tasks.empty()) {
            std::cout << ui::colorText("No tasks found to edit.\n", ui::colors::YELLOW);
            return;
        }
        std::cout << "Select task to edit:\n";
        for (size_t i = 0; i < tasks.size(); ++i) {
            std::string status_tag = (tasks[i].status == "paused") ? " [PAUSED]" : "";
            std::cout << "  " << (i + 1) << ". " << tasks[i].name << status_tag << "\n";
        }
        std::cout << "Choice [1]: ";
        std::string choice_in;
        std::getline(std::cin, choice_in);
        int idx = choice_in.empty() ? 0 : (std::stoi(choice_in) - 1);
        idx = std::clamp(idx, 0, static_cast<int>(tasks.size() - 1));
        selected_task = tasks[idx];
    }

    Task t = selected_task.value();

    std::cout << "New Task Name [" << t.name << "]: ";
    std::string name_in;
    std::getline(std::cin, name_in);
    if (!name_in.empty()) t.name = name_in;

    std::cout << "Task Type (1: One-Time, 2: Periodic, 3: Recurring, 4: Hobby) [" << taskTypeToString(t.type) << "]: ";
    std::string type_in;
    std::getline(std::cin, type_in);
    if (!type_in.empty()) {
        if (type_in == "1") t.type = TaskType::OneTime;
        else if (type_in == "2") t.type = TaskType::Periodic;
        else if (type_in == "3") t.type = TaskType::Recurring;
        else if (type_in == "4") t.type = TaskType::Hobby;
    }

    if (t.type == TaskType::Periodic) {
        int current_period = t.period_days.value_or(2);
        std::cout << "Period in Days [" << current_period << "]: ";
        std::string p_in;
        std::getline(std::cin, p_in);
        if (!p_in.empty()) {
            t.period_days = std::max(1, std::stoi(p_in));
        }
    } else if (t.type == TaskType::Recurring) {
        std::cout << "Recurring Weekdays [mon fri]: ";
        std::string rec_in;
        std::getline(std::cin, rec_in);
        if (!rec_in.empty()) {
            t.recurrence_mask = parseWeekdayMask(rec_in);
        }
    }

    std::cout << "Estimated Intended Time in Minutes [" << t.difficulty_current << "]: ";
    std::string diff_in;
    std::getline(std::cin, diff_in);
    if (!diff_in.empty()) {
        double d_val = std::stod(diff_in);
        t.difficulty_current = d_val;
        t.difficulty_original = d_val;
    }

    std::cout << "Base Priority (0.0 to 1.0) [" << t.priority_base << "]: ";
    std::string prio_in;
    std::getline(std::cin, prio_in);
    if (!prio_in.empty()) {
        double p_val = std::stod(prio_in);
        t.priority_base = std::clamp(p_val, 0.0, 1.0);
        t.priority_current = t.priority_base;
    }

    User u = db.getUser().value();
    std::string logical_today = TaskEngine::getLogicalDate(u.day_boundary_hour);
    int logical_wday = TaskEngine::getLogicalWeekday(u.day_boundary_hour);
    t.due_today = TaskEngine::isTaskDueToday(t, logical_today, logical_wday);

    db.updateTask(t);
    std::cout << ui::colorText("\n✓ Task '" + t.name + "' updated successfully!\n\n", ui::colors::GREEN + ui::colors::BOLD);
}

void runPauseTask(Database& db, int64_t user_id, const std::vector<std::string>& args) {
    std::cout << ui::colorText("\n⚡ Pause / Resume Task\n", ui::colors::LAVENDER + ui::colors::BOLD);

    std::optional<Task> selected_task;
    if (args.size() >= 2) {
        selected_task = db.getTaskByName(user_id, args[1]);
    }

    if (!selected_task.has_value()) {
        auto tasks = db.getAllTasksIncludingPaused(user_id);
        if (tasks.empty()) {
            std::cout << ui::colorText("No tasks found.\n", ui::colors::YELLOW);
            return;
        }
        std::cout << "Select task to pause/unpause:\n";
        for (size_t i = 0; i < tasks.size(); ++i) {
            std::string st = (tasks[i].status == "paused") ? ui::colors::YELLOW + " [PAUSED]" : ui::colors::GREEN + " [ACTIVE]";
            std::cout << "  " << (i + 1) << ". " << tasks[i].name << st << ui::colors::RESET << "\n";
        }
        std::cout << "Choice [1]: ";
        std::string choice_in;
        std::getline(std::cin, choice_in);
        int idx = choice_in.empty() ? 0 : (std::stoi(choice_in) - 1);
        idx = std::clamp(idx, 0, static_cast<int>(tasks.size() - 1));
        selected_task = tasks[idx];
    }

    Task t = selected_task.value();

    if (t.status == "paused") {
        db.setTaskStatus(t.id, "active");
        std::cout << ui::colorText("\n✓ Task '" + t.name + "' is now ACTIVE.\n\n", ui::colors::GREEN + ui::colors::BOLD);
    } else {
        db.setTaskStatus(t.id, "paused");
        std::cout << ui::colorText("\n✓ Task '" + t.name + "' is now PAUSED (will not appear in today list or accrue debt).\n\n", ui::colors::YELLOW + ui::colors::BOLD);
    }
}

} // namespace cli
} // namespace xlog
