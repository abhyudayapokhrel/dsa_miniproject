#include "huffman.h"

#include <fstream>
#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <bitset>
#include <cstring>
#include <sys/stat.h>

//Helpers to extract filename stem and extension

std::string Huffman::stemOf(const std::string& path) {
    // Get filename without directory
    size_t slash = path.find_last_of("/\\");
    std::string fname = (slash == std::string::npos) ? path : path.substr(slash + 1);
    // Remove extension
    size_t dot = fname.find_last_of('.');
    return (dot == std::string::npos) ? fname : fname.substr(0, dot);
}

//  Frequency Table 

std::unordered_map<unsigned char, uint64_t>
Huffman::buildFreqTable(const std::vector<unsigned char>& data) {
    std::unordered_map<unsigned char, uint64_t> freq;
    for (unsigned char b : data) freq[b]++;
    return freq;
}

//  Tree Building (Min-Heap / Priority Queue)

HuffNode* Huffman::buildTree(
    const std::unordered_map<unsigned char, uint64_t>& freq)
{
    MinHeap heap;

    // Push one leaf per unique character
    for (auto& [ch, f] : freq)
        heap.push(new HuffNode(ch, f));

    // Edge case: single unique character
    if (heap.size() == 1) {
        HuffNode* only = heap.top(); heap.pop();
        heap.push(new HuffNode(only->freq, only, nullptr));
    }

    // Standard Huffman merge loop
    while (heap.size() > 1) {
        HuffNode* l = heap.top(); heap.pop();
        HuffNode* r = heap.top(); heap.pop();
        heap.push(new HuffNode(l->freq + r->freq, l, r));
    }

    return heap.empty() ? nullptr : heap.top();
}

//  Code Table Generation via Tree Traversal

void Huffman::buildCodes(HuffNode* node,
                         const std::string& prefix,
                         CodeTable& table)
{
    if (!node) return;
    if (node->isLeaf()) {
        table[node->ch] = prefix.empty() ? "0" : prefix;
        return;
    }
    buildCodes(node->left,  prefix + "0", table);
    buildCodes(node->right, prefix + "1", table);
}

//  Tree Serialization 
// Format per node:  1 byte flag | if leaf: 1 byte char value
// Flag: 0x01 = leaf, 0x00 = internal

void Huffman::serializeTree(HuffNode* node, std::vector<uint8_t>& buf) {
    if (!node) return;
    if (node->isLeaf()) {
        buf.push_back(0x01);
        buf.push_back(static_cast<uint8_t>(node->ch));
    } else {
        buf.push_back(0x00);
        serializeTree(node->left,  buf);
        serializeTree(node->right, buf);
    }
}

HuffNode* Huffman::deserializeTree(const std::vector<uint8_t>& buf, size_t& pos) {
    if (pos >= buf.size()) return nullptr;
    uint8_t flag = buf[pos++];
    if (flag == 0x01) {
        unsigned char ch = static_cast<unsigned char>(buf[pos++]);
        return new HuffNode(ch, 0);
    } else {
        HuffNode* l = deserializeTree(buf, pos);
        HuffNode* r = deserializeTree(buf, pos);
        return new HuffNode(0, l, r);
    }
}

void Huffman::freeTree(HuffNode* node) {
    if (!node) return;
    freeTree(node->left);
    freeTree(node->right);
    delete node;
}

//  COMPRESS 
/*
.huff file layout:
 Magic: "HUFF" (4 bytes)            
 Original size (uint64_t, 8 bytes)  
 Tree size in bytes (uint32_t)      
 Serialized tree  (tree_size bytes) 
 Padding bits used (uint8_t)        
 Compressed bitstream               
  
*/

