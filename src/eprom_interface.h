
#pragma once

/**
 * @class EpromInterface
 * @brief Interface for reading and writing to an EPROM
 *
 */
class EpromInterface {
public:

    /**
     * @brief Initialize EPROM with 0xFF
     *
     */
    virtual void init(long size) = 0;

    virtual long getCapacity() = 0;
    
    /**
     * @brief Writes data to the EPROM
     *
     */
    virtual void write(char* data, long length, long offset) = 0;
    
    /**
     * @brief Reads data from the EPROM
     *
     */
    virtual void read(char* data, long length, long offset) = 0;

};
