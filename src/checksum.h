#pragma once

#include <cstdint>

/**
 * @class Checksum
 * @brief Class to calculate checksums
 *
 */
class Checksum {

public:
    
    static uint32_t checksum(char* data, long length) {
        uint32_t checksum = 0;
        for (size_t i = 0; i < length; ++i) {
            checksum ^= data[i];
        }
        return checksum;
    }
};
