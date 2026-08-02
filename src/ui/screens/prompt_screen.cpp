#include "xlog/ui/screens/prompt_screen.hpp"
#include "xlog/ui/colors.hpp"
#include "xlog/ui/widgets.hpp"
#include "xlog/math.hpp"
#include <iostream>
#include <iomanip>

namespace xlog {
namespace ui {

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

    std::cout << colors::LAVENDER << colors::BOLD << "⚡ xLog" << colors::RESET << " │ "
              << colors::TEXT << colors::BOLD << u.username << colors::RESET << "  "
              << renderRankBadge(u.rank_current, u.rating_current) << "  ";

    if (pts_to_next > 0) {
        std::cout << colors::OVERLAY2 << "(" << std::fixed << std::setprecision(0) << pts_to_next 
                  << " Rating to " << next_rank_name << ")" << colors::RESET;
    }

    std::cout << "  " << colors::PEACH << "🔥 " << u.streak_days << "d streak" << colors::RESET << "\n";

    if (q_opt) {
        std::cout << colors::MAUVE << colors::ITALIC << "💬 \"" << q_opt->text << "\"" 
                  << (q_opt->author.empty() ? "" : " — " + q_opt->author) << colors::RESET << "\n";
    }
}

} // namespace ui
} // namespace xlog
