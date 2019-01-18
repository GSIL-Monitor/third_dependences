#ifndef MORTRED_MY_RING_BUFFER
#define MORTRED_MY_RING_BUFFER

#include <atomic>
#include <exception>
#include <memory>

namespace artanis {
namespace my_util {

class MyRingBufferException : public std::exception {
public:
    MyRingBufferException(const std::string& what_str)
        :err_msg_(what_str) {}
    virtual ~MyRingBufferException() throw() {}
    virtual const char* what() {
        return err_msg_.c_str();
    }   
protected:
    std::string err_msg_;
};

template<typename T>
class MyRingBuffer {
 public:
  MyRingBuffer(size_t capacity)
    :buf_(new T[capacity]),
    capacity_(capacity),
    index_mask_(capacity - 1),
    head_cursor_(0),
    head_next_(1),
    tail_cursor_(0),
    tail_next_(1) {
    if ((capacity&capacity-1)) {
      throw MyRingBufferException("size must be power of 2");
    }
  }
  void put(const T& t) {
    while(!inner_put(t));
  }
  void take(T& t) {
    while(!inner_take(t));
  }
  //bool offer(T& x, int wait_nanosecs);
  //bool poll(T& x, int wait_nanosecs);
  size_t size() const {
    return head_next_ - tail_cursor_;
  }
  bool full() const {
    return head_next_ - tail_cursor_ > index_mask_;
  }
  bool empty() {
    return head_cursor_ == tail_cursor_;
  }
  void clear() {
    head_cursor_ = 0;
    head_next_ = 1;
    tail_cursor_ = 0;
    tail_next_ = 1;
    buf_.reset( new T[capacity_]);
    
  }
 private:
  bool inner_put(const T& t) {
    size_t slot;
    do {
      slot = head_next_;
      if (slot - tail_cursor_ > index_mask_) {
        return false;
      }
    } while (!head_next_.compare_exchange_weak(slot, slot + 1));
    buf_[slot & index_mask_] = t;
    size_t prev_slot = slot - 1;
    while(!head_cursor_.compare_exchange_weak(prev_slot, slot));
    return true;
  }
  bool inner_take(T& t) {
    size_t slot;
    do {
      slot = tail_next_;
      if (slot > head_cursor_) {
        return false;
      }
    } while (!tail_next_.compare_exchange_weak(slot, slot + 1));
    t = buf_[slot & index_mask_];
    size_t prev_slot = slot - 1;
    while(!tail_cursor_.compare_exchange_weak(prev_slot, slot));
    return true;
  }
 private:
  std::atomic<size_t> head_cursor_;
  std::atomic<size_t> head_next_;
  std::atomic<size_t> tail_cursor_;
  std::atomic<size_t> tail_next_;
  std::unique_ptr<T[]> buf_;
  size_t capacity_;
  size_t index_mask_;
};

}
}

#endif //MORTRED_MY_RING_BUFFER
