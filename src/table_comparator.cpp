#include "csv_handler.h"
#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>
#include <tuple> // For std::tuple

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

int main() {
    CsvHandler csvHandler;
    TableComparator comparator;

    string fileA = "data/tableA.csv"; // Replace with your CSV file name
    string fileB = "data/tableB.csv"; // Replace with your CSV file name

    // Read data from CSV files
    vector<vector<string>> tableA = csvHandler.readCSV(fileA);
    vector<vector<string>> tableB = csvHandler.readCSV(fileB);

    if (tableA.empty() || tableB.empty()) {
        cerr << "Error: One or both tables are empty." << endl;
        return 1;
    }

    // Ask the user for the primary key column names
    string pkColumnA, pkColumnB;
    cout << "Enter the primary key column name for Table A: ";
    cin >> pkColumnA;
    cout << "Enter the primary key column name for Table B: ";
    cin >> pkColumnB;

    // Perform the full comparison
    ComparisonResult result = comparator.compare(tableA, tableB, pkColumnA, pkColumnB);

    // Save the results to CSV files
    csvHandler.saveToCSV("matched.csv", result.matched);
    csvHandler.saveToCSV("unmatchedTableA.csv", result.unmatchedTableA);
    csvHandler.saveToCSV("unmatchedTableB.csv", result.unmatchedTableB);
    csvHandler.saveToCSV("consistencyTable.csv", result.consistencyTable);

    return 0;
}