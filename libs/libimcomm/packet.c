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

/*
 * packet.c
 *
 * Contains the packet code. This is inspired by FAIM's bstream code, and will
 * resolve the portability problems caused by my shoddy allocations in
 * previous versions.
 *
 * Basically, instead of doing everything manually by using assignment
 * statements like: unsigned char packet[] = {0x00, 0x01, 0x03} or
 * packet[pos++] = 0x04; we define a packet dynamically, allocating memory
 * once (pkt_init) and then creating a separate function for each data type
 * to be added, with a "raw" function for any unhandled data.
 *
 * This is mostly because the older code didn't compile on basically anything
 * but GCC. When I came around to porting to Macintosh, it became apparent
 * that my nasty code had to change!
 */

#include "imcomm.h"
#include <assert.h>

extern int      endianness;

/*
 * Create an empty packet.
 */

/* PROTO */
pkt_t          *
pkt_init(size_t len)
{
	pkt_t          *pkt = NULL;

	pkt = malloc((size_t) sizeof(struct IMComm_Packet));
	pkt->data = malloc(len);
	pkt->len = len;
	pkt->offset = 0;

	return pkt;
}

/* PROTO */
void
pkt_zero(pkt_t * pkt)
{
	assert(pkt != NULL);

	memset(pkt->data, 0, pkt->len);
}

/* PROTO */
void
pkt_free(pkt_t * pkt)
{
	assert(pkt != NULL);

	if (pkt->data != NULL) {
		free(pkt->data);
	}
	free(pkt);
}

/* PROTO */
void
pkt_freeP(pkt_t * pkt)
{
	assert(pkt != NULL);

	/* don't touch the data */
	free(pkt);
}


/*
 * This points the data to an already initialized buffer (say, a received
 * packet) instead of copying data.
 */

/* PROTO */
pkt_t          *
pkt_initP(uint8_t * data, uint16_t len)
{
	pkt_t          *pkt;

	pkt = malloc(sizeof(struct IMComm_Packet));
	pkt->data = data;
	pkt->len = (size_t) len;
	pkt->offset = 0;

	return pkt;
}

/* PROTO */
size_t
pkt_empty(pkt_t * pkt)
{
	return pkt->len - pkt->offset;
}

/* PROTO */
size_t
pkt_getoffset(pkt_t * pkt)
{
	return pkt->offset;
}

/* PROTO */
void
pkt_skip(pkt_t * pkt, size_t skipnum)
{
	/* assert(pkt->offset + skipnum < pkt->len); */
	pkt->offset += skipnum;
}

/* PROTO */
void
pkt_setoffset(pkt_t * pkt, size_t offset)
{
	assert(offset < pkt->len);
	pkt->offset = offset;
}

/* PROTO */
int
pkt_end(pkt_t * pkt)
{
	if (pkt->offset >= pkt->len)
		return 1;
	else
		return 0;
}

/* PROTO */
IMCOMM_RET
pkt_add8(pkt_t * pkt, uint8_t data)
{
	if (pkt_empty(pkt) < 1)
		return IMCOMM_RET_ERROR;

	memcpy(pkt->data + pkt->offset, &data, 1);
	pkt->offset++;
	return IMCOMM_RET_OK;
}

/*
 * AIM is big-endian.
 *
 * Endianness is determined at startup and stored in a global var.
 */

/* PROTO */
IMCOMM_RET
pkt_add16(pkt_t * pkt, uint16_t val)
{
	uint16_t        tmpval;

	if (pkt_empty(pkt) < 2)
		return IMCOMM_RET_ERROR;

	if (endianness == HOST_BIG_ENDIAN) {
		memcpy(pkt->data + pkt->offset, &val, 2);
	} else {
		tmpval = BYTE_SWAP_16(val);
		memcpy(pkt->data + pkt->offset, &tmpval, 2);
	}

	pkt->offset += 2;
	return IMCOMM_RET_OK;
}

/* PROTO */
IMCOMM_RET
pkt_add32(pkt_t * pkt, uint32_t val)
{
	uint32_t        tmpval;

	if (pkt_empty(pkt) < 4)
		return IMCOMM_RET_ERROR;

	if (endianness == HOST_BIG_ENDIAN) {
		memcpy(pkt->data + pkt->offset, &val, 4);
	} else {
		tmpval = BYTE_SWAP_32(val);
		memcpy(pkt->data + pkt->offset, &tmpval, 4);
	}

	pkt->offset += 4;
	return IMCOMM_RET_OK;
}

/* PROTO */
IMCOMM_RET
pkt_addraw(pkt_t * pkt, uint8_t * data, size_t len)
{
	if (pkt_empty(pkt) < len)
		return IMCOMM_RET_ERROR;

	memcpy(pkt->data + pkt->offset, data, len);
	pkt->offset += len;
	return IMCOMM_RET_OK;
}

/* PROTO */
uint8_t
pkt_get8(pkt_t * pkt)
{
	uint8_t         val;

	memcpy(&val, pkt->data + pkt->offset, 1);
	pkt->offset++;
	return val;
}

/* PROTO */
uint16_t
pkt_get16(pkt_t * pkt)
{
	uint16_t        val;

	memcpy(&val, pkt->data + pkt->offset, 2);

	if (endianness == HOST_LITTLE_ENDIAN)
		val = BYTE_SWAP_16(val);

	pkt->offset += 2;
	return val;
}

/* PROTO */
uint32_t
pkt_get32(pkt_t * pkt)
{
	uint32_t        val;

	memcpy(&val, pkt->data + pkt->offset, 4);

	if (endianness == HOST_LITTLE_ENDIAN)
		val = BYTE_SWAP_32(val);

	pkt->offset += 4;
	return val;
}

/* PROTO */
uint8_t        *
pkt_getraw(pkt_t * pkt, size_t len)
{
	uint8_t        *buf;

	buf = malloc(len);
	memcpy(buf, pkt->data + pkt->offset, len);

	pkt->offset += (uint16_t) len;

	return buf;
}

/* PROTO */
uint8_t        *
pkt_getstr(pkt_t * pkt, size_t len)
{
	uint8_t        *buf;

	buf = malloc(len + 1);
	memcpy(buf, pkt->data + pkt->offset, len);
	pkt->offset += (uint16_t) len;
	buf[len] = 0;
	return buf;
}

#ifdef DEBUG
/* PROTO */
void
pkt_dump(pkt_t * pkt)
{
	int             x;

	printf("!! ");
	for (x = 0; x < pkt->len; x++) {
		if (pkt->data[x] >= 32 && pkt->data[x] < 128)
			putchar(pkt->data[x]);
		else
			putchar('.');
	}

	printf("\n!! ");
	for (x = 0; x < pkt->len; x++)
		printf("%02X ", pkt->data[x]);

	printf("\n");
}
#endif
