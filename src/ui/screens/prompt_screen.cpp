#include "xlog/ui/screens/prompt_screen.hpp"
#include "xlog/ui/colors.hpp"
#include "xlog/ui/widgets.hpp"
#include "xlog/math/scoring.hpp"
#include <iostream>
#include <iomanip>
#include <sstream>

namespace xlog {
namespace ui {

static std::string fitText(const std::string& str, size_t target_len) {
    if (str.length() > target_len) {
        return str.substr(0, target_len - 3) + "...";
    }
    return str + std::string(target_len - str.length(), ' ');
}

void renderPrompt(Database& db, int64_t user_id) {
    auto u_opt = db.getUser();
    if (!u_opt) return;
    User u = *u_opt;

    double pts_to_next = math::pointsToNextRank(u.rating_current);
    Rank current_rank = math::ratingToRank(u.rating_current);
    std::string next_rank_name = "Max Rank";
    if (current_rank == Rank::Gray) next_rank_name = "Green";
    else if (current_rank == Rank::Green) next_rank_name = "Cyan";
    else if (current_rank == Rank::Cyan) next_rank_name = "Blue";
    else if (current_rank == Rank::Blue) next_rank_name = "Violet";
    else if (current_rank == Rank::Violet) next_rank_name = "Orange";
    else if (current_rank == Rank::Orange) next_rank_name = "Red";

    auto q_opt = db.getRandomQuote(user_id);

    const std::string border_color = colors::LAVENDER;
    const std::string res          = colors::RESET;
    const std::string bld          = colors::BOLD;

    std::cout << "\n" << border_color << "╭── Status & Quote ────────────────────────╮" << res << "\n";

    // Row 1: Username, Rank Badge, Streak
    std::string rank_str = renderRankBadge(u.rank_current, u.rating_current);
    std::string streak_str = "🔥 " + std::to_string(u.streak_days) + "d streak";

    std::cout << border_color << "│ " << res
              << colors::LAVENDER << bld << "⚡ " << colors::TEXT << u.username << res
              << " " << rank_str << "  "
              << colors::PEACH << streak_str << res;

    int vis1 = 2 + static_cast<int>(u.username.length()) + 1 + static_cast<int>(u.rank_current.length()) + 6 + 2 + static_cast<int>(streak_str.length());
    int pad1 = 40 - vis1;
    if (pad1 < 0) pad1 = 0;
    std::cout << std::string(pad1, ' ') << border_color << " │\n" << res;

    // Row 2: Rating progress
    if (pts_to_next > 0) {
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(1) << pts_to_next << " Rating to " << next_rank_name;
        std::string pts_str = ss.str();
        std::cout << border_color << "│ " << res
                  << colors::OVERLAY2 << fitText(pts_str, 40) << res
                  << border_color << " │\n" << res;
    }

    // Row 3: Quote
    if (q_opt) {
        std::string quote_text = "💬 \"" + q_opt->text + "\"";
        if (!q_opt->author.empty()) quote_text += " — " + q_opt->author;
        std::cout << border_color << "│ " << res
                  << colors::MAUVE << colors::ITALIC << fitText(quote_text, 40) << res
                  << border_color << " │\n" << res;
    }

    std::cout << border_color << "╰──────────────────────────────────────────╯" << res << "\n\n";
}

} // namespace ui
} // namespace xlog
