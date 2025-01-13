
#pragma once

/**
 * @struct FileInfo
 * @brief Structure to hold information about a file on the vEPROM
*/
struct FileInfo {
    std::string filename;
    uint32_t offset;
    uint32_t size;
    bool valid;  // Check sum matches
};
