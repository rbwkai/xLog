#ifndef XLOG_CLI_HPP
#define XLOG_CLI_HPP

#include "xlog/db.hpp"
#include <string>
#include <vector>

namespace xlog {
namespace cli {

std::string matchFuzzyCommand(const std::string& input);
void runOnboarding(Database& db);
void runAddTask(Database& db, int64_t user_id);
void runDoneTask(Database& db, int64_t user_id, const std::vector<std::string>& args);
void runQuoteCommand(Database& db, int64_t user_id, const std::vector<std::string>& args);
void dispatchCommand(Database& db, const std::vector<std::string>& args);

} // namespace cli
} // namespace xlog

#endif // XLOG_CLI_HPP
