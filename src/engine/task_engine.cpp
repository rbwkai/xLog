#include "xlog/task_engine.hpp"
#include "xlog/math/xp.hpp"
#include "xlog/math/calibration.hpp"
#include <chrono>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cmath>

namespace xlog {
namespace TaskEngine {

std::string getLogicalDate(int day_boundary_hour, int64_t epoch_sec) {
    std::time_t raw_time;
    if (epoch_sec < 0) {
        raw_time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    } else {
        raw_time = static_cast<std::time_t>(epoch_sec);
    }

    // Offset time by day_boundary_hour
    raw_time -= (day_boundary_hour * 3600);

    std::tm time_info;
#if defined(_WIN32)
    localtime_s(&time_info, &raw_time);
#else
    localtime_r(&raw_time, &time_info);
#endif

    std::stringstream ss;
    ss << std::put_time(&time_info, "%Y-%m-%d");
    return ss.str();
}

int getLogicalWeekday(int day_boundary_hour, int64_t epoch_sec) {
    std::time_t raw_time;
    if (epoch_sec < 0) {
        raw_time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    } else {
        raw_time = static_cast<std::time_t>(epoch_sec);
    }

    raw_time -= (day_boundary_hour * 3600);

    std::tm time_info;
#if defined(_WIN32)
    localtime_s(&time_info, &raw_time);
#else
    localtime_r(&raw_time, &time_info);
#endif

    // std::tm wday: 0 = Sun, 1 = Mon, ..., 6 = Sat
    // Convert to xLog convention: 0 = Mon, 1 = Tue, ..., 6 = Sun
    return (time_info.tm_wday + 6) % 7;
}

static std::tm parseIsoDate(const std::string& date_str) {
    std::tm tm{};
    std::stringstream ss(date_str);
    ss >> std::get_time(&tm, "%Y-%m-%d");
    tm.tm_hour = 12; // Noon to avoid DST transitions
    tm.tm_min = 0;
    tm.tm_sec = 0;
    return tm;
}

int calculateDaysBetween(const std::string& start_date, const std::string& end_date) {
    if (start_date.empty() || end_date.empty()) return 0;

    std::tm tm_start = parseIsoDate(start_date);
    std::tm tm_end = parseIsoDate(end_date);

    std::time_t time_start = std::mktime(&tm_start);
    std::time_t time_end = std::mktime(&tm_end);

    if (time_start == -1 || time_end == -1) return 0;

    double diff_seconds = std::difftime(time_end, time_start);
    return static_cast<int>(std::round(diff_seconds / 86400.0));
}

bool isTaskDueToday(const Task& task, const std::string& current_logical_date, int logical_weekday) {
    if (task.status != "active") return false;
    if (task.type == TaskType::Hobby) return false;

    // Same-day completion check: completing any task drops it from today for remainder of logical day
    if (task.last_completed_date.has_value() && *task.last_completed_date == current_logical_date) {
        return false;
    }

    if (task.type == TaskType::OneTime) {
        return true;
    }

    if (task.type == TaskType::Periodic) {
        if (!task.last_completed_date.has_value()) return true;
        int days_since = calculateDaysBetween(*task.last_completed_date, current_logical_date);
        int period = task.period_days.value_or(1);
        return days_since >= period;
    }

    if (task.type == TaskType::Recurring) {
        int mask = task.recurrence_mask.value_or(127);
        return (mask & (1 << logical_weekday)) != 0;
    }

    return false;
}

void updateTaskPrioritiesAndSchedules(Database& db, User& user, const std::string& current_logical_date) {
    int logical_wday = getLogicalWeekday(user.day_boundary_hour);
    auto all_tasks = db.getAllTasks(user.id);

    for (auto& t : all_tasks) {
        t.due_today = isTaskDueToday(t, current_logical_date, logical_wday);

        // Days overdue calculation
        int days_overdue = 0;
        if (t.type == TaskType::Periodic) {
            if (t.last_completed_date.has_value()) {
                int days_since = calculateDaysBetween(*t.last_completed_date, current_logical_date);
                int period = t.period_days.value_or(1);
                days_overdue = std::max(0, days_since - period);
            }
        } else if (t.due_today && t.first_appeared_date.has_value()) {
            days_overdue = std::max(0, calculateDaysBetween(*t.first_appeared_date, current_logical_date));
        }

        // Domain balance calculation
        double balance_val = 0.0;
        auto maj_sub_opt = db.getSubdomainById(t.major_subdomain_id);
        if (maj_sub_opt) {
            auto maj_dom_opt = db.getDomainById(maj_sub_opt->domain_id);
            if (maj_dom_opt) {
                balance_val = math::calculateBalance(maj_dom_opt->score_cached, user.rating_current);
            }
        }

        // Staleness calculation
        int days_stale = 0;
        if (t.started_at_date.has_value()) {
            days_stale = std::max(0, calculateDaysBetween(*t.started_at_date, current_logical_date));
        }

        // Priority calculation
        t.priority_current = math::calculatePriority(t.priority_base, days_overdue, balance_val, days_stale);

        db.updateTask(t);
    }
}

} // namespace TaskEngine
} // namespace xlog
