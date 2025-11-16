# r-index for LCP

This project extends the r-index to work with integer data by converting LCP cores' bit representations to corresponding integers and building efficient indexes.

## Overview

This project provides a complete pipeline for:
1. **LCP Analysis**: Converting strings to cores using LCP's deepen analysis
2. **Index Building**: Creating compressed r-index structures from integer sequences
3. **Pattern Matching**: Searching for cores in indexed data with efficient count and locate operations

## What This Project Does

### Core Functionality

1. **ri-lcp**: Processes input files (strings) and performs LCP analysis to extract cores, converting them to integer sequences
2. **ri-build**: Builds an r-index from integer sequences (space-separated integers in a file)
3. **ri-count**: Counts occurrences of integer patterns in the indexed data
4. **ri-locate**: Locates all positions where integer patterns occur in the indexed data
5. **ri-space**: Analyzes the space usage of built indexes

### Key Features

- **Integer-based indexing**: Works with 32-bit unsigned integers instead of character strings
- **Space-efficient**: Uses run-length encoded BWT for compact representation
- **Fast queries**: O(log(n/r)) locate time per occurrence
- **Pattern matching**: Search for sequences of integers in indexed data
- **Whitespace handling**: Automatically handles whitespace in input files

## Dependencies

### Required Libraries

1. **SDSL (Succinct Data Structure Library)**
   - Required for r-index functionality

2. **LCPtools (Locally Consistent Parsing)**
   - Required to complete the pipeline of creating indexes from fasta files

3. **CMake** (version 2.6 or higher)
   - Required for building r-index tools

4. **C++11 compatible compiler** (g++ recommended)

## Installation

### Clone Repository

```bash
git clone --recursive https://github.com/Lqxness/lcp-r-index.git
```

#### Install SDSL Library

Follow the instructions at (https://github.com/simongog/sdsl-lite) for installation.

#### Install lcptools Library

After installing sdsl-lite, navigate to lcptools directory:

```bash
cd ../lcp-r-index/lcptools
```

then run

```bash
# System-wide installation
sudo make install
```

or

```bash
# User-specific installation
# Install the library to a custom directory (e.g., `~/.local`):**
make install PREFIX=$(HOME)/.local
```

Ensure all your installed libraries' paths are configured. 

### Build Project

From the main project directory (/lcp-r-index):

```bash
make
```

This will:
1. Compile `ri-lcp` executable
2. Build r-index tools (`ri-build`, `ri-count`, `ri-locate`, `ri-space`)

You can use

```bash
make clean
```

to clean everything or

```bash
make rebuild
```

to clean and rebuild.

## Usage

### 1. ri-lcp: Convert Strings to Integers

Converts input files (fasta,fastq without headers - including only string to extract cores) to integer sequences using LCP analysis.

```bash
./ri-lcp -i <input_file> [-d <deepen_level>]
```

**Example:**
```bash
./ri-lcp -i test.fasta -d 2
# Creates: test.fasta-level2.txt
```

**Output:**
- Creates a file named `<input_file>-level<deepen_level>.txt` containing space-separated integers

**Notes:**
- Automatically removes all whitespace (spaces, tabs, newlines) from input
- Each integer represents a core extracted from the LCP analysis

### 2. ri-build: Build Index

Builds an r-index from integer sequences.

```bash
./ri-build [options] <input_file>
```

**Options:**
- `-o <basename>`: Output basename for index file (default: same as input filename)
- `-divsufsort`: Use divsufsort algorithm (faster, more memory)

**Example:**
```bash
./ri-build test.fasta-level2.txt
# Creates: test.fasta-level2.txt.ri
```

**Input Format:**
- File with space-separated 32-bit unsigned integers (one per line or all on one line)

**Output:**
- Creates `<input_file>.ri` index file

### 3. ri-count: Count Pattern Occurrences

Counts how many times an integer pattern appears in the indexed data.

```bash
./ri-count <index_file> <pattern_file>
```

**Example:**
```bash
# Create pattern file: pattern.txt
# Contents: 17 24 20

./ri-count test.fasta-level2.txt.ri pattern.txt
```

**Input Format:**
- Pattern file: space-separated integers representing the sequence to search for

**Output:**
- Prints the number of occurrences of the pattern

### 4. ri-locate: Locate Pattern Occurrences

Finds all positions where an integer pattern occurs in the indexed data.

```bash
./ri-locate [options] <index_file> <pattern_file> [original_integer_file]
```

**Options:**
- `-o <output_file>`: Write positions to file (otherwise prints to stdout)
- `[original_integer_file]`: Optional - if provided, returns integer positions instead of byte offsets

**Example:**
```bash
# Without original file (returns byte offsets)
./ri-locate test.fasta-level2.txt.ri pattern.txt

# With original file (returns integer positions)
./ri-locate test.fasta-level2.txt.ri pattern.txt test.fasta-level2.txt
```

**Output:**
- Prints positions where the pattern was found
- With `-o` flag: writes positions to specified file
- With original file: positions are integer indices (0-indexed)
- Without original file: positions are byte offsets in encoded string

### 5. ri-space: Analyze Index Space Usage

Analyzes the memory usage of a built index.

```bash
./ri-space <index_file>
```

**Example:**
```bash
./ri-space test.fasta-level2.txt.ri
```

**Output:**
- Detailed breakdown of space usage by component
- Total space in bytes

## Input/Output Formats

### Input Files

- **String files** (for ri-lcp): Any text file, whitespace is automatically removed
- **Integer files** (for ri-build, patterns): Space-separated 32-bit unsigned integers
  - Can be one integer per line or all on one line
  - Example: `17 24 20 33 45` or:
    ```
    17
    24
    20
    ```

### Output Files

- **Integer sequences** (from ri-lcp): Space-separated integers in a single line
- **Index files** (from ri-build): Binary `.ri` files
- **Position files** (from ri-locate with -o): One integer position per line

## Technical Details

### Integer Encoding

- Each integer is encoded as 4 bytes (little-endian)
- Reserved characters (0x0, 0x1) are avoided using escape sequences
- Delimiters (0xFA 0xFA 0xFA 0xFA) separate integers to prevent cross-integer pattern matches

### Index Structure

- Uses run-length encoded BWT
- Space complexity: O(r) where r is the number of BWT runs
- Supports 32-bit unsigned integers (0 to 4,294,967,295)