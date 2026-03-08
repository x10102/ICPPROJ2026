CC=g++
ARGS=-g

COMPILE=$(CC) $(ARGS)

program: program.cpp
	$(COMPILE) $^ -o $@.out

clean:
	rm program program.out

.PHONY: clean