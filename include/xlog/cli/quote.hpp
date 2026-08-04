#ifndef XLOG_CLI_QUOTE_HPP
#define XLOG_CLI_QUOTE_HPP

#include "xlog/db.hpp"
#include <vector>
#include <string>

namespace xlog {
namespace cli {

void runQuoteCommand(Database& db, int64_t user_id, const std::vector<std::string>& args);

} // namespace cli
} // namespace xlog

#endif // XLOG_CLI_QUOTE_HPP
