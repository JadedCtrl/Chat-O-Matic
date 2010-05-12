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
connect_socks5(void *handle, char *host, uint16_t port)
{
	IMCOMM         *h = (IMCOMM *) handle;
	struct sockaddr_in sin;
	struct hostent *he = NULL;
	long            addy;
	struct in_addr  ina = {0};
	pkt_t          *sockspkt;
	uint8_t         len;
	unsigned char   sockbuf[512];

#ifdef DEBUG
	printf("Connecting via SOCKS5 to %s/%u\n", host, port);
	printf("SOCKS5 proxy server: %s/%u\n", h->proxyserver, h->proxyport);
#endif

	if ((he = gethostbyname(h->proxyserver)) == NULL) {
		addy = inet_addr(h->proxyserver);
		ina.s_addr = addy;
	}
	if ((h->socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		if (h->callbacks[IMCOMM_ERROR])
			h->callbacks[IMCOMM_ERROR] (handle, IMCOMM_ERROR_PROXY,
						    PROXY_ERROR_CONNECT);

		return IMCOMM_RET_ERROR;
	}
	sin.sin_family = AF_INET;
	sin.sin_port = htons(h->proxyport);

	if (he == NULL)
		sin.sin_addr = ina;
	else
		sin.sin_addr = *((struct in_addr *) he->h_addr);

	memset(&(sin.sin_zero), 0, 8);

	if (connect
	    (h->socket, (struct sockaddr *) & sin,
	     sizeof(struct sockaddr)) == -1) {
		if (h->callbacks[IMCOMM_ERROR])
			h->callbacks[IMCOMM_ERROR] (handle, IMCOMM_ERROR_PROXY,
						    PROXY_ERROR_CONNECT);

		return IMCOMM_RET_ERROR;
	}
	sockspkt = pkt_init(3);
	pkt_add8(sockspkt, 0x05);
	pkt_add8(sockspkt, 0x01);
	pkt_add8(sockspkt, 0x00);

	if (send(h->socket, sockspkt->data, sockspkt->len, 0) < 0) {
		pkt_free(sockspkt);
		if (h->callbacks[IMCOMM_ERROR])
			h->callbacks[IMCOMM_ERROR] (handle, IMCOMM_ERROR_DISCONNECTED,
						    0);

		shutdown(h->socket, 0x02);
		h->socket = -1;
		return IMCOMM_RET_ERROR;
	}
	pkt_free(sockspkt);

	recv(h->socket, sockbuf, 2, 0);
	if (sockbuf[1] != 0x00) {
		if (h->callbacks[IMCOMM_ERROR])
			h->callbacks[IMCOMM_ERROR] (handle, IMCOMM_ERROR_PROXY,
						    PROXY_ERROR_AUTH);
		return IMCOMM_RET_ERROR;
	}
	sockspkt = pkt_init(7 + strlen(host));
	pkt_add8(sockspkt, 0x05);
	pkt_add8(sockspkt, 0x01);
	pkt_add8(sockspkt, 0x00);
	pkt_add8(sockspkt, 0x03);
	pkt_add8(sockspkt, (uint8_t) strlen(host));
	pkt_addraw(sockspkt, (unsigned char *) host, strlen(host));
	pkt_add16(sockspkt, port);

	if (send(h->socket, sockspkt->data, sockspkt->len, 0) < 0) {
		pkt_free(sockspkt);
		if (h->callbacks[IMCOMM_ERROR])
			h->callbacks[IMCOMM_ERROR] (handle, IMCOMM_ERROR_DISCONNECTED,
						    0);

		shutdown(h->socket, 0x02);
		h->socket = -1;
		return IMCOMM_RET_ERROR;
	}
	pkt_free(sockspkt);

	recv(h->socket, sockbuf, 4, 0);
	if (sockbuf[1] != 0x00) {
		if (h->callbacks[IMCOMM_ERROR])
			h->callbacks[IMCOMM_ERROR] (handle, IMCOMM_ERROR_PROXY,
						  PROXY_ERROR_PROXYCONNECT);
		shutdown(h->socket, 0x02);
		h->socket = -1;

		return IMCOMM_RET_ERROR;
	}
	if (sockbuf[3] == 0x03) {
		recv(h->socket, &len, 1, 0);
		recv(h->socket, sockbuf, len + 2, 0);
	} else if (sockbuf[3] == 0x01) {
		recv(h->socket, sockbuf, 6, 0);
	} else {
		if (h->callbacks[IMCOMM_ERROR])
			h->callbacks[IMCOMM_ERROR] (handle, IMCOMM_ERROR_PROXY,
						    PROXY_ERROR_UNKNOWN);

		shutdown(h->socket, 0x02);
		h->socket = -1;
		return IMCOMM_RET_ERROR;
	}

	/* now we should be set to read the connection */

	return IMCOMM_RET_OK;
}

/* PROTO */
int
connect_https(void *handle, char *host, uint16_t port)
{
	IMCOMM         *h = (IMCOMM *) handle;
	struct sockaddr_in sin;
	struct hostent *he = NULL;
	long            addy;
	struct in_addr  ina = {0};
	unsigned char   sockbuf[512];
	char           *sptr;
	int             received, retcode;

#ifdef DEBUG
	printf("Connecting via HTTPS to %s/%d.\n", host, port);
	printf("Proxy server: %s/%d\n", h->proxyserver, h->proxyport);
#endif

	if ((he = gethostbyname(h->proxyserver)) == NULL) {
		addy = inet_addr(h->proxyserver);
		ina.s_addr = addy;
	}
	if ((h->socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		if (h->callbacks[IMCOMM_ERROR])
			h->callbacks[IMCOMM_ERROR] (handle, IMCOMM_ERROR_PROXY,
						    PROXY_ERROR_CONNECT);

		return IMCOMM_RET_ERROR;
	}
	sin.sin_family = AF_INET;
	sin.sin_port = htons(h->proxyport);

	if (he == NULL)
		sin.sin_addr = ina;
	else
		sin.sin_addr = *((struct in_addr *) he->h_addr);

	memset(&(sin.sin_zero), 0, 8);

	if (connect
	    (h->socket, (struct sockaddr *) & sin,
	     sizeof(struct sockaddr)) == -1) {
		if (h->callbacks[IMCOMM_ERROR])
			h->callbacks[IMCOMM_ERROR] (handle, IMCOMM_ERROR_PROXY,
						    PROXY_ERROR_CONNECT);

		return IMCOMM_RET_ERROR;
	}
	/*
         * This user agent seems pretty well-accepted. Not sure if this is
         * any sort of problem...
         */

	snprintf((char *) sockbuf, sizeof(sockbuf),
		 "CONNECT %s:%d HTTP/1.0\r\nUser-agent: Mozilla/5.0 (Windows; U; Windows NT 5.1; en-US; rv:1.0.2) Gecko/20021120 Netscape/7.01\r\n\r\n",
		 host, port);
	if (send(h->socket, sockbuf, strlen((char *) sockbuf), 0) < 0) {
		if (h->callbacks[IMCOMM_ERROR])
			h->callbacks[IMCOMM_ERROR] (handle, IMCOMM_ERROR_DISCONNECTED,
						    0);

		shutdown(h->socket, 0x02);
		h->socket = -1;
		return IMCOMM_RET_ERROR;
	}
	received = recv(h->socket, sockbuf, sizeof(sockbuf), 0);
	sockbuf[received] = 0;

	sptr = strchr((char *) sockbuf, ' ');
	retcode = atoi(sptr + 1);

	if (retcode != 200) {
		if (h->callbacks[IMCOMM_ERROR])
			h->callbacks[IMCOMM_ERROR] (handle, IMCOMM_ERROR_PROXY,
						  PROXY_ERROR_PROXYCONNECT);

		shutdown(h->socket, 0x02);
		h->socket = -1;

		return IMCOMM_RET_ERROR;
	} else {
		while (strstr((char *) sockbuf, "\r\n\r\n") == NULL) {
			received = recv(h->socket, sockbuf, sizeof(sockbuf), 0);
			sockbuf[received] = 0;
		}
	}

	/* now we should be set to read the connection */

	return IMCOMM_RET_OK;
}
