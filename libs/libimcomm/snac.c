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

/*
 * OK, by a little cleaning up, I mean a lot of cleaning up.
 *
 * I need to split this into several files and comment it. If anyone is brave
 * enough to try to figure out this mess, go for it, but you've been warned.
 */

/*
 * This file could use a little cleaning up.
 *
 * A lot of this was hurried in an attempt to get a working client, but I never
 * bothered to fix it since it worked.
 */

extern int      endianness;
extern int      nodes_to_delete;

/* PROTO */
IMCOMM_RET
snac_decode(void *handle, uint8_t * data, uint16_t len)
{
	uint16_t        family, subfamily, flags;
	TLVLIST        *tlvlist;

	if (len <= 4)
		return IMCOMM_RET_OK;

	family = two_to_16(data);
	subfamily = two_to_16(data + 2);
	flags = two_to_16(data + 4);

	if (((IMCOMM *) handle)->ischild == 1) {
		family = two_to_16(data);
		subfamily = two_to_16(data + 2);

		if (family == 0x0001 && subfamily == 0x0003) {
			if (data[12] == 0x00 && data[13] == 0x10) {
				snac_finish_buddy_icon(handle);
#ifdef DEBUG
				printf("sent buddy icon\n");
#endif
				return IMCOMM_RET_OK;
			}
		} else if (family == 0x0010 && subfamily == 0x0003) {
			/* ack from buddy icon upload */
			((IMCOMM *) handle)->to_delete = 1;
			nodes_to_delete = 1;
#ifdef DEBUG
			printf
				("Received ACK for buddy icon upload, marking handle for deletion.\n");
#endif
			return IMCOMM_RET_OK;
		} else {
			printf("not really parsing child.\n");
			return IMCOMM_RET_OK;
		}

		return IMCOMM_RET_OK;
	}
#ifdef MD5_LOGIN
	if (family == 0x0017) {
		bos_md5snac(handle, data, len);
		return IMCOMM_RET_OK;
	}
#endif

	if (family == 0x0001) {
		char           *sn;
		uint8_t         sn_len;

		switch (subfamily) {
			/*
		         * supported families list respond with (01,17)
		         */
		case 0x0003:
			snac_get_srv_families(handle, data + 10, len - 10);
			return snac_send_versions(handle);
			break;
			/*
		         * we're not really worried about versions right now.
		         */
		case 0x0005:
			snac_new_subconnection(handle, data + 18, len - 18);
			return IMCOMM_RET_OK;
			break;
		case 0x0018:
			return snac_request_limits(handle);
			break;
		case 0x0007:
			return snac_ack_limits(handle, data + 10, len - 10);
			break;
		case 0x000a:
			if (((IMCOMM *) handle)->callbacks[IMCOMM_ERROR]) {
				((IMCOMM *) handle)->callbacks[IMCOMM_ERROR] (handle,
						     IMCOMM_RATE_LIMIT_WARN,
								  data[19]);
			}
			break;
#if 0
		case 0x000b:
			if (((IMCOMM *) handle)->callbacks[IMCOMM_ERROR]) {
				((IMCOMM *) handle)->callbacks[IMCOMM_ERROR] (handle,
							 IMCOMM_WARN_PAUSE);
			}
			return snac_ack_srv_pause(handle, data + 10, len - 10);
			break;
		case 0x000d:
			if (((IMCOMM *) handle)->callbacks[IMCOMM_ERROR]) {
				((IMCOMM *) handle)->callbacks[IMCOMM_ERROR] (handle,
						       IMCOMM_WARN_UNPAUSE);
			}
			if (((IMCOMM *) handle)->srv_pause)
				((IMCOMM *) handle)->srv_pause = 0;

			break;
#endif
		case 0x000f:
			if (flags == 0x8000)
				break;

			sn_len = data[10];
			sn = malloc(sn_len + 1);
			memcpy(sn, data + 11, sn_len);
			sn[sn_len] = 0;

			if (((IMCOMM *) handle)->callbacks[IMCOMM_FORMATTED_SN])
				((IMCOMM *) handle)->
					callbacks[IMCOMM_FORMATTED_SN] (handle, sn);

			free(sn);
			break;
#if 0
		case 0x0012:
			handle_srv_migration(handle, data, len);
			break;
#endif
		}
	} else if (family == 0x0002) {
		switch (subfamily) {
		case 0x0003:
			((IMCOMM *) handle)->max_profile_len = two_to_16(data + 14);
			((IMCOMM *) handle)->max_capabilities = two_to_16(data + 20);
			if (snac_set_location_info(handle) == IMCOMM_RET_ERROR)
				return IMCOMM_RET_ERROR;
			else
				return IMCOMM_RET_OK;
		case 0x0006:
			snac_get_user_info(handle, data + 10, len - 10);
			return IMCOMM_RET_OK;
		}
	} else if (family == 0x0003) {
		switch (subfamily) {
		case 0x0003:
			tlvlist = tlv_split(data + 10, len - 10, 3);
			{
				TLVLIST        *trav;
				for (trav = tlvlist; trav != NULL; trav = trav->next) {
					switch (trav->type) {
					case 0x01:
						if (trav->len >= 2)
							((IMCOMM *) handle)->max_buddylist_size =
								two_to_16(trav->value);
						break;
					case 0x02:
						if (trav->len >= 2)
							((IMCOMM *) handle)->max_num_watchers =
								two_to_16(trav->value);
						break;
					case 0x03:
						if (trav->len >= 2)
							((IMCOMM *) handle)->max_online_notifications =
								two_to_16(trav->value);
						break;
					}
				}
			}
			clear_tlv_list(tlvlist);
			return IMCOMM_RET_OK;
		case 0x0C:
			return snac_get_signoff(handle, data, len);
		case 0x0B:
			return snac_get_signon(handle, data, len);
		}
	} else if (family == 0x0004) {
		switch (subfamily) {
		case 0x0001:
			if (data[11] == 0x04) {
				if (((IMCOMM *) handle)->callbacks[IMCOMM_ERROR])
					((IMCOMM *) handle)->callbacks[IMCOMM_ERROR] (handle,
						 IMCOMM_ERROR_USER_OFFLINE);
			}
			return IMCOMM_RET_OK;
		case 0x0005:
			((IMCOMM *) handle)->max_message_size = two_to_16(data + 16);
			((IMCOMM *) handle)->max_sender_warning = two_to_16(data + 18);
			((IMCOMM *) handle)->max_receiver_warning =
				two_to_16(data + 20);
			((IMCOMM *) handle)->max_message_interval =
				two_to_16(data + 22);
			return snac_send_icbm_params(handle, two_to_16(data + 10),
						     1024);
		case 0x0007:
			return snac_get_incoming_im(handle, data, len);
		}
	} else if (family == 0x0009) {
		switch (subfamily) {
		case 0x0003:
			return snac_get_privacy_rights(handle, data, len);
		}
	} else if (family == 0x0013) {
		switch (subfamily) {
		case 0x0006:
			snac_ssi_get_list(handle, data, len);
			return snac_ssi_activate(handle);
		}
	}
	return IMCOMM_RET_OK;
}

