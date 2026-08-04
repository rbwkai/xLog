#include "xlog/ui/screens/tui_screen.hpp"
#include "xlog/ui/colors.hpp"
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>

namespace xlog {
namespace ui {

static std::string fitText(const std::string& str, size_t target_len) {
    if (str.length() > target_len) {
        return str.substr(0, target_len - 3) + "...";
    }
    return str + std::string(target_len - str.length(), ' ');
}

void renderTuiMenu(Database& db, int64_t user_id) {
    (void)db;
    (void)user_id;

    const std::string border_color = colors::LAVENDER;
    const std::string muted_color  = colors::OVERLAY2;
    const std::string res          = colors::RESET;
    const std::string bld          = colors::BOLD;
    const std::string text_col     = colors::TEXT;

    std::cout << "\n" << border_color << "╭── xLog TUI ──────────────────────────────╮" << res << "\n";

    struct Option {
        std::string num;
        std::string label;
        std::string color;
    };

    std::vector<Option> options = {
        {" 1", "Today Tasks",          colors::FLAMINGO},
        {" 2", "Profile & Stats",      colors::SKY},
        {" 3", "Prompt Status",        colors::MAUVE},
        {" 4", "Quick Tasks (<15m)",   colors::TEAL},
        {" 5", "Bored / Hobby Tasks",  colors::PEACH},
        {" 6", "Why Math Mechanics",   colors::PINK},
        {" 7", "Quotes Dashboard",     colors::YELLOW},
        {" 8", "Add New Task",         colors::GREEN},
        {" 9", "Complete Task",        colors::GREEN + colors::BOLD},
        {"10", "Edit Task",            colors::LAVENDER},
        {"11", "Pause / Resume Task",  colors::MAROON},
        {"12", "Help Reference",       colors::PINK},
        {" 0", "Exit TUI",             colors::OVERLAY2}
    };

    for (const auto& opt : options) {
        std::cout << border_color << "│ " << res;
        std::cout << muted_color << opt.num << ". " << res;
        std::string formatted_label = fitText(opt.label, 34);
        std::cout << opt.color << formatted_label << res;
        std::cout << border_color << " │\n" << res;
    }

    std::cout << border_color << "╰──────────────────────────────────────────╯" << res << "\n\n";
}

} // namespace ui
} // namespace xlog
