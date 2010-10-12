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

#ifdef MD5_LOGIN
#include "md5.h"
#endif

#if 0
#define CLIENT_IDENT "AOL Instant Messenger, version 5.5.3595/WIN32"
#define CLIENT_V1 0x0109
#define CLIENT_V2 0x0005
#define CLIENT_V3 0x0005
#define CLIENT_V4 0x0000
#define CLIENT_V5 0x0e0b
#define CLIENT_V6 0x00000104
#else
#define CLIENT_IDENT "Apple iChat"
#define CLIENT_V1 0x311a
#define CLIENT_V2 0x0001
#define CLIENT_V3 0x0000
#define CLIENT_V4 0x0000
#define CLIENT_V5 0x003c
#define CLIENT_V6 0x00000c6
#endif

#ifdef MACINTOSH_CLASSIC

/*
 * Thanks to PuTTY source code for a lot of help on the MacTCP stuff.
 * Learning obsolete API's can be rather difficult...
 *
 * This should explain the similarities on variable and function names.
 */

int             initialized = 0;
short           refnum;
ProcessSerialNumber psn;
static pascal void mactcp_lookupdone(struct hostInfo *, char *);
static pascal void
mactcp_asr(StreamPtr, unsigned short, Ptr,
	   unsigned short, struct ICMPReport *);
static ResultUPP mactcp_lookupdone_upp;
static TCPNotifyUPP mactcp_asr_upp;
OSErr           err;

static pascal void
mactcp_lookupdone(struct hostInfo * hi, char *cookie)
{
	volatile int   *donep = (int *) cookie;

	*donep = TRUE;
}

static pascal void
mactcp_asr(StreamPtr str, unsigned short event, Ptr cookie,
	   unsigned short termin_reason, struct ICMPReport * icmp)
{
	WakeUpProcess(&psn);
}

void
mactcp_close(void *handle)
{
	TCPiopb         pb;

	pb.ioCRefNum = refnum;
	pb.csCode = TCPClose;
	pb.tcpStream = ((IMCOMM *) handle)->s;
	pb.csParam.close.validityFlags = 0;
	pb.csParam.close.userDataPtr = (Ptr) handle;
	PBControlSync((ParmBlkPtr) & pb);

	pb.ioCRefNum = refnum;
	pb.csCode = TCPAbort;
	pb.tcpStream = ((IMCOMM *) handle)->s;
	pb.csParam.abort.userDataPtr = (Ptr) handle;
	PBControlSync((ParmBlkPtr) & pb);

	pb.ioCRefNum = refnum;
	pb.csCode = TCPRelease;
	pb.tcpStream = ((IMCOMM *) handle)->s;
	pb.csParam.create.userDataPtr = (Ptr) handle;
	err = PBControlSync((ParmBlkPtr) & pb);
	if (err == noErr)
		free(pb.csParam.create.rcvBuff);
}
#endif

