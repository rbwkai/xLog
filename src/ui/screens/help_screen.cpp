#include "xlog/ui/screens/help_screen.hpp"
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

void renderHelp() {
    const std::string border_color = colors::LAVENDER;
    const std::string res          = colors::RESET;
    const std::string bld          = colors::BOLD;
    const std::string dim          = colors::OVERLAY2;

    std::cout << "\n" << border_color << "╭── Command Reference ─────────────────────╮" << res << "\n";

    struct CmdInfo {
        std::string cmd;
        std::string desc;
    };

    std::vector<CmdInfo> cmds = {
        {"xlog prompt",   "Status & random quote"},
        {"xlog today",    "View daily task list"},
        {"xlog add",      "Create new task (interactive/fast)"},
        {"xlog done",     "Complete task & earn XP"},
        {"xlog edit",     "Edit task details & priority"},
        {"xlog pause",    "Pause or resume a task"},
        {"xlog tui",      "Interactive prompt-based TUI"},
        {"xlog profile",  "Ranks, domains & heatmap"},
        {"xlog bored",    "View hobby tasks"},
        {"xlog quick",    "View quick tasks (<15 min)"},
        {"xlog why",      "Engine mechanics & math"},
        {"xlog quote",    "View/add motivational quotes"},
        {"xlog setup",    "Re-run initial onboarding"},
        {"xlog help",     "Display command reference"}
    };

    for (const auto& c : cmds) {
        std::cout << border_color << "│ " << res
                  << colors::FLAMINGO << bld << fitText(c.cmd, 16) << res
                  << dim << fitText(c.desc, 24) << res
                  << border_color << " │\n" << res;
    }

    std::cout << border_color << "╰──────────────────────────────────────────╯" << res << "\n\n";
}

} // namespace ui
} // namespace xlog
