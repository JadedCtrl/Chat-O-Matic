# ![Chat-O-Matic Icon](data/icons/AppIcon.png) Chat-O-Matic

![GSoC 2021](https://img.shields.io/badge/GSoC-2021-green.svg)

Chat-O-Matic is a multi-protocol chat program based on [Caya](https://github.com/Augustolo/Caya).

![Screenshot](data/screenshots/update-final.png)

It can use protocols through native add-ons as well as through libpurple,
the library used by [Pidgin](https://pidgin.im/).

Protocols natively supported include IRC and XMPP.

Protocols generally supported through libpurple include GroupWise, Zephyr, and
[others through plugins](https://pidgin.im/plugins/?type=Protocol).


## Building
You can make Chat-O-Matic and its protocols with:

`$ make`

Or one-by-one:

`$ make libs; make app; make protocols`

Chat-O-Matic itself requires the `expat_devel` package, the XMPP protocol requires
`gloox_devel`, and the libpurple add-on requires `libpurple_devel` and
`glib2_devel`.


## Installation
Protocol add-ons can be installed in any add-ons directory under `chat-o-matic`
(i.e., `~/config/non-packaged/add-ons/chat-o-matic/`) or in the binary's CWD
(`./chat-o-matic/`).

libpurple plugins can be installed to any lib directory under `purple-2`
(i.e., `~/config/non-packaged/lib/purple-2/`).


## License
Chat-O-Matic is under the MIT license, but licenses vary for the included
libraries and add-ons.

The `xmpp` and `purple` add-ons are under the GPLv2+, and `irc` the MIT license.

`libsupport` is under the MIT license, though containing some PD code.
`librunview` contains code from Vision, and is under the MPL.
`libinterface` is under the MIT license.