/* PROTO */
IMCOMM_RET
imcomm_im_signon(void *handle, const char *sn, const char *pw)
{
	IMCOMM_RET      ret;
	pkt_t          *packet;

#ifdef MACINTOSH_CLASSIC
	OSErr           err;
	struct hostInfo hostinfo;
	volatile int    resolverDone = FALSE;
	TCPiopb         pb;
	UDPiopb         upb;
	size_t          buflen;
#else
	struct sockaddr_in sin;
	struct hostent *he;
#endif

#ifndef MD5_LOGIN
	const char      client_ident[] = CLIENT_IDENT;
	const unsigned char roaststring[] =
	{0xF3, 0x26, 0x81, 0xC4, 0x39, 0x86, 0xDB, 0x92, 0x71, 0xA3, 0xB9,
	0xE6, 0x53, 0x7A, 0x95, 0x7C};
	unsigned char  *roastpw;
	int             x;
#else
	pkt_t          *pk2;
#endif

#ifdef MACINTOSH_CLASSIC
	if (!initialized) {
		err = OpenDriver("\p.IPP", &refnum);
		if (err != noErr) {
			printf("Error initializing driver.\n");
			return IMCOMM_RET_ERROR;
		}
		err = OpenResolver(NULL);
		if (err != noErr) {
			printf("Error initializing resolver.\n");
			return IMCOMM_RET_ERROR;
		}
		mactcp_lookupdone_upp = NewResultProc(&mactcp_lookupdone);
		mactcp_asr_upp = NewTCPNotifyProc(&mactcp_asr);
		initialized = 1;
	}
	err =
		StrToAddr("login.oscar.aol.com", &hostinfo, mactcp_lookupdone_upp,
			  (char *) &resolverDone);
	if (err == cacheFault)
		while (!resolverDone)
			continue;

	upb.ioCRefNum = refnum;
	upb.csCode = UDPMaxMTUSize;
	upb.csParam.mtu.remoteHost = hostinfo.addr[0];
	upb.csParam.mtu.userDataPtr = (Ptr) handle;
	err = PBControlSync((ParmBlkPtr) & upb);
	if (err != noErr) {
		printf("Error retrieving MTU.\n");
		return IMCOMM_RET_ERROR;
	}
	buflen = upb.csParam.mtu.mtuSize * 4 + 1024;
	if (buflen < 4096)
		buflen = 4096;

	GetCurrentProcess(&psn);

	pb.ioCRefNum = refnum;
	pb.csCode = TCPCreate;
	pb.csParam.create.rcvBuff = malloc(buflen);
	pb.csParam.create.rcvBuffLen = buflen;
	pb.csParam.create.notifyProc = mactcp_asr_upp;
	pb.csParam.create.userDataPtr = (Ptr) handle;
	err = PBControlSync((ParmBlkPtr) & pb);
	if (err != noErr) {
		printf("Error creating TCP stream.\n");
		return IMCOMM_RET_ERROR;
	}
	((IMCOMM *) handle)->s = pb.tcpStream;

	pb.ioCRefNum = refnum;
	pb.csCode = TCPActiveOpen;
	pb.tcpStream = ((IMCOMM *) handle)->s;
	pb.csParam.open.validityFlags = 0;
	pb.csParam.open.remoteHost = hostinfo.addr[0];
	pb.csParam.open.remotePort = ((IMCOMM *) handle)->oscarport;
	pb.csParam.open.localPort = 0;
	pb.csParam.open.timeToLive = 0;
	pb.csParam.open.security = 0;
	pb.csParam.open.optionCnt = 0;
	pb.csParam.open.userDataPtr = (Ptr) handle;
	err = PBControlSync((ParmBlkPtr) & pb);
	if (err != noErr) {
		printf("Connection failed.\n");
		return IMCOMM_RET_ERROR;
	}
	((IMCOMM *) handle)->readable = 1;
#else
	/* XXX insert modular socket code here */
#ifdef __MINGW32__
	if (WSAStartup(0x101, &((IMCOMM *) handle)->wsadata)) {
		printf("ERROR: %i\n", WSAGetLastError());
		return IMCOMM_RET_ERROR;
	}
#endif

	if (((IMCOMM *) handle)->proxymode == PROXY_TYPE_SOCKS5) {
		connect_socks5(handle, "login.oscar.aol.com",
			       ((IMCOMM *) handle)->oscarport);
	} else if (((IMCOMM *) handle)->proxymode == PROXY_TYPE_HTTPS) {
		connect_https(handle, "login.oscar.aol.com",
			      ((IMCOMM *) handle)->oscarport);
	} else {
		if ((he = gethostbyname("login.oscar.aol.com")) == NULL) {
			perror("gethostbyname()");
			return IMCOMM_RET_ERROR;
		}
		if ((((IMCOMM *) handle)->socket =
		     socket(AF_INET, SOCK_STREAM, 0)) == -1) {
			perror("socket()");
			return IMCOMM_RET_ERROR;
		}
		sin.sin_family = AF_INET;
		sin.sin_port = htons(((IMCOMM *) handle)->oscarport);

		sin.sin_addr = *((struct in_addr *) he->h_addr);
		memset(&(sin.sin_zero), 0, 8);

		if (connect
		    (((IMCOMM *) handle)->socket, (struct sockaddr *) & sin,
		     sizeof(struct sockaddr)) == -1) {
			perror("connect()");
			return IMCOMM_RET_ERROR;
		}
	}
	/* end socket code for now */
#endif

#ifdef MD5_LOGIN
	((IMCOMM *) handle)->pw = strdup((char *) pw);
	((IMCOMM *) handle)->sn = strdup((char *) sn);
	packet = pkt_init(12 + strlen(sn));
#else
	roastpw = malloc(strlen(pw) + 1);
	memset(roastpw, 0, strlen(pw) + 1);

	for (x = 0; x < (int) strlen(pw); x++)
		roastpw[x] = (pw[x] ^ roaststring[x]);

	packet = pkt_init(66 + strlen(client_ident) + strlen(sn) + strlen(pw));

	pkt_add32(packet, 0x00000001);
#endif

	/*
         * I should really build a TLV here instead, but this works.
         */

	/*
         * Add SN length + SN
         */
	pkt_add16(packet, 0x0001);
	pkt_add16(packet, (uint16_t) strlen(sn));
	pkt_addraw(packet, (uint8_t *) sn, strlen(sn));

#ifdef MD5_LOGIN
	/*
         * Add unknown TLV's...
         */
	pkt_add16(packet, 0x004B);
	pkt_add16(packet, 0x0000);
	pkt_add16(packet, 0x005A);
	pkt_add16(packet, 0x0000);
#else
	/*
         * Add roasted PW length + roasted PW
         */
	pkt_add16(packet, 0x0002);
	pkt_add16(packet, (uint16_t) strlen(pw));
	pkt_addraw(packet, (uint8_t *) pw, strlen(pw));

	/*
         * Add client ident string
         */
	pkt_add16(packet, 0x0003);
	pkt_add16(packet, (uint16_t) strlen(client_ident));
	pkt_addraw(packet, (uint8_t *) client_ident, strlen(client_ident));

	/*
         * Add client ID (hardcoded)
         */
	pkt_add16(packet, 0x0016);
	pkt_add16(packet, 0x0002);
	pkt_add16(packet, 0x0109);

	/*
         * Add client versions (hardcoded)
         */
	pkt_add16(packet, 0x0017);
	pkt_add16(packet, 0x0002);
	pkt_add16(packet, 0x0005);
	pkt_add16(packet, 0x0018);
	pkt_add16(packet, 0x0002);
	pkt_add16(packet, 0x0005);
	pkt_add16(packet, 0x0019);
	pkt_add16(packet, 0x0002);
	pkt_add16(packet, 0x0000);
	pkt_add16(packet, 0x001A);
	pkt_add16(packet, 0x0002);
	pkt_add16(packet, 0x0057);
	pkt_add16(packet, 0x0014);
	pkt_add16(packet, 0x0004);
	pkt_add32(packet, 0x000000ef);
	pkt_add16(packet, 0x000F);
	pkt_add16(packet, 0x0002);
	pkt_addraw(packet, (unsigned char *) "en", 2);
	pkt_add16(packet, 0x000E);
	pkt_add16(packet, 0x0002);
	pkt_addraw(packet, (unsigned char *) "us", 2);
#endif

#ifdef MD5_LOGIN
	pk2 = pkt_init(4);
	pkt_add32(pk2, 0x00000001);
	flap_sendpkt(handle, 0x01, pk2, 0);
	pkt_free(pk2);

	ret = snac_sendpkt(handle, 0x17, 0x06, packet, 0);
#else
	ret = flap_sendpkt(handle, 0x01, packet, 0);
	free(roastpw);
#endif
	pkt_free(packet);
	return ret;

}

