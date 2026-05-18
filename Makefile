SRCDIR=./src

run: 
	cd $(SRCDIR) && make run

gui:
	cd $(SRCDIR) && make gui

clean: 
	cd $(SRCDIR) && make clean

pack: clean
	zip -r xtureko00-xsramea00-xkalin16.zip ./* -x .gitignore -x doc/* -x *.zip

.PHONY: run gui clean pack