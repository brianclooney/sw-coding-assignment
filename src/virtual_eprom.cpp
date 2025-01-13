
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <vector>
#include <memory>

#include "virtual_eprom.h"
#include "simple_file_system.h"
#include "file_info.h"


VirtualEprom::VirtualEprom(EpromInterface* interface) {
    this->epromInterface = interface;
}

void VirtualEprom::create(int capacity) {
    
    epromInterface->init(capacity);

    // Initialize file table
    std::unique_ptr<FileTable> fileTable(new FileTable());
    std::memset((void*)fileTable.get(), 0, sizeof(FileTable));
    fileTable->checksum = calculateChecksum((char*)fileTable.get()+sizeof(uint32_t), sizeof(FileTable)-sizeof(uint32_t));
    fileTable->freeOffset = sizeof(FileTable);

    epromInterface->write((char*)fileTable.get(), sizeof(FileTable), 0);
}

void VirtualEprom::writeFile(std::string filepath) {

    FileTable fileTable;
    epromInterface->read((char*)&fileTable, sizeof(FileTable), 0);

    // Find next empty slot in table for this file
    int index = -1;
    for (int i=0; i<MAX_FILE_COUNT; i++) {
        if (fileTable.fileOffsets[i] == 0) {
            index = i;
            break;
        }
    }

    if (index == -1) {
        throw std::runtime_error("Max file limit has been reached.  Cannot add file.");
    }

    auto buffer = readDataFile(filepath);

    // Check if the file will fit
    int capacity = epromInterface->getCapacity();
    if (fileTable.freeOffset+sizeof(FileHeader)+buffer.length() > capacity) {
        throw std::runtime_error("Data file will not fit on vEPROM");
    }
    
    // Write file header
    std::unique_ptr<FileHeader> fileHeader(new FileHeader());
    fileHeader->size = buffer.length();
    std::strncpy(fileHeader->filename, filepath.c_str(), MAX_FILENAME_LEN);
    fileHeader->checksum = calculateChecksum((char*)buffer.data(), buffer.length());
    fileHeader->checksum ^= calculateChecksum((char*)fileHeader.get()+sizeof(uint32_t), sizeof(FileHeader)-sizeof(uint32_t));
    epromInterface->write((char*)fileHeader.get(), sizeof(FileHeader), fileTable.freeOffset);
    
    // Write file contents
    epromInterface->write((char*)buffer.data(), buffer.length(), fileTable.freeOffset+sizeof(FileHeader));

    // Write updated file table
    fileTable.fileOffsets[index] = fileTable.freeOffset;
    fileTable.freeOffset = fileTable.freeOffset + buffer.length() + sizeof(FileHeader);
    epromInterface->write((char*)&fileTable, sizeof(FileTable), 0);
}

std::string VirtualEprom::readFile(std::string filename) {

    FileTable fileTable;
    FileHeader fileHeader;

    epromInterface->read((char*)&fileTable, sizeof(FileTable), 0);
    
    for (int i=0; i<MAX_FILE_COUNT; i++) {
        if (fileTable.fileOffsets[i] == 0) {
            break;
        }
        epromInterface->read((char*)&fileHeader, sizeof(FileHeader), fileTable.fileOffsets[i]);
        if (strcmp(fileHeader.filename, filename.c_str()) == 0) {
            // TODO: validate checksum
            return readRaw(fileTable.fileOffsets[i]+sizeof(FileHeader), fileHeader.size);
        }
    }

    return "";
}

void VirtualEprom::writeRaw(long address, std::string data) {

    // Check if the data will fit
    int capacity = epromInterface->getCapacity();
    if (address+data.size() > capacity) {
        throw std::runtime_error("Data will not fit on vEPROM");
    }

    epromInterface->write((char*)data.c_str(), data.size(), address);
}

std::string VirtualEprom::readRaw(long address, long length) {

    // Check that we are not trying to read outside the vEPROM capacity
    int capacity = epromInterface->getCapacity();
    if (address+length > capacity) {
        throw std::runtime_error("Failed to read outside the vEPROM capacity");
    }
    
    auto buffer = std::make_unique<char[]>(length + 1);
    epromInterface->read(buffer.get(), length, address);
    return buffer.get();
}

std::vector<FileInfo> VirtualEprom::listFiles() {
    
    FileTable fileTable;
    FileHeader fileHeader;
    std::vector<FileInfo> files;

    epromInterface->read((char*)&fileTable, sizeof(FileTable), 0);
    
    for (int i=0; i<MAX_FILE_COUNT; i++) {
        if (fileTable.fileOffsets[i] == 0) {
            break;
        }
        epromInterface->read((char*)&fileHeader, sizeof(FileHeader), fileTable.fileOffsets[i]);

        FileInfo fileInfo;
        fileInfo.filename = fileHeader.filename;
        fileInfo.offset = fileTable.fileOffsets[i];
        fileInfo.size = fileHeader.size + sizeof(FileHeader);
        fileInfo.valid = true;  // TODO: validate checksum
        files.push_back(fileInfo);
    }

    return files;
}

void VirtualEprom::erase() {
    create(epromInterface->getCapacity());
}

uint32_t VirtualEprom::calculateChecksum(char* data, long length) {
    uint32_t checksum = 0;
    for (size_t i = 0; i < length; ++i) {
        checksum ^= data[i];
    }
    return checksum;
}

std::string VirtualEprom::readDataFile(std::string filename) {
    
    // Open datafile
    std::ifstream infile(filename, std::ios::binary);
    
    if (!infile) {
        throw std::runtime_error("Failed to open input file");
    }

    // Read data file
    infile.seekg(0, infile.end);
    int len = infile.tellg();
    std::string buffer(len+1, '\0');
    infile.seekg(0, infile.beg);
    infile.read(&buffer[0], len);
    infile.close();

    return buffer;
}
