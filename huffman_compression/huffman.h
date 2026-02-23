#pragma once
#include <string>
#include <unordered_map>
#include <queue>
#include <vector>
#include <cstdint>

struct HuffNode{
    unsigned char ch;
    uint64_t freq;
    HuffNode* left;
    HuffNode* right;

    //Leaf node
    HuffNode(unsigned char c, uint64_t f)
        : ch(c), freq(f), left(nullptr), right(nullptr){}

    HuffNode (uint64_t f, HuffNode*l, HuffNode* r):
        ch(0), freq(f), left(l), right(r){}

    bool isLeaf() const{
        return !left && !right;
    }

};


// min heap comparator

struct NodeCmp {
    bool operator()(const HuffNode* a, const HuffNode* b) const {
        return a->freq > b->freq;   // min-heap by frequency
    }
};

using MinHeap = std::priority_queue<HuffNode*, std::vector<HuffNode*>, NodeCmp>;
using CodeTable = std::unordered_map<unsigned char, std::string>;

//Public API
class Huffman{
public:
    //compress garne file lai in following manner:
    // inputfilr -> outputDir/<basename>.huff
    static bool compress(const std::string& inputFile,
                         const std::string& outputDir);

    // decompress mgarne file lai in following manner:
    // inputFile.huff -> outputDir/<basename>.txt
        static bool decompress(const std::string& inputFile,
                               const std::string& outputDir);

private:
    // frequency table from raw byte
    static std::unordered_map<unsigned char, uint64_t>
        buildFreqTable(const std::vector<unsigned char>& data);

    // Building Huffman tree using a min-heap (priority queue)
    static HuffNode* buildTree(const std::unordered_map<unsigned char, uint64_t>& freq);

    // Traverse tree to generate code table
    static void buildCodes(HuffNode* node, const std::string& prefix, CodeTable& table);

    //Free memory
    static void freeTree(HuffNode* node);

    // Serialize / deserialize the tree into the file header
    static void   serializeTree(HuffNode* node, std::vector<uint8_t>& buf);
    static HuffNode* deserializeTree(const std::vector<uint8_t>& buf,
                                     size_t& pos);
    

    // Utility: extract filename stem and extension
    static std::string stemOf(const std::string& path);
};