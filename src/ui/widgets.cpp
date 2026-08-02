#include "xlog/ui/widgets.hpp"
#include "xlog/ui/colors.hpp"
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cmath>

namespace xlog {
namespace ui {

std::string renderProgressBar(double current, double max_val, int width) {
    if (max_val <= 0) max_val = 1.0;
    double ratio = std::clamp(current / max_val, 0.0, 1.0);
    int filled = static_cast<int>(std::round(ratio * width));

    std::stringstream ss;
    ss << colors::SAPPHIRE;
    for (int i = 0; i < filled; ++i) ss << "█";
    ss << colors::OVERLAY1;
    for (int i = filled; i < width; ++i) ss << "░";
    ss << colors::RESET;
    return ss.str();
}

std::string renderHeatmap(const std::vector<double>& daily_xp, int days) {
    std::stringstream ss;
    for (int i = 0; i < days; ++i) {
        double xp = (i < static_cast<int>(daily_xp.size())) ? daily_xp[i] : 0.0;
        if (xp <= 0) ss << colors::OVERLAY1 << "░ " << colors::RESET;
        else if (xp < 50) ss << colors::GREEN << "▒ " << colors::RESET;
        else ss << colors::GREEN << colors::BOLD << "█ " << colors::RESET;
    }
    return ss.str();
}

std::string renderRankBadge(const std::string& rank, double rating) {
    std::stringstream ss;
    std::string rank_col = getRankColor(rank);
    ss << rank_col << colors::BOLD << "🏆 " << rank << " (" << std::fixed << std::setprecision(0) << rating << ")" << colors::RESET;
    return ss.str();
}

} // namespace ui
} // namespace xlog
