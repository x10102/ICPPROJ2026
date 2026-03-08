CC=g++
ARGS=-g

COMPILE=$(CC) $(ARGS)
CPPFILES=program.cpp place.cpp transition.cpp

program: $(CPPFILES)
	$(COMPILE) $^ -o $@.out

clean:
	rm program program.out

.PHONY: clean