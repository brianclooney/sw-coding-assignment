
#pragma once

#include <string>

#include "eprom_interface.h"


/**
 * @class EpromFile
 * @brief Implements EPROM backed by a file
 *
 */
class EpromFile : public EpromInterface
{
public:

    /**
     * @brief Create instance of EPROM backed by the specified file
     *
     */    
    EpromFile(std::string filename);

    /**
     * @brief Initialize EPROM with 0xFF
     *
     */
    void init(long size) override;

    long getCapacity() override;
    
    /**
     * @brief Writes data to the EPROM file
     *
     */
    void write(char* data, long length, long offset) override;
    
    /**
     * @brief Reads data from the EPROM file
     *
     */
    void read(char* data, long length, long offset) override;

private:

    std::string filename;

};