/* PROTO */
IMCOMM_RET
bos_signon_phase2(void *handle, unsigned const char *server, unsigned const char *cookie, uint16_t cookie_len)
{
	char           *host;
	uint16_t        port;
	pkt_t          *packet;
	int             x, ret;
	int             newlen;

#ifdef MACINTOSH_CLASSIC
	OSErr           err;
	struct hostInfo hostinfo;
	volatile int    resolverDone = FALSE;
	TCPiopb         pb;
	UDPiopb         upb;
	size_t          buflen;
#else
	struct sockaddr_in sin;
#endif

#if defined(PLAN9) || defined(__MINGW32__)
	long            addy;
	struct in_addr  ina;
#else
	struct hostent *he;
#endif

	port = atoi(strchr((char *) server, ':') + 1);

	host = (char *) malloc(strlen((char *) server) + 1);

#ifdef __OpenBSD__
	strlcpy(host, (char *) server, strlen((char *) server));
#else
	strcpy(host, (char *) server);
#endif

	for (x = 0; x < (int) strlen(host); x++)
		if (host[x] == ':') {
			host[x] = 0;
			break;
		}
#ifdef MACINTOSH_CLASSIC
	mactcp_close(handle);
	((IMCOMM *) handle)->readable = 0;

	err =
		StrToAddr(host, &hostinfo, mactcp_lookupdone_upp,
			  (char *) &resolverDone);
	if (err == cacheFault)
		while (!resolverDone)
			continue;

	upb.ioCRefNum = refnum;
	upb.csCode = UDPMaxMTUSize;
	upb.csParam.mtu.remoteHost = hostinfo.addr[0];
	upb.csParam.mtu.userDataPtr = (Ptr) handle;
	err = PBControlSync((ParmBlkPtr) & upb);
	if (err != noErr) {
		printf("Error retrieving MTU.\n");
		return IMCOMM_RET_ERROR;
	}
	buflen = upb.csParam.mtu.mtuSize * 4 + 1024;
	if (buflen < 4096)
		buflen = 4096;

	pb.ioCRefNum = refnum;
	pb.csCode = TCPCreate;
	pb.csParam.create.rcvBuff = malloc(buflen);
	pb.csParam.create.rcvBuffLen = buflen;
	pb.csParam.create.notifyProc = mactcp_asr_upp;
	pb.csParam.create.userDataPtr = (Ptr) handle;
	err = PBControlSync((ParmBlkPtr) & pb);
	if (err != noErr) {
		printf("Error creating TCP stream.\n");
		return IMCOMM_RET_ERROR;
	}
	((IMCOMM *) handle)->s = pb.tcpStream;

	pb.ioCRefNum = refnum;
	pb.csCode = TCPActiveOpen;
	pb.tcpStream = ((IMCOMM *) handle)->s;
	pb.csParam.open.validityFlags = 0;
	pb.csParam.open.remoteHost = hostinfo.addr[0];
	pb.csParam.open.remotePort = port;
	pb.csParam.open.localPort = 0;
	pb.csParam.open.timeToLive = 0;
	pb.csParam.open.security = 0;
	pb.csParam.open.optionCnt = 0;
	pb.csParam.open.userDataPtr = (Ptr) handle;
	err = PBControlSync((ParmBlkPtr) & pb);
	if (err != noErr) {
		printf("Connection failed.\n");
		return IMCOMM_RET_ERROR;
	}
	((IMCOMM *) handle)->readable = 1;

#else
	if (((IMCOMM *) handle)->ischild == 0)
		shutdown(((IMCOMM *) handle)->socket, 0x02);

	if (((IMCOMM *) handle)->proxymode == PROXY_TYPE_SOCKS5) {
		connect_socks5(handle, host, port);
	} else if (((IMCOMM *) handle)->proxymode == PROXY_TYPE_HTTPS) {
		connect_https(handle, host, port);
	} else {

#if !defined(PLAN9) && !defined(__MINGW32__)
		if ((he = gethostbyname(host)) == NULL) {
			perror("gethostbyname()");
			free(host);
			return IMCOMM_RET_ERROR;
		}
		free(host);
#else
		addy = inet_addr(host);
		ina.s_addr = addy;
		free(host);
#endif

		if ((((IMCOMM *) handle)->socket =
		     socket(AF_INET, SOCK_STREAM, 0)) == -1) {
			perror("socket()");
			return IMCOMM_RET_ERROR;
		}
		sin.sin_family = AF_INET;
		sin.sin_port = htons(port);

#if defined(PLAN9) || defined(__MINGW32__)
		sin.sin_addr = ina;
#else
		sin.sin_addr = *((struct in_addr *) he->h_addr);
#endif

		memset(&(sin.sin_zero), 0, 8);

		if (connect
		    (((IMCOMM *) handle)->socket, (struct sockaddr *) & sin,
		     sizeof(struct sockaddr)) == -1) {
			perror("connect()");
			return IMCOMM_RET_ERROR;
		}
#endif
	}

	newlen = cookie_len + 8;
	packet = pkt_init(newlen);
	pkt_add32(packet, 0x00000001);
	pkt_add16(packet, 0x0006);
	pkt_add16(packet, cookie_len);
	pkt_addraw(packet, (uint8_t *) cookie, cookie_len);

	ret = flap_sendpkt(handle, 0x01, packet, 0);
	pkt_free(packet);
	return ret;
}

