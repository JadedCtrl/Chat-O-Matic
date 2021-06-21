# ![Icon](data/icons/Caya.png) Caya
![GSoC 2021](https://img.shields.io/badge/GSoC-2021-green.svg)

A multi-protocol chat program.

![Screenshot](data/screenshots/update-1.png)

## Building
You can make Caya and its protocols with:

`$ make`

Caya itself requires the `expat_devel` package, the XMPP protocol requires
`gloox_devel`, and the (provisional) IRC protocol requires
`libircclient_devel`. You can also build either independent of the other:

`$ make caya; make libs; make protocols; make -f protocols/xmpp/Makefile`

