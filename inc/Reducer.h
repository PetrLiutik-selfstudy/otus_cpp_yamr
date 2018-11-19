#pragma once

#include <string>
#include <unordered_map>

namespace mr {

/**
 * @brief Функтор операции свертки.
 */
struct Reducer
{
  auto operator() (const std::string& str) {
    if(++freq_[str] > 1) // Строка встречается более одного раза, необходимо проверять префиксы.
      if(str.length() > max_pref_) // Длина строки больше максимальной длины префикса.
        max_pref_ = str.length();
    return max_pref_;
  }

private:
  /// Максимальная длина префикса.
  size_t max_pref_{};
  /// Частотоность появления строк.
  std::unordered_map<std::string, size_t> freq_{};
};

} // namespace mr.