/* PROTO */
void
handle_srv_migration(void *handle, uint8_t * data, uint16_t len)
{
	TLVLIST        *tlv, *trav;
	uint16_t        num_family, cookie_len = 0;
	pkt_t          *inpkt;
	int             status = 0;
	char           *bos_server = 0;
	uint8_t        *cookie = 0;

	inpkt = pkt_initP(data + 10, len - 10);

	num_family = pkt_get16(inpkt);
	if (num_family != 0 && num_family != ((IMCOMM *) handle)->num_families) {
		printf("migration has different number of families.\n");
		printf("not proceeding (bifurcated?)\n");
		return;
	}
	pkt_freeP(inpkt);

	if (((IMCOMM *) handle)->srv_pause)
		((IMCOMM *) handle)->srv_pause = 0;

	tlv =
		tlv_split(data + 12 + (num_family * 2),
			  len - (12 + (num_family * 2)), 0);

	printf("migration beginning.\n");

	for (trav = tlv; trav != NULL; trav = trav->next) {
		if (trav->type == 0x0005) {
			bos_server = malloc(trav->len + 1);
			memcpy(bos_server, trav->value, trav->len);
			bos_server[trav->len] = 0;
			status++;
		} else if (trav->type == 0x0006) {
			cookie = malloc(trav->len);
			cookie_len = trav->len;
			memcpy(cookie, trav->value, trav->len);
			status++;
		}
	}

	clear_tlv_list(tlv);

	printf("server: %s\n", bos_server);

	if (((IMCOMM *) handle)->callbacks[IMCOMM_ERROR])
		((IMCOMM *) handle)->callbacks[IMCOMM_ERROR] (handle,
					       IMCOMM_STATUS_MIGRATIONDONE);

	bos_signon_phase2(handle, (unsigned char *) bos_server, cookie,
			  cookie_len);
	free(bos_server);
	free(cookie);
}

