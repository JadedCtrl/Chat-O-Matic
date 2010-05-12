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

extern int      endianness;

#ifdef MACINTOSH_CLASSIC
extern short    refnum;

OSErr
mactcp_recv(void *handle, char *inbuf, size_t len)
{
	TCPiopb         pb;
	OSErr           err;
	size_t          dataRead = 0;

	pb.ioCRefNum = refnum;
	pb.csCode = TCPRcv;
	pb.tcpStream = ((IMCOMM *) handle)->s;
	pb.csParam.receive.commandTimeoutValue = 2;
	pb.csParam.receive.rcvBuff = inbuf;
	pb.csParam.receive.rcvBuffLen = len;
	pb.csParam.receive.userDataPtr = (Ptr) handle;
	err = PBControlSync((ParmBlkPtr) & pb);
	if (err != noErr)
		return err;

	return noErr;
}
#endif

#ifndef SEND_QUEUES
/* PROTO */
IMCOMM_RET
flap_send(void *handle, uint8_t channel, unsigned char *packet, size_t len, int updateidle)
{
	pkt_t          *outpacket;
	size_t          totallen;
#ifdef MACINTOSH_CLASSIC
	TCPiopb         pb;
	wdsEntry        wds[2];
	OSErr           err;
#endif

	totallen = len + 6;
	outpacket = pkt_init(totallen);

	pkt_add8(outpacket, 0x2A);
	pkt_add8(outpacket, channel);
	pkt_add16(outpacket, ((IMCOMM *) handle)->seqnum);
	pkt_add16(outpacket, (uint16_t) len);
	pkt_addraw(outpacket, packet, len);

#ifdef MACINTOSH_CLASSIC
	wds[0].length = outpacket->len;
	wds[0].ptr = (char *) outpacket->data;
	wds[1].length = 0;

	pb.ioCRefNum = refnum;
	pb.csCode = TCPSend;
	pb.tcpStream = ((IMCOMM *) handle)->s;
	pb.csParam.send.validityFlags = 0;
	pb.csParam.send.pushFlag = TRUE;
	pb.csParam.send.urgentFlag = 0;
	pb.csParam.send.wdsPtr = (Ptr) wds;
	pb.csParam.send.userDataPtr = (Ptr) handle;
	err = PBControlSync((ParmBlkPtr) & pb);
#else
	if (send
	    (((IMCOMM *) handle)->socket, outpacket->data, outpacket->len,
	     0) < 0) {
		pkt_free(outpacket);
		return IMCOMM_RET_ERROR;
	}
#endif

	if (((IMCOMM *) handle)->seqnum < 0xFFFF)
		((IMCOMM *) handle)->seqnum++;
	else
		((IMCOMM *) handle)->seqnum = 0;

	if (updateidle == 1) {
		((IMCOMM *) handle)->last_operation_time = time(NULL);
		if (((IMCOMM *) handle)->isidle == 1)
			imcomm_set_idle_time(handle, 0);
	}
	pkt_free(outpacket);
	return IMCOMM_RET_OK;
}

/* PROTO */
struct MultiPacket *
MultiPktInit(void)
{
	struct MultiPacket *npkt;

	npkt = malloc(sizeof(struct MultiPacket));
	npkt->init = -1;
	return npkt;
}

/* PROTO */
void
MultiPktFree(struct MultiPacket * h)
{
	struct MultiPacket *trav, *tmp;

	for (trav = h; trav != NULL;) {
		free(trav->packet);
		tmp = trav;
		trav = trav->next;
		free(tmp);
	}
}

/* PROTO */
void
flap_addToMulti(struct MultiPacket * mpkt, uint8_t channel, unsigned char *packet, size_t len, int updateidle)
{
	struct MultiPacket *trav, *npkt;
	int             first;

	if (mpkt->init == -1) {
		npkt = mpkt;
		first = 1;
	} else {
		npkt = malloc(sizeof(struct MultiPacket));
		first = 0;
	}

	npkt->channel = channel;
	npkt->packet = malloc(len);
	memcpy(npkt->packet, packet, len);
	npkt->len = len;
	npkt->updateidle = updateidle;
	npkt->next = NULL;
	npkt->init = 1;

	if (!first) {
		for (trav = mpkt; trav->next != NULL; trav = trav->next);
		trav->next = npkt;
	}
}