/* PROTO */
void
snac_addToMulti(void *handle, struct MultiPacket * mpkt, uint16_t family, uint16_t subtype, unsigned char *data, uint16_t len, int updateidle)
{
	pkt_t          *snac_packet;

	snac_packet = pkt_init(len + 10);
	pkt_add16(snac_packet, family);
	pkt_add16(snac_packet, subtype);
	pkt_add16(snac_packet, 0);
	pkt_add32(snac_packet, ((IMCOMM *) handle)->snacreq);
	pkt_addraw(snac_packet, (uint8_t *) data, len);
	((IMCOMM *) handle)->snacreq++;

	flap_addToMulti(mpkt, 0x02, snac_packet->data, snac_packet->len,
			updateidle);
	pkt_free(snac_packet);
}

/* PROTO */
IMCOMM_RET
snac_send(void *handle, uint16_t family, uint16_t subtype, unsigned char *data, uint16_t len, int updateidle)
{
	pkt_t          *snac_packet;
	IMCOMM_RET      ret;

	if (((IMCOMM *) handle)->srv_pause)
		return IMCOMM_RET_OK;

	snac_packet = pkt_init(len + 10);
	pkt_add16(snac_packet, family);
	pkt_add16(snac_packet, subtype);
	pkt_add16(snac_packet, 0);
	pkt_add32(snac_packet, ((IMCOMM *) handle)->snacreq);
	pkt_addraw(snac_packet, (uint8_t *) data, len);
	((IMCOMM *) handle)->snacreq++;

	ret = flap_sendpkt(handle, 0x02, snac_packet, updateidle);
	pkt_free(snac_packet);
	return ret;
}

/* PROTO */
IMCOMM_RET
snac_sendpkt(void *handle, uint8_t family, uint8_t subtype, pkt_t * pkt, int updateidle)
{
	return snac_send(handle, family, subtype, pkt->data,
			 (uint16_t) pkt->len, updateidle);
}

/* PROTO */
IMCOMM_RET
snac_send_versions(void *handle)
{
	/*
         * I love it when I get really lazy...
         *
         * This works...
         */
	unsigned char   packet[] =
	{0x00, 0x13, 0x00, 0x03, 0x00, 0x02, 0x00, 0x01, 0x00, 0x03, 0x00,
		0x01, 0x00, 0x04, 0x00, 0x01, 0x00, 0x06, 0x00, 0x01, 0x00, 0x08, 0x00, 0x01, 0x00,
		0x09, 0x00, 0x01, 0x00, 0x0A, 0x00, 0x01, 0x00, 0x0B, 0x00, 0x01, 0x00, 0x0C, 0x00,
	0x01, 0x00, 0x01, 0x00, 0x04};

	return snac_send(handle, 0x01, 0x17, packet, (uint16_t) sizeof(packet),
			 0);
}

/* PROTO */
IMCOMM_RET
snac_request_limits(void *handle)
{
	return snac_send(handle, 0x01, 0x06, NULL, 0, 0);
}

/* PROTO */
IMCOMM_RET
snac_ack_srv_pause(void *handle, uint8_t * data, size_t len)
{
	pkt_t          *ackpkt;
	struct IMComm_Families *tr;
	int             count;

	for (count = 0, tr = ((IMCOMM *) handle)->families; tr; tr = tr->next)
		count++;

	ackpkt = pkt_init(count * 2);	/* each family is 16 bits */

	for (tr = ((IMCOMM *) handle)->families; tr; tr = tr->next) {
		pkt_add16(ackpkt, tr->family);
		printf("adding family %d (total %d).\n", tr->family, count);
	}

	((IMCOMM *) handle)->srv_pause = 1;

	/* return snac_send(handle, 0x01, 0x0c, data, len, 0); */
	snac_sendpkt(handle, 0x01, 0x0c, ackpkt, 0);

	pkt_free(ackpkt);

	return IMCOMM_RET_OK;
}

/* PROTO */
IMCOMM_RET
snac_get_srv_families(void *handle, uint8_t * data, size_t len)
{
	struct IMComm_Families *lastptr;
	pkt_t          *inpkt = pkt_initP(data, len);
	int             num_families = 0;

	if (pkt_end(inpkt))
		return IMCOMM_RET_ERROR;

	((IMCOMM *) handle)->families = malloc(sizeof(struct IMComm_Families));
	((IMCOMM *) handle)->families->family = pkt_get16(inpkt);
	num_families++;

	lastptr = ((IMCOMM *) handle)->families;
	lastptr->next = NULL;

	while (!pkt_end(inpkt)) {
		lastptr->next = malloc(sizeof(struct IMComm_Families));
		lastptr->next->family = pkt_get16(inpkt);
		lastptr->next->next = NULL;
		lastptr = lastptr->next;
		num_families++;
	}

	pkt_freeP(inpkt);

	((IMCOMM *) handle)->num_families = num_families;

	return IMCOMM_RET_OK;
}