#ifdef MD5_LOGIN

#define AIM_MD5_STRING "AOL Instant Messenger (SM)"

/*
 * New stuff for MD5-based login
 */

/* PROTO */
void
bos_md5snac(void *handle, uint8_t * data, uint16_t len)
{
	uint8_t        *authkey;
	uint16_t        authkeylen;
	md5_state_t     state;
	md5_byte_t      auth_hash[16];
	pkt_t          *packet;

	if (data[3] == 0x07) {
		const char      client_ident[] = CLIENT_IDENT;

		authkeylen = two_to_16(data + 10);
		authkey = malloc(authkeylen);
		memcpy(authkey, data + 12, authkeylen);

		md5_init(&state);
		md5_append(&state, (const md5_byte_t *) authkey, authkeylen);
		md5_append(&state, (const md5_byte_t *) ((IMCOMM *) handle)->pw,
			   strlen(((IMCOMM *) handle)->pw));
		md5_append(&state, (const md5_byte_t *) AIM_MD5_STRING,
			   strlen(AIM_MD5_STRING));
		md5_finish(&state, auth_hash);

		free(authkey);

		packet =
			pkt_init(67 + strlen(CLIENT_IDENT) + 16 +
				 strlen(((IMCOMM *) handle)->sn));

		/*
	         * I should really build a TLV here instead, but this works.
	         */

		/*
	         * Add SN length + SN
	         */
		pkt_add16(packet, 0x0001);
		pkt_add16(packet, (uint16_t) strlen(((IMCOMM *) handle)->sn));
		pkt_addraw(packet, (uint8_t *) ((IMCOMM *) handle)->sn,
			   strlen(((IMCOMM *) handle)->sn));
		pkt_add16(packet, 0x0025);
		pkt_add16(packet, 0x0010);
		pkt_addraw(packet, (uint8_t *) auth_hash, 16);


		/*
	         * Add client ident string
	         */
		pkt_add16(packet, 0x0003);
		pkt_add16(packet, (uint16_t) strlen(client_ident));
		pkt_addraw(packet, (uint8_t *) client_ident, strlen(client_ident));
		/*
	         * Add client ID (hardcoded)
	         */
		pkt_add16(packet, 0x0016);
		pkt_add16(packet, 0x0002);
		pkt_add16(packet, CLIENT_V1);

		/*
	         * Add client versions (hardcoded)
	         */
		pkt_add16(packet, 0x0017);
		pkt_add16(packet, 0x0002);
		pkt_add16(packet, CLIENT_V2);
		pkt_add16(packet, 0x0018);
		pkt_add16(packet, 0x0002);
		pkt_add16(packet, CLIENT_V3);
		pkt_add16(packet, 0x0019);
		pkt_add16(packet, 0x0002);
		pkt_add16(packet, CLIENT_V4);
		pkt_add16(packet, 0x001A);
		pkt_add16(packet, 0x0002);
		pkt_add16(packet, CLIENT_V5);
		pkt_add16(packet, 0x0014);
		pkt_add16(packet, 0x0004);
		pkt_add32(packet, CLIENT_V6);
		pkt_add16(packet, 0x000F);
		pkt_add16(packet, 0x0002);
		pkt_addraw(packet, (unsigned char *) "en", 2);
		pkt_add16(packet, 0x000E);
		pkt_add16(packet, 0x0002);
		pkt_addraw(packet, (unsigned char *) "us", 2);
		pkt_add16(packet, 0x004A);
		pkt_add16(packet, 0x0001);
		pkt_add8(packet, 0x01);

		snac_sendpkt(handle, 0x17, 0x02, packet, 0);
		pkt_free(packet);
		return;
	} else if (data[3] == 0x03) {
		TLVLIST        *tlv, *trav;
		uint16_t        numtlv, cookie_len = 0;
		int             status = 0;
		char           *bos_server = 0;
		uint8_t        *cookie = 0;

		numtlv = count_tlv(data + 10, len - 10);
		tlv = tlv_split(data + 10, len - 10, numtlv);

		for (trav = tlv; trav != NULL; trav = trav->next) {
			if (trav->type == 0x0008) {
				if (((IMCOMM *) handle)->callbacks[IMCOMM_ERROR])
					((IMCOMM *) handle)->callbacks[IMCOMM_ERROR] (handle,
						IMCOMM_ERROR_INVALID_LOGIN);
#ifdef MACINTOSH_CLASSIC

#else
				shutdown(((IMCOMM *) handle)->socket, 0x02);
				((IMCOMM *) handle)->socket = -1;
#endif
			} else if (trav->type == 0x0005) {
				bos_server = malloc(trav->len + 1);
				memcpy(bos_server, trav->value, trav->len);
				bos_server[trav->len] = 0;
				status++;
			} else if (trav->type == 0x0006) {
				cookie = malloc(trav->len);
				cookie_len = trav->len;
				memcpy(cookie, trav->value, trav->len);
				status++;
			}
		}

		clear_tlv_list(tlv);

		if (status == 2) {
			if (((IMCOMM *) handle)->callbacks[IMCOMM_ERROR])
				((IMCOMM *) handle)->callbacks[IMCOMM_ERROR] (handle,
						    IMCOMM_STATUS_AUTHDONE);
			bos_signon_phase2(handle, (unsigned char *) bos_server, cookie,
					  cookie_len);
			free(bos_server);
			free(cookie);
		}
	}
}

#endif				/* MD5_LOGIN */
