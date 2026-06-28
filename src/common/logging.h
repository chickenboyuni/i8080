#ifndef LOGGING_H
#define LOGGING_H 

#include<iostream>
#include<string>
#include<sstream>
#include <stdexcept>

class panic_exception: public std::exception {
public:
    panic_exception() {}
    const char* what() const noexcept override {
        return ""; 
    }
};

enum class LogLevel {
    INFO,
    WARNING,
    ERROR,
    FATAL,
    DEBUG
};

inline const char* to_string(LogLevel level) {
  switch(level) {
    case LogLevel::INFO: return "info";
    case LogLevel::WARNING: return "warning";
    case LogLevel::ERROR: return "error";
    case LogLevel::FATAL: return "fatal";
    case LogLevel::DEBUG: return "debug";
    default: throw std::invalid_argument("to_string(LogLevel): Unknown log level passed in");
  }
}

class Log {
public:
  Log(LogLevel level, const std::string& msg, const char* file, const char* func, int line) 
    : m_level(level), m_file(file), m_func(func), m_line(line) {
    m_stream << msg;
  }
  
  Log(LogLevel level, const std::string& msg)
   : m_level(level), m_file(__FILE__), m_func(__FUNCTION__), m_line(__LINE__) {
    m_stream << msg;
  }

  ~Log() {
    m_stream << '\n';
    std::cout << "\e[1m" << m_file << ":" << m_func << "():" << m_line << ": " << to_string(m_level) << ": " << "\e[0m" << m_stream.rdbuf() << std::flush;
  }

 template <class T>
    Log& operator<<(const T& obj) { 
      m_stream << obj;
      return *this;
    }

private:
  std::stringstream m_stream;

  LogLevel m_level;
  const char* m_file{};
  const char* m_func{};
  int m_line{};

};

#ifndef NDEBUG
  #define LOG_DEBUG(msg) Log(LogLevel::DEBUG, msg, __FILE__, __FUNCTION__, __LINE__)
#else
  #define LOG_DEBUG(_) do {} while(0)
#endif

#define LOG_INFO(msg) Log(LogLevel::INFO, msg, __FILE__, __FUNCTION__, __LINE__)
#define LOG_WARNING(msg) Log(LogLevel::WARNING, msg, __FILE__, __FUNCTION__, __LINE__)
#define LOG_ERROR(msg) Log(LogLevel::ERROR, msg, __FILE__, __FUNCTION__, __LINE__)
#define LOG_FATAL(msg) Log(LogLevel::FATAL, msg, __FILE__, __FUNCTION__, __LINE__)
#define LOG(level, msg) LOG_##level(msg)

#define PANIC(msg) LOG_FATAL(msg); throw panic_exception()

#endif /* LOGGING_H */
