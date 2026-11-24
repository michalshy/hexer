#include "app.h"
#include "common.h"
#include "terminal.h"
#include <cctype>
#include <memory>
#include <print>

constexpr int ctrl_press(char c) { return c & 0x01f; }

app::app() : term(std::make_unique<terminal>()) {}

app::~app() {}

void app::run() {
  init();
  while (running) {
    term->refresh_screen();

    if (result res = read_key(); !res.has_value()) {
      std::print("application failed during read: {}",
                 static_cast<int>(res.error()));
    }

    if (!process()) {
      break;
    }
  }
  deinit();
}

result app::read_key() { return term->read_one(); }

bool app::process() {
  int symbol = term->get_symbol();
  if (symbol == ctrl_press('q')) {
    return false;
  }
  if (symbol == keys::page_down || symbol == keys::page_up)
    term->move_row(symbol);
  if (symbol == keys::home_key || symbol == keys::end_key)
    term->move_col(symbol);
  if (symbol == keys::arrow_down || symbol == keys::arrow_left ||
      symbol == keys::arrow_right || symbol == keys::arrow_up) {
    term->move_cursor(symbol);
  }
  return true;
}

void app::init() {
  if (result res = term->init(); !res) {
    std::print("application failed to initialize: {}",
               static_cast<int>(res.error()));
  }
}

void app::deinit() {
  if (result res = term->deinit(); !res) {
    std::print("application failed to deinitialize: {}",
               static_cast<int>(res.error()));
  };
}
