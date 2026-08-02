#include "xlog/ui/screens/quote_screen.hpp"
#include "xlog/ui/colors.hpp"
#include <iostream>

namespace xlog {
namespace ui {

void renderQuote(Database& db, int64_t user_id) {
    auto q_opt = db.getRandomQuote(user_id);
    std::cout << "\n";
    if (q_opt) {
        std::cout << colors::MAUVE << colors::ITALIC << "💬 \"" << q_opt->text << "\"" << colors::RESET << "\n";
        if (!q_opt->author.empty()) {
            std::cout << colors::OVERLAY2 << "  — " << q_opt->author << colors::RESET << "\n";
        }
    } else {
        std::cout << colors::OVERLAY2 << "  No quotes found. Add quotes with 'xlog quote add \"text\" \"author\"'." << colors::RESET << "\n";
    }
    std::cout << "\n";
}

} // namespace ui
} // namespace xlog
