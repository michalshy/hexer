#include "app.h"
#include "terminal.h"
#include <cctype>
#include <memory>
#include <print>

constexpr int ctrl_press(char c) { return c & 0x01f; }

app::app() : term(std::make_unique<terminal>()) {}

app::~app() {}

void app::run() {
  if (result res = init(); !res.has_value()) {
    std::print("application failed to initialize: {}",
               static_cast<int>(res.error()));
    return;
  }

  while (true) {
    term->refresh_screen();

    if (result res = read_key(); !res.has_value()) {
      std::print("application failed during read: {}",
                 static_cast<int>(res.error()));
    }

    if (!process()) {
      break;
    }
  }

  if (result res = deinit(); !res.has_value()) {
    std::print("application failed to deinitialize: {}",
               static_cast<int>(res.error()));
  }
}

result app::read_key() { return term->read_one(); }

bool app::process() {
  char symbol = term->get_symbol();
  if (symbol == ctrl_press('q')) {
    return false;
  }
  return true;
}

result app::init() { return term->init(); }

result app::deinit() { return term->deinit(); }
