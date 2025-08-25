#ifndef UTILS_H
#define UTILS_H

#include <vector>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <unistd.h>

namespace Utils {

    template<typename T>
    void appendBytes(std::vector<uint8_t>& bytes, const T& value) {
        const uint8_t* ptr = reinterpret_cast<const uint8_t*>(&value);
        bytes.insert(bytes.end(), ptr, ptr + sizeof(T));
    }
    
    template<typename T>
    T readBytes(const std::vector<uint8_t>& bytes, size_t& offset) {
        T value;
        std::memcpy(&value, bytes.data() + offset, sizeof(T));
        offset += sizeof(T);
        return value;
    }

    void checkRead(int err) {
        if (err == -1) {
            throw std::runtime_error(strerror(errno));            
        }
    }
}

#endif