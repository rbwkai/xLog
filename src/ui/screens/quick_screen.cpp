#include "xlog/ui/screens/quick_screen.hpp"
#include "xlog/ui/colors.hpp"
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

void renderQuick(Database& db, int64_t user_id) {
    auto tasks = db.getQuickTasks(user_id);
    const std::string border_color = colors::LAVENDER;
    const std::string res          = colors::RESET;
    const std::string dim          = colors::OVERLAY2;

    std::cout << "\n" << border_color << "╭── Quick Tasks (<15m) ────────────────────╮" << res << "\n";

    if (tasks.empty()) {
        std::cout << border_color << "│ " << res
                  << dim << fitText("No quick tasks under 15m found.", 40) << res
                  << border_color << " │\n" << res;
    } else {
        for (const auto& t : tasks) {
            std::ostringstream d_stream;
            d_stream << std::setw(3) << static_cast<int>(t.difficulty_current) << "m";

            std::cout << border_color << "│ " << res
                      << colors::TEAL << "⚡ " << res
                      << colors::TEXT << fitText(t.name, 30) << res
                      << " " << colors::MAUVE << colors::ITALIC << d_stream.str() << res
                      << border_color << " │\n" << res;
        }
    }

    std::cout << border_color << "╰──────────────────────────────────────────╯" << res << "\n\n";
}

} // namespace ui
} // namespace xlog
