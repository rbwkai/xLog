#include "xlog/cli/fuzzy.hpp"
#include <vector>
#include <algorithm>
#include <limits>

namespace xlog {
namespace cli {

int levenshteinDistance(const std::string& s1, const std::string& s2) {
    size_t m = s1.size();
    size_t n = s2.size();
    std::vector<std::vector<int>> dp(m + 1, std::vector<int>(n + 1, 0));

    for (size_t i = 0; i <= m; ++i) dp[i][0] = static_cast<int>(i);
    for (size_t j = 0; j <= n; ++j) dp[0][j] = static_cast<int>(j);

    for (size_t i = 1; i <= m; ++i) {
        for (size_t j = 1; j <= n; ++j) {
            if (s1[i - 1] == s2[j - 1]) dp[i][j] = dp[i - 1][j - 1];
            else dp[i][j] = 1 + std::min({dp[i - 1][j], dp[i][j - 1], dp[i - 1][j - 1]});
        }
    }
    return dp[m][n];
}

std::string matchFuzzyCommand(const std::string& input) {
    std::vector<std::string> valid_cmds = {
        "prompt", "today", "add", "done", "profile", "bored", "quick", "why", "quote", "setup", "help"
    };

    std::string lower_in = input;
    std::transform(lower_in.begin(), lower_in.end(), lower_in.begin(), ::tolower);

    for (const auto& cmd : valid_cmds) {
        if (cmd == lower_in || cmd.rfind(lower_in, 0) == 0) {
            return cmd;
        }
    }

    std::string best_match = input;
    int min_dist = std::numeric_limits<int>::max();
    for (const auto& cmd : valid_cmds) {
        int dist = levenshteinDistance(lower_in, cmd);
        if (dist < min_dist && dist <= 2) {
            min_dist = dist;
            best_match = cmd;
        }
    }

    return best_match;
}

} // namespace cli
} // namespace xlog
