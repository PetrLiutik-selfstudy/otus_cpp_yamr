#pragma once

#include "ThreadPool.h"

#include <algorithm>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

namespace mr {

/**
 * @brief Шаблон класса MapReduce.
 * @tparam Mapper - функтор отображения.
 * @tparam Reducer - функтор свертки.
 */
template<typename Mapper, typename Reducer>
class MapReduce : ThreadPool {
    /// Тип, описывающий секцию для разбиения исходного файла.
    using section_t = std::pair<size_t, size_t>;
    /// Вектор строк, содержащий результат отображения.
    using str_vec_t = std::vector<std::string>;

  public:

    /**
     * @brief Конструктор.
     * @param filename - имя исходного файла.
     * @param map_num - количество потоков отображения.
     * @param red_num - количество потоков свертки.
     */
    explicit MapReduce(const std::string& filename, size_t map_num, size_t red_num) :
      filename_{filename}, map_num_{map_num}, red_num_{red_num} {
    }

    ~MapReduce() = default;


    /**
     * @brief Процесс MapReduce.
     */
    void process() {
      // Предварительное разбиение файла.
      split();

      // Отображение.
      map();

      // Перемешивание.
      shuffle();

      // Свертка.
      reduce();
    }

  private:

    /**
     * @brief Разбиение исходного файла.
     */
    void split() {
      std::ifstream ifs(filename_);
      if (!ifs.is_open()) {
        throw std::invalid_argument("The file can't be opened");
      }

      // Поиск позиций символов '\n'.
      std::string str;
      std::vector<size_t> str_ends;
      for(auto str_end = ifs.tellg(); std::getline(ifs, str) || (str_end >= 0); str_end = ifs.tellg())
        str_ends.emplace_back(str_end);

      ifs.close();

      auto strs_num = str_ends.size() - 1;
      // Уменьшение количества потоков отображения при малом числе обрабатываемых строк.
      map_num_ = std::min(map_num_, strs_num);

      auto mstrs_num = strs_num / map_num_;

      for(size_t i = 0; i < strs_num; i += mstrs_num) {
        auto beg = str_ends[i];
        auto end = (i + mstrs_num < strs_num) ? str_ends[i + mstrs_num] : str_ends.back();
        sections_.emplace_back(section_t{beg, end});
      }
    }

    /**
     * @brief Отображение.
     */
    void map() {
      mapped_.resize(map_num_);

      start(map_num_);

      size_t i{};
      for(const auto& it: sections_) {
        add_job([this, i, it](){
          std::ifstream ifs(filename_);
          if(ifs.is_open()) {
            Mapper mapper;

            auto beg = it.first;
            auto end = it.second;
            ifs.seekg(beg, std::ios::beg);

            std::string str;
            for(; std::getline(ifs, str);) {
              if(!str.empty()) {
                auto result = mapper(str);
                mapped_[i].insert(mapped_[i].end(),
                                       std::make_move_iterator(result.begin()),
                                       std::make_move_iterator(result.end()));
              }
              if(end <= ifs.tellg())
                break;
            }
            std::sort(mapped_[i].begin(), mapped_[i].end());
          }
        });
        ++i;
      }

      stop();
    }

    /**
     * @brief Перемешивание результатов отображения.
     */
    void shuffle() {
      shuffled_.resize(red_num_);

      std::vector<std::mutex> reduce_guard(red_num_);

      start(map_num_);

      size_t i{};
      for(auto& map: mapped_) {
        add_job([this, i, &map, &reduce_guard](){
          std::hash<std::string> hash;
          for(auto& it: map) {
            if(!it.empty()){
              auto j = hash(it) % red_num_;

              std::lock_guard<std::mutex> lock(reduce_guard[j]);
              shuffled_[j].emplace_back(std::move(it));
            }
          }
        });
        ++i;
      }
      stop();
    }

    /**
     * @brief Свертка.
     */
    void reduce() {
      start(red_num_);

      size_t i{};
      for(const auto& it: shuffled_) {
        add_job([this, i, &it](){
          Reducer reducer;

          size_t result{};
          for(const auto& str: it)
            result = std::max(result, reducer(str));

          std::string filename = "result_" + std::to_string(i) + ".txt";
          std::ofstream fs(filename);
          if(fs.is_open()) {
            fs << result << std::endl;
            fs.close();
          }
        });
        ++i;
      }

      stop();
    }

    /// Имя исходного файла.
    std::string filename_{};
    /// Результат отображения.
    std::vector<str_vec_t> mapped_;
    /// Результат свертки.
    std::vector<str_vec_t> shuffled_;
    /// Результат разбиения исходного файла.ы
    std::vector<section_t> sections_;
    /// Количество потоков отображения.
    size_t map_num_{};
    /// Количество потоков свертки.
    size_t red_num_{};
};

} // namespace mr.