/* PROTO */
IMCOMM_RET
snac_ack_limits(void *handle, unsigned char *data, size_t len)
{
	pkt_t          *pkt;
	uint16_t        x, y, num_classes, *ids;

	num_classes = two_to_16(data);
	ids = malloc(num_classes * 2);	/* it's a 16 bit type, so we need
					 * twice the bytes. Thanks to djgpp
					 * for catching this bug! UNIX/win32
					 * let it go. */


	for (x = 2, y = 0; y < num_classes; y++) {
		ids[y] = two_to_16(data + x);
		x += 35;
	}

	pkt = pkt_init(num_classes * 2);

	for (y = 0; y < num_classes; y++)
		pkt_add16(pkt, ids[y]);

	if (snac_sendpkt(handle, 0x01, 0x08, pkt, 0) != IMCOMM_RET_OK) {
		pkt_free(pkt);
		free(ids);
		return IMCOMM_RET_ERROR;
	}
	pkt_free(pkt);
	free(ids);

	return snac_multireq(handle);
}

/* PROTO */
IMCOMM_RET
snac_multireq(void *handle)
{
	struct MultiPacket *mpkt;

	mpkt = MultiPktInit();

	snac_addToMulti(handle, mpkt, 0x01, 0x0e, NULL, 0, 0);
	snac_addToMulti(handle, mpkt, 0x13, 0x02, NULL, 0, 0);
	snac_addToMulti(handle, mpkt, 0x13, 0x04, NULL, 0, 0);
	snac_addToMulti(handle, mpkt, 0x02, 0x02, NULL, 0, 0);
	snac_addToMulti(handle, mpkt, 0x03, 0x02, NULL, 0, 0);
	snac_addToMulti(handle, mpkt, 0x04, 0x04, NULL, 0, 0);
	snac_addToMulti(handle, mpkt, 0x09, 0x02, NULL, 0, 0);

	flap_sendMulti(handle, mpkt);

	MultiPktFree(mpkt);
	return IMCOMM_RET_OK;
}

/* PROTO */
IMCOMM_RET
snac_get_privacy_rights(void *handle, uint8_t * data, uint16_t len)
{
	TLVLIST        *tlvlist, *trav;

	tlvlist = tlv_split(data + 10, len - 10, 2);

	for (trav = tlvlist; trav != NULL; trav = trav->next) {
		if (trav->type == 0x0001) {
			((IMCOMM *) handle)->max_visible_list_size =
				two_to_16(trav->value);
		} else if (trav->type == 0x0002) {
			((IMCOMM *) handle)->max_invisible_list_size =
				two_to_16(trav->value);
		}
	}

	clear_tlv_list(tlvlist);

	return IMCOMM_RET_OK;
}

/* PROTO */
IMCOMM_RET
snac_set_location_info(void *handle)
{
	pkt_t          *packet;
	char           *idstr = "text/aolrtf; charset=\"iso-8859-1\"";
	int             pktlen = 0;
	uint8_t         capabilities[] =
	{0x09, 0x46, 0x13, 0x46, 0x4c, 0x7f, 0x11, 0xd1, 0x82, 0x22, 0x44,
		0x45, 0x53, 0x54,
		0x00, 0x00,
		0x09, 0x46, 0x13, 0x4d, 0x4c, 0x7f, 0x11, 0xd1, 0x82, 0x22, 0x44,
		0x45, 0x53, 0x54, 0x00, 0x00,
		0x09, 0x46, 0x00, 0x00, 0x4c, 0x7f, 0x11, 0xd1, 0x82, 0x22, 0x44,
		0x45, 0x53, 0x54, 0x00, 0x00
	};

	snac_send_cli_update(handle);

	if (((IMCOMM *) handle)->profile_str != NULL)
		pktlen +=
			8 + strlen(idstr) +
			strlen((char *) ((IMCOMM *) handle)->profile_str);
	if (((IMCOMM *) handle)->away_msg != NULL)
		pktlen += strlen((char *) ((IMCOMM *) handle)->away_msg);

	packet =
		pkt_init(pktlen + 10 + 8 + 4 + strlen(idstr) +
			 sizeof(capabilities));

	pkt_add16(packet, 0x0001);
	pkt_add16(packet, (uint16_t) strlen(idstr));
	pkt_addraw(packet, (uint8_t *) idstr, strlen(idstr));

	if (((IMCOMM *) handle)->profile_str != NULL) {
		pkt_add16(packet, 0x0002);
		pkt_add16(packet,
			  (uint16_t) strlen((char *) ((IMCOMM *) handle)->
					    profile_str));
		pkt_addraw(packet, ((IMCOMM *) handle)->profile_str,
			 strlen((char *) ((IMCOMM *) handle)->profile_str));
	}
	pkt_add16(packet, 0x0003);
	pkt_add16(packet, (uint16_t) strlen(idstr));
	pkt_addraw(packet, (uint8_t *) idstr, strlen(idstr));
	pkt_add16(packet, 0x0004);

	if (((IMCOMM *) handle)->away_msg != NULL) {
		pkt_add16(packet,
			  (uint16_t) strlen((char *) ((IMCOMM *) handle)->
					    away_msg));
		pkt_addraw(packet, ((IMCOMM *) handle)->away_msg,
			   (uint16_t) strlen((char *) ((IMCOMM *) handle)->
					     away_msg));
	} else {
		pkt_add16(packet, 0x0000);
	}

	pkt_add16(packet, 0x0005);
	pkt_add16(packet, (uint16_t) sizeof(capabilities));
	pkt_addraw(packet, capabilities, sizeof(capabilities));
	pkt_add16(packet, 0x0006);
	pkt_add16(packet, 0x0006);
	pkt_add16(packet, 0x0004);
	pkt_add32(packet, 0x00020002);

	snac_sendpkt(handle, 0x02, 0x04, packet, 0);
	pkt_free(packet);
	return IMCOMM_RET_OK;
}

