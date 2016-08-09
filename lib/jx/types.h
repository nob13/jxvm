#pragma once

#include <vector>
#include <stdint.h>
#include <memory>
#include <stdexcept>
#include <unordered_map>
#include <iostream>

typedef std::vector<uint8_t> ByteArray;
typedef std::shared_ptr<ByteArray> ByteArrayPtr;

namespace hash {

// Some small hash methods
inline size_t combineHash(const size_t& a, const size_t& b){
    return a * 39 ^ b;
}

inline size_t stringHash(const std::string& s){
    return ::std::hash<std::string>()(s);
}

/** Hash algorithm which delegates to the .hash() method. */
template <class T> struct MethodHash {
    size_t operator()(const T &id ) const  {
        return id.hash();
    }
};


}