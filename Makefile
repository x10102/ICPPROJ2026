CC=g++
ARGS=-g -std=c++17

COMPILE=$(CC) $(ARGS)
CPPFILES=program.cpp place.cpp transition.cpp

program: $(CPPFILES)
	$(COMPILE) $^ -o $@.out

program-dbg: $(CPPFILES)
	$(COMPILE) $^ -DDEBUG -o program.out

program-trace: $(CPPFILES)
	$(COMPILE) $^ -DTRACE -o program.out

clean:
	rm program program.out

.PHONY: clean