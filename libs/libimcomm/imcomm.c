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

IMCOMM_HANDLES *handles = NULL;
int             endianness;
int             nodes_to_delete = 0;

#ifdef MACINTOSH_CLASSIC
extern short    refnum;
#endif

/* PROTO */
void           *
imcomm_create_handle(void)
{
	IMCOMM         *handle;
	IMCOMM_HANDLES *tmp;
	int             xx;

	handle = malloc(sizeof(IMCOMM));
	handle->proxymode = PROXY_TYPE_NONE;
	handle->proxyserver = NULL;
	handle->proxyport = 0;

	handle->to_delete = 0;
	handle->ischild = 0;
	handle->seqnum = 0x1000;
	handle->snacreq = 0;
	handle->families = NULL;
	handle->num_families = 0;
	handle->buddylist = NULL;
	handle->buddies_online = NULL;
	handle->isidle = 0;
	handle->isinvisible = 0;
	handle->last_operation_time = time(NULL);
	handle->profile_str = NULL;
	handle->away_msg = NULL;
	handle->icondata = 0;
	handle->iconlen = 0;
	handle->socket = 0;
	handle->connected = 0;
	handle->srv_pause = 0;
	handle->data = NULL;
	handle->header_pos = 0;
	handle->oscarport = 5190;
#ifdef MD5_LOGIN
	handle->pw = NULL;
	handle->sn = NULL;
#endif

#ifdef IMCOMM_KEEPALIVE
	handle->last_keepalive_time = 0;
#endif
#ifdef SEND_QUEUES
	handle->s_queue = NULL;
#endif

#ifdef MACINTOSH_CLASSIC
	handle->readable = 0;
#endif

	endianness = getbyteorder();

	for (xx = 0; xx < NUM_CALLBACKS; xx++)
		handle->callbacks[xx] = NULL;

	if (handles == NULL) {
		handles = malloc(sizeof(IMCOMM_HANDLES));
		handles->handle = handle;
		handles->next = NULL;
	} else {
		tmp = handles;

		while (tmp->next != NULL)
			tmp = tmp->next;

		tmp->next = malloc(sizeof(IMCOMM_HANDLES));
		tmp->next->handle = handle;
		tmp->next->next = NULL;
	}
	return (void *) handle;
}

/* PROTO */
int
imcomm_delete_handle_now(void *vhandle)
{
	IMCOMM_HANDLES *tmp, *tr;

	if (handles == NULL)
		return 0;

	if (handles->handle == (IMCOMM *) vhandle) {
		tmp = handles;
		handles = handles->next;

		imcomm_delete_handle_only((void *) tmp->handle);
		free(tmp);
	} else {
		for (tr = handles; tr;) {
			if (tr->handle == (IMCOMM *) vhandle) {
				tmp = tr;
				tr = tr->next;

				imcomm_delete_handle_only((void *) tmp->handle);
				free(tmp);
				continue;
			}
			tr = tr->next;
		}

	}

	return 1;
}

/* PROTO */
int
imcomm_delete_handle(void *vhandle)
{
	((IMCOMM *) vhandle)->to_delete = 1;
	nodes_to_delete = 1;

	return 1;
}

/* PROTO */
int
imcomm_delete_handle_only(void *vhandle)
{
	IMCOMM         *handle = (IMCOMM *) vhandle;

	if (handle->buddylist)
		imcomm_delete_buddylist(handle->buddylist);
	if (handle->buddies_online)
		imcomm_delete_buddylist(handle->buddies_online);
	if (handle->families)
		imcomm_delete_familieslist(handle->families);

	if (handle->profile_str)
		free(handle->profile_str);
	if (handle->away_msg)
		free(handle->away_msg);
	if (handle->icondata)
		free(handle->icondata);
	if (handle->data)
		free(handle->data);
#ifdef MD5_LOGIN
	if (handle->sn)
		free(handle->sn);
	if (handle->pw)
		free(handle->pw);
#endif
	if (handle->proxyserver)
		free(handle->proxyserver);

	if (handle->socket != -1)
		shutdown(handle->socket, 0x02);

	free(vhandle);

	return 1;
}

/* PROTO */
void
imcomm_delete_buddylist(struct IMComm_BuddyList * buddylist)
{
	struct IMComm_BuddyList *tr, *tmp;

	for (tr = buddylist; tr;) {
		if (tr->sn)
			free(tr->sn);
		if (tr->formattedsn)
			free(tr->formattedsn);

		tmp = tr;
		tr = tr->next;

		free(tmp);
	}
}

