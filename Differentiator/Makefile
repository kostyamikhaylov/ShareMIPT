include ../Makefile

FLAGS += -Wlarger-than=32768

HDR = diff.h
FILES = main.cpp tree.cpp dump.cpp calc.cpp

all: $(FILES) $(HDR)
	$(CC) $(FLAGS) $(FILES)

clean:
	rm -f *.o
