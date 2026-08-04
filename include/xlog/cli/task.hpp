#ifndef XLOG_CLI_TASK_HPP
#define XLOG_CLI_TASK_HPP

#include "xlog/db.hpp"
#include <vector>
#include <string>

namespace xlog {
namespace cli {

int parseWeekdayMask(const std::string& input);
void runAddTask(Database& db, int64_t user_id, const std::vector<std::string>& args = {});
void runDoneTask(Database& db, int64_t user_id, const std::vector<std::string>& args = {});
void runEditTask(Database& db, int64_t user_id, const std::vector<std::string>& args = {});
void runPauseTask(Database& db, int64_t user_id, const std::vector<std::string>& args = {});

} // namespace cli
} // namespace xlog

#endif // XLOG_CLI_TASK_HPP
