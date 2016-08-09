#pragma once

#include <string>
#include <sstream>

namespace util {

template <typename T> std::string toHex(const T&t) {
    std::stringstream stream;
    stream << "0x" << std::hex << t << std::dec;
    return stream.str();
}

template <typename T> std::string toString(const T&t){
    std::stringstream stream;
    stream << t;
    return stream.str();
}

/** to be called on startup to figure out executable directory. */
void onStart(int argc, char* argv[]);

/** Returns directory of executable (assuming onStart was called). */
const std::string& executableDirectory();

}
