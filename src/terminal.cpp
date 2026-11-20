#include "terminal.h"
#include <memory>
#include <termios.h>
#include <unistd.h>

struct terminal::pimpl {
  struct termios org_termios;
  char c;
};

terminal::terminal() : impl(std::make_unique<pimpl>()) {
  tcgetattr(STDIN_FILENO, &impl->org_termios);

  struct termios raw = impl->org_termios;

  raw.c_lflag &= ~(ECHO | ICANON);

  tcsetattr(STDERR_FILENO, TCSAFLUSH, &raw);
}

terminal::~terminal() {
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &impl->org_termios);
}

int terminal::read_one() { return read(STDIN_FILENO, &impl->c, 1); }

char terminal::get_symbol() { return impl->c; }
