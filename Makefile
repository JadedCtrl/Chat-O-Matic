.DEFAULT_GOAL := default

app:
	$(MAKE) -f application/Makefile

libs:
	$(MAKE) -f libs/Makefile

irc:
	$(MAKE) -f protocols/irc/Makefile

xmpp:
	$(MAKE) -f protocols/xmpp/Makefile

matrix:
	$(MAKE) -f protocols/matrix/Makefile

purple:
ifneq ($(shell uname -m), x86_gcc2)
	$(MAKE) -f protocols/purple/Makefile
endif

protocols: irc xmpp purple

all: libs protocols app

clean:
	$(MAKE) -f application/Makefile clean

.PHONY: libs protocols

default: all
