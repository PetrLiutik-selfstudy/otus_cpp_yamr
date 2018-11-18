#pragma once

#include <algorithm>
#include <string>
#include <vector>

namespace mr {

/**
 * @brief Функтор операции свертки.
 */
struct Reducer
{
  auto operator() (const std::string& str) {
    return 1;
  }
};

} // namespace mr.
