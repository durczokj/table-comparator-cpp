#ifndef CSV_HANDLER_H
#define CSV_HANDLER_H

#include <vector>
#include <string>

class CsvHandler {
public:
    // Method to read a CSV file
    std::vector<std::vector<std::string>> readCSV(const std::string& filename);

    // Method to save a table to a CSV file
    void saveToCSV(const std::string& filename, const std::vector<std::vector<std::string>>& table);
};

#endif // CSV_READER_H