/* PROTO */
IMCOMM_RET
snac_send_icbm_params(void *handle, uint16_t channel, uint16_t max_msg_size)
{
	IMCOMM_RET      ret;
	pkt_t          *packet;

	packet = pkt_init(16);
	pkt_add32(packet, 0x00000000);
	pkt_add16(packet, 0x000b);
	pkt_add16(packet, 0x1f40);
	pkt_add16(packet, ((IMCOMM *) handle)->max_sender_warning);
	pkt_add16(packet, ((IMCOMM *) handle)->max_receiver_warning);
	pkt_add32(packet, 0x00000000);

	ret = snac_sendpkt(handle, 0x04, 0x02, packet, 0);
	pkt_free(packet);
	return ret;
}

/* PROTO */
IMCOMM_RET
snac_ssi_activate(void *handle)
{
	if (!((IMCOMM *) handle)->connected) {
		((IMCOMM *) handle)->connected = 1;
		return multi_ssiact_cliready(handle);
	} else {
		return IMCOMM_RET_OK;
	}
}

/* PROTO */
IMCOMM_RET
snac_ssi_get_list(void *handle, uint8_t * data, uint16_t len)
{
	uint8_t         version, *name;
	uint16_t        x, count, length, group_id, id, type, tlvlen;
	pkt_t          *pkt;

	pkt = pkt_initP(data + 10, len - 10);

	version = pkt_get8(pkt);
	count = pkt_get16(pkt);

	for (x = 0; x < count; x++) {
		length = pkt_get16(pkt);
		if (length > 0) {
			name = pkt_getstr(pkt, length);
		} else {
			name = NULL;
		}

		group_id = pkt_get16(pkt);
		id = pkt_get16(pkt);
		type = pkt_get16(pkt);
		tlvlen = pkt_get16(pkt);
		pkt_skip(pkt, tlvlen);

		if (type == 0x0000 && name != NULL) {
			imcomm_addtobuddylist(handle, (char *) name, id, group_id);
		}
		free(name);
	}

	pkt_freeP(pkt);
	return IMCOMM_RET_OK;
}



/* PROTO */
IMCOMM_RET
snac_send_cli_update(void *handle)
{
	IMCOMM_RET      ret;
	pkt_t          *pkt;

	/*
         * This makes the server send us extended status information
         */
#if 0
	pkt = pkt_init(12);
	pkt_add16(pkt, 0x001d);
	pkt_add16(pkt, 0x0008);
	pkt_add32(pkt, 0x00020404);
	pkt_add32(pkt, 0x00000000);
#endif

	pkt = pkt_init(8);
	pkt_add16(pkt, 0x0006);
	pkt_add16(pkt, 0x0004);
	if (((IMCOMM *) handle)->isinvisible)
		pkt_add32(pkt, 0x00000100);
	else
		pkt_add32(pkt, 0x00000000);

	ret = snac_sendpkt(handle, 0x01, 0x1e, pkt, 0);
	pkt_free(pkt);
	return ret;
}

/* PROTO */
IMCOMM_RET
multi_ssiact_cliready(void *handle)
{
	struct MultiPacket *multipkt;
	pkt_t          *cliupdpkt;
	unsigned char   cli_ready_packet[] = {0x00,
		0x13, 0x00, 0x03, 0x01, 0x10, 0x08, 0xE5, 0x00, 0x0b, 0x00, 0x01,
		0x01, 0x10, 0x08, 0xe5, 0x00, 0x0a, 0x00, 0x01, 0x01, 0x10,
		0x08, 0xe5, 0x00, 0x09, 0x00, 0x01, 0x01, 0x10, 0x08, 0xe5,
		0x00, 0x08, 0x00, 0x01, 0x01, 0x04, 0x00, 0x01, 0x00, 0x06,
		0x00, 0x01, 0x01, 0x10, 0x08, 0xe5, 0x00, 0x04, 0x00, 0x01,
		0x01, 0x10, 0x08, 0xe5, 0x00, 0x03, 0x00, 0x01, 0x01, 0x10,
		0x08, 0xe5, 0x00, 0x02, 0x00, 0x01, 0x01, 0x10, 0x08, 0xe5,
		0x00, 0x01, 0x00, 0x04, 0x01, 0x10, 0x08, 0xe5
	};

	multipkt = MultiPktInit();

	cliupdpkt = pkt_init(12);
	pkt_add16(cliupdpkt, 0x001d);
	pkt_add16(cliupdpkt, 0x0008);
	pkt_add32(cliupdpkt, 0x00020404);
	pkt_add32(cliupdpkt, 0x00000000);

	snac_addToMulti(handle, multipkt, 0x01, 0x1e, cliupdpkt->data,
			cliupdpkt->len, 0);
	snac_addToMulti(handle, multipkt, 0x01, 0x02, cli_ready_packet,
			sizeof(cli_ready_packet), 0);
	snac_addToMulti(handle, multipkt, 0x13, 0x07, NULL, 0, 0);

	flap_sendMulti(handle, multipkt);
	MultiPktFree(multipkt);
	pkt_free(cliupdpkt);

	if (((IMCOMM *) handle)->callbacks[IMCOMM_ERROR])
		((IMCOMM *) handle)->callbacks[IMCOMM_ERROR] (handle,
						   IMCOMM_STATUS_CONNECTED);

	return IMCOMM_RET_OK;
}

