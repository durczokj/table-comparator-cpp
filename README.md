# 🔍 Table Comparator

## 📄 Description
The Table Comparator is a C++ command-line tool designed to compare two CSV tables based on a specified primary key column.  
It identifies matched rows, unmatched rows, and evaluates the consistency of matched rows across the two tables. The results are saved as CSV files in a specified output directory.

```mermaid
graph TD
    A[tableA]
    B[tableB]
    P[pkColumn]
    C[TableComparator]
    D[matched]
    E[unmatchedTableA]
    F[unmatchedTableB]
    G[consistencyTable]

    A --> C
    B --> C
    P --> C
    C --> D
    C --> E
    C --> F
    C --> G
```
- `tableA` and `tableB`: Input DFs to be compared.  
- `pkColumn`: The primary key column used for comparison.  
- `matched`: Contains rows from both tables that match based on the primary key.  
- `unmatchedTableA`: Rows from Table A without a match in Table B.  
- `unmatchedTableB`: Rows from Table B without a match in Table A.  
- `consistencyTable`: Detailed comparison of matched rows, checking each column.

## ⚙️ How to Run

### 🖥️ Command
```bash
./table_comparator --table-a-path <path_to_tableA> --table-b-path <path_to_tableB> --pk-column <primary_key_column> --result-path <output_directory>
```

### 📥 Arguments
- `--table-a-path`: Path to the first input CSV file (Table A).  
- `--table-b-path`: Path to the second input CSV file (Table B).  
- `--pk-column`: Name of the primary key column used for comparison.  
- `--result-path`: Path to the directory where the output files will be saved.

## 🔧 Example

### 📂 Input

To compare `tableA.csv` and `tableB.csv` using the `PESEL` column as the primary key and save the results in `comparison_result`, run:

```bash
./table_comparator --table-a-path ./data/tableA.csv --table-b-path ./data/tableB.csv --pk-column PESEL --result-path ./data/comparison_result
```

#### 🧾 tableA.csv
| PESEL        | Imię       | Nazwisko       | Zawód            |
|--------------|------------|----------------|------------------|
| 86485735888  | Jan        | Kowalski       | informatyk       |
| 79073971505  | Tomasz     | Nowak          | przedsiębiorca   |
| 29432533772  | Anna       | Chmielewska    | tancerka         |
| 64188385581  | Barbara    | Romczyk        | urzędniczka      |
| 48395489710  | Andrzej    | Mostkowiak     | mechanik         |
| 23594251784  | Robert     | Tomaszewski    | nauczyciel       |
| 95097548444  | Aneta      | Morawska       | sprzedawczyni    |
| 66488254309  | Aleksander | Tomas          | wykończeniowiec  |

#### 🧾 tableB.csv
| PESEL        | Imię       | Nazwisko       | Zawód            |
|--------------|------------|----------------|------------------|
| 29432533772  | Anna       | Chmielewska    | tancerka         |
| 64188385581  | Barbara    | Kurzewska      | urzędniczka      |
| 48395489710  | Andrzej    | Mostkowiak     | mechanik         |
| 23594251784  | Robert     | Tomaszewski    | nauczyciel       |
| 95097548444  | Aneta      | Morawska       | sprzedawca       |
| 66488254309  | Olek       | Tomas          | wykończeniowiec  |
| 12345678987  | Anna       | Kołodziej      | bezrobotna       |
| 98765432123  | Kacper     | Stawarski      | ogrodnik         |

### 📤 Output

After running the program, you'll find these files in `comparison_result`:

- **📄 unmatchedTableA.csv**
- **📄 unmatchedTableB.csv**
- **📄 consistencyTable.csv**
- **📄 matched.csv**

#### 📄 unmatchedTableA.csv
Rows from `tableA.csv` without matching `PESEL` in `tableB.csv`.

| PESEL        | Imię       | Nazwisko       | Zawód            |
|--------------|------------|----------------|------------------|
| 86485735888  | Jan        | Kowalski       | informatyk       |
| 79073971505  | Tomasz     | Nowak          | przedsiębiorca   |

#### 📄 unmatchedTableB.csv
Rows from `tableB.csv` without matching `PESEL` in `tableA.csv`.

| PESEL        | Imię       | Nazwisko       | Zawód            |
|--------------|------------|----------------|------------------|
| 12345678987  | Anna       | Kołodziej      | bezrobotna       |
| 98765432123  | Kacper     | Stawarski      | ogrodnik         |

#### 📄 consistencyTable.csv
Comparison of matching rows, showing `TRUE`/`FALSE` for each field.

|PESEL      |Imię      |Nazwisko   |Zawód          |
|-----------|----------|-----------|---------------|
|29432533772|Anna      |Chmielewska|tancerka       |
|29432533772|Anna      |Chmielewska|tancerka       |
|TRUE       |TRUE      |TRUE       |TRUE           |
|64188385581|Barbara   |Romczyk    |urzędniczka    |
|64188385581|Barbara   |Kurzewska  |urzędniczka    |
|TRUE       |TRUE      |FALSE      |TRUE           |
|...        |...       |...        |...            |

#### 📄 matched.csv
Joined view of matching rows from both tables.

| PESEL        | Imię       | Nazwisko       | Zawód            | PESEL        | Imię       | Nazwisko       | Zawód            |
|--------------|------------|----------------|------------------|--------------|------------|----------------|------------------|
| 29432533772  | Anna       | Chmielewska    | tancerka         | 29432533772  | Anna       | Chmielewska    | tancerka         |
| 64188385581  | Barbara    | Romczyk        | urzędniczka      | 64188385581  | Barbara    | Kurzewska      | urzędniczka      |
| ...          | ...        | ...            | ...              | ...          | ...        | ...            | ...              |

## 🧩 How It Works

**Table Comparator** uses three core classes:  
- `DataFrame`  
- `ComparisonResult`  
- `TableComparator`

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

### ✅ Input Validation
- Ensures both tables are:
  - Non-empty  
  - Have matching column counts  
  - Have identical headers  
- Done via `TableComparator::validateTables`.

### 🔑 Primary Key Indexing
- Locates primary key index using `TableComparator::findColumnIndex`.

### 🔍 Availability Comparison
- Handled by `compareAvailability`:
  - Matches → `matched`  
  - A-only → `unmatchedTableA`  
  - B-only → `unmatchedTableB`  
- Efficient via `unordered_map` and `unordered_set`.

### 🧪 Consistency Check
- `compareConsistency` uses `compareRow` to assess each field.  
- Results go into `consistencyTable`.

### 🔄 Full Comparison
- `compare()` glues all pieces and returns a `ComparisonResult`.

### 💾 Output
- `save()` writes all results to disk in the target output directory.

### 🖥️ Command-Line Usage
- `getopt_long` is used to parse arguments.

### 🚀 Execution Flow
In `main()`:
1. Load CSVs  
2. Compare tables  
3. Save results
