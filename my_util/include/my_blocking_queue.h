#ifndef  MORTRED_BLOCKING_QUEUE_H
#define  MORTRED_BLOCKING_QUEUE_H

#include <deque>
#include <mutex>
#include <condition_variable>
#include <boost/noncopyable.hpp>
#include <boost/timer/timer.hpp>

namespace artanis {
namespace my_util {

template<class T>
class BlockingQueue : public boost::noncopyable {
public:
    BlockingQueue(int max_size) : max_size_(max_size) { }

    void put(const T& x);
    void take(T& x);
    bool offer(T& x, int wait_nanosecs);
    bool poll(T& x, int wait_nanosecs);

    size_t size();
    bool full();
    bool empty();
    void clear();

private:
    mutable std::mutex mutex_;
    std::condition_variable not_empty_;
    std::condition_variable not_full_;
    size_t max_size_;
    std::deque<T> queue_;
};
template<class T>
void BlockingQueue<T>::put(const T& x) {
    std::unique_lock<std::mutex> lock(mutex_);
    while(queue_.size() >= max_size_) {
        not_full_.wait(lock);
    }
    queue_.push_back(x);
    not_empty_.notify_one(); 
}

template<class T>
void BlockingQueue<T>::take(T& x) {
    std::unique_lock<std::mutex> lock(mutex_);
    while(queue_.empty()) {
        not_empty_.wait(lock);
    }
    x = queue_.front();
    queue_.pop_front();
    not_full_.notify_one();
}

template<class T>
bool BlockingQueue<T>::offer(T& x, int wait_nanosecs) {
    boost::timer::cpu_timer my_timer;
    std::unique_lock<std::mutex> lock(mutex_);
    for(;;) {
        if(!full()) {
            queue_.push_back(x);
            not_empty_.notify_one(); 
            return true;
        } else {
            not_full_.wait_for(lock, std::chrono::nanoseconds(wait_nanosecs));
        }

        if(my_timer.elapsed().wall >= wait_nanosecs) {
            return false;
        }
    }
    return false;
}

template<class T>
bool BlockingQueue<T>::poll(T& x, int wait_nanosecs) {
    boost::timer::cpu_timer my_timer;
    std::unique_lock<std::mutex> lock(mutex_);
    for(;;) {
        if(!queue_.empty()) {
            x = queue_.front();
            queue_.pop_front();
            not_full_.notify_one();
            return true;
        } else {
            not_empty_.wait_for(lock, std::chrono::nanoseconds(wait_nanosecs));
        }

        if(my_timer.elapsed().wall >= wait_nanosecs) {
            return false;
        }
    }
    return false;
}

template<class T>
size_t BlockingQueue<T>::size() {
    return queue_.size();
}

template<class T>
bool BlockingQueue<T>::full() {
    return queue_.size() >= max_size_;
}

template<class T>
bool BlockingQueue<T>::empty() {
    return queue_.empty();
}

template<class T>
void BlockingQueue<T>::clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    while (!queue_.empty()) {
        queue_.pop_front(); 
    }
}

}
}

#endif  //MORTRED_BLOCKING_QUEUE_H
