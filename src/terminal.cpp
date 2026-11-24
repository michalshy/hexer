#include "terminal.h"
#include "common.h"
#include "error.h"
#include <cerrno>
#include <cstdio>
#include <expected>
#include <format>
#include <memory>
#include <net/if_var.h>
#include <string>
#include <string_view>
#include <sys/ioctl.h>
#include <sys/ttycom.h>
#include <termios.h>
#include <unistd.h>
#include <vector>

struct cursor {
  int x{0};
  int y{0};

  void validate(int max_x, int max_y) {
    if (x < 0)
      x = 0;
    if (y < 0)
      y = 0;
    if (x > max_x)
      x = max_x;
    if (y > max_y)
      y = max_y;
  }
};

struct window {
  int cols{0};
  int rows{0};
};

struct terminal::pimpl {
  int symbol;
  struct termios org_termios;
  window window;
  cursor cursor;
  std::vector<char> buff;

  void append(std::string_view s) {
    for (const auto &symbol : s) {
      buff.push_back(symbol);
    }
  }
};

terminal::terminal() : impl(std::make_unique<pimpl>()) {}

terminal::~terminal() {}

result terminal::init() {
  if (!enable_raw())
    return std::unexpected(AppError::TERMINAL);

  if (!update_screen_size())
    return std::unexpected(AppError::TERMINAL);

  init_buffer();
  return {};
}

result terminal::enable_raw() {
  if (tcgetattr(STDIN_FILENO, &impl->org_termios) == -1)
    return std::unexpected(AppError::TERMINAL);

  struct termios raw = impl->org_termios;

  raw.c_iflag &= ~(BRKINT | INPCK | ISTRIP | IXON | ICRNL);
  raw.c_oflag &= ~(OPOST);
  raw.c_cflag |= (CS8);
  raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
  raw.c_cc[VMIN] = 0;
  raw.c_cc[VTIME] = 1;

  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
    return std::unexpected(AppError::TERMINAL);
  return {};
}

result terminal::update_screen_size() {
  struct winsize ws;

  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
    if (write(STDOUT_FILENO, commands::move_max_right_down.data(),
              commands::move_max_right_down.size()) != 12)
      return std::unexpected(AppError::TERMINAL);
    else
      return get_cursor_position();
  } else {
    impl->window.cols = ws.ws_col;
    impl->window.rows = ws.ws_row;
    return {};
  }
}

result terminal::get_cursor_position() {
  char buf[32];
  unsigned int i = 0;

  if (write(STDERR_FILENO, commands::device_status_report.data(),
            commands::device_status_report.size()) == -1)
    return std::unexpected(AppError::TERMINAL);

  while (i < sizeof(buf) - 1) {
    if (read(STDIN_FILENO, &buf[i], 1) != 1)
      break;
    if (buf[i] == 'R')
      break;
    i++;
  }
  buf[i] = '\0';

  if (buf[0] != '\x1b' || buf[1] != '[')
    return std::unexpected(AppError::TERMINAL);

  if (sscanf(&buf[2], "%d;%d", &impl->window.rows, &impl->window.cols) != 2)
    return std::unexpected(AppError::TERMINAL);

  return {};
}

void terminal::init_buffer() {
  const int screen_size = impl->window.cols * impl->window.rows;
  const int new_lines = 2 * impl->window.rows;
  impl->buff.reserve(screen_size + new_lines);
}

result terminal::deinit() {
  impl->append(commands::reset_cursor);
  refresh_screen();
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &impl->org_termios)) {
    return std::unexpected(AppError::TERMINAL);
  }
  return {};
}

result terminal::read_one() {
  int nread{};
  char c;
  while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
    if (nread == -1 && errno != EAGAIN) {
      return std::unexpected(AppError::TERMINAL);
    }
  }

  impl->symbol = c;

  if (impl->symbol == '\x1b') {
    char seq[3];

    if (read(STDIN_FILENO, &seq[0], 1) != 1)
      return {};
    if (read(STDIN_FILENO, &seq[1], 1) != 1)
      return {};

    if (seq[0] == '[') {
      if (seq[1] >= '0' && seq[1] <= '9') {
        if (read(STDIN_FILENO, &seq[2], 1) != 1)
          return {};
        if (seq[2] == '~') {
          switch (seq[1]) {
          case '1':
            impl->symbol = keys::home_key;
            break;
          case '3':
            impl->symbol = keys::del_key;
            break;
          case '4':
            impl->symbol = keys::end_key;
            break;
          case '5':
            impl->symbol = keys::page_up;
            break;
          case '6':
            impl->symbol = keys::page_down;
            break;
          case '7':
            impl->symbol = keys::home_key;
            break;
          case '8':
            impl->symbol = keys::end_key;
            break;
          }
        }
      } else {
        switch (seq[1]) {
        case 'A':
          impl->symbol = keys::arrow_up;
          break;
        case 'B':
          impl->symbol = keys::arrow_down;
          break;
        case 'C':
          impl->symbol = keys::arrow_right;
          break;
        case 'D':
          impl->symbol = keys::arrow_left;
          break;
        case 'H':
          impl->symbol = keys::home_key;
          break;
        case 'F':
          impl->symbol = keys::end_key;
          break;
        }
      }
    } else if (seq[0] == '0') {
      switch (seq[1]) {
      case 'H':
        impl->symbol = keys::home_key;
        break;
      case 'F':
        impl->symbol = keys::end_key;
        break;
      }
    }
  }
  return {};
}

void terminal::refresh_screen() {
  impl->append(commands::hide_cursor);
  impl->append(commands::reset_cursor);

  prepare_screen();
  impl->append(commands::move_cursor(impl->cursor.x + 1, impl->cursor.y + 1));
  impl->append(commands::show_cursor);

  write(STDOUT_FILENO, impl->buff.data(), impl->buff.size());
  impl->buff.clear();
}

int terminal::get_symbol() { return impl->symbol; }

void terminal::prepare_screen() {
  for (size_t y = 0; y < impl->window.rows; y++) {
    if (y == impl->window.rows / 3) {
      std::string message =
          std::format("hexer editor -- version {}", settings::version);
      std::string empty;
      empty.resize(impl->window.cols / 2 - message.size() / 2, ' ');
      impl->append(empty);
      impl->append(message);
    } else {
      impl->append("~");
    }

    impl->append(commands::clear_line);
    if (y < impl->window.rows - 1) {
      impl->append(actions::new_line_return_caret);
    }
  }
}

void terminal::move_cursor(int c) {
  switch (c) {
  case keys::arrow_left:
    impl->cursor.x--;
    break;
  case keys::arrow_right:
    impl->cursor.x++;
    break;
  case keys::arrow_up:
    impl->cursor.y--;
    break;
  case keys::arrow_down:
    impl->cursor.y++;
    break;
  }

  impl->cursor.validate(impl->window.cols - 1, impl->window.rows - 1);
}

void terminal::move_row(int type) {
  int times = impl->window.rows;
  while (times--)
    move_cursor(type == keys::page_up ? keys::arrow_up : keys::arrow_down);
}

void terminal::move_col(int type) {
  if (type == keys::home_key)
    impl->cursor.x = 0;
  if (type == keys::end_key)
    impl->cursor.x = impl->window.cols - 1;
}
