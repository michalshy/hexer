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
  int get_symbol();

  void move_cursor(int c);
  void move_row(int type);
  void move_col(int type);

private:
  result enable_raw();
  result update_screen_size();
  void init_buffer();
  result get_cursor_position();

  void prepare_screen();
};

#endif