/* PROTO */
IMCOMM_RET
flap_sendMulti(void *handle, struct MultiPacket * pktlist)
{
	pkt_t          *outpacket;
	size_t          totallen;
	struct MultiPacket *trav;
	int             updateidle = 0;
#ifdef MACINTOSH_CLASSIC
	TCPiopb         pb;
	wdsEntry        wds[2];
	OSErr           err;
#endif

	for (trav = pktlist, totallen = 0; trav != NULL; trav = trav->next) {
		totallen += 6 + trav->len;
		if (trav->updateidle == 1)
			updateidle = 1;
	}

	outpacket = pkt_init(totallen);

	for (trav = pktlist; trav != NULL; trav = trav->next) {
		pkt_add8(outpacket, 0x2A);
		pkt_add8(outpacket, trav->channel);
		pkt_add16(outpacket, ((IMCOMM *) handle)->seqnum);
		pkt_add16(outpacket, (uint16_t) trav->len);
		pkt_addraw(outpacket, trav->packet, trav->len);

		if (((IMCOMM *) handle)->seqnum < 0xFFFF)
			((IMCOMM *) handle)->seqnum++;
		else
			((IMCOMM *) handle)->seqnum = 0;
	}

#ifdef MACINTOSH_CLASSIC
	wds[0].length = outpacket->len;
	wds[0].ptr = (char *) outpacket->data;
	wds[1].length = 0;

	pb.ioCRefNum = refnum;
	pb.csCode = TCPSend;
	pb.tcpStream = ((IMCOMM *) handle)->s;
	pb.csParam.send.validityFlags = 0;
	pb.csParam.send.pushFlag = TRUE;
	pb.csParam.send.urgentFlag = 0;
	pb.csParam.send.wdsPtr = (Ptr) wds;
	pb.csParam.send.userDataPtr = (Ptr) handle;
	err = PBControlSync((ParmBlkPtr) & pb);
#else
	if (send
	    (((IMCOMM *) handle)->socket, outpacket->data, outpacket->len,
	     0) < 0) {
		pkt_free(outpacket);
		return IMCOMM_RET_ERROR;
	}
#endif
	if (updateidle == 1) {
		((IMCOMM *) handle)->last_operation_time = time(NULL);
		if (((IMCOMM *) handle)->isidle == 1)
			imcomm_set_idle_time(handle, 0);
	}
	pkt_free(outpacket);
	return IMCOMM_RET_OK;
}

#else
/* PROTO */
IMCOMM_RET
flap_sendAct(void *handle, uint8_t channel, unsigned char *packet, size_t len, int updateidle)
{
	pkt_t          *outpacket;
	size_t          totallen;
#ifdef MACINTOSH_CLASSIC
	TCPiopb         pb;
	wdsEntry        wds[2];
	OSErr           err;
#endif

	totallen = len + 6;
	outpacket = pkt_init(totallen);

	pkt_add8(outpacket, 0x2A);
	pkt_add8(outpacket, channel);
	pkt_add16(outpacket, ((IMCOMM *) handle)->seqnum);
	pkt_add16(outpacket, (uint16_t) len);
	pkt_addraw(outpacket, packet, len);

#ifdef MACINTOSH_CLASSIC
	wds[0].length = outpacket->len;
	wds[0].ptr = (char *) outpacket->data;
	wds[1].length = 0;

	pb.ioCRefNum = refnum;
	pb.csCode = TCPSend;
	pb.tcpStream = ((IMCOMM *) handle)->s;
	pb.csParam.send.validityFlags = 0;
	pb.csParam.send.pushFlag = TRUE;
	pb.csParam.send.urgentFlag = 0;
	pb.csParam.send.wdsPtr = (Ptr) wds;
	pb.csParam.send.userDataPtr = (Ptr) handle;
	err = PBControlSync((ParmBlkPtr) & pb);
#else
	if (send
	    (((IMCOMM *) handle)->socket, outpacket->data, outpacket->len,
	     0) < 0) {
		pkt_free(outpacket);
		return IMCOMM_RET_ERROR;
	}
#endif

	if (((IMCOMM *) handle)->seqnum < 0xFFFF)
		((IMCOMM *) handle)->seqnum++;
	else
		((IMCOMM *) handle)->seqnum = 0;

	if (updateidle == 1) {
		((IMCOMM *) handle)->last_operation_time = time(NULL);
		if (((IMCOMM *) handle)->isidle == 1)
			imcomm_set_idle_time(handle, 0);
	}
	pkt_free(outpacket);
	return IMCOMM_RET_OK;
}

/* PROTO */
IMCOMM_RET
flap_sendnext(void *handle)
{
	IMCOMM_RET      ret = IMCOMM_RET_OK;
	send_q         *nq, *head = ((IMCOMM *) handle)->s_queue;

	if (head) {
		ret =
			flap_sendAct(handle, head->channel, head->data, head->len,
				     head->updateidle);
		nq = head;
		head = head->next;
		((IMCOMM *) handle)->s_queue = head;
		free(nq->data);
		free(nq);
	}
	return ret;
}

IMCOMM_RET
flap_send(void *handle, uint8_t channel, unsigned char *packet, size_t len, int updateidle)
{
	send_q         *nq, *trav, *head = ((IMCOMM *) handle)->s_queue;

	nq = malloc(sizeof(struct IMCommSendQ));
	nq->next = NULL;
	nq->updateidle = updateidle;
	nq->data = malloc(len);
	nq->channel = channel;
	memcpy(nq->data, packet, len);
	nq->len = len;

	if (head == NULL) {
		((IMCOMM *) handle)->s_queue = nq;
	} else {
		for (trav = head; trav->next != NULL; trav = trav->next);

		trav->next = nq;
	}

	return IMCOMM_RET_OK;
}
#endif				/* SEND_QUEUES */

