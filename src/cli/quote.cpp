#include "xlog/cli/quote.hpp"
#include "xlog/ui/screens/quote_screen.hpp"
#include "xlog/ui/colors.hpp"
#include <iostream>

namespace xlog {
namespace cli {

void runQuoteCommand(Database& db, int64_t user_id, const std::vector<std::string>& args) {
    if (args.size() <= 1) {
        ui::renderQuote(db, user_id);
        return;
    }

    std::string subcmd = args[1];
    if (subcmd == "add") {
        if (args.size() < 3) {
            std::cout << "Usage: xlog quote add \"Quote text\" [Author]\n";
            return;
        }
        std::string text = args[2];
        std::string author = (args.size() >= 4) ? args[3] : "";
        db.addQuote(user_id, text, author);
        std::cout << ui::colorText("✓ Quote added.\n", ui::colors::GREEN);
    } else if (subcmd == "list") {
        auto quotes = db.getAllQuotes(user_id);
        std::cout << "\n" << ui::colors::LAVENDER << ui::colors::BOLD << "── Saved Quotes ────────────────────────────────────" << ui::colors::RESET << "\n";
        for (const auto& q : quotes) {
            std::cout << "  " << ui::colors::OVERLAY2 << "[" << q.id << "] " << ui::colors::TEXT << "\"" << q.text << "\""
                      << (q.author.empty() ? "" : " — " + q.author) << ui::colors::RESET << "\n";
        }
        std::cout << "\n";
    }
}

} // namespace cli
} // namespace xlog
