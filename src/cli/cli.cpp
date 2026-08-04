#include "xlog/cli.hpp"
#include "xlog/ui.hpp"
#include "xlog/task_engine.hpp"
#include <iostream>

namespace xlog {
namespace cli {

void dispatchCommand(Database& db, const std::vector<std::string>& args) {
    auto u_opt = db.getUser();
    if (!u_opt && (args.empty() || args[0] != "setup")) {
        runOnboarding(db);
        u_opt = db.getUser();
        if (!u_opt) return;
    }

    User u = u_opt.value();
    std::string logical_today = TaskEngine::getLogicalDate(u.day_boundary_hour);
    db.checkAndRunDailyRollover(u, logical_today);

    std::string cmd = args.empty() ? "prompt" : matchFuzzyCommand(args[0]);

    if (cmd == "prompt") {
        ui::renderPrompt(db, u.id);
    } else if (cmd == "today") {
        ui::renderToday(db, u.id);
    } else if (cmd == "add") {
        runAddTask(db, u.id, args);
    } else if (cmd == "done") {
        runDoneTask(db, u.id, args);
    } else if (cmd == "edit") {
        runEditTask(db, u.id, args);
    } else if (cmd == "pause") {
        runPauseTask(db, u.id, args);
    } else if (cmd == "tui") {
        runTuiCommand(db, u.id);
    } else if (cmd == "profile") {
        ui::renderProfile(db, u.id);
    } else if (cmd == "bored") {
        ui::renderBored(db, u.id);
    } else if (cmd == "quick") {
        ui::renderQuick(db, u.id);
    } else if (cmd == "why") {
        ui::renderWhy();
    } else if (cmd == "quote") {
        runQuoteCommand(db, u.id, args);
    } else if (cmd == "setup") {
        runOnboarding(db);
    } else {
        std::cout << "xLog Commands:\n"
                  << "  xlog prompt   - fastfetch style status & quote\n"
                  << "  xlog today    - view daily task list\n"
                  << "  xlog add      - create a new task (or xlog add <name>)\n"
                  << "  xlog done     - complete a task and earn XP\n"
                  << "  xlog edit     - edit task properties\n"
                  << "  xlog pause    - pause or resume a task\n"
                  << "  xlog tui      - interactive menu for all functions & screens\n"
                  << "  xlog profile  - detailed rank, domain progress & heatmap\n"
                  << "  xlog bored    - view hobby tasks\n"
                  << "  xlog quick    - view quick tasks (<15 min)\n"
                  << "  xlog why      - system math & mechanics overview\n"
                  << "  xlog quote    - display or manage motivational quotes\n";
    }
}

} // namespace cli
} // namespace xlog