bool Huffman::compress(const std::string& inputFile,
                       const std::string& outputDir)
{
    // 1. Read input
    std::ifstream fin(inputFile, std::ios::binary);
    if (!fin) {
        std::cerr << "[Error] Cannot open input file: " << inputFile << "\n";
        return false;
    }
    std::vector<unsigned char> data(
        (std::istreambuf_iterator<char>(fin)),
        std::istreambuf_iterator<char>());
    fin.close();

    if (data.empty()) {
        std::cerr << "[Error] Input file is empty.\n";
        return false;
    }

    std::cout << "[Compress] Read " << data.size() << " bytes from " << inputFile << "\n";

    // 2. Frequency table
    auto freq = buildFreqTable(data);
    std::cout << "[Compress] Unique symbols: " << freq.size() << "\n";

    // 3. Build Huffman tree (min-heap)
    HuffNode* root = buildTree(freq);

    // 4. Generate code table
    CodeTable codes;
    buildCodes(root, "", codes);

    // 5. Encode data → bitstring
    std::string bitstream;
    bitstream.reserve(data.size() * 4);
    for (unsigned char b : data)
        bitstream += codes[b];

    // 6. Pack bitstring into bytes
    uint8_t padding = (8 - (bitstream.size() % 8)) % 8;
    for (uint8_t i = 0; i < padding; ++i) bitstream += '0';

    std::vector<uint8_t> packedBits;
    packedBits.reserve(bitstream.size() / 8);
    for (size_t i = 0; i < bitstream.size(); i += 8) {
        uint8_t byte = 0;
        for (int b = 0; b < 8; ++b)
            if (bitstream[i + b] == '1') byte |= (1 << (7 - b));
        packedBits.push_back(byte);
    }

    // 7. Serialize tree
    std::vector<uint8_t> treeBuf;
    serializeTree(root, treeBuf);
    freeTree(root);

    // 8. Build output path
    std::string outPath = outputDir + "/" + stemOf(inputFile) + ".huff";
    std::ofstream fout(outPath, std::ios::binary);
    if (!fout) {
        std::cerr << "[Error] Cannot create output file: " << outPath << "\n";
        return false;
    }

    // 9. Write header + data
    const char magic[] = "HUFF";
    fout.write(magic, 4);

    uint64_t origSize = static_cast<uint64_t>(data.size());
    fout.write(reinterpret_cast<const char*>(&origSize), 8);

    uint32_t treeSize = static_cast<uint32_t>(treeBuf.size());
    fout.write(reinterpret_cast<const char*>(&treeSize), 4);
    fout.write(reinterpret_cast<const char*>(treeBuf.data()), treeSize);

    fout.write(reinterpret_cast<const char*>(&padding), 1);
    fout.write(reinterpret_cast<const char*>(packedBits.data()), packedBits.size());
    fout.close();

    // 10. Stats
    double ratio = 100.0 * (1.0 - (double)packedBits.size() / data.size());
    std::cout << "[Compress] Output  → " << outPath << "\n";
    std::cout << "[Compress] Original size : " << data.size()      << " bytes\n";
    std::cout << "[Compress] Compressed    : " << packedBits.size() << " bytes (+ "
              << (4 + 8 + 4 + treeSize + 1) << " bytes header)\n";
    std::cout << "[Compress] Space saving  : " << ratio            << " %\n";

    return true;
}

//  DECOMPRESS

bool Huffman::decompress(const std::string& inputFile,
                         const std::string& outputDir)
{
    std::ifstream fin(inputFile, std::ios::binary);
    if (!fin) {
        std::cerr << "[Error] Cannot open compressed file: " << inputFile << "\n";
        return false;
    }

    // 1. Magic
    char magic[5] = {0};
    fin.read(magic, 4);
    if (std::string(magic) != "HUFF") {
        std::cerr << "[Error] Not a valid .huff file.\n";
        return false;
    }

    // 2. Original size
    uint64_t origSize = 0;
    fin.read(reinterpret_cast<char*>(&origSize), 8);

    // 3. Tree
    uint32_t treeSize = 0;
    fin.read(reinterpret_cast<char*>(&treeSize), 4);
    std::vector<uint8_t> treeBuf(treeSize);
    fin.read(reinterpret_cast<char*>(treeBuf.data()), treeSize);
    size_t treePos = 0;
    HuffNode* root = deserializeTree(treeBuf, treePos);

    // 4. Padding
    uint8_t padding = 0;
    fin.read(reinterpret_cast<char*>(&padding), 1);

    // 5. Compressed bytes
    std::vector<uint8_t> packedBits(
        (std::istreambuf_iterator<char>(fin)),
        std::istreambuf_iterator<char>());
    fin.close();

    // 6. Unpack bits
    std::string bitstream;
    bitstream.reserve(packedBits.size() * 8);
    for (uint8_t byte : packedBits)
        for (int b = 7; b >= 0; --b)
            bitstream += ((byte >> b) & 1) ? '1' : '0';

    // Remove padding bits
    if (padding > 0 && bitstream.size() >= padding)
        bitstream.resize(bitstream.size() - padding);

    // 7. Decode via tree traversal
    std::vector<unsigned char> decoded;
    decoded.reserve(origSize);
    HuffNode* cur = root;
    for (char bit : bitstream) {
        cur = (bit == '0') ? cur->left : cur->right;
        if (!cur) { // safety
            std::cerr << "[Error] Corrupt data during decode.\n";
            freeTree(root);
            return false;
        }
        if (cur->isLeaf()) {
            decoded.push_back(cur->ch);
            cur = root;
            if (decoded.size() == origSize) break;
        }
    }
    freeTree(root);

    if (decoded.size() != origSize) {
        std::cerr << "[Error] Decoded size mismatch: expected " << origSize
                  << " got " << decoded.size() << "\n";
        return false;
    }

    // 8. Write output
    std::string outPath = outputDir + "/" + stemOf(inputFile) + "_decompressed.txt";
    std::ofstream fout(outPath, std::ios::binary);
    if (!fout) {
        std::cerr << "[Error] Cannot create output file: " << outPath << "\n";
        return false;
    }
    fout.write(reinterpret_cast<const char*>(decoded.data()), decoded.size());
    fout.close();

    std::cout << "[Decompress] Output → " << outPath << "\n";
    std::cout << "[Decompress] Restored " << decoded.size() << " bytes\n";

    return true;
}
