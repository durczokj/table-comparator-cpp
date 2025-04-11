#include "csv_handler.h"
#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>
#include <tuple> // For std::tuple
#include <getopt.h>
#include <filesystem>

using namespace std;

// Updated ComparisonResult struct with a constructor
struct ComparisonResult {
    vector<vector<string>> tableA;
    vector<vector<string>> tableB;
    string pkColumnA;
    string pkColumnB;
    vector<vector<string>> matched;
    vector<vector<string>> unmatchedTableA;
    vector<vector<string>> unmatchedTableB;
    vector<vector<string>> consistencyTable;

    // Constructor
    ComparisonResult(
        const vector<vector<string>>& tableA,
        const vector<vector<string>>& tableB,
        const string& pkColumnA,
        const string& pkColumnB
    ) : tableA(tableA), tableB(tableB), pkColumnA(pkColumnA), pkColumnB(pkColumnB) {}

    // Method to save results to CSV files
    void save(const string& resultPath, CsvHandler& csvHandler, bool omitInput = false) const {
        std::filesystem::path resultDir(resultPath);

        // Define file paths
        std::filesystem::path tableAFile = resultDir / "tableA.csv";
        std::filesystem::path tableBFile = resultDir / "tableB.csv";
        std::filesystem::path matchedFile = resultDir / "matched.csv";
        std::filesystem::path unmatchedTableAFile = resultDir / "unmatchedTableA.csv";
        std::filesystem::path unmatchedTableBFile = resultDir / "unmatchedTableB.csv";
        std::filesystem::path consistencyTableFile = resultDir / "consistencyTable.csv";

        // Conditionally save input tables
        if (!omitInput) {
            csvHandler.saveToCSV(tableAFile.string(), tableA);
            csvHandler.saveToCSV(tableBFile.string(), tableB);
        }

        // Save results
        csvHandler.saveToCSV(matchedFile.string(), matched);
        csvHandler.saveToCSV(unmatchedTableAFile.string(), unmatchedTableA);
        csvHandler.saveToCSV(unmatchedTableBFile.string(), unmatchedTableB);
        csvHandler.saveToCSV(consistencyTableFile.string(), consistencyTable);
    }
};

class TableComparator {
public:
    // Method to find the index of a column by its name
    int findColumnIndex(const vector<string>& header, const string& columnName) {
        for (size_t i = 0; i < header.size(); ++i) {
            if (header[i] == columnName) {
                return i;
            }
        }
        return -1; // Column not found
    }

    // Method to compare tables by a specified primary key column
    tuple<vector<vector<string>>, vector<vector<string>>, vector<vector<string>>> compareAvailability(
        const vector<vector<string>>& tableA,
        const vector<vector<string>>& tableB,
        const string& pkColumnA,
        const string& pkColumnB
    ) {
        vector<vector<string>> matched;
        vector<vector<string>> unmatchedTableA;
        vector<vector<string>> unmatchedTableB;

        if (tableA.empty() || tableB.empty()) {
            cerr << "Error: One or both tables are empty." << endl;
            return {matched, unmatchedTableA, unmatchedTableB};
        }

        // Find the index of the primary key columns
        int pkIndexA = findColumnIndex(tableA[0], pkColumnA);
        int pkIndexB = findColumnIndex(tableB[0], pkColumnB);

        if (pkIndexA == -1 || pkIndexB == -1) {
            cerr << "Error: Primary key column not found in one or both tables." << endl;
            return {matched, unmatchedTableA, unmatchedTableB};
        }

        unordered_map<string, vector<string>> tableBMap;
        unordered_set<string> matchedKeys;

        // Map rows of tableB by the primary key
        for (size_t i = 1; i < tableB.size(); ++i) {
            string key = tableB[i][pkIndexB];
            tableBMap[key] = tableB[i];
        }

        // Compare rows of tableA with tableB
        matched.push_back(tableA[0]); // Add headers from tableA
        matched[0].insert(matched[0].end(), tableB[0].begin(), tableB[0].end()); // Add headers from tableB

        for (size_t i = 1; i < tableA.size(); ++i) {
            string key = tableA[i][pkIndexA];
            if (tableBMap.find(key) != tableBMap.end()) {
                vector<string> joinedRow = tableA[i];
                joinedRow.insert(joinedRow.end(), tableBMap[key].begin(), tableBMap[key].end());
                matched.push_back(joinedRow);
                matchedKeys.insert(key);
            } else {
                unmatchedTableA.push_back(tableA[i]);
            }
        }

        // Find unmatched rows in tableB
        for (size_t i = 1; i < tableB.size(); ++i) {
            string key = tableB[i][pkIndexB];
            if (matchedKeys.find(key) == matchedKeys.end()) {
                unmatchedTableB.push_back(tableB[i]);
            }
        }

        return {matched, unmatchedTableA, unmatchedTableB};
    }

    // Method to compare consistency between matched rows
    vector<vector<string>> compareConsistency(const vector<vector<string>>& matched) {
        vector<vector<string>> consistencyTable;

        for (size_t i = 1; i < matched.size(); ++i) {
            vector<string> row = matched[i];
            vector<string> rowTableA;
            vector<string> rowTableB;
            vector<string> isEqual;
            size_t rowLength = row.size(); // Assign the length of the row to a variable
            for (size_t j = 0; j < rowLength / 2; ++j) { // Conventional for loop
                rowTableA.push_back(row[j]); // Get the value from the first half of the row
                rowTableB.push_back(row[j + rowLength / 2]); // Get the corresponding value from the second half of the row
                isEqual.push_back((rowTableA[j] == rowTableB[j]) ? "TRUE" : "FALSE"); // Compare the two values
            }
            consistencyTable.push_back(rowTableA); // Add the first half of the row to the consistency table
            consistencyTable.push_back(rowTableB); // Add the second half of the row to the consistency table
            consistencyTable.push_back(isEqual); // Add the comparison result to the consistency table
        }

        return consistencyTable;
    }

    // New method to perform the full comparison
    ComparisonResult compare(
        const vector<vector<string>>& tableA,
        const vector<vector<string>>& tableB,
        const string& pkColumnA,
        const string& pkColumnB
    ) {
        // Initialize the result with the constructor
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
    CsvHandler csvHandler;
    TableComparator comparator;

    string fileA, fileB, pkColumnA, pkColumnB, resultPath;

    // Define long options
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

    // Read data from CSV files
    vector<vector<string>> tableA = csvHandler.readCSV(fileA);
    vector<vector<string>> tableB = csvHandler.readCSV(fileB);

    if (tableA.empty() || tableB.empty()) {
        cerr << "Error: One or both tables are empty." << endl;
        return 1;
    }

    // Perform the full comparison
    ComparisonResult result = comparator.compare(tableA, tableB, pkColumnA, pkColumnB);

    // Save the results using the save method
    result.save(resultPath, csvHandler, true);

    cout << "Comparison results saved to " << resultPath << endl;

    return 0;
}