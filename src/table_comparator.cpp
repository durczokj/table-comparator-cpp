#include "csv_handler.h"
#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>

using namespace std;

// Struct to hold the comparison results
struct ComparisonResult {
    vector<vector<string>> matched;
    vector<vector<string>> unmatchedTableA;
    vector<vector<string>> unmatchedTableB;
};

// Class to handle table comparison
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
    ComparisonResult compare_availability(
        const vector<vector<string>>& tableA,
        const vector<vector<string>>& tableB,
        const string& pkColumnA,
        const string& pkColumnB
    ) {
        ComparisonResult result;

        if (tableA.empty() || tableB.empty()) {
            cerr << "Error: One or both tables are empty." << endl;
            return result;
        }

        // Find the index of the primary key columns
        int pkIndexA = findColumnIndex(tableA[0], pkColumnA);
        int pkIndexB = findColumnIndex(tableB[0], pkColumnB);

        if (pkIndexA == -1 || pkIndexB == -1) {
            cerr << "Error: Primary key column not found in one or both tables." << endl;
            return result;
        }

        unordered_map<string, vector<string>> tableBMap;
        unordered_set<string> matchedKeys;

        // Map rows of tableB by the primary key
        for (size_t i = 1; i < tableB.size(); ++i) {
            string key = tableB[i][pkIndexB];
            tableBMap[key] = tableB[i];
        }

        // Compare rows of tableA with tableB
        result.matched.push_back(tableA[0]); // Add headers from tableA
        result.matched[0].insert(result.matched[0].end(), tableB[0].begin(), tableB[0].end()); // Add headers from tableB

        for (size_t i = 1; i < tableA.size(); ++i) {
            string key = tableA[i][pkIndexA];
            if (tableBMap.find(key) != tableBMap.end()) {
                vector<string> joinedRow = tableA[i];
                joinedRow.insert(joinedRow.end(), tableBMap[key].begin(), tableBMap[key].end());
                result.matched.push_back(joinedRow);
                matchedKeys.insert(key);
            } else {
                result.unmatchedTableA.push_back(tableA[i]);
            }
        }

        // Find unmatched rows in tableB
        for (size_t i = 1; i < tableB.size(); ++i) {
            string key = tableB[i][pkIndexB];
            if (matchedKeys.find(key) == matchedKeys.end()) {
                result.unmatchedTableB.push_back(tableB[i]);
            }
        }

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

    // Compare tables by the specified primary key columns
    ComparisonResult result = comparator.compare_availability(tableA, tableB, pkColumnA, pkColumnB);

    // Save the results to CSV files
    csvHandler.saveToCSV("matched.csv", result.matched);
    csvHandler.saveToCSV("unmatched_tableA.csv", result.unmatchedTableA);
    csvHandler.saveToCSV("unmatched_tableB.csv", result.unmatchedTableB);

    cout << "Comparison results saved to files:" << endl;
    cout << " - Matched rows: matched.csv" << endl;
    cout << " - Unmatched rows from Table A: unmatched_tableA.csv" << endl;
    cout << " - Unmatched rows from Table B: unmatched_tableB.csv" << endl;

    return 0;
}