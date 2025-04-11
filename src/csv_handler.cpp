#include "csv_handler.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <filesystem>

// Method to read a CSV file
std::vector<std::vector<std::string>> CsvHandler::readCSV(const std::string& filename) {
    std::vector<std::vector<std::string>> table;
    std::ifstream file(filename);

    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return table;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::vector<std::string> row;
        std::stringstream ss(line);
        std::string cell;

        while (std::getline(ss, cell, ',')) {
            row.push_back(cell);
        }

        table.push_back(row);
    }

    file.close();
    return table;
}

// Method to save a table to a CSV file
void CsvHandler::saveToCSV(const std::string& filename, const std::vector<std::vector<std::string>>& table) {
    // Extract the directory path from the filename
    std::filesystem::path filePath(filename);
    std::filesystem::path dirPath = filePath.parent_path();

    // Check if the directory exists, and create it if it doesn't
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

    for (const auto& row : table) {
        for (size_t i = 0; i < row.size(); ++i) {
            file << row[i];
            if (i < row.size() - 1) {
                file << ",";
            }
        }
        file << "\n";
    }

    file.close();
}