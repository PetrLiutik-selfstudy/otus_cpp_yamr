#pragma once

#include <string>
#include <vector>

namespace mr {

/**
 * @brief Функтор операции отображения.
 */
class Mapper
{
public:
  auto operator()(const std::string& str) {
    // Составление списка всех строк входящих в данную (от начала строки).
    std::vector<std::string> result;
    auto size = str.size();
    for(size_t i = 1; i <= size; ++i)
      result.emplace_back(str.substr(0, i));
    return result;
  }
};

} // namespace mr.
