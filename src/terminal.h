#ifndef terminal_h
#define terminal_h

#include "common.h"
#include <memory>

class terminal {
  struct pimpl;
  std::unique_ptr<pimpl> impl;

public:
  terminal();
  ~terminal();

  result init();
  result deinit();

  result read_one();
  void refresh_screen();
  char get_symbol();

private:
  void prepare_screen();
};

#endif
