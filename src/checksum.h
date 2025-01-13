#pragma once

#include <cstdint>

/**
 * @class Checksum
 * @brief Class to calculate checksums
 *
 */
class Checksum {

public:
    
    /**
     * @brief Calculate the checksum of a block of data
     *
     * @param data Pointer to the data
     * @param length Length of the data
     */
    static uint32_t checksum(char* data, long length) {
        uint32_t checksum = 0;
        for (size_t i = 0; i < length; ++i) {
            checksum ^= data[i];
        }
        return checksum;
    }
};
