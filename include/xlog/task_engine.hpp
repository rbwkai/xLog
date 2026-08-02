#ifndef XLOG_TASK_ENGINE_HPP
#define XLOG_TASK_ENGINE_HPP

#include "xlog/types.hpp"
#include "xlog/db.hpp"
#include <string>
#include <vector>
#include <cstdint>

namespace xlog {
namespace TaskEngine {

// Logical Date Utilities (offset by user's day_boundary_hour)
std::string getLogicalDate(int day_boundary_hour = 4, int64_t epoch_sec = -1);
int getLogicalWeekday(int day_boundary_hour = 4, int64_t epoch_sec = -1); // 0 = Mon ... 6 = Sun
int calculateDaysBetween(const std::string& start_date, const std::string& end_date);

// Task Scheduling Logic
bool isTaskDueToday(const Task& task, const std::string& current_logical_date, int logical_weekday);

// Dynamic Calibration Pipeline (Updates due_today and priority_current across active tasks)
void updateTaskPrioritiesAndSchedules(Database& db, User& user, const std::string& current_logical_date);

} // namespace TaskEngine
} // namespace xlog

#endif // XLOG_TASK_ENGINE_HPP
