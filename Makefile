CC=g++
ARGS=-g -std=c++17

COMPILE=$(CC) $(ARGS)
CPPFILES=program.cpp place.cpp transition.cpp interp.cpp scripting_helper.cpp
CPPGEN=program.generated.cpp place.cpp transition.cpp interp.cpp scripting_helper.cpp

run: gui
	cd ./gui && ./petri-editor

gui:
	mkdir -p ./gui/build && cd ./gui/build && cmake .. && make && mv petri-editor ..

clean: clean-interp
	cd ./gui && cmake . --target clean

program: $(CPPFILES)
	$(COMPILE) $^ -o $@.out

program-dbg: $(CPPFILES)
	$(COMPILE) $^ -DDEBUG -o program.out

program-trace: $(CPPFILES)
	$(COMPILE) $^ -DTRACE -o program.out

program-generated: $(CPPGEN)
	$(COMPILE) $^ -DDEBUG -o generated.out

clean-interp:
	rm -f program program.out

.PHONY: clean gui run clean-interp