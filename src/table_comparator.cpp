#include "data_frame.h"
#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>
#include <tuple> // For std::tuple
#include <getopt.h>
#include <filesystem>

using namespace std;

// Represents the result of table comparison
struct ComparisonResult {
    DataFrame tableA;
    DataFrame tableB;
    string pkColumnA;
    string pkColumnB;
    DataFrame matched;
    DataFrame unmatchedTableA;
    DataFrame unmatchedTableB;
    DataFrame consistencyTable;

    // Initializes the comparison result
    ComparisonResult(
        const DataFrame& tableA,
        const DataFrame& tableB,
        const string& pkColumnA,
        const string& pkColumnB
    ) : tableA(tableA), tableB(tableB), pkColumnA(pkColumnA), pkColumnB(pkColumnB) {}

    // Saves comparison results to CSV files
    void save(const string& resultPath, bool omitInput = false) const {
        std::filesystem::path resultDir(resultPath);

        // Define file paths for output
        std::filesystem::path tableAFile = resultDir / "tableA.csv";
        std::filesystem::path tableBFile = resultDir / "tableB.csv";
        std::filesystem::path matchedFile = resultDir / "matched.csv";
        std::filesystem::path unmatchedTableAFile = resultDir / "unmatchedTableA.csv";
        std::filesystem::path unmatchedTableBFile = resultDir / "unmatchedTableB.csv";
        std::filesystem::path consistencyTableFile = resultDir / "consistencyTable.csv";

        // Save input tables if not omitted
        if (!omitInput) {
            tableA.toCsv(tableAFile.string());
            tableB.toCsv(tableBFile.string());
        }

        // Save comparison results
        matched.toCsv(matchedFile.string());
        unmatchedTableA.toCsv(unmatchedTableAFile.string());
        unmatchedTableB.toCsv(unmatchedTableBFile.string());
        consistencyTable.toCsv(consistencyTableFile.string());
    }
};

class TableComparator {
public:
    // Finds the index of a column by its name
    int findColumnIndex(const vector<string>& header, const string& columnName) {
        for (size_t i = 0; i < header.size(); ++i) {
            if (header[i] == columnName) {
                return i;
            }
        }
        return -1; // Column not found
    }

    // Compares two tables based on primary key columns
    tuple<DataFrame, DataFrame, DataFrame> compareAvailability(
        const DataFrame& tableA,
        const DataFrame& tableB,
        const string& pkColumnA,
        const string& pkColumnB
    ) {
        vector<vector<string>> matched;
        vector<vector<string>> unmatchedTableA;
        vector<vector<string>> unmatchedTableB;
    
        const auto& dataA = tableA.getData();
        const auto& dataB = tableB.getData();
    
        if (dataA.empty() || dataB.empty()) {
            cerr << "Error: One or both tables are empty." << endl;
            return {DataFrame(matched), DataFrame(unmatchedTableA), DataFrame(unmatchedTableB)};
        }
    
        // Get the index of primary key columns
        int pkIndexA = findColumnIndex(dataA[0], pkColumnA);
        int pkIndexB = findColumnIndex(dataB[0], pkColumnB);
    
        if (pkIndexA == -1 || pkIndexB == -1) {
            cerr << "Error: Primary key column not found in one or both tables." << endl;
            return {DataFrame(matched), DataFrame(unmatchedTableA), DataFrame(unmatchedTableB)};
        }
    
        unordered_map<string, vector<string>> tableBMap;
        unordered_set<string> matchedKeys;
    
        // Map rows of tableB by primary key
        for (size_t i = 1; i < dataB.size(); ++i) {
            string key = dataB[i][pkIndexB];
            tableBMap[key] = dataB[i];
        }
    
        // Add headers to unmatched tables
        unmatchedTableA.push_back(dataA[0]);
        unmatchedTableB.push_back(dataB[0]);
    
        // Compare rows of tableA with tableB
        matched.push_back(dataA[0]); // Add headers from tableA
        matched[0].insert(matched[0].end(), dataB[0].begin(), dataB[0].end()); // Add headers from tableB
    
        for (size_t i = 1; i < dataA.size(); ++i) {
            string key = dataA[i][pkIndexA];
            if (tableBMap.find(key) != tableBMap.end()) {
                vector<string> joinedRow = dataA[i];
                joinedRow.insert(joinedRow.end(), tableBMap[key].begin(), tableBMap[key].end());
                matched.push_back(joinedRow);
                matchedKeys.insert(key);
            } else {
                unmatchedTableA.push_back(dataA[i]);
            }
        }
    
        // Identify unmatched rows in tableB
        for (size_t i = 1; i < dataB.size(); ++i) {
            string key = dataB[i][pkIndexB];
            if (matchedKeys.find(key) == matchedKeys.end()) {
                unmatchedTableB.push_back(dataB[i]);
            }
        }
    
        return {DataFrame(matched), DataFrame(unmatchedTableA), DataFrame(unmatchedTableB)};
    }

