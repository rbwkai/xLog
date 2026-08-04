#include "xlog/ui/screens/today_screen.hpp"
#include "xlog/ui/colors.hpp"
#include "xlog/db.hpp"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include <optional>

namespace xlog {
namespace ui {

// Helper to truncate or pad strings to exact visible widths
static std::string fitText(const std::string& str, size_t target_len) {
    if (str.length() > target_len) {
        return str.substr(0, target_len - 3) + "...";
    }
    return str + std::string(target_len - str.length(), ' ');
}

void renderToday(Database& db, int64_t user_id) {
    auto tasks = db.getTodayTasks(user_id);

    // Hardcoded color palette for domains (matching your pinned layout)
    const std::vector<std::string> dom_cols = {
        colors::FLAMINGO,
        colors::SKY,
        colors::LAVENDER,
        colors::PINK
    };

    // Box dimensions (44 characters total width)
    const std::string border_color = colors::LAVENDER;
    const std::string muted_color  = colors::OVERLAY2;

    // --- Top Border (exactly 44 cols) ---
    std::cout << "\n" 
              << border_color << "╭── Today ─────────────────────────────────╮" 
              << colors::RESET << "\n";

    if (tasks.empty()) {
        std::cout << border_color << "│ " << colors::RESET
                  << muted_color << fitText("No tasks scheduled for today!", 40) << colors::RESET
                  << border_color << " │\n" << colors::RESET;
        std::cout << border_color << "╰──────────────────────────────────────────╯" 
                  << colors::RESET << "\n\n";
        return;
    }

    bool first = true;
    for (const auto& t : tasks) {
        bool is_quick_task    = t.difficulty_current <= 15.0;

        // Safely fetch Subdomain to determine the correct domain color and shape
        int64_t domain_id = 0;
        if (t.major_subdomain_id > 0) {
            std::optional<Subdomain> sub = db.getSubdomainById(t.major_subdomain_id);
            if (sub.has_value()) {
                domain_id = sub->domain_id;
            }
        }
        
        std::string bullet_color = dom_cols[domain_id % 4];

        std::cout << border_color << "│ " << colors::RESET;

        // 1. Bullet (2 chars) — styled by domain color & shape
        std::cout << bullet_color << "▪ " << colors::RESET; 

        // 2. Task Name (24 chars fixed width)
        std::string name_formatted = fitText(t.name, 24);
        if(first){
            std::cout << colors::TEXT << colors::UNDERLINE << name_formatted << colors::RESET;
            first = false;
        }else{
            std::cout << colors::TEXT << name_formatted << colors::RESET;
        }

        // 3. Priority Number (6 chars total: "p:0.85")
        std::ostringstream p_num;
        p_num << std::fixed << std::setprecision(2) << t.priority_current;
        
        std::cout << " "; // 1 char gap
        
        // Print "p:" part (keeping the muted style, preserving bold if high priority)
        std::cout << muted_color << "p:" << colors::RESET;
        
        // Print numeric part with conditional color
        if (t.priority_current >= 0.85) {
            std::cout << colors::RED;
        } else if (t.priority_current >= 0.50) {
            std::cout << colors::GREEN;
        } else {
            std::cout << colors::BLUE;
        }
        std::cout << p_num.str() << colors::RESET;

        // 4. Duration Number (4 chars total: e.g., " 15m" or "120m")
        std::ostringstream d_stream;
        d_stream << std::setw(3) << static_cast<int>(t.difficulty_current) << "m";

        std::cout << "   "; // 3 char gap to perfectly align the border
        if (is_quick_task) {
            std::cout << colors::MAUVE << colors::ITALIC << d_stream.str() << colors::RESET;
        } else {
            std::cout << muted_color << d_stream.str() << colors::RESET;
        }

        std::cout << border_color << " │\n" << colors::RESET;
    }

    // --- Bottom Border (exactly 44 cols) ---
    std::cout << border_color << "╰──────────────────────────────────────────╯" 
              << colors::RESET << "\n\n";
}

} // namespace ui
} // namespace xlog