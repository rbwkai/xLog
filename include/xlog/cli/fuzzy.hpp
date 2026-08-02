#ifndef XLOG_CLI_FUZZY_HPP
#define XLOG_CLI_FUZZY_HPP

#include <string>

namespace xlog {
namespace cli {

int levenshteinDistance(const std::string& s1, const std::string& s2);
std::string matchFuzzyCommand(const std::string& input);

} // namespace cli
} // namespace xlog

#endif // XLOG_CLI_FUZZY_HPP
