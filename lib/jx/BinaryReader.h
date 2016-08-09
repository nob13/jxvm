#include <boost/noncopyable.hpp>
#include <boost/iostreams/device/mapped_file.hpp>
#include "types.h"

/** Java class file compatible binary reader from Memory or memory mapped files. */
class BinaryReader : boost::noncopyable {
public:
    
    BinaryReader(const uint8_t * bytes, size_t size){
        mBytes = bytes;
        mSize = size;
    }
    
    BinaryReader(const std::string& filename){
        using boost::iostreams::mapped_file_params;
        using boost::iostreams::mapped_file_source;
        
        mapped_file_params params;
        params.path = filename;
        params.flags = boost::iostreams::mapped_file_base::readonly;
        
        mMappedFile = mapped_file_source(params);
        mBytes = reinterpret_cast<const uint8_t*>(mMappedFile.data());
        mSize  = mMappedFile.size();
    }

    BinaryReader(const ByteArray& bytes){
        mBytes = &(*bytes.begin());
        mSize = bytes.size();
    }
    
    void readInto (uint8_t & data) {
        data = readNative<uint8_t>();
    }
    
    void readInto (uint32_t & data) {
        data = ntohl(readNative<uint32_t>());
    }
    
    void readInto (uint16_t & data) {
        data = ntohs(readNative<uint16_t>());
    }
    
    void readIntoBytes(uint8_t * bytes, size_t length){
        const uint8_t * p = readBytes(length);
        for (size_t i = 0; i < length; i++){
            bytes[i] = p[i];
        }
    }
    
    template <class Iterator> void readIntoByteIterator(Iterator iterator, size_t length){
        const uint8_t * p = readBytes(length);
        for (size_t i = 0; i < length; i++){
            *iterator = p[i];
            iterator++;
        }
    }
    
    const uint8_t* readBytes(size_t length) {
        if (mPos + length > mSize) {
            throw std::invalid_argument("Reading out of length, invalid file");
        }
        const uint8_t * result = mBytes + mPos;
        mPos += length;
        return result;
    }
    
    /** Read bytes into a ByteArray vector, returning the start position. */
    const size_t readBytesInto (std::vector<uint8_t> & target, size_t length){
        size_t start = target.size();
        target.reserve(target.size() + length);
        if (mPos + length > mSize){
            throw std::invalid_argument("Reading out of length, invalid file");
        }
        for (size_t i = 0; i < length; i++){
            target.push_back(*(mBytes + mPos));
            mPos += 1;
        }
        return start;
    }
    
    size_t rest() const {
        return mSize - mPos;
    }
    
private:
    
    template <typename T> T readNative(){
        return *reinterpret_cast<const T*>(readBytes(sizeof(T)));
    }
    
    const uint8_t * mBytes;
    size_t mSize;
    size_t mPos = 0;
    
    boost::iostreams::mapped_file_source mMappedFile;
};
