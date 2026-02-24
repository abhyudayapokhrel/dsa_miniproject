CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -O2
TARGET = huffman
SRCDIR = src
SRCS = $(SRCDIR)/main.cpp $(SRCDIR)/huffman.cpp
OBJS = $(SRCS:.cpp=.o)

.PHONY: all clean run_compress run_decompress

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

#convenience targets
run_compress:
	./$(TARGET) compress input/sample.txt output

run_decompress:
	./$(TARGET) decompress output/sample.huff output

clean:
	rm -f $(SRCDIR)/*.o $(TARGET)
	rm -f output/*.huff output/*_decompressed.txt
