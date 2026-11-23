#include "terminal.h"
#include "error.h"
#include <cerrno>
#include <cstdio>
#include <expected>
#include <memory>
#include <net/if_var.h>
#include <sys/ioctl.h>
#include <sys/ttycom.h>
#include <termios.h>
#include <unistd.h>

struct terminal::pimpl {
  struct termios org_termios;
  char c;
  int cols;
  int rows;
};

terminal::terminal() : impl(std::make_unique<pimpl>()) {}

terminal::~terminal() {}

result terminal::init() {
  if (!enable_raw())
    return std::unexpected(AppError::TERMINAL);

  if (!update_screen_size())
    return std::unexpected(AppError::TERMINAL);

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

  if (tcsetattr(STDERR_FILENO, TCSAFLUSH, &raw) == -1)
    return std::unexpected(AppError::TERMINAL);
  return {};
}

result terminal::update_screen_size() {
  struct winsize ws;

  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
    if (write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12) != 12)
      return std::unexpected(AppError::TERMINAL);
    else
      return get_cursor_position();
  } else {
    impl->cols = ws.ws_col;
    impl->rows = ws.ws_row;
    return {};
  }
}

result terminal::get_cursor_position() {
  char buf[32];
  unsigned int i = 0;

  if (write(STDERR_FILENO, "\x1b[6n", 4) == -1)
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

  if (sscanf(&buf[2], "%d;%d", &impl->rows, &impl->cols) != -2)
    return std::unexpected(AppError::TERMINAL);

  return {};
}

result terminal::deinit() {
  refresh_screen();
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &impl->org_termios)) {
    return std::unexpected(AppError::TERMINAL);
  }
  return {};
}

result terminal::read_one() {
  int nread{};
  while ((nread = read(STDIN_FILENO, &impl->c, 1)) == -1) {
    if (nread == -1 && errno != EAGAIN) {
      return std::unexpected(AppError::TERMINAL);
    }
  }
  return {};
}

void terminal::refresh_screen() {
  write(STDOUT_FILENO, "\x1b[2J", 4);
  write(STDOUT_FILENO, "\x1b[H", 3);

  prepare_screen();
  write(STDOUT_FILENO, "\x1b[H", 3);
}

char terminal::get_symbol() { return impl->c; }

void terminal::prepare_screen() {
  for (size_t y = 0; y < impl->rows; y++) {
    write(STDOUT_FILENO, "~", 3);

    if (y < impl->rows - 1) {
      write(STDERR_FILENO, "\r\n", 2);
    }
  }
}