/* PROTO */
void
imcomm_delete_familieslist(struct IMComm_Families * families)
{
	struct IMComm_Families *tr, *tmp;

	for (tr = families; tr;) {
		tmp = tr;
		tr = tr->next;
		free(tmp);
	}

}

/* PROTO */
void
imcomm_set_oscar_port(void *handle, uint16_t port)
{
	((IMCOMM *) handle)->oscarport = port;
}

/* PROTO */
void           *
imcomm_create_child_handle(void *parent)
{
	IMCOMM         *handle;
	IMCOMM_HANDLES *tmp;
	int             xx;

	handle = malloc(sizeof(IMCOMM));

	handle->ischild = 1;
	handle->proxymode = ((IMCOMM *) parent)->proxymode;
	handle->proxyserver = ((IMCOMM *) parent)->proxyserver;
	handle->proxyport = ((IMCOMM *) parent)->proxyport;
	handle->parent = parent;
	handle->to_delete = 0;
	handle->seqnum = 0x1000;
	handle->snacreq = 0;
	handle->families = NULL;
	handle->buddylist = NULL;
	handle->buddies_online = NULL;
	handle->isidle = 0;
	handle->last_operation_time = time(NULL);
	handle->profile_str = NULL;
	handle->away_msg = NULL;
	handle->icondata = 0;
	handle->iconlen = 0;
	handle->socket = 0;
	handle->connected = 0;
	handle->data = NULL;
	handle->header_pos = 0;
#ifdef IMCOMM_KEEPALIVE
	handle->last_keepalive_time = 0;
#endif
#ifdef SEND_QUEUES
	handle->s_queue = NULL;
#endif

#ifdef MACINTOSH_CLASSIC
	handle->readable = 0;
#endif

	for (xx = 0; xx < NUM_CALLBACKS; xx++)
		handle->callbacks[xx] = ((IMCOMM *) parent)->callbacks[xx];

	if (handles == NULL) {
		handles = malloc(sizeof(IMCOMM_HANDLES));
		handles->handle = handle;
		handles->next = NULL;
	} else {
		tmp = handles;

		while (tmp->next != NULL)
			tmp = tmp->next;

		tmp->next = malloc(sizeof(IMCOMM_HANDLES));
		tmp->next->handle = handle;
		tmp->next->next = NULL;
	}
	return (void *) handle;
}

/* PROTO */
void
remove_deleted_handles(void)
{
	IMCOMM_HANDLES *trav, *tmp;

	if (handles == NULL)
		return;

	if (handles->handle->to_delete == 1) {

		/*
	         * the last thing it'll do before deleting itself is to send
	         * a callback saying it's about to be deleted.
	         */

		if (handles->handle->callbacks[IMCOMM_HANDLE_DELETED])
			handles->handle->
				callbacks[IMCOMM_HANDLE_DELETED] ((void *) handles->
								  handle);

		tmp = handles;
		handles = handles->next;

		imcomm_delete_handle_only(tmp->handle);

		free(tmp);

		remove_deleted_handles();
	} else {
		for (trav = handles; trav->next;) {
			if (trav->next->handle->to_delete == 1) {
				tmp = trav->next;
				trav->next = trav->next->next;
				trav = trav->next;

				if (tmp->handle->callbacks[IMCOMM_HANDLE_DELETED])
					tmp->handle->
						callbacks[IMCOMM_HANDLE_DELETED] ((void *) tmp->
								    handle);

				imcomm_delete_handle_only(tmp->handle);
				free(tmp);
			}
		}
	}
}

/* PROTO */
void
imcomm_set_proxy(void *handle, int type, char *proxyserver, uint16_t proxyport)
{
	((IMCOMM *) handle)->proxymode = type;
	((IMCOMM *) handle)->proxyserver = strdup(proxyserver);
	((IMCOMM *) handle)->proxyport = proxyport;
}

/* PROTO */
void
imcomm_register_callback(void *handle, int event, void (*ptr) ())
{
	((IMCOMM *) handle)->callbacks[event] = ptr;
}

