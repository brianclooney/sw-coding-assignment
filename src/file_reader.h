#pragma once

#include <fstream>
#include <string>

/**
 * @class FileReader
 * @brief Class to read the contents of a file
 *
 */
class FileReader
{

public:
    /**
     * @brief Reads the contents of a file and returns it as a string.
     *
     */
    static std::string readFile(std::string filename)
    {
        // Open datafile
        std::ifstream infile(filename, std::ios::binary);

        if (!infile)
        {
            throw std::runtime_error("Failed to open input file");
        }

        // Read data file
        infile.seekg(0, infile.end);
        int len = infile.tellg();
        std::string buffer(len + 1, '\0');
        infile.seekg(0, infile.beg);
        infile.read(&buffer[0], len);
        infile.close();

        return buffer;
    }
};
