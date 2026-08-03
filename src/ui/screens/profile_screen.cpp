#include "xlog/ui/screens/profile_screen.hpp"
#include "xlog/ui/colors.hpp"
#include "xlog/ui/widgets.hpp"
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <algorithm>
#include <cstdio>

namespace xlog {
namespace ui {

static std::string format_number(double val, int precision = 1) {
    char buf[32];
    snprintf(buf, sizeof(buf), "%.*f", precision, val);
    return std::string(buf);
}

void renderProfile(Database& db, int64_t user_id) {
    auto stats = db.getProfileStats(user_id);
    User u = stats.user;

    std::string dim = colors::OVERLAY2;
    std::string border_col = colors::OVERLAY0;
    std::string it = colors::ITALIC;
    std::string und = "\033[4m"; // Standard ANSI underline
    std::string res = colors::RESET;
    std::string bld = colors::BOLD;
    std::string rc = getRankColor(u.rank_current);

    // EXACTLY 38 dashes. 
    // (1 left border + 1 space + 12 logo + 2 space + 20 text + 1 space + 1 right border = 38 total width)
    std::cout << "\n" << border_col << "╭────────────────────────────────────╮\n" << res;

    int text_col_width = 20; 

    auto print_fastfetch_row = [&](const std::string& logo_part, const std::string& text_part, int text_vis_len) {
        int right_pad = text_col_width - text_vis_len;
        std::cout << border_col << "│ "
                  << colors::LAVENDER << bld << logo_part << res
                  << "  " << text_part
                  << std::string(std::max(0, right_pad), ' ')
                  << border_col << " │\n" << res;
    };

    std::vector<std::string> logo = {
        "   __  __   ",
        "   \\ \\/ /   ",
        "    \\  /    ",
        "    /  \\    ",
        "   /_/\\_\\   "
    };

    struct Line {
        std::string text;
        int vis_len;
    };
    std::vector<Line> text_lines;

    // 1. Username (Rank colored, bold, italic, underlined)
    text_lines.push_back({
        rc + bld + it + und + u.username + res,
        static_cast<int>(u.username.length())
    });

    // 2. Rating (Normal text left, Rank colored number right)
    std::string r_left = "Rating:";
    std::string r_right = format_number(u.rating_current, 1);
    int r_pad = text_col_width - r_left.length() - r_right.length();
    
    text_lines.push_back({
        res + r_left + std::string(std::max(1, r_pad), ' ') + rc + r_right + res,
        text_col_width
    });

    // 5. Domains - pinned strictly to the absolute right edge
    std::vector<std::string> dom_cols = {
        colors::FLAMINGO,
        colors::SKY,
        colors::LAVENDER,
        colors::PINK
    };
    
    for (size_t i = 0; i < stats.domains.size(); i++) {
        std::string d_name = stats.domains[i].first.name;
        std::string s_score = format_number(stats.domains[i].first.score_cached, 1);
        std::string col = dom_cols[i % 4];

        int space_pad = text_col_width - d_name.length() - s_score.length();
        if (space_pad < 1) space_pad = 1;

        std::string line_str = col + bld + d_name + res + std::string(space_pad, ' ') + dim + s_score + res;
        text_lines.push_back({line_str, text_col_width});
    }

    // Print composed section
    size_t total_rows = std::max(logo.size(), text_lines.size());
    for (size_t i = 0; i < total_rows; ++i) {
        std::string l_part = (i < logo.size()) ? logo[i] : "            ";
        std::string t_part = (i < text_lines.size()) ? text_lines[i].text : "";
        int t_vis = (i < text_lines.size()) ? text_lines[i].vis_len : 0;
        print_fastfetch_row(l_part, t_part, t_vis);
    }

    // Exactly 38 dashes
    std::cout << border_col << "╰────────────────────────────────────╯\n" << colors::RESET;
}

} // namespace ui
} // namespace xlog