.DEFAULT_GOAL := default

app:
	$(MAKE) -f application/Makefile

protocols:
	$(MAKE) -f protocols/Makefile
libs:
	$(MAKE) -f libs/Makefile
	
clean:
	$(MAKE) -f application/Makefile clean

all: libs protocols app

.PHONY: libs protocols

default: all