/* PROTO */
IMCOMM_RET
snac_send_cli_ready(void *handle)
{
	unsigned char   packet[] =
	{0x00, 0x01, 0x00, 0x04, 0x01, 0x10, 0x08, 0xf1, 0x00, 0x13, 0x00,
		0x03,
		0x01, 0x10, 0x08, 0xf1,
		0x00, 0x02, 0x00, 0x01, 0x01, 0x10, 0x08, 0xf1, 0x00, 0x03, 0x00,
		0x01,
		0x01, 0x10, 0x08, 0xf1,
		0x00, 0x08, 0x00, 0x01, 0x01, 0x04, 0x00, 0x01, 0x00, 0x04, 0x00,
		0x01,
		0x01, 0x10, 0x08, 0xf1,
		0x00, 0x06, 0x00, 0x01, 0x01, 0x10, 0x08, 0xf1, 0x00, 0x09, 0x00,
		0x01,
		0x01, 0x10, 0x08, 0xf1,
		0x00, 0x0A, 0x00, 0x01, 0x01, 0x10, 0x08, 0xf1, 0x00, 0x0B, 0x00,
		0x01,
		0x01, 0x10, 0x08, 0xf1
	};
	snac_send(handle, 0x01, 0x02, packet, sizeof(packet), 0);
	if (((IMCOMM *) handle)->callbacks[IMCOMM_ERROR])
		((IMCOMM *) handle)->callbacks[IMCOMM_ERROR] (handle,
						   IMCOMM_STATUS_CONNECTED);

	return IMCOMM_RET_OK;
}

/* PROTO */
size_t
count_tlv(unsigned char *data, size_t len)
{
	pkt_t          *tp;
	size_t          num;

	tp = pkt_initP(data, len);

	num = count_tlv_pkt(tp);
	pkt_freeP(tp);

	return num;
}

/* PROTO */
size_t
count_tlv_pkt(pkt_t * tp)
{
	size_t          num = 0;

	while (tp->offset < tp->len) {
		pkt_get16(tp);
		pkt_skip(tp, pkt_get16(tp));
		num++;
	}

	return num;
}

/* PROTO */
TLVLIST        *
tlv_split(unsigned char *data, size_t len, size_t numtlv)
{
	TLVLIST        *newtlv = 0, *tlvlist = 0, *firsttlv = 0;
	uint16_t        x = 0, counter = 0;
	int             firstTime = 1;

	while (x < len) {
		newtlv = malloc(sizeof(TLVLIST));
		newtlv->type = two_to_16(data + x);
		newtlv->len = two_to_16(data + x + 2);

		x += 4;
		if (newtlv->len != 0) {
			newtlv->value = malloc(newtlv->len);
			memcpy(newtlv->value, data + x, newtlv->len);
		}
		newtlv->next = NULL;
		if (firstTime) {
			tlvlist = newtlv;
			firsttlv = newtlv;
			firstTime = 0;
		} else {
			tlvlist->next = newtlv;
			tlvlist = tlvlist->next;
		}

		if (newtlv->len != 0) {
			x += newtlv->len;
			counter++;
		}
	}

#ifdef DEBUG
	for (tlvlist = firsttlv; tlvlist != NULL; tlvlist = tlvlist->next)
		printf("TLV type: %04X length: %04X\n", tlvlist->type,
		       tlvlist->len);
#endif

	return firsttlv;
}

/* PROTO */
void
clear_tlv_list(TLVLIST * tlvlist)
{
	TLVLIST        *temp, *trav = tlvlist;

	while (trav != NULL) {
		temp = trav;
		trav = trav->next;
		if (temp->len > 0)
			free(temp->value);
		free(temp);
	}
}

/* PROTO */
uint32_t
four_to_32(unsigned char *value)
{
	uint32_t        temp;

	memcpy(&temp, value, 4);

	if (endianness == HOST_LITTLE_ENDIAN)
		temp = BYTE_SWAP_32(temp);

	return temp;
}

/* PROTO */
uint16_t
two_to_16(unsigned char *value)
{
	uint16_t        temp;

	memcpy(&temp, value, 2);

	if (endianness == HOST_LITTLE_ENDIAN)
		temp = BYTE_SWAP_16(temp);

	return temp;
}

/* PROTO */
IMCOMM_RET
snac_get_incoming_im(void *handle, uint8_t * data, uint16_t len)
{
	char           *sn;
	uint8_t         sn_len = data[20];
	char           *msg;
	TLVLIST        *tlvlist, *trav, *trav2, *subtlv;
	int             auto_resp = 0;
	int             numtlv;


	numtlv = two_to_16(data + sn_len + 23);
	sn = malloc(sn_len + 1);
	memcpy(sn, data + 21, sn_len);
	sn[sn_len] = 0;

	tlvlist = tlv_split(data + 25 + sn_len, len - 25 - sn_len, numtlv + 1);
	for (trav = tlvlist; trav != NULL; trav = trav->next) {
		if (trav->type == 0x0004 && trav->len == 0)
			auto_resp = 1;

		if (trav->type == 0x0003) {
			imcomm_update_buddy_times(handle, sn, 2,
						  four_to_32(trav->value));
		}
	}

	if (!auto_resp)
		imcomm_update_buddy_times(handle, sn, 1, 0);

	for (trav = tlvlist; trav != NULL; trav = trav->next)
		if (trav->type == 0x0002)
			break;

	if (trav == NULL) {
		clear_tlv_list(tlvlist);
		free(sn);
		return IMCOMM_RET_ERROR;
	}
	subtlv = tlv_split(trav->value, trav->len, 2);
	for (trav2 = subtlv; trav2 != NULL; trav2 = trav2->next)
		if (trav2->type == 0x0101)
			break;

	if (trav2 == NULL) {
		clear_tlv_list(subtlv);
		clear_tlv_list(tlvlist);
		free(sn);
		return IMCOMM_RET_ERROR;
	}
	msg = malloc(trav2->len - 3);
	memcpy(msg, trav2->value + 4, trav2->len - 4);
	msg[trav2->len - 4] = 0;

	if (((IMCOMM *) handle)->callbacks[IMCOMM_IM_INCOMING])
		(((IMCOMM *) handle)->callbacks[IMCOMM_IM_INCOMING]) (handle, sn,
								  auto_resp,
								      msg);

	free(msg);
	clear_tlv_list(subtlv);
	clear_tlv_list(tlvlist);
	free(sn);
	return IMCOMM_RET_OK;
}

