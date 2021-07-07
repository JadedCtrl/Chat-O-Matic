# Cardie
![GSoC 2021](https://img.shields.io/badge/GSoC-2021-green.svg)

A multi-protocol chat program.

![Screenshot](data/screenshots/update-1.png)

## Building
You can make Cardie and its protocols with:

`$ make`

Or one-by-one:

`$ make libs; make app; make protocols`

Cardie itself requires the `expat_devel` package, the XMPP protocol requires
`gloox_devel`, and the (provisional) IRC protocol requires `libircclient_devel`,
`openssl_devel`, and `zlib_devel`. The (experimental) libpurple add-on requires
`libpurple_devel` and `glib2_devel`.

## License
Cardie itself is under the MIT license, but licenses vary for the included
libraries and add-ons.

The `xmpp` and `purple` add-ons are under the GPLv2+, and `irc` the MIT license.

`libsupport` is under the MIT license, though containing some PD code.
`librunview` contains code from Vision, and is under the MPL.
`libinterface` is under the MIT license.
