#pragma once
#include "types.h"

struct ByteRange {
    ByteRange(){
        this->end = this->begin;
    }

    ByteRange(std::vector<uint8_t>::const_iterator begin, std::vector<uint8_t>::const_iterator end) {
        this->begin = begin;
        this->end = end;
    }

    ByteRange(const std::vector<uint8_t>& vector, size_t start, size_t length){
        this->begin = vector.begin() + start;
        this->end = this->begin + length;
    }


    ByteRange subRangeUpTo(size_t length) {
        if (this->begin + length > this->end){
            throw std::invalid_argument("Out of range");
        }
        return ByteRange(this->begin, this->begin + length);
    }

    uint16_t readUint16() {
        uint16_t result = fetchUint16(begin);
        begin+=2;
        return result;
    }

    uint8_t readUint8(){
        uint8_t result = fetchUint8(begin);
        begin+=1;
        return result;
    }

    uint32_t readUint32(){
        uint32_t result = fetchUint32(begin);
        begin+=4;
        return result;
    }

    uint16_t fetchUint16(size_t pos) const {
        return fetchUint16(begin + pos);
    }

    uint16_t fetchUint16(std::vector<uint8_t>::const_iterator pos) const {
        if (pos + 2 > end){
            throw std::invalid_argument("Out of index");
        }
        return ntohs(*reinterpret_cast<const uint16_t*> (&(*(pos))));
    }

    int8_t fetchInt8(std::vector<uint8_t>::const_iterator pos) const {
        if (pos + 1 > end){
            throw std::invalid_argument("Out of index");
        }
        return *reinterpret_cast<const int8_t*> (&(*(pos)));
    }

    int16_t fetchInt16(std::vector<uint8_t>::const_iterator pos) const {
        if (pos + 2 > end){
            throw std::invalid_argument("Out of index");
        }
        return ntohs(*reinterpret_cast<const int16_t*> (&(*(pos))));
    }

    uint32_t fetchUint32(std::vector<uint8_t>::const_iterator pos) const {
        if (pos + 4 > end){
            throw std::invalid_argument("Out of index");
        }
        return ntohl(*reinterpret_cast<const uint32_t*> (&(*(pos))));
    }

    int32_t fetchInt32(std::vector<uint8_t>::const_iterator pos) const {
        if (pos + 4 > end){
            throw std::invalid_argument("Out of index");
        }
        return ntohl(*reinterpret_cast<const int32_t*> (&(*(pos))));
    }

    uint8_t fetchUint8(std::vector<uint8_t>::const_iterator pos) const {
        if (pos + 1 > end){
            throw std::invalid_argument("Out of index");
        }
        return *pos;
    }

    std::vector<uint8_t>::const_iterator begin;
    std::vector<uint8_t>::const_iterator end;
};
