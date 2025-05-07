#include "data_frame.h"
#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>
#include <tuple>
#include <getopt.h>
#include <filesystem>

using namespace std;

// Represents the result of table comparison
struct ComparisonResult {
    DataFrame tableA;
    DataFrame tableB;
    vector<string> pkColumns;
    DataFrame matched;
    DataFrame unmatchedTableA;
    DataFrame unmatchedTableB;
    DataFrame consistencyTable;

    // Initializes the comparison result
    ComparisonResult(
        const DataFrame& tableA,
        const DataFrame& tableB,
        const vector<string>& pkColumns
    ) : tableA(tableA), tableB(tableB), pkColumns(pkColumns) {}

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
    // Checks if the tables are valid for comparison
    void validateTables(const DataFrame& tableA, const DataFrame& tableB) {
        const auto& dataA = tableA.getData();
        const auto& dataB = tableB.getData();

        // Check if any table is empty
        if (dataA.empty() || dataB.empty()) {
            throw runtime_error("Error: One or both tables are empty.");
        }

        // Check if the tables have the same number of columns
        if (dataA[0].size() != dataB[0].size()) {
            throw runtime_error("Error: Tables have different numbers of columns.");
        }

        // Check if the header rows are identical
        if (dataA[0] != dataB[0]) {
            throw runtime_error("Error: Header rows of the tables are not identical.");
        }
    }

    // Finds the index of a column by its name
    int findColumnIndex(const vector<string>& header, const string& columnName) {
        for (size_t i = 0; i < header.size(); ++i) {
            if (header[i] == columnName) {
                return i;
            }
        }
        return -1; // Column not found
    }

    // Compares two tables based on multiple primary key columns
    tuple<DataFrame, DataFrame, DataFrame> compareAvailability(
        const DataFrame& tableA,
        const DataFrame& tableB,
        const vector<string>& pkColumns
    ) {
        vector<vector<string>> matched;
        vector<vector<string>> unmatchedTableA;
        vector<vector<string>> unmatchedTableB;

        const auto& dataA = tableA.getData();
        const auto& dataB = tableB.getData();

        // Get the indices of the primary key columns in both tables
        vector<int> pkIndicesA, pkIndicesB;
        for (const auto& pkColumn : pkColumns) {
            int indexA = findColumnIndex(dataA[0], pkColumn);
            int indexB = findColumnIndex(dataB[0], pkColumn);
            if (indexA == -1 || indexB == -1) {
                cerr << "Error: Primary key column '" << pkColumn << "' not found in one or both tables." << endl;
                return {DataFrame(matched), DataFrame(unmatchedTableA), DataFrame(unmatchedTableB)};
            }
            pkIndicesA.push_back(indexA);
            pkIndicesB.push_back(indexB);
        }

        unordered_map<string, vector<string>> tableBMap;
        unordered_set<string> matchedKeys;

        // Helper function to generate a composite key
        auto generateCompositeKey = [](const vector<string>& row, const vector<int>& indices) -> string {
            string key;
            for (int index : indices) {
                key += row[index] + "|"; // Use a delimiter to separate column values
            }
            return key;
        };

        // Map rows of tableB by composite primary key
        for (size_t i = 1; i < dataB.size(); ++i) {
            string key = generateCompositeKey(dataB[i], pkIndicesB);
            tableBMap[key] = dataB[i];
        }

        // Add headers to unmatched tables
        unmatchedTableA.push_back(dataA[0]);
        unmatchedTableB.push_back(dataB[0]);

        // Compare rows of tableA with tableB
        matched.push_back(dataA[0]); // Add headers from tableA
        matched[0].insert(matched[0].end(), dataB[0].begin(), dataB[0].end()); // Add headers from tableB

        for (size_t i = 1; i < dataA.size(); ++i) {
            string key = generateCompositeKey(dataA[i], pkIndicesA);
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
            string key = generateCompositeKey(dataB[i], pkIndicesB);
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
            throw runtime_error("Matched table is empty.");
        }

        // Extract headers
        vector<string> header = extractHeaders(matchedData[0]);
        consistencyTable.push_back(header);

        // Compare rows
        for (size_t i = 1; i < matchedData.size(); ++i) {
            auto [tableAValues, tableBValues, comparisonResults] = compareRow(matchedData[i]);
            consistencyTable.push_back(tableAValues);
            consistencyTable.push_back(tableBValues);
            consistencyTable.push_back(comparisonResults);
        }

        return DataFrame(consistencyTable);
    }

