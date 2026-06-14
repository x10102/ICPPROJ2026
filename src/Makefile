CC=g++
ARGS=-g -std=c++17

COMPILE=$(CC) $(ARGS)
CPPFILES=program.cpp place.cpp transition.cpp interp.cpp scripting_helper.cpp guiconnector.cpp
CPPGEN=program.generated.cpp place.cpp transition.cpp interp.cpp scripting_helper.cpp guiconnector.cpp

run: gui
	cd ./gui && ./petri-editor

gui:
	cmake -S ./gui -B ./gui/build && cmake --build ./gui/build && mv ./gui/build/petri-editor ./gui

clean: clean-interp
	rm -rf ./gui/build ./gui/petri-editor

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