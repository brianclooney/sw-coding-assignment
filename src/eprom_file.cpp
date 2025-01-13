#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <vector>
#include <memory>

#include "eprom_file.h"

EpromFile::EpromFile(std::string filename) {
    this->filename = filename;
}

void EpromFile::init(long size) {
    std::ofstream outfile(this->filename, std::ios::out | std::ios::binary);
    for (int i = 0; i<size; i++) {
        outfile.put(0xFF);
    }
}

long EpromFile::getCapacity() {
    std::ifstream infile(this->filename, std::ios::binary);
    if (infile) {
        infile.seekg(0, infile.end);
        return infile.tellg();
    }
    return -1;
}

void EpromFile::write(char* data, long length, long offset) {
    std::fstream file(this->filename, std::ios::binary | std::ios::in | std::ios::out);

    if (!file) {
        throw std::runtime_error("Failed to open EPROM file");
    }
    
    file.seekp(offset);
    file.write(data, length);
    file.close();
}

void EpromFile::read(char* data, long length, long offset) {
    std::ifstream file(this->filename, std::ios::binary);

    if (!file) {
        throw std::runtime_error("Failed to open EPROM file");
    }

    file.seekg(offset);
    file.read(data, length);
    file.close();
}
