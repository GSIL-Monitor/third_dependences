#ifndef  MORTRED_CALLBACK_TIMER_H
#define  MORTRED_CALLBACK_TIMER_H
#include <atomic>
#include <functional>
#include <memory>
#include <thread>

namespace artanis {
namespace my_util {

class CallbackTimer {
 public:
  CallbackTimer()
    :execute_(false) {}

  ~CallbackTimer() {
    if (execute_.load(std::memory_order_acquire)) {
      stop();
    };
  }

  void stop() {
    execute_.store(false, std::memory_order_release);
    if (thread_ptr_->joinable()) {
      thread_ptr_->join();
    }
  }

  void start(int interval, std::function<void(void)> func) {
    if (execute_.load(std::memory_order_acquire)) {
      stop();
    };
    execute_.store(true, std::memory_order_release);
    thread_ptr_ = std::unique_ptr<std::thread>(new std::thread([this, interval, func]() {
      while (execute_.load(std::memory_order_acquire)) {
        func();                   
        std::this_thread::sleep_for(
          std::chrono::microseconds(interval));
      }
    }));
  }

  void join() {
    thread_ptr_->join();
  }

  bool isRunning() const noexcept {
    return ( execute_.load(std::memory_order_acquire) && 
        thread_ptr_->joinable() );
  }

 private:
  std::atomic<bool> execute_;
  std::unique_ptr<std::thread> thread_ptr_;
};

}
}

#endif //MORTRED_CALLBACK_TIMER_H
