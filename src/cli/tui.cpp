#include "xlog/cli/tui.hpp"
#include "xlog/cli/task.hpp"
#include "xlog/cli/quote.hpp"
#include "xlog/ui/screens.hpp"
#include "xlog/ui/colors.hpp"
#include <iostream>
#include <string>

namespace xlog {
namespace cli {

void runTuiCommand(Database& db, int64_t user_id) {
    while (true) {
        ui::renderTuiMenu(db, user_id);
        std::cout << ui::colorText("Select Option (0-11): ", ui::colors::LAVENDER + ui::colors::BOLD);

        std::string choice;
        if (!std::getline(std::cin, choice)) {
            break;
        }

        if (choice == "0" || choice == "q" || choice == "exit" || choice == "quit") {
            std::cout << ui::colorText("Exiting xLog TUI.\n\n", ui::colors::OVERLAY2);
            break;
        }

        if (choice == "1") {
            ui::renderToday(db, user_id);
        } else if (choice == "2") {
            ui::renderProfile(db, user_id);
        } else if (choice == "3") {
            ui::renderPrompt(db, user_id);
        } else if (choice == "4") {
            ui::renderQuick(db, user_id);
        } else if (choice == "5") {
            ui::renderBored(db, user_id);
        } else if (choice == "6") {
            ui::renderWhy();
        } else if (choice == "7") {
            ui::renderQuote(db, user_id);
        } else if (choice == "8") {
            runAddTask(db, user_id);
        } else if (choice == "9") {
            runDoneTask(db, user_id, {});
        } else if (choice == "10") {
            runEditTask(db, user_id, {});
        } else if (choice == "11") {
            runPauseTask(db, user_id, {});
        } else {
            std::cout << ui::colorText("Invalid option. Try again.\n", ui::colors::RED);
            continue;
        }

        std::cout << ui::colorText("\nPress ENTER to return to menu...", ui::colors::OVERLAY2);
        std::string wait_enter;
        std::getline(std::cin, wait_enter);
    }
}

} // namespace cli
} // namespace xlog
