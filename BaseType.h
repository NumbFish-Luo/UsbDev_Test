#pragma once
#include <cstdio>
#include <Windows.h>
#include <cassert>
#include <string>
#include <string_view>
#include <vector>
#include <map>
#include <iostream>
#include <sstream>
#include <array>
#include <thread>
#include <mutex>
#include <fstream>
namespace NF {
    using std::string;
    using std::string_view;
    using std::to_string;
    using std::vector;
    using std::map;
    using std::pair;
    using std::cout;
    using std::cerr;
    using std::clog;
    using std::endl;
    using std::stringstream;
    using std::ostream;
    using std::array;
    using std::ifstream;
    using std::ofstream;
    using std::getline;
    typedef unsigned char byte;
    typedef unsigned int Timeout;
    template <typename T>
    class Singleton {
    public:
        static T& Instance() {
            if (m_instance == nullptr) {
                if (m_instance == nullptr) {
                    m_instance = new T();
                    atexit(Destroy);
                }
                return *m_instance;
            }
            return *m_instance;
        }
    protected:
        Singleton() = default;
        ~Singleton() = default;
    private:
        Singleton(const Singleton& rhs) = delete;
        Singleton& operator = (const Singleton& rhs) = delete;
        static void Destroy() {
            if (m_instance != nullptr) { delete m_instance; }
            m_instance = nullptr;
        }
        static T* volatile m_instance;
    };
    template <typename T>
    T* volatile Singleton<T>::m_instance = nullptr;
    class BegLen {
    public:
        size_t beg;
        size_t len;
    };
    enum DataType {
        DataType_Dec,
        DataType_Hex,
        DataType_Str
    };
}
