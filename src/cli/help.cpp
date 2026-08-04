#include "xlog/cli/help.hpp"
#include "xlog/ui/screens/help_screen.hpp"

namespace xlog {
namespace cli {

void runHelpCommand() {
    ui::renderHelp();
}

} // namespace cli
} // namespace xlog