    tuple<vector<string>, vector<string>, vector<string>> compareRow(const vector<string>& row) {
        size_t rowLength = row.size();
        vector<string> tableAValues, tableBValues, comparisonResults;

        for (size_t j = 0; j < rowLength / 2; ++j) {
            tableAValues.push_back(row[j]);
            tableBValues.push_back(row[j + rowLength / 2]);
            comparisonResults.push_back((tableAValues[j] == tableBValues[j]) ? "TRUE" : "FALSE");
        }

        return {tableAValues, tableBValues, comparisonResults};
    }

    vector<string> extractHeaders(const vector<string>& row) {
        vector<string> header;
        for (size_t j = 0; j < row.size() / 2; ++j) {
            header.push_back(row[j]);
        }
        return header;
    }

    // Performs the full comparison process
    ComparisonResult compare(
        const DataFrame& tableA,
        const DataFrame& tableB,
        const vector<string>& pkColumns
    ) {
        // Validate the tables before comparison
        validateTables(tableA, tableB);

        // Initialize the result
        ComparisonResult result(tableA, tableB, pkColumns);

        // Perform availability comparison
        tie(result.matched, result.unmatchedTableA, result.unmatchedTableB) =
            compareAvailability(tableA, tableB, pkColumns);

        // Perform consistency comparison
        result.consistencyTable = compareConsistency(result.matched);

        return result;
    }
};

int main(int argc, char* argv[]) {
    DataFrame tableA, tableB;
    TableComparator comparator;

    string fileA, fileB, resultPath;
    vector<string> pkColumns;
    char separatorA = ','; // Default separator for tableA
    char separatorB = ','; // Default separator for tableB

    // Define command-line options
    static struct option longOptions[] = {
        {"table-a-path", required_argument, nullptr, 'a'},
        {"table-b-path", required_argument, nullptr, 'b'},
        {"pk-column", required_argument, nullptr, 'p'},
        {"result-path", required_argument, nullptr, 'r'},
        {"table-a-separator", required_argument, nullptr, 's'},
        {"table-b-separator", required_argument, nullptr, 't'},
        {nullptr, 0, nullptr, 0}
    };

    int opt;
    while ((opt = getopt_long(argc, argv, "a:b:p:r:s:t:", longOptions, nullptr)) != -1) {
        switch (opt) {
            case 'a':
                fileA = optarg;
                break;
            case 'b':
                fileB = optarg;
                break;
            case 'p':
                pkColumns.push_back(optarg); // Collect multiple primary key columns
                break;
            case 'r':
                resultPath = optarg;
                break;
            case 's':
                separatorA = optarg[0]; // Set custom separator for tableA
                break;
            case 't':
                separatorB = optarg[0]; // Set custom separator for tableB
                break;
            default:
                cerr << "Usage: ./table_comparator --table-a-path <path> --table-b-path <path> "
                     << "--pk-column <column> [--pk-column <column> ...] --result-path <path> "
                     << "[--table-a-separator <char>] [--table-b-separator <char>]" << endl;
                return 1;
        }
    }

    if (fileA.empty() || fileB.empty() || pkColumns.empty() || resultPath.empty()) {
        cerr << "Error: Missing required arguments." << endl;
        cerr << "Usage: ./table_comparator --table-a-path <path> --table-b-path <path> "
             << "--pk-column <column> [--pk-column <column> ...] --result-path <path> "
             << "[--table-a-separator <char>] [--table-b-separator <char>]" << endl;
        return 1;
    }

    // Load data from CSV files with custom separators
    tableA.fromCsv(fileA, separatorA);
    tableB.fromCsv(fileB, separatorB);

    if (tableA.getData().empty() || tableB.getData().empty()) {
        cerr << "Error: One or both tables are empty." << endl;
        return 1;
    }

    // Execute the comparison
    ComparisonResult result = comparator.compare(tableA, tableB, pkColumns);

    // Save the results
    result.save(resultPath, true);

    cout << "Comparison results saved to " << resultPath << endl;

    return 0;
}
