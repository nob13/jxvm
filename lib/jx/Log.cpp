#include "Log.h"
#include <iostream>
#include "assert.h"
#include <sstream>
#include <boost/thread/mutex.hpp>
#include <boost/thread/tss.hpp>

namespace logging {

static boost::mutex gGlobalLockMutex;
static LogLevel gMinLogLevel = LL_INFO;
static boost::thread_specific_ptr< std::string> gThreadName;


static std::ostream & logStream(LogLevel level) {
    return std::cerr;
}

static const char * logLevelToString (LogLevel level) {
    switch (level) {
        case LL_DEBUG:  return  " Debug";
        case LL_INFO:   return  "  Info";
        case LL_NOTICE: return  "Notice";
        case LL_WARNING: return "  Warn";
        case LL_ERROR:  return  " Error";
        default:
            assert (!"Unknown loglevel");
            return "Unknown";
    }
}

static int countLogLevel (LogLevel level) {
    static int count[LL_COUNT] = { 0 } ;
    return count[level]++;
}

/// Strips directory from basename
static const char * myBaseName (const char * file) {
    const char * result = file;
    const char * p = result;
    while (*p != 0) {
        if (*p == '/' || *p == '\\') {
            result = p + 1;
        }
        p++;
    }
    return result;
}

static const char * threadName() {
    if (gThreadName.get()) {
        return gThreadName->c_str();
    } else {
        return "";
    }
}

std::ostream & logStreamPreformat (const char * file, int line, LogLevel level) {
    return logStream (level) << "[" << logLevelToString (level) << "]" << threadName() << " " << countLogLevel(level) <<  " " << myBaseName(file) << ":" << line <<  " ";
}

void logExecLog (LogLevel level, std::ostream& stream) {
    stream << std::endl;
}

bool logIsRequested (const LogLevel level) {
    // No locking, if there is indeed inter-thread-log-level changing
    // nobody cares about race conditions of log levels.
    return level >= gMinLogLevel;
}

void logSetMinLockLevel (const LogLevel level) {
    LogLockGuard guard;
    gMinLogLevel = level;
}

LogLockGuard::LogLockGuard() {
    gGlobalLockMutex.lock();
}

LogLockGuard::~LogLockGuard() {
    gGlobalLockMutex.unlock();
}

void logSetThreadName(const std::string& threadName) {
    gThreadName.reset (new std::string (std::string ("[") + threadName + "]")); // appending [..] to better format it
}


}

