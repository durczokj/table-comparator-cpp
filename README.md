# Table Comparator

## Description
The Table Comparator is a C++ command-line tool designed to compare two CSV tables based on a specified primary key column. It identifies matched rows, unmatched rows, and evaluates the consistency of matched rows across the two tables. The results are saved as CSV files in a specified output directory.

---

## Inputs and Outputs

```mermaid
graph TD
    A[Table A] -->|--table-a-path| C[Table Comparator]
    B[Table B] -->|--table-b-path| C[Table Comparator]
    P[Primary Key] -->|--pk-column| C[Table Comparator]
    C -->|Matched Rows| D[matched.csv]
    C -->|Unmatched Rows in Table A| E[unmatchedTableA.csv]
    C -->|Unmatched Rows in Table B| F[unmatchedTableB.csv]
    C -->|Consistency Table| G[consistencyTable.csv]
```

---

## How to Run

### Command
```bash
./table_comparator --table-a-path <path_to_tableA> --table-b-path <path_to_tableB> --pk-column <primary_key_column> --result-path <output_directory>
```

### Arguments
- `--table-a-path`: Path to the first input CSV file (Table A).
- `--table-b-path`: Path to the second input CSV file (Table B).
- `--pk-column`: Name of the primary key column used for comparison.
- `--result-path`: Path to the directory where the output files will be saved.

### Example
To compare `data/tableA.csv` and `data/tableB.csv` using the `PESEL` column as the primary key and save the results in `data/comparison_result`, run:
```bash
./table_comparator --table-a-path tableA.csv --table-b-path tableB.csv --pk-column PESEL --result-path data/comparison_result
```

---

## Class Diagram

```mermaid
classDiagram
    class DataFrame {
        - vector<vector<string>> data
        + DataFrame()
        + DataFrame(vector<vector<string>>)
        + const vector<vector<string>>& getData() const
        + void setData(vector<vector<string>>)
        + void fromCsv(string)
        + void toCsv(string) const
    }

    class ComparisonResult {
        - DataFrame tableA
        - DataFrame tableB
        - string pkColumn
        - DataFrame matched
        - DataFrame unmatchedTableA
        - DataFrame unmatchedTableB
        - DataFrame consistencyTable
        + ComparisonResult(DataFrame, DataFrame, string)
        + void save(string, bool) const
    }

    class TableComparator {
        + void validateTables(DataFrame, DataFrame)
        + int findColumnIndex(vector<string>, string)
        + tuple<DataFrame, DataFrame, DataFrame> compareAvailability(DataFrame, DataFrame, string)
        + DataFrame compareConsistency(DataFrame)
        + tuple<vector<string>, vector<string>, vector<string>> compareRow(vector<string>)
        + vector<string> extractHeaders(vector<string>)
        + ComparisonResult compare(DataFrame, DataFrame, string)
    }

    %% Relationships
    DataFrame <|-- ComparisonResult : "Composition (contains DataFrames)"
    TableComparator --> ComparisonResult : "Dependency (uses ComparisonResult)"
```

---

## Output Files
- `matched.csv`: Contains rows from both tables that match based on the primary key.
- `unmatchedTableA.csv`: Contains rows from Table A that do not have a match in Table B.
- `unmatchedTableB.csv`: Contains rows from Table B that do not have a match in Table A.
- `consistencyTable.csv`: Contains a detailed comparison of matched rows, including consistency checks for each column.
