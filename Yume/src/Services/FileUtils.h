#pragma once

#include "ServiceLocator/ServiceLocator.h"

#include <string>
#include <vector>
#include <fstream>

namespace Yume {

class FileUtils : public ServiceLocator::IService {

public:
bool ReadFile(const std::string& filePath, std::vector<char>& buffer) {
    std::ifstream file(filePath, std::ios::ate);

    if (!file.is_open()) {
        throw std::runtime_error("failed to open file!");
    }

    buffer.resize(file.tellg());
    file.seekg(0, std::ios::beg);
    file.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));

    file.close();

    return true;
}

};

}