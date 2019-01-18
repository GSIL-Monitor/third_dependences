#ifndef  MORTRED_SINGLETON_H
#define  MORTRED_SINGLETON_H

#include <thread>
#include <mutex>
#include <cstdlib> 

namespace artanis {

template<typename T>
class Singleton {
    public:
        static T& instance()
        {
            std::call_once(s_once_, &Singleton::init);
            return *s_value_;
        }

    private:
        Singleton() = default;
        ~Singleton() = default;
        Singleton(const Singleton&) = delete;
        Singleton& operator= (const Singleton&) = delete;

        static void init()
        {
            s_value_ = new T();
            std::atexit(destroy);
        }

        static void destroy()
        {
            typedef char T_must_be_complete_type[sizeof(T) == 0 ? -1 : 1];
            delete s_value_;
        }

    private:
        static std::once_flag s_once_;
        static T* s_value_;
};

template<typename T>
std::once_flag Singleton<T>::s_once_;

template<typename T>
T* Singleton<T>::s_value_ = NULL;
}

#endif  //MORTRED_SINGLETON_H
