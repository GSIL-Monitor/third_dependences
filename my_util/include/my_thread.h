#ifndef MORTRED_MY_THREAD_H
#define MORTRED_MY_THREAD_H

#include <thread>
#include <functional> 
#include <memory>

namespace artanis {
namespace my_util {

class MyThread {
public:
    virtual ~MyThread(){}
    void start() {
        mythread_ptr_ = std::unique_ptr<std::thread>(new std::thread(std::bind(&MyThread::run, this)));
    }
    void join() {
        if (mythread_ptr_.get()) {
            mythread_ptr_->join();
        }
    }
    std::thread::id getId() {
      return mythread_ptr_->get_id();
    }
    virtual void run() = 0;
private:
    std::unique_ptr<std::thread> mythread_ptr_;
};

}
}

#endif
