include ../Makefile

FLAGS += -Wlarger-than=16384

FLAGS_UNIT_TESTING = $(FLAGS) -D UNIT_TESTING -lm

HDR = stack.h

FILES = main.cpp stack.cpp debug.cpp

FILES_UNIT_TESTING = $(FILES) unittests.cpp

stack: $(FILES) $(HDR)
	$(CC) $(FLAGS) $(FILES) -o $@

unit_testing: $(FILES_UNIT_TESTING) $(HDR)
	$(CC) $(FLAGS_UNIT_TESTING) $(FILES_UNIT_TESTING) -o stack


clean:
	rm -f *.o
