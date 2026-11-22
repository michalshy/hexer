#ifndef common_h
#define common_h

#include "error.h"
#include <expected>

using result = std::expected<void, AppError>;

#endif