/* PROTO */
IMCOMM_RET
snac_get_signoff(void *handle, uint8_t * data, uint16_t len)
{
	char           *sn;
	int             sn_len = (int) data[10];

	sn = malloc(sn_len + 1);
	memcpy(sn, data + 11, sn_len);
	sn[sn_len] = 0;
	imcomm_internal_delete_buddy(handle, sn);

	if (((IMCOMM *) handle)->callbacks[IMCOMM_IM_SIGNOFF])
		((IMCOMM *) handle)->callbacks[IMCOMM_IM_SIGNOFF] (handle, sn);

	free(sn);
	return IMCOMM_RET_OK;
}

/* PROTO */
IMCOMM_RET
snac_get_signon(void *handle, uint8_t * data, uint16_t len)
{
	char           *sn;
	int             sn_len = (int) data[10];
	int             numtlv = two_to_16(data + 13 + sn_len);
	unsigned long   idletime = 0;
	unsigned long   onlinetime = 0;
	int             isaway = 0;
	TLVLIST        *temp, *tlvlist;

	sn = malloc(sn_len + 1);
	memcpy(sn, data + 11, sn_len);
	sn[sn_len] = 0;
	tlvlist =
		tlv_split(data + 11 + sn_len + 4, len - 11 - sn_len - 4, numtlv);

	for (temp = tlvlist; temp != NULL; temp = temp->next) {
		if (temp->type == 0x0004) {
			idletime = two_to_16(temp->value);
		}
		if (temp->type == 0x0001) {
			if (temp->value[1] == 0x30 || temp->value[1] == 0x31
			    || temp->value[1] == 0x24)
				isaway = 1;
		}
		if (temp->type == 0x0003) {
			onlinetime = four_to_32(temp->value);
			imcomm_update_buddy_times(handle, sn, 2, onlinetime);
		}
	}
	if (imcomm_internal_add_buddy(handle, sn, idletime, onlinetime, isaway)
	    == 0) {
		imcomm_update_buddy_times(handle, sn, 1, idletime);
		imcomm_update_buddy_away(handle, sn, isaway);
	} else {
		if (((IMCOMM *) handle)->callbacks[IMCOMM_IM_SIGNON])
			((IMCOMM *) handle)->callbacks[IMCOMM_IM_SIGNON] (handle, sn);
		if (((IMCOMM *) handle)->callbacks[IMCOMM_IM_IDLEINFO])
			((IMCOMM *) handle)->callbacks[IMCOMM_IM_IDLEINFO] (handle, sn,
								  idletime);
		if (isaway)
			if (((IMCOMM *) handle)->callbacks[IMCOMM_IM_BUDDYAWAY])
				((IMCOMM *) handle)->
					callbacks[IMCOMM_IM_BUDDYAWAY] (handle, sn);
	}
	clear_tlv_list(tlvlist);

	free(sn);
	return IMCOMM_RET_OK;
}


/* PROTO */
void
snac_get_user_info(void *handle, uint8_t * data, uint16_t len)
{
	int             sn_len = (int) data[0];
	char           *sn;
	char           *profilestr = NULL;
	char           *awaymsg = NULL;
	TLVLIST        *tlvlist, *trav;

	sn = malloc(sn_len + 1);
	memcpy(sn, data + 1, sn_len);
	sn[sn_len] = 0;

	tlvlist =
		tlv_split(data + 5 + sn_len, len - 5 - sn_len,
			  data[sn_len + 4] + 2);

	if (tlvlist == NULL) {
		free(sn);
		return;
	}
	if (tlvlist->next == NULL) {
		free(sn);
		clear_tlv_list(tlvlist);
		return;
	}
	for (trav = tlvlist->next->next; trav != NULL; trav = trav->next) {
		switch (trav->type) {
		case 0x02:
			profilestr = malloc(trav->len + 1);
			strncpy(profilestr, (char *) trav->value, trav->len);
			profilestr[trav->len] = 0;
			break;
		case 0x04:
			if (trav->len == 2)	/* idle time? */
				break;

			awaymsg = malloc(trav->len + 1);
			strncpy(awaymsg, (char *) trav->value, trav->len);
			awaymsg[trav->len] = 0;
			break;
		}
	}

	if (profilestr != NULL) {
		if (((IMCOMM *) handle)->callbacks[IMCOMM_IM_PROFILE]) {
			((IMCOMM *) handle)->callbacks[IMCOMM_IM_PROFILE] (handle, sn,
								profilestr);
			free(profilestr);
		}
	} else if (awaymsg != NULL) {
		if (((IMCOMM *) handle)->callbacks[IMCOMM_IM_AWAYMSG]) {
			((IMCOMM *) handle)->callbacks[IMCOMM_IM_AWAYMSG] (handle, sn,
								   awaymsg);
			free(awaymsg);
		}
	}
	free(sn);
	clear_tlv_list(tlvlist);
}

/* PROTO */
void
imcomm_im_send_message(void *handle, const char *whom, const char *msg, int automsg)
{
	pkt_t          *packet;

	packet = pkt_init(28 + strlen(whom) + strlen(msg) + (4 * automsg));

	/*
         * Message cookie?
         *
         * Putting something random seems to work.
         */
	pkt_add32(packet, 0x01020304);
	pkt_add32(packet, 0x05060708);

	pkt_add16(packet, 0x0001);
	pkt_add8(packet, (uint8_t) strlen(whom));
	pkt_addraw(packet, (uint8_t *) whom, strlen(whom));
	pkt_add16(packet, 0x0002);
	pkt_add16(packet, (uint16_t) (uint16_t) (strlen(msg) + 13));
	pkt_add32(packet, 0x05010001);
	pkt_add8(packet, 0x01);
	pkt_add16(packet, 0x0101);
	pkt_add16(packet, (uint16_t) (uint16_t) (strlen(msg) + 4));
	pkt_add32(packet, 0x00000000);
	pkt_addraw(packet, (uint8_t *) msg, strlen(msg));

	if (automsg) {
		pkt_add16(packet, 0x0004);
		pkt_add16(packet, 0x0000);
	}
	snac_sendpkt(handle, 0x04, 0x06, packet, (automsg ? 0 : 1));
	pkt_free(packet);
}

