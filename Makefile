SRCDIR=./src

gui:
	cd $(SRCDIR) && make gui

run: 
	cd $(SRCDIR) && make run

clean: 
	cd $(SRCDIR) && make clean

doxygen:
	cd src && doxygen

pack: clean
	zip -r xtureko00-xkalin16-xsramea00-45-30-25.zip ./* -x .gitignore -x doc/* -x *.zip

.PHONY: run gui clean pack doxygen