/* PROTO */
IMCOMM_RET
imcomm_select(int nfds, fd_set * readfds, fd_set * writefds, fd_set * exceptfds, struct timeval * timeout)
{
	IMCOMM_HANDLES *tmp;
	IMCOMM_RET      ret = IMCOMM_RET_OK;
	int             maxfd = nfds;
#ifdef FULL_PACKET_AT_ONCE
	int             bytesread;
#endif

	/*
         * trim the handle list
         */
	if (nodes_to_delete == 1) {
		remove_deleted_handles();
		nodes_to_delete = 0;
	}
	tmp = handles;

#ifdef MACINTOSH_CLASSIC
	TCPiopb         pb;
	OSErr           err;

	for (tmp = handles; tmp != NULL; tmp = tmp->next) {
		if (tmp->handle->readable == 0)
			continue;

		pb.ioCRefNum = refnum;
		pb.csCode = TCPStatus;
		pb.tcpStream = tmp->handle->s;
		pb.csParam.status.userDataPtr = (Ptr) tmp->handle;
		err = PBControlSync((ParmBlkPtr) & pb);
		if (err != noErr)
			continue;

		if (pb.csParam.status.amtUnreadData >= 6) {
			mactcp_recv(tmp->handle, (char *) tmp->handle->header, 6);
			tmp->handle->data_len = two_to_16(tmp->handle->header + 4);
			tmp->handle->data = malloc(tmp->handle->data_len);
			mactcp_recv(tmp->handle, (char *) tmp->handle->data,
				    tmp->handle->data_len);
			ret =
				flap_decode(tmp->handle, tmp->handle->header,
					    tmp->handle->data);
			tmp->handle->data = NULL;
		}
	}
#else
	while (tmp != NULL) {
		if ((int) tmp->handle->socket > maxfd)
			maxfd = tmp->handle->socket;
		if (tmp->handle->socket != -1)
			FD_SET(tmp->handle->socket, readfds);
		tmp = tmp->next;
	}

	if (select(maxfd + 1, readfds, writefds, exceptfds, timeout) == -1)
		return IMCOMM_RET_ERROR;

	for (tmp = handles; tmp; tmp = tmp->next) {
		if (tmp->handle->socket == -1)
			continue;

#ifdef SEND_QUEUES
		if (tmp->handle->s_queue != NULL)
			flap_sendnext(tmp->handle);
#endif

		if (FD_ISSET(tmp->handle->socket, readfds)) {
#ifdef FULL_PACKET_AT_ONCE
			bytesread = 0;
			do {
				if ((bytesread +=
				     recv(tmp->handle->socket,
					  tmp->handle->header + bytesread, 6 - bytesread,
					  0)) <= 0) {
					shutdown(tmp->handle->socket, 0x02);
					tmp->handle->socket = -1;
					tmp->handle->data = NULL;
					tmp->handle->connected = 0;
					if (tmp->handle->callbacks[IMCOMM_ERROR])
						tmp->handle->callbacks[IMCOMM_ERROR] (tmp->handle,
						 IMCOMM_ERROR_DISCONNECTED);
					return IMCOMM_RET_ERROR;
				}
			} while (bytesread < 6);

			tmp->handle->data_len = two_to_16(tmp->handle->header + 4);
			tmp->handle->data = malloc(tmp->handle->data_len);
			bytesread = 0;
			do {
				if ((bytesread +=
				     recv(tmp->handle->socket,
					  tmp->handle->data + bytesread,
					  tmp->handle->data_len - bytesread, 0)) <= 0) {
					shutdown(tmp->handle->socket, 0x02);
					tmp->handle->socket = -1;
					tmp->handle->connected = 0;
					tmp->handle->data = NULL;
					if (tmp->handle->callbacks[IMCOMM_ERROR])
						tmp->handle->callbacks[IMCOMM_ERROR] (tmp->handle,
						 IMCOMM_ERROR_DISCONNECTED);
					return IMCOMM_RET_ERROR;
				}
			} while (bytesread < tmp->handle->data_len);

			ret =
				flap_decode(tmp->handle, tmp->handle->header,
					    tmp->handle->data);
			tmp->handle->data = NULL;
#else
			if (tmp->handle->header_pos < 6) {
				if (recv
				    (tmp->handle->socket,
				     &tmp->handle->header[tmp->handle->header_pos], 1,
				     0) <= 0) {
					shutdown(tmp->handle->socket, 0x02);
					tmp->handle->socket = -1;
					tmp->handle->connected = 0;
					tmp->handle->header_pos = 0;
					tmp->handle->data = NULL;
					if (tmp->handle->callbacks[IMCOMM_ERROR])
						tmp->handle->callbacks[IMCOMM_ERROR] (tmp->handle,
						 IMCOMM_ERROR_DISCONNECTED);
					return IMCOMM_RET_ERROR;
				}
				tmp->handle->header_pos++;
				if (tmp->handle->header_pos == 6) {
					tmp->handle->data_len =
						two_to_16(tmp->handle->header + 4);
					tmp->handle->data = malloc(tmp->handle->data_len);
					tmp->handle->data_pos = 0;
				}
			} else {
				if (recv
				    (tmp->handle->socket,
				&tmp->handle->data[tmp->handle->data_pos], 1,
				     0) <= 0) {
					free(tmp->handle->data);
					tmp->handle->data = NULL;
					shutdown(tmp->handle->socket, 0x02);
					tmp->handle->socket = -1;
					tmp->handle->connected = 0;
					tmp->handle->header_pos = 0;
					if (tmp->handle->callbacks[IMCOMM_ERROR])
						tmp->handle->callbacks[IMCOMM_ERROR] (tmp->handle,
						 IMCOMM_ERROR_DISCONNECTED);
					return IMCOMM_RET_ERROR;
				}
				tmp->handle->data_pos++;
				if (tmp->handle->data_pos == tmp->handle->data_len) {
					ret =
						flap_decode(tmp->handle, tmp->handle->header,
							 tmp->handle->data);
					tmp->handle->data = NULL;
					tmp->handle->header_pos = 0;
				}
			}
#endif
		}
		/*
	         * This seems to slow down the DOS port on slower machines,
	         * so let's get rid of it...
	         */
#ifdef IMCOMM_KEEPALIVE
		if (time(NULL) - tmp->handle->last_keepalive_time > 300) {
			tmp->handle->last_keepalive_time = time(NULL);
			if (tmp->handle->srv_pause == 0)
				flap_send(tmp->handle, 0x05, NULL, 0, 0);
		}
#endif

#if !defined(__DJGPP__) && !defined(NO_AUTO_IDLE)
		imcomm_set_idle_time(tmp->handle,
				     time(NULL) -
				     tmp->handle->last_operation_time);
#endif
	}
#endif
	return ret;
}

