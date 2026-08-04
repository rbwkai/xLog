#ifndef XLOG_CLI_HPP
#define XLOG_CLI_HPP

#include "xlog/db.hpp"
#include "xlog/cli/fuzzy.hpp"
#include "xlog/cli/onboarding.hpp"
#include "xlog/cli/task.hpp"
#include "xlog/cli/quote.hpp"
#include "xlog/cli/tui.hpp"
#include <string>
#include <vector>

namespace xlog {
namespace cli {

void dispatchCommand(Database& db, const std::vector<std::string>& args);

} // namespace cli
} // namespace xlog

#endif // XLOG_CLI_HPP
