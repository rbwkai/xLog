#include "xlog/ui/screens/bored_screen.hpp"
#include "xlog/ui/colors.hpp"
#include <iostream>
#include <iomanip>

namespace xlog {
namespace ui {

void renderBored(Database& db, int64_t user_id) {
    auto tasks = db.getBoredTasks(user_id);
    std::cout << "\n" << colors::LAVENDER << colors::BOLD << "── Hobby & Side Quest List ───────────────────────────────" << colors::RESET << "\n";

    if (tasks.empty()) {
        std::cout << colors::OVERLAY2 << "  No hobby tasks found. Add a task with type 'hobby' using 'xlog add'." << colors::RESET << "\n\n";
        return;
    }

    for (const auto& t : tasks) {
        std::cout << "  " << colors::YELLOW << "★ " << colors::RESET << colors::TEXT << colors::BOLD << t.name << colors::RESET 
                  << " " << colors::OVERLAY2 << "(" << std::fixed << std::setprecision(0) << t.difficulty_current << "m)" << colors::RESET << "\n";
    }
    std::cout << "\n";
}

} // namespace ui
} // namespace xlog
