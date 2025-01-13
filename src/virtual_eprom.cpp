#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <vector>
#include <memory>

#include "virtual_eprom.h"
#include "simple_file_system.h"
#include "file_info.h"
#include "checksum.h"
#include "file_reader.h"


VirtualEprom::VirtualEprom(EpromInterface* interface) {
    this->epromInterface = interface;
}

void VirtualEprom::create(int capacity) {
    
    epromInterface->init(capacity);

    // Initialize file table
    std::unique_ptr<FileTable> fileTable(new FileTable());
    std::memset((void*)fileTable.get(), 0, sizeof(FileTable));
    fileTable->checksum = Checksum::checksum((char*)fileTable.get()+sizeof(uint32_t), sizeof(FileTable)-sizeof(uint32_t));
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

    auto buffer = FileReader::readFile(filepath);

    // Check if the file will fit
    if (!willFit(fileTable.freeOffset, sizeof(FileHeader)+buffer.length())) {
        throw std::runtime_error("Data file will not fit on vEPROM");
    }
    
    // Write file header
    std::unique_ptr<FileHeader> fileHeader(new FileHeader());
    std::memset((void*)fileHeader.get(), 0, sizeof(FileHeader));
    fileHeader->size = buffer.length();
    std::strncpy(fileHeader->filename, filepath.c_str(), MAX_FILENAME_LEN);
    fileHeader->checksum = calculateFileChecksum(fileHeader.get(), buffer);
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
            auto data = readRaw(fileTable.fileOffsets[i]+sizeof(FileHeader), fileHeader.size);
            if (!validateFileChecksum(&fileHeader, data)) {
                throw std::runtime_error("Checksum failed for file");
            }

            return data;
        }
    }

    return "";
}

void VirtualEprom::writeRaw(long address, std::string data) {

    // Check if the data will fit
    if (!willFit(address, data.size())) {
        throw std::runtime_error("Data will not fit on vEPROM");
    }

    epromInterface->write((char*)data.c_str(), data.size(), address);
}

std::string VirtualEprom::readRaw(long address, long length) {

    // Check that we are not trying to read outside the vEPROM capacity
    if (!willFit(address, length)) {
        throw std::runtime_error("Read data is not within the vEPROM capacity");
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
        fileInfo.valid = validateFileChecksum(&fileHeader, readRaw(fileTable.fileOffsets[i] + sizeof(FileHeader), fileHeader.size));
        files.push_back(fileInfo);
    }

    return files;
}

void VirtualEprom::erase() {
    create(epromInterface->getCapacity());
}

// Private methods

uint32_t VirtualEprom::calculateFileChecksum(FileHeader* fileHeader, std::string data) {
    uint32_t checksum = Checksum::checksum((char*)data.c_str(), data.size());
    checksum += Checksum::checksum((char*)fileHeader+sizeof(uint32_t), sizeof(FileHeader)-sizeof(uint32_t));
    return checksum;
}

bool VirtualEprom::validateFileChecksum(FileHeader* fileHeader, std::string data) {
    bool result = calculateFileChecksum(fileHeader, data) == fileHeader->checksum;
    return result;
}

// Check if data will fit on vEPROM
bool VirtualEprom::willFit(int address, int length) {
    int capacity = epromInterface->getCapacity();
    return address + length < capacity && address > 0;
}
