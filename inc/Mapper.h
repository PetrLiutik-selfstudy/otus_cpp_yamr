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
    std::vector<std::string> result;
    auto size = str.size();
    for(size_t i = 1; i <= size; ++i)
      result.emplace_back(str.substr(0, i));
    return result;
  }
};

} // namespace mr.
