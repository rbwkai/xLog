#include "xlog/db.hpp"
#include "xlog/cli.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <cstdlib>

int main(int argc, char* argv[]) {
    try {
        std::string db_path = "xlog.sqlite3";
        const char* home = std::getenv("HOME");
        if (home) {
            db_path = std::string(home) + "/.xlog.sqlite3";
        }

        xlog::Database db(db_path);
        db.initializeSchema();

        std::vector<std::string> args;
        for (int i = 1; i < argc; ++i) {
            args.push_back(argv[i]);
        }

        xlog::cli::dispatchCommand(db, args);
    } catch (const std::exception& ex) {
        std::cerr << "xLog Fatal Error: " << ex.what() << "\n";
        return 1;
    }
    return 0;
}
