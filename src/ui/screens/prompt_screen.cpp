#include "xlog/ui/screens/prompt_screen.hpp"
#include "xlog/ui/colors.hpp"
#include "xlog/ui/widgets.hpp"
#include "xlog/db.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <optional>

namespace xlog {
namespace ui {

void renderPrompt(Database& db, int64_t user_id) {
    auto u_opt = db.getUser();
    if (!u_opt) return;
    User u = *u_opt;

    const std::string border_col = colors::LAVENDER;
    const std::string res        = colors::RESET;
    const std::string bld        = colors::BOLD;
    const std::string itl        = colors::ITALIC;
    const std::string rc         = getRankColor(u.rank_current);

    const std::vector<std::string> dom_cols = {
        colors::FLAMINGO,
        colors::SKY,
        colors::LAVENDER,
        colors::PINK
    };

    // --- Top Border (exactly 44 chars total width) ---
    std::cout << border_col << "╭── Prompt ────────────────────────────────╮\n" << res;

    // 1. Rank color-coded username
    std::string user_str = rc + bld + itl + u.username + res;
    int user_vis_len = static_cast<int>(u.username.length());

    // 2. Highest priority task from today's list
    auto tasks = db.getTodayTasks(user_id);

    std::string task_part = "";
    int task_vis_len = 0;

    if (!tasks.empty()) {
        const auto& first_task = tasks[0];

        // Resolve domain color from subdomain
        int64_t domain_id = 0;
        if (first_task.major_subdomain_id > 0) {
            std::optional<Subdomain> sub = db.getSubdomainById(first_task.major_subdomain_id);
            if (sub.has_value()) {
                domain_id = sub->domain_id;
            }
        }
        std::string bullet_color = dom_cols[domain_id % dom_cols.size()];

        // Calculate available visible width for task name (40 max inner width)
        // Format structure: " [" + "▪ " + task_name + "]" -> 5 fixed visible chars
        int fixed_extra_vis = 1 + 1 + 2 + 1; // " [" (2) + "▪ " (2) + "]" (1)
        int max_task_name_len = 40 - user_vis_len - fixed_extra_vis;
        if (max_task_name_len < 1) max_task_name_len = 1;

        std::string task_name = first_task.name;
        if (task_name.length() > static_cast<size_t>(max_task_name_len)) {
            if (max_task_name_len > 3) {
                task_name = task_name.substr(0, max_task_name_len - 3) + "...";
            } else {
                task_name = task_name.substr(0, max_task_name_len);
            }
        }

        task_part = " [" + bullet_color + "▪ " + res + colors::TEXT + task_name + res + "]";
        task_vis_len = 1 + 1 + 2 + static_cast<int>(task_name.length()) + 1;
    } else {
        std::string no_task_msg = " [no tasks]";
        task_part = colors::OVERLAY2 + no_task_msg + res;
        task_vis_len = static_cast<int>(no_task_msg.length());
    }

    // --- Content Row (40 visible chars padded) ---
    int total_vis = user_vis_len + task_vis_len;
    int pad = 40 - total_vis;
    if (pad < 0) pad = 0;

    std::cout << border_col << "│ " << res
              << user_str << task_part
              << std::string(pad, ' ')
              << border_col << " │\n" << res;

    // --- Bottom Border (exactly 44 chars total width) ---
    std::cout << border_col << "╰──────────────────────────────────────────╯\n" << res;
}

} // namespace ui
} // namespace xlog