#ifndef common_h
#define common_h

#include "error.h"
#include <expected>
#include <format>
#include <string>

using result = std::expected<void, AppError>;

namespace settings {
const std::string version = "0.0.1";
}

namespace keys {
enum : int {
  arrow_left = 1000,
  arrow_right,
  arrow_up,
  arrow_down,
  del_key,
  home_key,
  end_key,
  page_up,
  page_down
};
}

namespace commands {
const std::string move_max_right_down = "\x1b[999C\x1b[999B";
const std::string device_status_report = "\x1b[6n";
const std::string reset_cursor = "\x1b[H";
const std::string clear_display = "\x1b[2J";
const std::string clear_line = "\x1b[K";
const std::string hide_cursor = "\x1b[?25l";
const std::string show_cursor = "\x1b[?25h";

constexpr std::string move_cursor(int x, int y) {
  return std::format("\x1b[{:d};{:d}H", y, x);
}
} // namespace commands

namespace actions {
const std::string new_line_return_caret = "\r\n";
}

#endif
