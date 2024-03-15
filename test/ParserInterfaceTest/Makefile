# Compiler and flags
CC := gcc
CXX := g++
CFLAGS := -Wall -Wextra
CXXFLAGS := -Wall -Wextra -I../../include -I../../submodules/SheetReader-r/src
CPPOBJS := ../../submodules/SheetReader-r/src/XlsxFile.o ../../submodules/SheetReader-r/src/XlsxSheet.o ../../submodules/SheetReader-r/src/miniz/miniz.o ../../src/ParserInterface.o
COBJS := TestParserInterface.o

# Rules
.PHONY: all clean objclean

all: output

$(CPPOBJS): %.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(COBJS): %.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

output: $(COBJS) $(CPPOBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@

clean:
	rm -f *.o output

objclean:
	rm -f $(CPPOBJS) $(COBJS)