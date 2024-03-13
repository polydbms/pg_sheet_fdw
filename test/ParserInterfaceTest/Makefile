# Compiler and flags
CC := gcc
CXX := g++
CFLAGS := -Wall -Wextra
CXXFLAGS := -Wall -Wextra -I../../include -I../../submodules/SheetReader-r/src
MOREOBJS := ../../submodules/SheetReader-r/src/XlsxFile.o ../../submodules/SheetReader-r/src/XlsxSheet.o ../../submodules/SheetReader-r/src/miniz/miniz.o
OBJS := TestParserInterface.o ParserInterface.o $(MOREOBJS)

# Rules
.PHONY: all clean

all: output

$(MOREOBJS): %.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

ParserInterface.o: ../../src/ParserInterface.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

TestParserInterface.o: TestParserInterface.c
	$(CC) $(CFLAGS) -c $< -o $@

output: $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@

clean:
	rm -f *.o output

objclean:
	rm -f $(MOREOBJS)