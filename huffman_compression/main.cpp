#include <iostream>
#include <string>
#include <filesystem>
#include "huffman.h"

namespace fs = std::filesystem;

static void printUsage(const char*prog){
    std::cout << "Usage:\n";
    std::cout << "  " << prog << " compress   <input.txt>  [output_dir]\n";
    std::cout << "  " << prog << " decompress <input.huff> [output_dir]\n\n";
    std::cout << "Defaults:\n";
    std::cout << "  output_dir = ./output\n\n";
    std::cout << "Examples:\n";
    std::cout << "  " << prog << " compress   input/sample.txt\n";
    std::cout << "  " << prog << " decompress output/sample.huff\n\n";
}

int main(int argc, char* argv[]){
    if (argc < 3){
        printUsage(argv[0]);
        return 1;
    }

    std::string mode      = argv[1];
    std::string inputFile = argv[2];
    std::string outputDir = (argc >= 4) ? argv[3] : "./output";

    // Ensure output directory exists
    fs::create_directories(outputDir);    
    
    // Validate input file exists
    if (!fs::exists(inputFile)) {
        std::cerr << "[Error] Input file not found: " << inputFile << "\n";
        return 1;
    }

    if (mode == "compress") {
        // Validate .txt extension
        if (inputFile.size() < 4 ||
            inputFile.substr(inputFile.size() - 4) != ".txt") {
            std::cerr << "[Error] compress mode requires a .txt input file.\n";
            return 1;
        }
        return Huffman::compress(inputFile, outputDir) ? 0 : 1;

    } else if (mode == "decompress") {
        // Validate .huff extension
        if (inputFile.size() < 5 ||
            inputFile.substr(inputFile.size() - 5) != ".huff") {
            std::cerr << "[Error] decompress mode requires a .huff input file.\n";
            return 1;
        }
        return Huffman::decompress(inputFile, outputDir) ? 0 : 1;

    } else {
        std::cerr << "[Error] Unknown mode: " << mode << "\n";
        printUsage(argv[0]);
        return 1;
    }
}