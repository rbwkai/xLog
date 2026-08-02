#ifndef XLOG_UI_BORED_SCREEN_HPP
#define XLOG_UI_BORED_SCREEN_HPP

#include "xlog/db.hpp"

namespace xlog {
namespace ui {

void renderBored(Database& db, int64_t user_id);

} // namespace ui
} // namespace xlog

#endif // XLOG_UI_BORED_SCREEN_HPP
