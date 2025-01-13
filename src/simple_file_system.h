
#pragma once
 
#define MAX_FILE_COUNT 16
#define MAX_FILENAME_LEN 64

/**
 * @struct FileTable
 * @brief Structure to hold information about the files on the vEPROM
 */
struct FileTable {
    uint32_t checksum;
    uint32_t freeOffset;
    uint32_t fileOffsets[MAX_FILE_COUNT];  
};

/**
 * @struct FileHeader
 * @brief Structure to hold information about a file on the vEPROM
 */
struct FileHeader {
    uint32_t checksum;
    uint32_t size;
    char filename[MAX_FILENAME_LEN];
};
