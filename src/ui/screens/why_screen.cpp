#include "xlog/ui/screens/why_screen.hpp"
#include "xlog/ui/colors.hpp"
#include <iostream>
#include <vector>

namespace xlog {
namespace ui {

static std::string fitText(const std::string& str, size_t target_len) {
    if (str.length() > target_len) {
        return str.substr(0, target_len - 3) + "...";
    }
    return str + std::string(target_len - str.length(), ' ');
}

void renderWhy() {
    const std::string border_color = colors::LAVENDER;
    const std::string res          = colors::RESET;
    const std::string bld          = colors::BOLD;
    const std::string text_col     = colors::TEXT;
    const std::string dim          = colors::OVERLAY2;

    std::cout << "\n" << border_color << "╭── System Mechanics ──────────────────────╮" << res << "\n";

    std::vector<std::pair<std::string, std::string>> rules = {
        {"1. 1 Min = 1 XP", "Calibrated to task difficulty"},
        {"2. Flow Calibration", "Difficulty scales to 70-85% rate"},
        {"3. Domain Rollup", "Harmonic mean across 4 domains"},
        {"4. Eat the Frog", "Priority task gives 1.5x-4.0x XP"},
        {"5. Soft Rust Decay", "Exponential decay to 70% floor"}
    };

    for (const auto& rule : rules) {
        std::cout << border_color << "│ " << res
                  << colors::FLAMINGO << bld << fitText(rule.first, 18) << res
                  << dim << fitText(rule.second, 22) << res
                  << border_color << " │\n" << res;
    }

    std::cout << border_color << "╰──────────────────────────────────────────╯" << res << "\n\n";
}

} // namespace ui
} // namespace xlog
