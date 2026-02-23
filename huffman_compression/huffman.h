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

