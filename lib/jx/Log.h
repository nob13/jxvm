#pragma once

#include <ostream>

namespace logging {

// Note: logging copy and pasted from libpcore, https://github.com/nob13/pcore
// Just threw out android support

// Different log levels
enum LogLevel { LL_DEBUG, LL_INFO, LL_NOTICE, LL_WARNING, LL_ERROR, LL_COUNT };

/// Returns the logstream for a given file, line and log level
std::ostream & logStreamPreformat (const char * file, int line, LogLevel level);

/// Executes a given Log
void logExecLog (LogLevel logLevel, std::ostream&);

/// Log level is requested
bool logIsRequested (const LogLevel level);

/// Set minimum log level
void logSetMinLockLevel (const LogLevel level);

/// Set a log thread name for this thread, keep string valid
void logSetThreadName(const std::string & threadName);

/// RAII Structure locking the log
struct LogLockGuard {
    LogLockGuard ();
    ~LogLockGuard();
private:
    LogLockGuard(const LogLockGuard&);
    LogLockGuard& operator=(const LogLockGuard&);
};

template <class A>
void log (const char * file, int line, LogLevel level, const A&a) {
    if (!logIsRequested (level)) return;
    LogLockGuard guard;
    logExecLog (level, logStreamPreformat (file, line, level) << a);
}
template <class A, class B>
void log (const char * file, int line, LogLevel level, const A&a, const B&b) {
    if (!logIsRequested (level)) return;
    LogLockGuard guard;
    logExecLog (level, logStreamPreformat (file, line, level) << a << " " << b);
}
template <class A, class B, class C>
void log (const char * file, int line, LogLevel level, const A&a, const B&b, const C&c) {
    if (!logIsRequested (level)) return;
    LogLockGuard guard;
    logExecLog (level, logStreamPreformat (file, line, level) << a << " " << b << " " << c);
}
template <class A, class B, class C, class D>
void log (const char * file, int line, LogLevel level, const A&a, const B&b, const C&c, const D&d) {
    if (!logIsRequested (level)) return;
    LogLockGuard guard;
    logExecLog (level, logStreamPreformat (file, line, level) << a << " " << b << " " << c  << " " << d);
}
template <class A, class B, class C, class D, class E>
void log (const char * file, int line, LogLevel level, const A&a, const B&b, const C&c, const D&d, const E&e) {
    if (!logIsRequested (level)) return;
    LogLockGuard guard;
    logExecLog (level, logStreamPreformat (file, line, level) << a << " " << b << " " << c  << " " << d << " " << e);
}
template <class A, class B, class C, class D, class E, class F>
void log (const char * file, int line, LogLevel level, const A&a, const B&b, const C&c, const D&d, const E&e, const F&f) {
    if (!logIsRequested (level)) return;
    LogLockGuard guard;
    logExecLog (level, logStreamPreformat (file, line, level) << a << " " << b << " " << c  << " " << d << " " << e << " " << f);
}

}

// Variadic macros, Part of C99 and Supported by GCC and VS >= 2008
#define logd(...) ::logging::log (__FILE__, __LINE__, ::logging::LL_DEBUG, __VA_ARGS__)
#define logi(...) ::logging::log (__FILE__, __LINE__, ::logging::LL_INFO, __VA_ARGS__)
#define logn(...) ::logging::log (__FILE__, __LINE__, ::logging::LL_NOTICE, __VA_ARGS__)
#define logw(...) ::logging::log (__FILE__, __LINE__, ::logging::LL_WARNING, __VA_ARGS__)
#define loge(...) ::logging::log (__FILE__, __LINE__, ::logging::LL_ERROR, __VA_ARGS__)

// Provide your own loglevel log_customlevel --> log_customlevel (loglevel, args)
#define log_customlevel(...)::pc::log (__FILE__, __LINE__, __VA_ARGS__)


// Disable logd, will be compiled out
#undef logd
// Disable logd
#define logd(...)
