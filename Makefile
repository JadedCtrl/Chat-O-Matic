.DEFAULT_GOAL := default

caya:
	$(MAKE) -f application/Makefile

protocols:
	$(MAKE) -f protocols/Makefile
libs:
	$(MAKE) -f libs/Makefile
	
clean:
	$(MAKE) -f application/Makefile clean

all: libs protocols caya

.PHONY: libs protocols

default: all
