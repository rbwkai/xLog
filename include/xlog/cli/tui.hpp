#ifndef XLOG_CLI_TUI_HPP
#define XLOG_CLI_TUI_HPP

#include "xlog/db.hpp"

namespace xlog {
namespace cli {

void runTuiCommand(Database& db, int64_t user_id);

} // namespace cli
} // namespace xlog

#endif // XLOG_CLI_TUI_HPP
