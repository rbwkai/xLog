#include "xlog/ui/screens/quick_screen.hpp"
#include "xlog/ui/colors.hpp"
#include <iostream>
#include <iomanip>

namespace xlog {
namespace ui {

void renderQuick(Database& db, int64_t user_id) {
    auto tasks = db.getQuickTasks(user_id);
    std::cout << "\n" << colors::LAVENDER << colors::BOLD << "── Quick Tasks (<15 min) ─────────────────────────────────" << colors::RESET << "\n";

    if (tasks.empty()) {
        std::cout << colors::OVERLAY2 << "  No tasks under 15 minutes found." << colors::RESET << "\n\n";
        return;
    }

    for (const auto& t : tasks) {
        std::cout << "  " << colors::TEAL << "⚡ " << colors::RESET << colors::TEXT << colors::BOLD << t.name << colors::RESET 
                  << " " << colors::OVERLAY2 << "(" << std::fixed << std::setprecision(0) << t.difficulty_current << "m)" << colors::RESET << "\n";
    }
    std::cout << "\n";
}

} // namespace ui
} // namespace xlog
