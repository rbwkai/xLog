#include "xlog/ui/screens/profile_screen.hpp"
#include "xlog/ui/colors.hpp"
#include "xlog/ui/widgets.hpp"
#include <iostream>
#include <iomanip>

namespace xlog {
namespace ui {

void renderProfile(Database& db, int64_t user_id) {
    auto stats = db.getProfileStats(user_id);
    User u = stats.user;

    std::cout << "\n" << colors::LAVENDER << colors::BOLD << "╭──────────────────────────────────────────────────────────╮" << colors::RESET << "\n";
    std::cout << colors::LAVENDER << colors::BOLD << "│ USER PROFILE: " << colors::TEXT << u.username << std::string(43 - u.username.length(), ' ') << colors::LAVENDER << "│" << colors::RESET << "\n";
    std::cout << colors::LAVENDER << colors::BOLD << "╰──────────────────────────────────────────────────────────╯" << colors::RESET << "\n";

    std::cout << "  Rank:      " << renderRankBadge(u.rank_current, u.rating_current) << "\n";
    std::cout << "  Streak:    " << colors::PEACH << u.streak_days << " days" << colors::RESET << " (Longest: " << u.longest_streak << " days)\n";
    std::cout << "  Completed: " << colors::GREEN << stats.total_tasks_completed << " tasks" << colors::RESET << " (" << std::fixed << std::setprecision(0) << stats.total_xp_earned << " total XP)\n\n";

    std::cout << colors::LAVENDER << colors::BOLD << "── Domain Progression ────────────────────────────────────" << colors::RESET << "\n";
    for (size_t i = 0; i < stats.domains.size(); ++i) {
        Domain dom = stats.domains[i].first;
        std::string dom_col = getDomainColor(dom.order_index);
        std::cout << "  " << dom_col << colors::BOLD << dom.name << colors::RESET 
                  << " (Score: " << std::fixed << std::setprecision(1) << dom.score_cached << ")\n";

        for (const auto& sub : stats.domains[i].second) {
            std::cout << "    └─ " << colors::TEXT << sub.name << colors::RESET 
                      << " " << renderProgressBar(sub.score_cached, 3000.0, 14)
                      << " " << colors::OVERLAY2 << std::fixed << std::setprecision(0) << sub.xp_raw_total << " XP" << colors::RESET << "\n";
        }
    }
    std::cout << "\n";
}

} // namespace ui
} // namespace xlog
