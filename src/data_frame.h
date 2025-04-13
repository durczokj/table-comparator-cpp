#ifndef DATA_FRAME_H
#define DATA_FRAME_H

#include <vector>
#include <string>

class DataFrame {
private:
    std::vector<std::vector<std::string>> data;

public:
    // Constructor
    DataFrame() = default;
    explicit DataFrame(const std::vector<std::vector<std::string>>& data) : data(data) {}

    // Getters
    const std::vector<std::vector<std::string>>& getData() const { return data; }
    void setData(const std::vector<std::vector<std::string>>& newData) { data = newData; }

    // Methods to read from and write to CSV
    void fromCsv(const std::string& filename, char separator = ',');
    void toCsv(const std::string& filename, char separator = ',') const;
};

#endif // DATA_FRAME_H