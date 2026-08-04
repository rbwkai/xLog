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
    std::string border_col = colors::LAVENDER; // Matching Today screen border
    std::string und = "\033[4m";
    std::string res = colors::RESET;
    std::string bld = colors::BOLD;
    std::string itl = colors::ITALIC;
    std::string rc = getRankColor(u.rank_current);

    // EXACTLY 44 columns total width
    std::cout << "\n" << border_col << "╭── Profile ───────────────────────────────╮\n" << res;

    // Layout dimensions:
    // 2 (│ ) + 12 (logo) + 2 (  ) + 26 (text) + 2 ( │) = 44 total width
    int text_col_width = 26; 

    auto print_fastfetch_row = [&](const std::string& logo_part, const std::string& text_part, int text_vis_len) {
        int right_pad = text_col_width - text_vis_len;
        if (right_pad < 0) right_pad = 0;
        
        std::cout << border_col << "│ "
                  << colors::LAVENDER << bld << logo_part << res
                  << "  " << text_part
                  << std::string(right_pad, ' ')
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

    // 1. Top padding line (aligns with logo line 1)
    text_lines.push_back({"", 0});

    // 2. Username on Left, Rating Number on Right (Aligned with 2nd column)
    int left_target_width = 13; // Exactly half of 26-column text section (matches 2nd domain column start)

    std::string user_part = rc + bld + itl + und + u.username + res;
    int user_vis_len = static_cast<int>(u.username.length());

    std::string r_right = format_number(u.rating_current, 1);
    std::string rating_part = rc + bld + r_right + res;
    int rating_vis_len = static_cast<int>(r_right.length());

    int u_pad = left_target_width - user_vis_len;
    if (u_pad < 1) u_pad = 1;

    std::string row_user_rating = user_part + std::string(u_pad, ' ') + rating_part;
    int row_user_rating_len = user_vis_len + u_pad + rating_vis_len;

    text_lines.push_back({row_user_rating, row_user_rating_len});

    // 3. Spacing line
    text_lines.push_back({"", 0});

    // 4. Domains (2 per row in grid format using distinct geometric shapes & domain colors)
    const std::vector<std::string> dom_cols = {
        colors::FLAMINGO,
        colors::SKY,
        colors::LAVENDER,
        colors::PINK
    };

    const std::vector<std::string> shapes = {
        "◆ F:", // Diamond
        "▲ I:", // Triangle
        "■ P:", // Square
        "✦ A:"  // Four-pointed star
    };

    auto make_domain_cell = [&](size_t idx) {
        if (idx >= stats.domains.size()) {
            return std::make_pair(std::string(""), 0);
        }
        std::string col = dom_cols[idx % dom_cols.size()];
        std::string shape = shapes[idx % shapes.size()];
        std::string score_str = format_number(stats.domains[idx].first.score_cached, 1);

        // Visual: Colored shape + space + dimmed score
        std::string cell_text = col + shape + res + " " + dim + score_str + res;
        int vis_len = 5 + static_cast<int>(score_str.length());
        
        return std::make_pair(cell_text, vis_len);
    };

    // Render domains two per row dynamically
    for (size_t i = 0; i < stats.domains.size(); i += 2) {
        auto cell_left = make_domain_cell(i);
        auto cell_right = make_domain_cell(i + 1);

        int space_pad = left_target_width - cell_left.second;
        if (space_pad < 1) space_pad = 1;

        std::string row_text = cell_left.first + std::string(space_pad, ' ') + cell_right.first;
        int row_vis_len = cell_left.second + space_pad + cell_right.second;

        text_lines.push_back({row_text, row_vis_len});
    }

    // Render side-by-side with Fastfetch ASCII logo
    size_t total_rows = std::max(logo.size(), text_lines.size());
    for (size_t i = 0; i < total_rows; ++i) {
        std::string l_part = (i < logo.size()) ? logo[i] : "            ";
        std::string t_part = (i < text_lines.size()) ? text_lines[i].text : "";
        int t_vis = (i < text_lines.size()) ? text_lines[i].vis_len : 0;
        print_fastfetch_row(l_part, t_part, t_vis);
    }

    // Bottom border (Exactly 44 total width)
    std::cout << border_col << "╰──────────────────────────────────────────╯\n\n" << colors::RESET;
}

} // namespace ui
} // namespace xlog