#include "xlog/ui/screens/quote_screen.hpp"
#include "xlog/ui/colors.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <sstream>

namespace xlog {
namespace ui {

// Helper to pad strings to exact visible widths for right border alignment
static std::string padRight(const std::string& str, size_t target_len) {
    if (str.length() > target_len) {
        return str.substr(0, target_len);
    }
    return str + std::string(target_len - str.length(), ' ');
}

// Helper to split text into multiple lines on whole words
static std::vector<std::string> wrapText(const std::string& text, size_t max_len) {
    std::vector<std::string> lines;
    std::istringstream words(text);
    std::string word;
    std::string current_line;

    while (words >> word) {
        if (current_line.empty()) {
            current_line = word;
        } else if (current_line.length() + 1 + word.length() <= max_len) {
            current_line += " " + word;
        } else {
            lines.push_back(current_line);
            current_line = word;
        }
    }
    
    if (!current_line.empty()) {
        lines.push_back(current_line);
    }
    
    return lines;
}

void renderQuote(Database& db, int64_t user_id) {
    auto q_opt = db.getRandomQuote(user_id);
    const std::string border_color = colors::LAVENDER;
    const std::string res          = colors::RESET;
    const std::string dim          = colors::OVERLAY2;

    std::cout << border_color << "╭── Quote ─────────────────────────────────╮" << res << "\n";

    if (q_opt) {
        // Wrap text cleanly to 40 characters without breaking mid-word
        std::vector<std::string> lines = wrapText(q_opt->text, 40);
        
        for (const auto& line : lines) {
            std::cout << border_color << "│ " << res
                      << colors::RED << colors::ITALIC << padRight(line, 40) << res
                      << border_color << " │\n" << res;
        }
    } else {
        std::cout << border_color << "│ " << res
                  << dim << padRight("No quotes found. Use 'xlog quote add'.", 40) << res
                  << border_color << " │\n" << res;
    }

    std::cout << border_color << "╰──────────────────────────────────────────╯" << res << "\n";
}

} // namespace ui
} // namespace xlog