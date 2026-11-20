#include "app.h"
#include "terminal.h"
#include <memory>

app::app() : term(std::make_unique<terminal>()) {}

app::~app() {}

void app::run() {
  while (true) {
    if (term->read_one() == 1) {
      break;
    }
    if (term->get_symbol() == 'q') {
      break;
    }
  }
}
