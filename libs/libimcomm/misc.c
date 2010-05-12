/**  _
 ** (_)_ __  __ ___ _ __  _ __
 ** | | '  \/ _/ _ \ '  \| '  \
 ** |_|_|_|_\__\___/_|_|_|_|_|_|
 **
 ** Copyright (C) 2003-2005, Claudio Leite
 ** All rights reserved.
 **
 ** Please see the file 'COPYING' for licensing information.
 **/

#include "imcomm.h"

/* PROTO */
int
getbyteorder(void)
{
	uint16_t        blah = 0x5533;
	uint8_t        *ptr = (uint8_t *) & blah;

	if ((*ptr) == 0x55)
		return HOST_BIG_ENDIAN;
	else
		return HOST_LITTLE_ENDIAN;
}
