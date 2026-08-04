#include "xlog/ui/screens/quote_screen.hpp"
#include "xlog/ui/colors.hpp"
#include <iostream>

namespace xlog {
namespace ui {

static std::string fitText(const std::string& str, size_t target_len) {
    if (str.length() > target_len) {
        return str.substr(0, target_len - 3) + "...";
    }
    return str + std::string(target_len - str.length(), ' ');
}

void renderQuote(Database& db, int64_t user_id) {
    auto q_opt = db.getRandomQuote(user_id);
    const std::string border_color = colors::LAVENDER;
    const std::string res          = colors::RESET;
    const std::string dim          = colors::OVERLAY2;

    std::cout << "\n" << border_color << "╭── Motivational Quote ────────────────────╮" << res << "\n";

    if (q_opt) {
        std::string quote_text = "💬 \"" + q_opt->text + "\"";
        std::cout << border_color << "│ " << res
                  << colors::MAUVE << colors::ITALIC << fitText(quote_text, 40) << res
                  << border_color << " │\n" << res;

        if (!q_opt->author.empty()) {
            std::string author_text = "  — " + q_opt->author;
            std::cout << border_color << "│ " << res
                      << dim << fitText(author_text, 40) << res
                      << border_color << " │\n" << res;
        }
    } else {
        std::cout << border_color << "│ " << res
                  << dim << fitText("No quotes found. Use 'xlog quote add'.", 40) << res
                  << border_color << " │\n" << res;
    }

    std::cout << border_color << "╰──────────────────────────────────────────╯" << res << "\n\n";
}

} // namespace ui
} // namespace xlog
