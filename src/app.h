#ifndef app_h
#define app_h

#include "terminal.h"
#include <memory>

class app {
  std::unique_ptr<terminal> term;

public:
  app();
  ~app();

  void run();
};

#endif
