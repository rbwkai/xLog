#ifndef XLOG_UI_WIDGETS_HPP
#define XLOG_UI_WIDGETS_HPP

#include <string>
#include <vector>

namespace xlog {
namespace ui {

std::string renderProgressBar(double current, double max_val, int width = 15);
std::string renderHeatmap(const std::vector<double>& daily_xp, int days = 30);
std::string renderRankBadge(const std::string& rank, double rating);

} // namespace ui
} // namespace xlog

#endif // XLOG_UI_WIDGETS_HPP