/* PROTO */
IMCOMM_RET
flap_sendpkt(void *handle, uint8_t channel, pkt_t * pkt, int updateidle)
{
	return flap_send(handle, channel, pkt->data, pkt->len, updateidle);
}

/* PROTO */
IMCOMM_RET
flap_decode(void *handle, unsigned char *header, unsigned char *data)
{
	pkt_t          *packet;
#ifdef DEBUG
	int             y, z;
	int             x;
#endif
	uint16_t        len, sn_len;
	uint16_t        family, subfamily;
	unsigned char  *sn = NULL;
	IMCOMM_RET      ret = IMCOMM_RET_OK;
#ifdef DUMP
	FILE           *dump;
	int             xx;
	dump = fopen("imcomm.dump", "a");
#endif

	memcpy(&len, header + 4, 2);
	if (endianness == HOST_LITTLE_ENDIAN)
		len = BYTE_SWAP_16(len);

	packet = pkt_initP(data, len);

#ifdef DUMP
	for (xx = 0; x < 6; xx++)
		fputc(header[xx], dump);

	for (xx = 0; x < len; xx++)
		fputc(data[xx], dump);

	fclose(dump);
#endif

#ifdef DEBUG
	printf("Packet length: %u\n", (unsigned int) packet->len);

	for (x = 0, y = 0; x < len; x++) {
		printf("%02X ", data[x]);
		y++;
		if (y == 16) {
			for (y = 15; y >= 0; y--) {
				if (data[x - y] > 31 && data[x - y] < 128)
					putchar(data[x - y]);
				else
					putchar('.');
			}
			printf("\n");
			y = 0;
		}
	}

	if (y < 16) {
		for (z = 0; z < (16 - y); z++)
			printf("   ");
		while (y > 0) {
			if (data[x - y] > 31 && data[x - y] < 128)
				putchar(data[x - y]);
			else
				putchar('.');
			y--;
		}
		printf("\n");
	}
	printf("\n");
#endif

	switch (header[1]) {
	case 0x02:
		ret = snac_decode(handle, data, len);
		break;
	case 0x04:
		family = pkt_get16(packet);

		if (family == 0x0009) {
			subfamily = pkt_get16(packet);
			if (subfamily == 0x0002) {
				if (((IMCOMM *) handle)->callbacks[IMCOMM_ERROR])
					((IMCOMM *) handle)->callbacks[IMCOMM_ERROR] (handle,
						 IMCOMM_ERROR_OTHER_SIGNON);
#ifdef MACINTOSH_CLASSIC

#else
				shutdown(((IMCOMM *) handle)->socket, 2);
				((IMCOMM *) handle)->socket = -1;
#endif
				break;
			}
		} else if (family == 0x0001) {

			/*
		         * SRV_COOKIE
		         */

			sn_len = pkt_get16(packet);
			sn = pkt_getstr(packet, sn_len);

			if (((IMCOMM *) handle)->callbacks[IMCOMM_FORMATTED_SN])
				((IMCOMM *) handle)->
					callbacks[IMCOMM_FORMATTED_SN] (handle, sn);
			subfamily = pkt_get16(packet);

			switch (subfamily) {
			case 0x0005:
				{
					unsigned char  *server, *cookie;
					uint16_t        server_len, cookie_len;

					server_len = pkt_get16(packet);
					server = pkt_getstr(packet, server_len);

					subfamily = pkt_get16(packet);
#ifdef DEBUG
					if (subfamily != 0x0006) {
						printf("WARNING: Cookie family not 0x0006!\n");
					}
#endif

					cookie_len = pkt_get16(packet);
					cookie = pkt_getstr(packet, cookie_len);

					if (((IMCOMM *) handle)->callbacks[IMCOMM_ERROR])
						((IMCOMM *) handle)->
							callbacks[IMCOMM_ERROR] (handle,
						    IMCOMM_STATUS_AUTHDONE);
					ret =
						bos_signon_phase2(handle, server, cookie,
								cookie_len);
					free(server);
					free(cookie);
				}
				break;
			case 0x0008:
			case 0x0004:
				if (((IMCOMM *) handle)->callbacks[IMCOMM_ERROR])
					((IMCOMM *) handle)->callbacks[IMCOMM_ERROR] (handle,
						IMCOMM_ERROR_INVALID_LOGIN);
#ifdef MACINTOSH_CLASSIC

#else
				shutdown(((IMCOMM *) handle)->socket, 0x02);
				((IMCOMM *) handle)->socket = -1;
#endif
				break;
			}
		}
		break;
	}

	if (sn != NULL)
		free(sn);

	/*
         * This'll also free the incoming data buffer
         */
	pkt_free(packet);

	return ret;
}
