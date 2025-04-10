#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>

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
    // Method to read CSV file and store data in a table
    vector<vector<string>> readCSV(const string& filename) {
        vector<vector<string>> table;
        ifstream file(filename);

        if (!file.is_open()) {
            cerr << "Error: Could not open file " << filename << endl;
            return table;
        }

        string line;
        while (getline(file, line)) {
            vector<string> row;
            stringstream ss(line);
            string cell;

            while (getline(ss, cell, ',')) {
                row.push_back(cell);
            }

            table.push_back(row);
        }

        file.close();
        return table;
    }

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
    ComparisonResult compareTables(
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
    TableComparator comparator;

    string fileA = "data/tableA.csv"; // Replace with your CSV file name
    string fileB = "data/tableB.csv"; // Replace with your CSV file name

    // Read data from CSV files
    vector<vector<string>> tableA = comparator.readCSV(fileA);
    vector<vector<string>> tableB = comparator.readCSV(fileB);

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
    ComparisonResult result = comparator.compareTables(tableA, tableB, pkColumnA, pkColumnB);

    // Print the matched table
    cout << "Matched Table:" << endl;
    for (const auto& row : result.matched) {
        for (const auto& cell : row) {
            cout << cell << "\t";
        }
        cout << endl;
    }

    // Print unmatched rows from tableA
    cout << "\nUnmatched Rows from TableA:" << endl;
    for (const auto& row : result.unmatchedTableA) {
        for (const auto& cell : row) {
            cout << cell << "\t";
        }
        cout << endl;
    }

    // Print unmatched rows from tableB
    cout << "\nUnmatched Rows from TableB:" << endl;
    for (const auto& row : result.unmatchedTableB) {
        for (const auto& cell : row) {
            cout << cell << "\t";
        }
        cout << endl;
    }

    return 0;
}