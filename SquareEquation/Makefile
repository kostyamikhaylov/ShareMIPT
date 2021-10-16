include ../Makefile

FLAGS_UNIT_TESTING = $(FLAGS) -D UNIT_TESTING

FILES = main.cpp
FILES_UNIT_TESTING = $(FILES) unittests.cpp

all:
	$(CC) $(FLAGS) $(FILES)

unit_testing:
	$(CC) $(FLAGS_UNIT_TESTING) $(FILES_UNIT_TESTING)


clean:
	rm -f *.o