/* PROTO */
int
imcomm_internal_add_buddy(void *handle, char *sn, const unsigned long idletime, const unsigned long onlinetime, int isaway)
{
	IMCOMM_BUDDYLIST *temp, *trav;
	char           *sname;

	sname = imcomm_simplify_sn(sn);

	for (trav = ((IMCOMM *) handle)->buddies_online; trav != NULL;
	     trav = trav->next) {
		if (strcmp(sname, trav->sn) == 0) {
			free(sname);
			return 0;
		}
	}

	temp = malloc(sizeof(IMCOMM_BUDDYLIST));
	temp->sn = sname;
	temp->formattedsn = NULL;
	temp->next = NULL;
	temp->isaway = isaway;
	temp->idletime = idletime;
	temp->onlinetime = onlinetime;

	if (((IMCOMM *) handle)->buddies_online == NULL)
		((IMCOMM *) handle)->buddies_online = temp;
	else {
		for (trav = ((IMCOMM *) handle)->buddies_online;
		     trav->next != NULL; trav = trav->next);

		trav->next = temp;
	}

	return 1;
}

/* PROTO */
void
imcomm_update_buddy_times(void *handle, const char *sn, int type, unsigned long value)
{
	IMCOMM_BUDDYLIST *trav = ((IMCOMM *) handle)->buddies_online;
	char           *sname;

	sname = imcomm_simplify_sn(sn);

	for (; trav != NULL; trav = trav->next) {
		if (strcmp(sname, trav->sn) == 0) {
			switch (type) {
			case 1:
				trav->idletime = value;
				if (((IMCOMM *) handle)->callbacks[IMCOMM_IM_IDLEINFO])
					((IMCOMM *) handle)->
						callbacks[IMCOMM_IM_IDLEINFO] (handle, sn, value);
				break;
			case 2:
				trav->onlinetime = value;
				break;
			}
		}
	}
	free(sname);
}

/* PROTO */
void
imcomm_update_buddy_away(void *handle, const char *sn, int isaway)
{
	char           *sname;
	IMCOMM_BUDDYLIST *trav = ((IMCOMM *) handle)->buddies_online;

	sname = imcomm_simplify_sn(sn);

	for (; trav != NULL; trav = trav->next) {
		if (strcmp(sname, trav->sn) == 0) {
			if (isaway) {
				if (!trav->isaway) {
					trav->isaway = 1;
					if (((IMCOMM *) handle)->
					    callbacks[IMCOMM_IM_BUDDYAWAY])
						((IMCOMM *) handle)->
							callbacks[IMCOMM_IM_BUDDYAWAY] (handle, sn);
					break;
				}
			} else if (trav->isaway) {
				trav->isaway = 0;
				if (((IMCOMM *) handle)->callbacks[IMCOMM_IM_BUDDYUNAWAY])
					((IMCOMM *) handle)->
						callbacks[IMCOMM_IM_BUDDYUNAWAY] (handle, sn);
				break;
			}
		}
	}

	free(sname);
}

