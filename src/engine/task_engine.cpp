#include "xlog/task_engine.hpp"
#include "xlog/math/xp.hpp"
#include "xlog/math/calibration.hpp"
#include <chrono>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cmath>
#include <optional>

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

// For a Recurring task, returns the number of days since its FIRST unresolved scheduled
// occurrence - the oldest scheduled weekday that has come and gone since the task's last
// completion (or since it first appeared, if it has never been completed) - measured up
// to current_logical_date. Returns std::nullopt if the task is not currently due (no
// scheduled occurrence has come due yet).
//
// Anchoring on the *first* unresolved occurrence (rather than the most recent one) means
// debt keeps growing the longer a task goes undone: due Sat/Sun, ignored for three weeks,
// the overdue count keeps climbing from that original first missed Saturday rather than
// quietly resetting every time another Sat/Sun also gets missed. Completing the task wipes
// all of that out at once - the next cycle starts counting fresh from the next scheduled
// weekday after the completion date, so nothing piles up going forward.
static std::optional<int> recurringDaysOverdue(const Task& task, const std::string& current_logical_date, int logical_weekday) {
    int mask = task.recurrence_mask.value_or(127);
    if (mask <= 0) return std::nullopt; // no scheduled weekdays set on this task

    std::string origin;
    int start_offset; // 0 = origin day itself counts as a possible occurrence, 1 = it doesn't
    if (task.last_completed_date.has_value()) {
        origin = *task.last_completed_date;
        start_offset = 1; // completing on a scheduled day resolves that day's occurrence
    } else if (task.first_appeared_date.has_value()) {
        origin = *task.first_appeared_date;
        start_offset = 0; // a brand new task can be due on the very day it appears
    } else {
        // No reference date at all - fall back to the simplest possible check.
        return (mask & (1 << logical_weekday)) != 0 ? std::make_optional(0) : std::nullopt;
    }

    int days_since_origin = calculateDaysBetween(origin, current_logical_date);
    if (days_since_origin < start_offset) return std::nullopt;

    // Weekdays cycle every single day regardless of month/leap-year boundaries, so the
    // origin's weekday can be derived from today's without any extra date parsing.
    int origin_weekday = ((logical_weekday - days_since_origin) % 7 + 7) % 7;

    // Scan forward from the origin for the earliest matching scheduled weekday. Since the
    // mask is non-empty, a match (if any lies within range) is always found within 7 steps.
    for (int i = start_offset; i <= days_since_origin; ++i) {
        int wd = (origin_weekday + i) % 7;
        if (mask & (1 << wd)) {
            return days_since_origin - i;
        }
    }
    return std::nullopt; // origin is set, but no scheduled day has occurred yet
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
        return recurringDaysOverdue(task, current_logical_date, logical_weekday).has_value();
    }

    return false;
}

void updateTaskPrioritiesAndSchedules(Database& db, User& user, const std::string& current_logical_date) {
    int logical_wday = getLogicalWeekday(user.day_boundary_hour);
    auto all_tasks = db.getAllTasks(user.id);

    for (auto& t : all_tasks) {
        // Invariant: non-active (paused) tasks are frozen in time - no debt, no overdue
        // pressure, no date-field mutation while paused, and resuming must not retroactively
        // penalize. db.getAllTasks() already filters to status == "active", but we guard
        // here too so this invariant holds regardless of how the task list was fetched.
        if (t.status != "active") {
            continue;
        }

        // Self-heal: first_appeared_date should always be set for a task that exists and is
        // active, since it's the origin point for One-Time debt and for a Periodic task's
        // very first (pre-completion) occurrence. Rows created before this field was wired
        // up (or via any path that doesn't set it) will still have it unset - backfill it
        // the first time the engine sees them, rather than silently treating them as 0 days
        // overdue forever.
        if (!t.first_appeared_date.has_value() &&
            (t.type == TaskType::OneTime || t.type == TaskType::Periodic)) {
            t.first_appeared_date = current_logical_date;
        }

        t.due_today = isTaskDueToday(t, current_logical_date, logical_wday);

        // Days overdue calculation
        int days_overdue = 0;
        if (t.type == TaskType::Periodic) {
            if (t.last_completed_date.has_value()) {
                // Debt resets the moment the task is completed: the next cycle is measured
                // from *that* completion date forward, so a long-missed task never piles up
                // extra debt from previous cycles once it's finally done.
                int days_since = calculateDaysBetween(*t.last_completed_date, current_logical_date);
                int period = t.period_days.value_or(1);
                days_overdue = std::max(0, days_since - period);
            } else if (t.first_appeared_date.has_value()) {
                // Never completed yet: there's no prior cycle to measure the period against,
                // so it behaves like a One-Time task - due and accruing debt from the day it
                // appeared.
                days_overdue = std::max(0, calculateDaysBetween(*t.first_appeared_date, current_logical_date));
            }
        } else if (t.type == TaskType::Recurring) {
            // Overdue = days since the FIRST unresolved scheduled occurrence since the
            // last completion (or since the task appeared, if never completed) - debt
            // keeps growing the longer it's ignored, and is fully cleared on completion.
            days_overdue = recurringDaysOverdue(t, current_logical_date, logical_wday).value_or(0);
        } else if (t.type == TaskType::OneTime) {
            if (t.due_today && t.first_appeared_date.has_value()) {
                days_overdue = std::max(0, calculateDaysBetween(*t.first_appeared_date, current_logical_date));
            }
        }
        // Hobby tasks: days_overdue stays 0 - hobbies never generate overdue pressure.

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