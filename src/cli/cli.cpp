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
    } else if (cmd == "help") {
        runHelpCommand();
    } else {
        runHelpCommand();
    }
}

} // namespace cli
} // namespace xlog
