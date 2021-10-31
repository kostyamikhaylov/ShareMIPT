include ../Makefile

FLAGS += -Wlarger-than=32768

BASIC_FILES = version.cpp registers.cpp

ALL_FILES = processor.cpp compiler.cpp ../Stack/stack.cpp ../Stack/debug.cpp

PROCESSOR_FILES = $(BASIC_FILES) processor.cpp ../Stack/stack.cpp ../Stack/debug.cpp
COMPILER_FILES = $(BASIC_FILES) compiler.cpp
DISASSEMBLER_FILES = $(BASIC_FILES) disassembler.cpp
LISTING_FILES = $(BASIC_FILES) listing.cpp

all: compiler processor disassembler listing

compiler: $(COMPILER_FILES) processor.h
	$(CC) $(FLAGS) $(COMPILER_FILES) -o $@

processor: $(PROCESSOR_FILES) processor.h
	$(CC) $(FLAGS) $(PROCESSOR_FILES) -o $@

disassembler: $(DISASSEMBLER_FILES) processor.h
	$(CC) $(FLAGS) $(DISASSEMBLER_FILES) -o $@

listing: $(LISTING_FILES) processor.h
	$(CC) $(FLAGS) $(LISTING_FILES) -o $@

clean:
	rm -f *.o
