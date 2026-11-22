#include "terminal.h"
#include "error.h"
#include <cerrno>
#include <expected>
#include <memory>
#include <termios.h>
#include <unistd.h>

struct terminal::pimpl {
  struct termios org_termios;
  char c;
};

terminal::terminal() : impl(std::make_unique<pimpl>()) {}

terminal::~terminal() {}

result terminal::init() {
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

result terminal::deinit() {
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

void terminal::refresh_screen() { write(STDOUT_FILENO, "\x1b[2J", 4); }

char terminal::get_symbol() { return impl->c; }
