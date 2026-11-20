#ifndef terminal_h
#define terminal_h

#include <memory>

class terminal {
  struct pimpl;
  std::unique_ptr<pimpl> impl;

public:
  int read_one();
  char get_symbol();

  terminal();
  ~terminal();
};

#endif
