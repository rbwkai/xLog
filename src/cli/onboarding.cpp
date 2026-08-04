#include "xlog/cli/onboarding.hpp"
#include "xlog/ui/colors.hpp"
#include <iostream>
#include <vector>
#include <string>

namespace xlog {
namespace cli {

void runOnboarding(Database& db) {
    std::cout << ui::colorText("\n⚡ Welcome to xLog Gamification Engine Setup\n", ui::colors::LAVENDER + ui::colors::BOLD);
    std::cout << ui::colorText("Enter your username: ", ui::colors::TEXT);
    std::string username;
    std::getline(std::cin, username);
    if (username.empty()) username = "Hero";

    std::cout << ui::colorText("\nxLog structures life into 4 Domains, each with 4 Subdomains.\n", ui::colors::SUBTEXT);

    std::vector<std::pair<std::string, std::vector<std::string>>> domains;
    std::vector<std::string> default_dom_names = {"Faith", "Intellect", "Physique", "Artistry"};
    std::vector<std::vector<std::string>> default_subs = {
        {"Qur'an", "Arabic", "Dhikr", "Discipline"},
        {"Heuristics", "Theory", "Exploration", "Craft"},
        {"Combat", "Strength", "Agility", "Aesthetics"},
        {"Sketching", "Articulation", "Versatility", "Character"}
    };

    for (int i = 0; i < 4; ++i) {
        std::cout << ui::colorText("\nDomain " + std::to_string(i + 1) + " name [" + default_dom_names[i] + "]: ", ui::colors::TEXT);
        std::string dom_input;
        std::getline(std::cin, dom_input);
        std::string dom_name = dom_input.empty() ? default_dom_names[i] : dom_input;

        std::vector<std::string> subs;
        for (int j = 0; j < 4; ++j) {
            std::cout << "  Subdomain " + std::to_string(j + 1) + " [" + default_subs[i][j] + "]: ";
            std::string sub_input;
            std::getline(std::cin, sub_input);
            subs.push_back(sub_input.empty() ? default_subs[i][j] : sub_input);
        }
        domains.push_back({dom_name, subs});
    }

    User u = db.onboardUser(username, domains);
    std::cout << ui::colorText("\n✓ Onboarding complete! Account created for " + u.username + ".\n\n", ui::colors::GREEN + ui::colors::BOLD);
}

} // namespace cli
} // namespace xlog
