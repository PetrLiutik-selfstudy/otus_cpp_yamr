#pragma once

#include <array>
#include <atomic>
#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <thread>

namespace mr {

/**
 * @brief Шаблон пула потоков.
 * @tparam N - количество потоков пула.
 */
template <size_t N = 1>
class ThreadPool {
  public:
    explicit ThreadPool() = default;

    ThreadPool(const ThreadPool&) = delete;
    ThreadPool(ThreadPool&&) = delete;

    ~ThreadPool() = default;

    ThreadPool& operator = (const ThreadPool&) = delete;
    ThreadPool& operator = (ThreadPool&&) = delete;

    /**
     * @brief Запуск всех потоков пула.
     */
    void start() {
      for(auto i = 0; i < N; ++i) {
        threads_[i] = std::thread([this] {
          for(;;) {
            std::function<void()> task;
            {
              std::unique_lock<std::mutex> lock(job_mutex_);
              job_avail_.wait(lock, [this]{ return stop_ || !jobs_.empty(); });
              if(stop_ && jobs_.empty())
                return;
              task = std::move(jobs_.front());
              jobs_.pop();
              job_id_++;
            }
            task();
          }
        });
      }
    }

    /**
     * @brief Завершение работы всех потоков пула.
     */
    void stop() {
      {
        std::unique_lock<std::mutex> lock(job_mutex_);
        stop_ = true;
      }
      job_avail_.notify_all();
      for(auto i = 0; i < N; ++i) {
        if(threads_[i].joinable())
          threads_[i].join();
      }
    }

    /**
     * @brief Добавление задачи для пула потоков.
     * @tparam F - тип задачи.
     * @tparam Args - типы аргументов задачи.
     * @param f - задача.
     * @tparam args - аргументы задачи.
     * @return результат выполнения задачи.
     */
    template<typename F, typename...Args>
    auto add_job(F&& f, Args&&... args) -> std::future<decltype(f(args...))> {
      std::unique_lock<std::mutex> lock(job_mutex_);

      std::function<decltype(f(args...))()> func = std::bind(std::forward<F>(f),
                                                             std::forward<Args>(args)...);

      auto func_ptr = std::make_shared<std::packaged_task<decltype(f(args...))()>>(func);
      std::function<void()> wrappered_func = [func_ptr]() { (*func_ptr)(); };
      jobs_.push(wrappered_func);

      job_avail_.notify_one();

      return func_ptr->get_future();
    }

    /**
     * @brief Дать id текущей выполняемой задачи.
     * @return id текущей выполняемой задачи.
     */
    unsigned char get_job_id() {
      return job_id_;
    }

  private:

    std::atomic_bool                  stop_{};
    std::atomic_uchar                 job_id_{};
    std::mutex                        job_mutex_{};
    std::condition_variable           job_avail_{};
    std::queue<std::function<void()>> jobs_{};
    std::array<std::thread, N>        threads_{};
};

} // namespace mr.

