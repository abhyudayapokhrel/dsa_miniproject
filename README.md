# dsa_miniproject
This repo is the mini project done for COMP202 (Data Structures and Algorithms)

This is a command line text compression tool implementing **Huffman Coding** entirely in C++17.

## DSA Concepts Used

| Concept | Where Used |
|---|---|
| **Min-Heap (Priority Queue)** | Building the Huffman tree by always merging lowest-frequency nodes |
| **Binary Tree** | The Huffman tree itself — each leaf is a character |
| **Hash Map** (`unordered_map`) | Frequency table + code table |
| **Greedy Algorithm** | Core of Huffman: always pick the two smallest nodes |
| **Bit Manipulation** | Packing/unpacking the encoded bitstream into bytes |
| **Recursion** | Tree traversal for code generation + tree serialization |

## Project Structure

```
huffman_compress/
├── src/
│   ├── main.cpp        ← CLI entry point
│   ├── huffman.h       ← Class declaration
│   └── huffman.cpp     ← Full implementation
├── input/
│   └── sample.txt      ← Put your .txt files here
├── output/             ← Compressed / decompressed files appear here
├── Makefile
└── README.md
```

## Build

```bash
make
```

Requirement: `g++` with C++17 support.

## Usage

```bash
# Compress a .txt file
./huffman compress input/sample.txt

# Compress to a custom output directory
./huffman compress input/myfile.txt /path/to/output

# Decompress a .huff file
./huffman decompress output/sample.huff

# Quick make targets
make run_compress
make run_decompress

# Clean build artifacts
make clean
```


## How Huffman Coding Works

1. **Count frequencies** of each character in the input.
2. **Build a min-heap** where each character is a node with its frequency.
3. **Merge the two smallest nodes** repeatedly until one tree remains.
4. **Assign binary codes**: left edge = `0`, right edge = `1`.
5. **Encode** the input by replacing each character with its code.
6. **Pack** the bitstream into bytes and write with the tree header.

Decompression reverses the process using the stored tree.
