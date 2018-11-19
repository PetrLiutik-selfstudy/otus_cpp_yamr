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
    /// Тип вектора строк, содержащего результат отображения.
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
      std::ifstream ifs(filename_, std::ios::in | std::ios::binary);
      if (!ifs.is_open()) {
        throw std::invalid_argument("The file can't be opened");
      }

      ifs.seekg(0, ifs.end);
      size_t file_size = ifs.tellg();
      ifs.seekg(0, ifs.beg);

      // Предварительный расчет размера секции, для операций отображения.
      size_t sect_size = file_size / map_num_;
      if(sect_size < 1)
        sect_size = 1;
      sections_.resize(map_num_);
      sections_[0].first = 0;

      for(size_t i{1}; i < map_num_; ++i) {
        const size_t pos = sect_size * i;
        if(pos > file_size) {
          // Уменьшение количества потоков отображения при малом числе обрабатываемых строк.
          map_num_ = i + 1;
          break;
        }

        ifs.seekg(pos, std::ios::beg);

        size_t section{pos};
        for(char ch = 0; ch != '\n' && ch != '\r';) {
          ifs.read(&ch, 1);
          ++section;
        }

        sections_[i - 1].second = section;
        sections_[i].first = section + 1;
      }
      sections_[map_num_ - 1].second = file_size - 1;
      sections_.resize(map_num_); // Удаление пустых секций.

      ifs.close();
    }

    /**
     * @brief Отображение.
     */
    void map() {
      mapped_.resize(map_num_);

      start(map_num_); // Запуск mnum потоков отображения.

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
            std::stable_sort(mapped_[i].begin(), mapped_[i].end());
          }
        });
        ++i;
      }

      stop(); // Останов потоков отображения.
    }

    /**
     * @brief Перемешивание результатов отображения.
     */
    void shuffle() {
      shuffled_.resize(red_num_);
      shuffle_guard_.resize(red_num_);

      for(auto &it: shuffle_guard_)
        it = std::make_unique<std::mutex>();

      start(map_num_); // Запуск mnum потоков перемешивания.

      size_t i{};
      for(auto& str_vec: mapped_) {
        add_job([this, i, &str_vec](){
          std::hash<std::string> hash;
          for(auto& it: str_vec) {
            if(!it.empty()){
              auto j = hash(it) % red_num_; // Определение номера будещего потока свертки.

              std::lock_guard<std::mutex> lock(*shuffle_guard_[j]);
              shuffled_[j].emplace_back(std::move(it));
            }
          }
        });
        ++i;
      }

      stop(); // Останов потоков перемешивания.
    }

    /**
     * @brief Свертка.
     */
    void reduce() {
      start(red_num_); // Запуск rnum потоков свертки.

      size_t i{};
      for(const auto& it: shuffled_) {
        add_job([this, i, &it](){
          Reducer reducer;

          size_t result{};
          for(const auto& str: it)
            result = reducer(str);

          std::string filename = "result_" + std::to_string(i) + ".txt";
          std::ofstream ofs(filename);
          if(ofs.is_open()) {
            ofs << result + 1 << std::endl;
            ofs.close();
          }
        });
        ++i;
      }

      stop(); // Останов потоков свертки.
    }

    /// Имя исходного файла.
    std::string filename_{};
    /// Результат отображения.
    std::vector<str_vec_t> mapped_;
    /// Мьютексы разграничения доступа к результатам перемешивания.
    std::vector<std::unique_ptr<std::mutex>> shuffle_guard_;
    /// Результат перемешивания.
    std::vector<str_vec_t> shuffled_;
    /// Результат разбиения исходного файла (пары начало/конец).
    std::vector<section_t> sections_;
    /// Количество потоков отображения.
    size_t map_num_{};
    /// Количество потоков свертки.
    size_t red_num_{};
};

} // namespace mr.
