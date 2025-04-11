#ifndef CSV_HANDLER_H
#define CSV_HANDLER_H

#include <vector>
#include <string>

class CsvHandler {
public:
    std::vector<std::vector<std::string>> readCSV(const std::string& filename);
    void saveToCSV(const std::string& filename, const std::vector<std::vector<std::string>>& table);
};

#endif // CSV_HANDLER_H