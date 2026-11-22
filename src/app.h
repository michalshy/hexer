#ifndef app_h
#define app_h

#include "terminal.h"
#include <memory>

class app {
  std::unique_ptr<terminal> term;
  bool running{true};

public:
  app();
  ~app();

  void run();

private:
  result read_key();
  bool process();

  void init();
  void deinit();
};

#endif