/* PROTO */
void
imcomm_internal_delete_buddy(void *handle, const char *sn)
{
	char           *sname;
	IMCOMM_BUDDYLIST *temp, *trav;

	if (((IMCOMM *) handle)->buddies_online == NULL)
		return;

	sname = imcomm_simplify_sn(sn);

	if (strcmp(((IMCOMM *) handle)->buddies_online->sn, sname) == 0) {
		temp = ((IMCOMM *) handle)->buddies_online;
		((IMCOMM *) handle)->buddies_online =
			((IMCOMM *) handle)->buddies_online->next;
		free(temp->sn);
		free(temp);
		free(sname);
	} else {
		for (trav = ((IMCOMM *) handle)->buddies_online;
		     trav->next != NULL; trav = trav->next) {
			if (strcmp(trav->next->sn, sname) == 0) {
				temp = trav->next;
				trav->next = trav->next->next;
				free(temp->sn);
				free(temp);
				free(sname);
				break;
			}
		}
	}
}

/* PROTO */
char           *
imcomm_simplify_sn(const char *sn)
{
	char           *temp;
	int             x, count;

	temp = malloc(strlen(sn) + 1);
	for (x = 0, count = 0; x < (int) strlen(sn); x++) {
		if (sn[x] == ' ')
			continue;
		temp[count] = tolower(sn[x]);
		count++;
	}

	temp = realloc(temp, count + 1);
	temp[count] = 0;
	return temp;
}

/* PROTO */
void
imcomm_set_idle_time(void *handle, uint32_t idlesecs)
{
	pkt_t          *packet;
	packet = pkt_init(4);

	pkt_add32(packet, idlesecs);

	if (idlesecs > 600) {
		if (((IMCOMM *) handle)->isidle == 0) {
			((IMCOMM *) handle)->isidle = 1;
			snac_sendpkt(handle, 0x01, 0x11, packet, 0);
		}
	} else {
		if (((IMCOMM *) handle)->isidle && idlesecs == 0) {
			((IMCOMM *) handle)->isidle = 0;
			snac_sendpkt(handle, 0x01, 0x11, packet, 0);
		}
	}

	pkt_free(packet);
}

/* PROTO */
void
imcomm_set_profile(void *handle, char *profile)
{
	if (((IMCOMM *) handle)->profile_str != NULL)
		free(((IMCOMM *) handle)->profile_str);

	((IMCOMM *) handle)->profile_str = (unsigned char *) strdup(profile);
	if (((IMCOMM *) handle)->socket != 0)
		snac_set_location_info(handle);
}

/* PROTO */
void
imcomm_set_invisible(void *handle, int inv)
{
	((IMCOMM *) handle)->isinvisible = inv;
	snac_send_cli_update(handle);
}

/* PROTO */
void
imcomm_set_away(void *handle, char *msg)
{
	IMCOMM         *tmp = (IMCOMM *) handle;

	tmp->isaway = 1;
	tmp->away_msg = (unsigned char *) strdup(msg);
	if (tmp->socket != 0)
		snac_set_location_info(handle);
}

/* PROTO */
void
imcomm_set_unaway(void *handle)
{
	IMCOMM         *tmp = (IMCOMM *) handle;

	if (tmp->isaway) {
		free(tmp->away_msg);
		tmp->isaway = 0;
		tmp->away_msg = NULL;
		if (tmp->socket != 0)
			snac_set_location_info(handle);
	}
}

/* PROTO */
int
imcomm_compare_nicks(void *handle, const char *s1, const char *s2)
{
	char           *s3, *s4;
	int             ret = 0;

	s3 = imcomm_simplify_sn(s1);
	s4 = imcomm_simplify_sn(s2);

	if (strcmp(s3, s4) == 0)
		ret = 1;

	free(s3);
	free(s4);

	return ret;
}

/* PROTO */
uint16_t
imcomm_get_max_message_size(void *handle)
{
	return ((IMCOMM *) handle)->max_message_size;
}