/* PROTO */
void
imcomm_request_awayprofile(void *handle, char *sn)
{
	pkt_t          *packet;
	struct MultiPacket *mpkt;

	mpkt = MultiPktInit();

	packet = pkt_init(3 + (uint8_t) strlen(sn));

	pkt_add16(packet, 0x0003);
	pkt_add8(packet, (uint8_t) strlen(sn));
	pkt_addraw(packet, (uint8_t *) sn, strlen(sn));

	snac_addToMulti(handle, mpkt, 0x02, 0x05, packet->data, packet->len,
			0);

	pkt_free(packet);

	packet = pkt_init(3 + (uint8_t) strlen(sn));

	pkt_add16(packet, 0x0001);
	pkt_add8(packet, (uint8_t) strlen(sn));
	pkt_addraw(packet, (uint8_t *) sn, strlen(sn));

	snac_addToMulti(handle, mpkt, 0x02, 0x05, packet->data, packet->len,
			0);
	pkt_free(packet);

	flap_sendMulti(handle, mpkt);
	MultiPktFree(mpkt);
}

/* PROTO */
void
imcomm_request_profile(void *handle, char *sn)
{
	pkt_t          *packet;

	packet = pkt_init(3 + (uint8_t) strlen(sn));

	pkt_add16(packet, 0x0001);
	pkt_add8(packet, (uint8_t) strlen(sn));
	pkt_addraw(packet, (uint8_t *) sn, strlen(sn));

	snac_sendpkt(handle, 0x02, 0x05, packet, 0);
	pkt_free(packet);
}

/* PROTO */
void
imcomm_request_awaymsg(void *handle, char *sn)
{
	pkt_t          *packet;

	packet = pkt_init(3 + (uint8_t) strlen(sn));

	pkt_add16(packet, 0x0003);
	pkt_add8(packet, (uint8_t) strlen(sn));
	pkt_addraw(packet, (uint8_t *) sn, strlen(sn));

	snac_sendpkt(handle, 0x02, 0x05, packet, 0);
	pkt_free(packet);
}

/* PROTO */
void
imcomm_request_massawaymsg(void *handle, char **sns, int num)
{
	pkt_t          *packet;
	int             x;
	struct MultiPacket *multipkt;

	multipkt = MultiPktInit();

	for (x = 0; x < num; x++) {
		packet = pkt_init(3 + (uint8_t) strlen(sns[x]));

		pkt_add16(packet, 0x0003);
		pkt_add8(packet, (uint8_t) strlen(sns[x]));
		pkt_addraw(packet, (uint8_t *) sns[x], strlen(sns[x]));

		snac_addToMulti(handle, multipkt, 0x02, 0x05, packet->data,
				(uint16_t) packet->len, 0);
		pkt_free(packet);
	}

	flap_sendMulti(handle, multipkt);
	MultiPktFree(multipkt);
}

/* PROTO */
void
imcomm_addtobuddylist(void *handle, char *sn, uint16_t id, uint16_t group_id)
{
	IMCOMM_BUDDYLIST *temp, *trav;
	char           *sname;

	sname = imcomm_simplify_sn(sn);

	for (trav = ((IMCOMM *) handle)->buddylist; trav != NULL;
	     trav = trav->next) {
		if (strcmp(sname, trav->sn) == 0) {
			free(sname);
			return;
		}
	}

	temp = malloc(sizeof(IMCOMM_BUDDYLIST));
	temp->sn = sname;
	temp->formattedsn = strdup(sn);
	temp->ssi_id = id;
	temp->group_id = group_id;
	temp->next = NULL;

	if (((IMCOMM *) handle)->buddylist == NULL)
		((IMCOMM *) handle)->buddylist = temp;
	else {
		for (trav = ((IMCOMM *) handle)->buddylist; trav->next != NULL;
		     trav = trav->next);

		trav->next = temp;
	}
}

/* PROTO */
uint16_t
imcomm_get_next_id(void *handle)
{
	IMCOMM_BUDDYLIST *trav;
	uint16_t        firstid = 0x0001;
	uint8_t         found;

	/* this is supremely gross */

	while (1) {
		found = 0;

		for (trav = ((IMCOMM *) handle)->buddylist; trav != NULL;
		     trav = trav->next) {
			if (trav->ssi_id == firstid) {
				found = 1;
				break;
			}
		}

		if (found == 1) {
			firstid++;
		} else {
			break;
		}
	}

	return firstid;
}

/* PROTO */
void
imcomm_im_add_buddy(void *handle, char *sn)
{
	IMCOMM_BUDDYLIST *temp, *trav;
	struct MultiPacket *mpkt;
	pkt_t          *packet;
	char           *sname;
	uint16_t        nextid;

	sname = imcomm_simplify_sn(sn);

	for (trav = ((IMCOMM *) handle)->buddylist; trav != NULL;
	     trav = trav->next) {
		if (strcmp(sname, trav->sn) == 0) {
			free(sname);
			return;
		}
	}

	nextid = imcomm_get_next_id(handle);

	temp = malloc(sizeof(IMCOMM_BUDDYLIST));
	temp->sn = sname;
	temp->formattedsn = strdup(sname);
	temp->ssi_id = nextid;
	temp->next = NULL;

	if (((IMCOMM *) handle)->buddylist == NULL)
		((IMCOMM *) handle)->buddylist = temp;
	else {
		for (trav = ((IMCOMM *) handle)->buddylist; trav->next != NULL;
		     trav = trav->next);

		trav->next = temp;
	}

	mpkt = MultiPktInit();

	snac_addToMulti(handle, mpkt, 0x13, 0x11, NULL, 0, 0);
	packet = pkt_init(10 + strlen(sname));
	pkt_add16(packet, (uint16_t) strlen(sname));
	pkt_addraw(packet, (unsigned char *) sname, strlen(sname));
	pkt_add16(packet, 0x0001);
	pkt_add16(packet, nextid);
	pkt_add16(packet, 0x0000);
	pkt_add16(packet, 0x0000);
	snac_addToMulti(handle, mpkt, 0x13, 0x08, packet->data, packet->len,
			0);
	pkt_free(packet);

	snac_addToMulti(handle, mpkt, 0x13, 0x012, NULL, 0, 0);

	flap_sendMulti(handle, mpkt);
	MultiPktFree(mpkt);
}