    // Compares consistency between matched rows
    DataFrame compareConsistency(const DataFrame& matched) {
        vector<vector<string>> consistencyTable;
        const auto& matchedData = matched.getData();
    
        if (matchedData.empty()) {
            cerr << "Error: Matched table is empty." << endl;
            return DataFrame(consistencyTable);
        }
    
        for (size_t i = 0; i < matchedData.size(); ++i) {
            vector<string> row = matchedData[i];
            size_t rowLength = row.size();
    
            if (i == 0) {
                // Extract headers from the first row
                vector<string> header;
                for (size_t j = 0; j < rowLength / 2; ++j) {
                    header.push_back(row[j]);
                }
                consistencyTable.push_back(header);
            } else {
                // Compare values from both tables
                vector<string> rowTableA;
                vector<string> rowTableB;
                vector<string> isEqual;
    
                for (size_t j = 0; j < rowLength / 2; ++j) {
                    rowTableA.push_back(row[j]);
                    rowTableB.push_back(row[j + rowLength / 2]);
                    isEqual.push_back((rowTableA[j] == rowTableB[j]) ? "TRUE" : "FALSE");
                }
    
                // Add comparison results to the table
                consistencyTable.push_back(rowTableA);
                consistencyTable.push_back(rowTableB);
                consistencyTable.push_back(isEqual);
            }
        }
    
        return DataFrame(consistencyTable);
    }

    // Performs the full comparison process
    ComparisonResult compare(
        const DataFrame& tableA,
        const DataFrame& tableB,
        const string& pkColumnA,
        const string& pkColumnB
    ) {
        // Initialize the result
        ComparisonResult result(tableA, tableB, pkColumnA, pkColumnB);

        // Perform availability comparison
        tie(result.matched, result.unmatchedTableA, result.unmatchedTableB) =
            compareAvailability(tableA, tableB, pkColumnA, pkColumnB);

        // Perform consistency comparison
        result.consistencyTable = compareConsistency(result.matched);

        return result;
    }
};

int main(int argc, char* argv[]) {
    DataFrame tableA, tableB;
    TableComparator comparator;

    string fileA, fileB, pkColumnA, pkColumnB, resultPath;

    // Define command-line options
    static struct option longOptions[] = {
        {"table-a-path", required_argument, nullptr, 'a'},
        {"table-b-path", required_argument, nullptr, 'b'},
        {"table-a-pk", required_argument, nullptr, 'p'},
        {"table-b-pk", required_argument, nullptr, 'q'},
        {"result-path", required_argument, nullptr, 'r'},
        {nullptr, 0, nullptr, 0}
    };

    int opt;
    while ((opt = getopt_long(argc, argv, "a:b:p:q:r:", longOptions, nullptr)) != -1) {
        switch (opt) {
            case 'a':
                fileA = optarg;
                break;
            case 'b':
                fileB = optarg;
                break;
            case 'p':
                pkColumnA = optarg;
                break;
            case 'q':
                pkColumnB = optarg;
                break;
            case 'r':
                resultPath = optarg;
                break;
            default:
                cerr << "Usage: ./table_comparator --table-a-path <path> --table-b-path <path> "
                     << "--table-a-pk <column> --table-b-pk <column> --result-path <path>" << endl;
                return 1;
        }
    }

    if (fileA.empty() || fileB.empty() || pkColumnA.empty() || pkColumnB.empty() || resultPath.empty()) {
        cerr << "Error: Missing required arguments." << endl;
        cerr << "Usage: ./table_comparator --table-a-path <path> --table-b-path <path> "
             << "--table-a-pk <column> --table-b-pk <column> --result-path <path>" << endl;
        return 1;
    }

    // Load data from CSV files
    tableA.fromCsv(fileA);
    tableB.fromCsv(fileB);

    if (tableA.getData().empty() || tableB.getData().empty()) {
        cerr << "Error: One or both tables are empty." << endl;
        return 1;
    }

    // Execute the comparison
    ComparisonResult result = comparator.compare(tableA, tableB, pkColumnA, pkColumnB);

    // Save the results
    result.save(resultPath, true);

    cout << "Comparison results saved to " << resultPath << endl;

    return 0;
}
