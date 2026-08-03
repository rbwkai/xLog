#ifndef XLOG_UI_COLORS_HPP
#define XLOG_UI_COLORS_HPP

#include <string>

namespace xlog {
namespace ui {
namespace colors {

const std::string RESET     = "\033[0m";
const std::string BOLD      = "\033[1m";
const std::string DIM       = "\033[2m";
const std::string ITALIC    = "\033[3m";

const std::string FLAMINGO  = "\033[38;2;242;205;205m";
const std::string PINK      = "\033[38;2;245;194;231m";
const std::string MAUVE     = "\033[38;2;203;166;247m";
const std::string RED       = "\033[38;2;243;139;168m";
const std::string MAROON    = "\033[38;2;235;160;172m";
const std::string PEACH     = "\033[38;2;250;179;135m";
const std::string YELLOW    = "\033[38;2;249;226;175m";
const std::string GREEN     = "\033[38;2;166;227;161m";
const std::string TEAL      = "\033[38;2;148;226;213m";
const std::string SKY       = "\033[38;2;137;220;235m";
const std::string SAPPHIRE  = "\033[38;2;116;199;236m";
const std::string BLUE      = "\033[38;2;137;180;250m";
const std::string LAVENDER  = "\033[38;2;180;190;254m";
const std::string TEXT      = "\033[38;2;205;214;244m";
const std::string SUBTEXT   = "\033[38;2;186;194;222m";
const std::string OVERLAY2  = "\033[38;2;147;153;178m";
const std::string OVERLAY1  = "\033[38;2;127;132;156m";
const std::string OVERLAY0  = "\033[38;2;108;112;134m"; 

} // namespace colors

inline std::string colorText(const std::string& text, const std::string& ansi_color) {
    return ansi_color + text + colors::RESET;
}

inline std::string bold(const std::string& text) {
    return colors::BOLD + text + colors::RESET;
}

inline std::string italic(const std::string& text) {
    return colors::ITALIC + text + colors::RESET;
}

inline std::string dim(const std::string& text) {
    return colors::DIM + text + colors::RESET;
}

inline std::string getRankColor(const std::string& rank) {
    if (rank == "Green") return colors::GREEN;
    if (rank == "Cyan") return colors::TEAL;
    if (rank == "Blue") return colors::BLUE;
    if (rank == "Violet") return colors::MAUVE;
    if (rank == "Orange") return colors::PEACH;
    if (rank == "Red") return colors::RED;
    return colors::OVERLAY1;
}

inline std::string getDomainColor(int order_index) {
    switch (order_index % 4) {
        case 0: return colors::FLAMINGO;
        case 1: return colors::SKY;
        case 2: return colors::LAVENDER;
        case 3: return colors::PINK;
    }
    return colors::TEXT;
}

} // namespace ui
} // namespace xlog

#endif // XLOG_UI_COLORS_HPP