/* PROTO */
void
imcomm_im_remove_buddy(void *handle, const char *sn)
{
	char           *sname, *snsend = NULL;
	pkt_t          *packet;
	IMCOMM_BUDDYLIST *temp, *trav;
	struct MultiPacket *mpkt;
	uint16_t        buddy_id = 0x00, buddy_group_id = 0x00;

	sname = imcomm_simplify_sn(sn);

	if (((IMCOMM *) handle)->buddylist != NULL) {
		if (strcmp(((IMCOMM *) handle)->buddylist->sn, sname) == 0) {
			temp = ((IMCOMM *) handle)->buddylist;
			((IMCOMM *) handle)->buddylist =
				((IMCOMM *) handle)->buddylist->next;

			buddy_id = temp->ssi_id;
			buddy_group_id = temp->group_id;
			snsend = temp->formattedsn;
			free(temp->sn);
			free(temp);
		} else {
			for (trav = ((IMCOMM *) handle)->buddylist; trav->next != NULL;
			     trav = trav->next) {
				if (strcmp(trav->next->sn, sname) == 0) {
					temp = trav->next;
					trav->next = trav->next->next;

					buddy_id = temp->ssi_id;
					buddy_group_id = temp->group_id;
					snsend = temp->formattedsn;
					free(temp->sn);
					free(temp);
					break;
				}
			}
		}
	}
	imcomm_internal_delete_buddy(handle, sn);

	if (snsend == NULL) {
		free(sname);
		return;
	}
	/*
         * packet = malloc(strlen(sname) + 1); packet[0] = strlen(sname);
         * memcpy(packet + 1, sname, strlen(sname));
         *
         * snac_send(handle, 0x03, 0x05, packet, (uint16_t) (strlen(sname) + 1),
         * 0); free(packet);
         */

	mpkt = MultiPktInit();

	snac_addToMulti(handle, mpkt, 0x13, 0x11, NULL, 0, 0);
	packet = pkt_init(10 + strlen(snsend));
	pkt_add16(packet, (uint16_t) strlen(snsend));
	pkt_addraw(packet, (unsigned char *) snsend, strlen(snsend));
	pkt_add16(packet, buddy_group_id);
	pkt_add16(packet, buddy_id);
	pkt_add16(packet, 0x0000);
	pkt_add16(packet, 0x0000);
	snac_addToMulti(handle, mpkt, 0x13, 0x0a, packet->data, packet->len,
			0);
	pkt_free(packet);

	snac_addToMulti(handle, mpkt, 0x13, 0x012, NULL, 0, 0);

	flap_sendMulti(handle, mpkt);
	MultiPktFree(mpkt);
	free(sname);
	free(snsend);
}

/* PROTO */
void
snac_request_new_service(void *handle, uint16_t service)
{
	pkt_t          *pkt;

	pkt = pkt_init(2);
	pkt_add16(pkt, service);
	snac_sendpkt(handle, 0x01, 0x04, pkt, 0);
	pkt_free(pkt);
}

/* PROTO */
void
snac_new_subconnection(void *handle, unsigned char *data, uint16_t len)
{
	TLVLIST        *tv, *trav;
	uint16_t        subtype = 0, cookie_len = 0;
	void           *h;
	unsigned char  *cookie = 0, *server = 0, *serverport;

	tv = tlv_split(data, len, 3);

	for (trav = tv; trav != NULL; trav = trav->next) {
		if (trav->type == 0x000D) {
			subtype = two_to_16(trav->value);
		} else if (trav->type == 0x0005) {
			server = malloc((trav->len) + 1);
			memcpy(server, trav->value, trav->len);
			server[trav->len] = 0;
		} else if (trav->type == 0x0006) {
			cookie = malloc(trav->len);
			memcpy(cookie, trav->value, trav->len);
			cookie_len = trav->len;
		}
	}

	clear_tlv_list(tv);

	if (strchr((char *) server, ':') == NULL) {
		serverport = malloc(strlen((char *) server) + 7);
		snprintf((char *) serverport, strlen((char *) server) + 7, "%s:%d",
			 (char *) server, ((IMCOMM *) handle)->oscarport);
		free(server);
		server = serverport;
	}
#ifdef DEBUG
	printf
		("New connection info: Type %02X, server %s. Cookie len %d bytes.\n",
		 subtype, server, cookie_len);
#endif

	h = imcomm_create_child_handle(handle);
	bos_signon_phase2(h, server, cookie, cookie_len);

	free(server);
	free(cookie);

	return;
}

/* PROTO */
void
imcomm_upload_icon(void *handle, uint8_t * data, uint16_t icon_len)
{
	/* this is really badly broken */
	/* I'm disabling it for now */

	/*
         * snac_request_new_service(handle, 0x10); ((IMCOMM *)
         * handle)->icondata = malloc(icon_len); memcpy(((IMCOMM *)
         * handle)->icondata, data, icon_len); ((IMCOMM *) handle)->iconlen =
         * icon_len;
         */
}

/* PROTO */
void
snac_finish_buddy_icon(void *handle)
{
	/*
         * The icon data is contained in the parent's node
         */

	pkt_t          *newpkt;
	void           *pptr = ((IMCOMM *) handle)->parent;

	newpkt = pkt_init(4 + ((IMCOMM *) pptr)->iconlen);
	pkt_add16(newpkt, 0x0001);
	pkt_add16(newpkt, ((IMCOMM *) pptr)->iconlen);
	pkt_addraw(newpkt, ((IMCOMM *) pptr)->icondata,
		   ((IMCOMM *) pptr)->iconlen);

	snac_sendpkt(handle, 0x10, 0x02, newpkt, 0);
	pkt_free(newpkt);
}
