#include "xlog/ui/screens/why_screen.hpp"
#include "xlog/ui/colors.hpp"
#include <iostream>

namespace xlog {
namespace ui {

void renderWhy() {
    std::cout << "\n" << colors::LAVENDER << colors::BOLD << "── xLog System Mechanics & Math Engine ───────────────────" << colors::RESET << "\n";
    std::cout << colors::TEXT << "  1. " << colors::BOLD << "1 Minute = 1 XP Base:" << colors::RESET << " XP is calibrated to task difficulty minutes.\n";
    std::cout << colors::TEXT << "  2. " << colors::BOLD << "Flow Channel Calibration:" << colors::RESET << " Task difficulty automatically scales to maintain a 70–85% completion rate.\n";
    std::cout << colors::TEXT << "  3. " << colors::BOLD << "Harmonic Domain Rollup:" << colors::RESET << " Global Rating uses a Harmonic Mean across 4 Domains, penalizing neglected areas.\n";
    std::cout << colors::TEXT << "  4. " << colors::BOLD << "Eat the Frog Protocol:" << colors::RESET << " Daily priority task grants a 1.5x–4.0x variable XP multiplier.\n";
    std::cout << colors::TEXT << "  5. " << colors::BOLD << "Soft Rust Decay:" << colors::RESET << " Neglected subdomains decay exponentially toward a 70% floor over a 90-day constant.\n\n";
}

} // namespace ui
} // namespace xlog
