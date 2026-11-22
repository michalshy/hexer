#ifndef app_h
#define app_h

#include "common.h"
#include "terminal.h"
#include <memory>

class app {
  std::unique_ptr<terminal> term;

public:
  app();
  ~app();

  void run();

private:
  result read_key();
  bool process();

  result init();
  result deinit();
};

#endif
