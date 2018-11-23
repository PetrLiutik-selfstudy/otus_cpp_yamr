#pragma once

#include <string>
#include <map>

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

  bool check_keys() {
    if(freq_.cbegin() != freq_.cend()) {
      auto it = freq_.cbegin();
      auto key = it->first;
      it++;

      for(; it != freq_.cend(); ++it) {
        if(key != it->first.substr(0, key.size()))
          return false;
        key = it->first;
      }
      return true;
    }
    return false;
  }

private:
  /// Максимальная длина префикса.
  size_t max_pref_{};
  /// Частотоность появления строк.
  std::map<std::string, size_t> freq_{};
};

} // namespace mr.
