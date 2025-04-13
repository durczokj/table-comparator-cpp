#include "data_frame.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>

// Method to read data from a CSV file with a customizable separator
void DataFrame::fromCsv(const std::string& filename, char separator) {
    data.clear();
    std::ifstream file(filename);

    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        // Trim trailing '\r' if present (for Windows-style line endings)
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }

        std::vector<std::string> row;
        std::stringstream ss(line);
        std::string cell;

        while (std::getline(ss, cell, separator)) {
            row.push_back(cell);
        }

        data.push_back(row);
    }

    file.close();
}

// Method to write data to a CSV file with a customizable separator
void DataFrame::toCsv(const std::string& filename, char separator) const {
    std::filesystem::path filePath(filename);
    std::filesystem::path dirPath = filePath.parent_path();

    // Ensure the directory exists
    if (!std::filesystem::exists(dirPath)) {
        if (!std::filesystem::create_directories(dirPath)) {
            std::cerr << "Error: Could not create directory " << dirPath << std::endl;
            return;
        }
    }

    std::ofstream file(filename);

    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << " for writing." << std::endl;
        return;
    }

    for (const auto& row : data) {
        for (size_t i = 0; i < row.size(); ++i) {
            file << row[i];
            if (i < row.size() - 1) {
                file << separator;
            }
        }
        file << "\n";
    }

    file.close();
}
