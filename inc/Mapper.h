#pragma once

#include <algorithm>
#include <string>
#include <vector>

namespace mr {

/**
 * @brief Функтор операции отображения.
 */
struct Mapper
{
  auto operator()(const std::string& str) {
    std::vector<std::string> map;

    return map;
  }
};

} // namespace mr.
