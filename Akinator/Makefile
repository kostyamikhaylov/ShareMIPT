include ../Makefile

FLAGS += -Wlarger-than=32768

HDRS = akinator.h ../Stack/stack.h
FILES = main.cpp akinator.cpp ../Stack/stack.cpp ../Stack/debug.cpp

all: $(FILES) $(HDRS)
	$(CC) $(FLAGS) $(FILES)

clean:
	rm -f *.o
