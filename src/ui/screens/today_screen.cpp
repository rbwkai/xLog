#include "xlog/ui/screens/today_screen.hpp"
#include "xlog/ui/colors.hpp"
#include <iostream>
#include <iomanip>

namespace xlog {
namespace ui {

void renderToday(Database& db, int64_t user_id) {
    auto tasks = db.getTodayTasks(user_id);
    std::cout << "\n" << colors::LAVENDER << colors::BOLD << "── Today's Objectives ────────────────────────────────────" << colors::RESET << "\n";

    if (tasks.empty()) {
        std::cout << colors::OVERLAY2 << "  No tasks scheduled for today! Use 'xlog add' to create one." << colors::RESET << "\n\n";
        return;
    }

    bool first = true;
    for (const auto& t : tasks) {
        std::cout << "  ";
        if (first) {
            std::cout << colors::PEACH << colors::BOLD << "🐸 " << colors::RESET;
            first = false;
        } else {
            std::cout << colors::TEXT << "• " << colors::RESET;
        }

        std::string priority_icon = "▲";
        if (t.priority_current > 0.7) priority_icon = "▲▲▲";
        else if (t.priority_current > 0.4) priority_icon = "▲▲";

        std::cout << colors::TEXT << colors::BOLD << t.name << colors::RESET << " "
                  << colors::OVERLAY2 << priority_icon << " (" << std::fixed << std::setprecision(0) 
                  << t.difficulty_current << "m)" << colors::RESET << "\n";
    }
    std::cout << "\n";
}

} // namespace ui
} // namespace xlog
