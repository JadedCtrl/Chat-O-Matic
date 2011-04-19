/*
 * Based on the sample client from libyahoo2. This is a plugin ported from im_kit to Caya,
 * the code was updated to support libyahoo2.
 *
 * Copyright (C) 2002-2004, Philip S Tellis <philip.tellis AT gmx.net>
 * Copyright (C) 2010-2011, Barrett
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include "YahooConnection.h"

#include <netdb.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <termios.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <openssl/ssl.h>

#include <assert.h>
#include "YahooConnection.h"

#include <yahoo2.h>
#include <yahoo2_callbacks.h>

#include <yahoo_util.h>

#include <yahoo2_types.h>

#define LOG printf

int fileno(FILE * stream);

#define MAX_PREF_LEN 255

static time_t curTime = 0;
static time_t pingTimer = 0;

extern "C" void register_callbacks();

typedef struct {
	int id;
	char *label;
} yahoo_idlabel;

typedef struct {
	int id;
	char *who;
} yahoo_authorize_data;

static int connection_tags=0;

struct connect_callback_data {
	yahoo_connect_callback callback;
	void * callback_data;
	int id;
	int tag;
};

yahoo_idlabel yahoo_status_codes[] = {
	{YAHOO_STATUS_AVAILABLE, "Available"},
	{YAHOO_STATUS_BRB, "BRB"},
	{YAHOO_STATUS_BUSY, "Busy"},
	{YAHOO_STATUS_NOTATHOME, "Not Home"},
	{YAHOO_STATUS_NOTATDESK, "Not at Desk"},
	{YAHOO_STATUS_NOTINOFFICE, "Not in Office"},
	{YAHOO_STATUS_ONPHONE, "On Phone"},
	{YAHOO_STATUS_ONVACATION, "On Vacation"},
	{YAHOO_STATUS_OUTTOLUNCH, "Out to Lunch"},
	{YAHOO_STATUS_STEPPEDOUT, "Stepped Out"},
	{YAHOO_STATUS_INVISIBLE, "Invisible"},
	{YAHOO_STATUS_IDLE, "Idle"},
	{YAHOO_STATUS_OFFLINE, "Offline"},
	{YAHOO_STATUS_CUSTOM, "[Custom]"},
	{YPACKET_STATUS_NOTIFY, "Notify"},
	{0, NULL}
};

#define YAHOO_DEBUGLOG ext_yahoo_log

static int expired(time_t timer)
{
	if (timer && curTime >= timer)
		return 1;

	return 0;
}

static void rearm(time_t *timer, int seconds)
{
	time(timer);
	*timer += seconds;
}


static int yahoo_ping_timeout_callback(int id)
{
	yahoo_keepalive(id);
	rearm(&pingTimer, 600);
	return 1;
}

FILE *popen(const char *command, const char *type);
int pclose(FILE *stream);
int gethostname(char *name, size_t len);


static int yahoo_ping_timeout_callback( int id, time_t & pingTimer )
{
	yahoo_keepalive(id);
	rearm(&pingTimer, 600);
	return 1;
}


extern "C" int ext_yahoo_log(const char *fmt,...)
{
	va_list ap;

	va_start(ap, fmt);

	vfprintf(stderr, fmt, ap);
	fflush(stderr);
	va_end(ap);
	return 0;
}

extern "C" void ext_yahoo_got_conf_invite(int id, const char */*me*/, const char *who, const char *room, const char *msg, YList *members)
{
}

extern "C" void ext_yahoo_conf_userdecline(int /*id*/, const char */*me*/, const char */*who*/, const char */*room*/, const char */*msg*/)
{
}

extern "C" void ext_yahoo_conf_userjoin(int /*id*/, const char */*me*/, const char */*who*/, const char */*room*/)
{
}

extern "C" void ext_yahoo_conf_userleave(int /*id*/, const char */*me*/, const char */*who*/, const char */*room*/)
{
}

extern "C" void ext_yahoo_conf_message(int /*id*/, const char */*me*/, const char */*who*/, const char */*room*/, const char */*msg*/, int /*utf8*/)
{
}

extern "C" void ext_yahoo_chat_cat_xml(int /*id*/, const char */*xml*/) 
{
}

extern "C" void ext_yahoo_chat_join(int id, const char *me, const char *room, const char *topic, YList *members, void *fd)
{
}

extern "C" void ext_yahoo_chat_userjoin(int id, const char *me, const char* room, struct yahoo_chat_member */*who*/)
{

}

extern "C" void ext_yahoo_chat_userleave(int id, const char *me, const char *room, const char *who)
{

}

extern "C" void ext_yahoo_chat_message(int /*id*/, const char */*me*/, const char */*who*/, const char */*room*/, const char */*msg*/, int /*msgtype*/, int /*utf8*/)
{
	LOG("ext_yahoo_chat_message\n");
}

extern "C" void ext_yahoo_status_changed(int id, const char *who, int stat, const char *msg, int away, int idle, int /*mobile*/)
{
	LOG("ext_yahoo_status_changed\n");
	assert(gYahooConnections[id] != NULL);
	gYahooConnections[id]->cbStatusChanged(who, stat, msg, away, idle);
}

extern "C" void ext_yahoo_got_buddies(int id, YList * buds)
{
	LOG("ext_yahoo_got_buddies\n");
	assert( gYahooConnections[id] != NULL );
	gYahooConnections[id]->cbGotBuddies(buds);
}

extern "C" void ext_yahoo_got_ignore(int /*id*/, YList * /*igns*/)
{
}

extern "C" void ext_yahoo_got_im(int id,const char* /*me*/, const char *who, const char *msg, long tm, int stat, int utf8)
{
	LOG("ext_yahoo_got_im\n");
	assert( gYahooConnections[id] != NULL );
	gYahooConnections[id]->cbGotIM(who,msg,tm,stat,utf8);
}

extern "C" void ext_yahoo_rejected(int /*id*/, const char */*who*/, const char */*msg*/)
{
}

extern "C" void ext_yahoo_contact_added(int id, const char */*myid*/, const char *who, const char *msg)
{
}

extern "C" void ext_yahoo_typing_notify(int id, const char */*me*/, const char *who, int stat)
{
	assert( gYahooConnections[id] != NULL );
	gYahooConnections[id]->cbTypeNotify(who,stat);
}

extern "C" void ext_yahoo_game_notify(int id, const char *me, const char *who, int stat, const char *msg)
{
}

extern "C" void ext_yahoo_mail_notify(int id, const char *from, const char *subj, int cnt)
{
}

extern "C" void ext_yahoo_got_buddyicon(int id, const char */*me*/, const char *who, const char *url, int checksum)
{
//	yahoo_http_get(id,url,NULL, 0 , 0, NULL,(void*)who);
}

extern "C" void ext_yahoo_got_buddyicon_checksum(int id, const char */*me*/,const char *who, int /*checksum*/)
{
	//yahoo_buddyicon_request(id,who);
}

extern "C" void ext_yahoo_got_buddyicon_request(int id, const char */*me*/, const char *who)
{

}

extern "C" void ext_yahoo_buddyicon_uploaded(int id, const char *url)
{

}

extern "C" void ext_yahoo_got_webcam_image(int /*id*/, const char */*who*/,
		const unsigned char */*image*/, unsigned int /*image_size*/, unsigned int /*real_size*/,
		unsigned int /*timestamp*/)
{
}

extern "C" void ext_yahoo_webcam_viewer(int /*id*/, const char */*who*/, int /*connect*/)
{
}

extern "C" void ext_yahoo_webcam_closed(int /*id*/, const char */*who*/, int /*reason*/)
{
}

extern "C" void ext_yahoo_webcam_data_request(int /*id*/, int /*send*/)
{
}

extern "C" void ext_yahoo_webcam_invite(int /*id*/, const char */*me*/, const char */*from*/)
{
}

extern "C" void ext_yahoo_webcam_invite_reply(int /*id*/, const char */*me*/, const char */*from*/, int /*accept*/)
{
}

extern "C" void ext_yahoo_system_message(int id, const char *me, const char *who, const char *msg)
{
	LOG("%s\n",msg);
}

extern "C" void ext_yahoo_got_file(int id, const char *me, const char *who, const char *msg, const char *fname, unsigned long fesize, char *trid)
{
}

extern "C" void ext_yahoo_got_identities(int /*id*/, YList * /*ids*/)
{
}

extern "C" void ext_yahoo_chat_yahoologout(int /*id*/, const char */*me*/)
{ 
}

extern "C" void ext_yahoo_chat_yahooerror(int /*id*/, const char */*me*/)
{ 
}

extern "C" void ext_yahoo_got_search_result(int /*id*/, int /*found*/, int /*start*/, int /*total*/, YList */*contacts*/)
{
}

extern "C" void ext_yahoo_got_cookies(int id)
{
	assert( gYahooConnections[id] != NULL );
	yahoo_get_yab(id);
}

extern "C" void ext_yahoo_login_response(int id, int succ, const char *url)
{
	LOG("ext_yahoo_login_response %d\n", id);
	assert( gYahooConnections[id] != NULL );
	gYahooConnections[id]->cbLoginResponse(succ,url);
}

extern "C" void ext_yahoo_error(int id, const char *err, int fatal, int /*num*/)
{
	LOG("ext_yahoo_error\n");
	assert( gYahooConnections[id] != NULL );
	gYahooConnections[id]->cbYahooError(err,fatal);
}

extern "C" void ext_yahoo_got_ping(int /*id*/, const char */*errormsg*/)
{
}


extern "C" int ext_yahoo_add_handler(int id, void *d, yahoo_input_condition cond, void *data)
{
	LOG("ext_yahoo_add_handler %d\n", id);

	if (id < 0) { // 'connect' connection
		struct conn_handler *c = y_new0(struct conn_handler, 1);
		c->tag = ++connection_tags;
		c->id = id;
		c->con = (_conn*) d;
		c->cond = cond;
		c->data = data;

		return c->tag;
	}
	
	assert( gYahooConnections[id] != NULL );

	struct conn_handler* c = y_new0(struct conn_handler, 1);
	c->tag = ++connection_tags;
	c->id = id;
	c->con = (_conn*) d;
	c->cond = cond;
	c->data = data;

	LOG("Add fd %p for %d, tag %d, cond %d\n", c->con->fd, id, c->tag, c->cond);

	gYahooConnections[id]->AddConnection( c );

	return c->tag;
}

extern "C" void ext_yahoo_remove_handler(int id, int tag)
{
	LOG("ext_yahoo_remove_handler %d\n", id);

	if (!tag)
		return;

	assert(gYahooConnections[id] != NULL);
	
	YahooConnection * conn = gYahooConnections[id];
	
	for (int i=0; i < conn->CountConnections(); i++) {
		struct conn_handler *c = conn->ConnectionAt(i);
		if(c->tag == tag) {
			/* don't actually remove it, just mark it for removal */
			/* we'll remove when we start the next poll cycle */
			LOG("  Marking id:%d con:%p tag:%d for removal\n", c->id, c->con, c->tag);
			c->remove = 1;
			return;
		}
	}
}

extern "C" int ext_yahoo_connect(const char *host, int port)
{
	//deprecated
	return -1;
}


static int ext_yahoo_write(void *fd, char *buf, int len)
{
	LOG("ext_yahoo_write\n");
	struct _conn *c = (_conn*)fd;

	if (c->use_ssl)
		return SSL_write(c->ssl, buf, len);
	else
		return write(c->fd, buf, len);
}

static int ext_yahoo_read(void *fd, char *buf, int len)
{
	LOG("ext_yahoo_read\n");
	struct _conn *c = (_conn*)fd;

	if (c->use_ssl)
		return SSL_read(c->ssl, buf, len);
	else
		return read(c->fd, buf, len);
}

static void ext_yahoo_close(void *fd)
{
	LOG("ext_yahoo_close\n");
	struct _conn *c = (_conn*)fd;

	if (c->use_ssl)
		SSL_free(c->ssl);

	close(c->fd);
	c->fd = 0;

	std::map<int, YahooConnection*>::iterator it;
	int h = 0;
	for (it = gYahooConnections.begin(); it != gYahooConnections.end(); it++) {
		int i = 0;
		if (it->second == NULL)
			continue;

		int count = it->second->CountConnections();
		while (i < count) {
			if (it->second->ConnectionAt(i)->con == c) {
				it->second->ConnectionAt(i)->remove = 1;
			}
			i++;
		}
	}

	c->remove = 1;
}


static SSL *do_ssl_connect(int fd)
{
	SSL *ssl;
	SSL_CTX *ctx;

	LOG("SSL Handshake\n");

	SSL_library_init ();
	ctx = SSL_CTX_new(SSLv23_client_method());
	ssl = SSL_new(ctx);
	SSL_CTX_free(ctx);
	SSL_set_fd(ssl, fd);

	if (SSL_connect(ssl) == 1)
		return ssl;

	return NULL;
}


extern "C" int ext_yahoo_connect_async(int id, const char *host, int port, 
		yahoo_connect_callback callback, void *data, int use_ssl)
{
	struct sockaddr_in serv_addr;
	static struct hostent *server;
	int servfd;
	struct connect_callback_data * ccd;
	int error;
	SSL *ssl = NULL;

	struct _conn *c;

	LOG("Connecting to %s:%d\n", host, port);
	
	if (!(server = gethostbyname(host))) {
		errno=h_errno;
		return -1;
	}

	if ((servfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		return -1;
	}

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	memcpy(&serv_addr.sin_addr.s_addr, *server->h_addr_list, server->h_length);
	serv_addr.sin_port = htons(port);

	c = y_new0(struct _conn, 1);
	c->fd = servfd;
	c->use_ssl = use_ssl;

	error = connect(servfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));

	LOG("Trying to connect: fd:%d error:%d\n", servfd, error);
	if (!error) {
		LOG("Connected\n");
		if (use_ssl) {
			ssl = do_ssl_connect(servfd);

			if (!ssl) {
				LOG("SSL Handshake Failed!\n");
				ext_yahoo_close(c);

				callback(NULL, 0, data);
				return -1;
			}
		}

		c->ssl = ssl;
		fcntl(c->fd, F_SETFL, O_NONBLOCK);

		callback(c, 0, data);
		return 0;
	} else if (error == -1 && errno == EINPROGRESS) {
		ccd = (connect_callback_data*) calloc(1, sizeof(struct connect_callback_data));
		ccd->callback = callback;
		ccd->callback_data = data;
		ccd->id = id;

		ccd->tag = ext_yahoo_add_handler(-1, c, YAHOO_INPUT_WRITE, ccd);
		return ccd->tag;
	} else {
		if(error == -1)
			LOG("Connection failure: %s\n", strerror(errno));

		ext_yahoo_close(c);

		callback(NULL, errno, data);
		return -1;
	}
}


/*************************************
 * Callback handling code starts here
 */
 
void cb_yahoo_url_handle(int id, int fd, int error, const char */*filename*/, unsigned long size, void *data) 
{
	const char *who = (const char*)data;
	char byte;
	BString buff;
	unsigned long count = 0;
	while (count <= size) {
		read(fd,&byte,1);
		count++;
		buff << byte;
	}
	assert( gYahooConnections[id] != NULL );
	LOG(buff.String());
	gYahooConnections[id]->cbGotBuddyIcon(who,size,buff.String());
}


static void connect_complete(void *data, struct _conn *source, yahoo_input_condition condition)
{
	struct connect_callback_data *ccd = (connect_callback_data *)data;
	int error, err_size = sizeof(error);

	ext_yahoo_remove_handler(0, ccd->tag);
	getsockopt(source->fd, SOL_SOCKET, SO_ERROR, &error, (socklen_t *)&err_size);
	// TODO remove this
	if(error)
		goto err;

	LOG("Connected fd: %d, error: %d", source->fd, error);

	if (source->use_ssl) {
		source->ssl = do_ssl_connect(source->fd);

		if (!source->ssl) {
err:
			LOG("SSL Handshake Failed!\n");
			ext_yahoo_close(source);

			ccd->callback(NULL, 0, ccd->callback_data);
			free(ccd);
			return;
		}
	}

	fcntl(source->fd, F_SETFL, O_NONBLOCK);

	ccd->callback(source, error, ccd->callback_data);
	free(ccd);
}


void yahoo_callback(struct conn_handler *c, yahoo_input_condition cond)
{
	LOG("yahoo_callback\n");
	int ret=1;
	char buff[1024]={0};

	if(c->id < 0) {
		connect_complete(c->data, c->con, cond);
	} else {
		if(cond & YAHOO_INPUT_READ)
			ret = yahoo_read_ready(c->id, c->con, c->data);
		if(ret>0 && cond & YAHOO_INPUT_WRITE)
			ret = yahoo_write_ready(c->id, c->con, c->data);

		if(ret == -1)
			snprintf(buff, sizeof(buff), 
				"Yahoo read error (%d): %s", errno, strerror(errno));
		else if(ret == 0)
			snprintf(buff, sizeof(buff), 
				"Yahoo read error: Server closed socket");

		if(buff[0])
			LOG((buff));
	}
}


static void ext_yahoo_got_buzz(int id, const char *me, const char *who, long tm)
{
	LOG("ext_yahoo_got_buzz()\n");
}


static void ext_yahoo_got_ft_data(int id, const unsigned char *in, int count, void *data)
{
	LOG("ext_yahoo_got_ft_data\n");
}


static char *ext_yahoo_get_ip_addr(const char *domain)
{
	LOG("ext_yahoo_get_ip_addr()\n");
	return NULL;
}


static void ext_yahoo_file_transfer_done(int id, int response, void *data)
{

}



static void ext_yahoo_got_buddy_change_group(int id, const char *me, const char *who, 
	const char *old_group, const char *new_group)
{

}



/*
 * Callback handling code ends here
 ***********************************/

static char * get_local_addresses()
{
	static char addresses[1024];
	char buff[1024];
	struct hostent * hn;

	gethostname(buff,sizeof(buff));

	hn = gethostbyname(buff);
	if(hn)
		strncpy(addresses, inet_ntoa( *((struct in_addr*)hn->h_addr)), sizeof(addresses) );
	else
		addresses[0] = 0;

	return addresses;
}


int32
yahoo_io_thread( void * _data )
{
	YahooConnection * conn = (YahooConnection*)_data;
	register_callbacks();

	conn->fID = yahoo_init_with_attributes(conn->fYahooID, conn->fPassword,
				"local_host", "95.252.70.62",
				"pager_port", 5050, NULL);

	LOG("yahoo_io_thread: id: %s, pass: %s\n", conn->fYahooID, conn->fPassword );

	gYahooConnections[conn->fID] = conn;

	yahoo_login(conn->fID, YAHOO_STATUS_AVAILABLE);

	int lfd = 0;

	fd_set inp, outp;
	struct timeval tv;

	while (conn->IsAlive()) {
		snooze(10000);
		FD_ZERO(&inp);
		FD_ZERO(&outp);

		tv.tv_sec=3;
		tv.tv_usec=1E4;
		lfd=0;
		int i;
		
		for(i = 0; i < conn->CountConnections(); i++) {
			struct conn_handler *c = conn->ConnectionAt(i);
			if(c->remove) {
				conn->RemoveConnection(c);
				c->remove = 0;
				free(c);
			} else {
				if(c->cond & YAHOO_INPUT_READ)
					FD_SET(c->con->fd, &inp);
				if(c->cond & YAHOO_INPUT_WRITE)
					FD_SET(c->con->fd, &outp);
				if(lfd < c->con->fd)
					lfd = c->con->fd;
			}
		}

		select(lfd + 1, &inp, &outp, NULL, &tv);
		time(&curTime);

		for(i = 0; i < conn->CountConnections(); i++) {
			struct conn_handler *c = conn->ConnectionAt(i);
			if(c->con->remove) {
				free(c->con);
				c->con = NULL;
				break;
			}
			if(c->remove)
				continue;
			if(FD_ISSET(c->con->fd, &inp))
				yahoo_callback(c, YAHOO_INPUT_READ);
			if(FD_ISSET(c->con->fd, &outp))
				yahoo_callback(c, YAHOO_INPUT_WRITE);
		}

		if(expired(pingTimer))
			yahoo_ping_timeout_callback(conn->fID);
	//	if(expired(webcamTimer))	yahoo_webcam_timeout_callback(webcam_id);
	}
	LOG("Exited loop");

	for(int i = 0; i < conn->CountConnections(); i++) {
		struct conn_handler *c = conn->ConnectionAt(i);
		free(c);
		conn->RemoveConnection(c);
	}
	return 0;
}

extern "C" void register_callbacks()
{
	static struct yahoo_callbacks yc;

	yc.ext_yahoo_login_response = ext_yahoo_login_response;
	yc.ext_yahoo_got_buddies = ext_yahoo_got_buddies;
	yc.ext_yahoo_got_ignore = ext_yahoo_got_ignore;
	yc.ext_yahoo_got_identities = ext_yahoo_got_identities;
	yc.ext_yahoo_got_cookies = ext_yahoo_got_cookies;
	yc.ext_yahoo_status_changed = ext_yahoo_status_changed;
	yc.ext_yahoo_got_im = ext_yahoo_got_im;
	yc.ext_yahoo_got_buzz = ext_yahoo_got_buzz;
	yc.ext_yahoo_got_conf_invite = ext_yahoo_got_conf_invite;
	yc.ext_yahoo_conf_userdecline = ext_yahoo_conf_userdecline;
	yc.ext_yahoo_conf_userjoin = ext_yahoo_conf_userjoin;
	yc.ext_yahoo_conf_userleave = ext_yahoo_conf_userleave;
	yc.ext_yahoo_conf_message = ext_yahoo_conf_message;
	yc.ext_yahoo_chat_cat_xml = ext_yahoo_chat_cat_xml;
	yc.ext_yahoo_chat_join = ext_yahoo_chat_join;
	yc.ext_yahoo_chat_userjoin = ext_yahoo_chat_userjoin;
	yc.ext_yahoo_chat_userleave = ext_yahoo_chat_userleave;
	yc.ext_yahoo_chat_message = ext_yahoo_chat_message;
	yc.ext_yahoo_chat_yahoologout = ext_yahoo_chat_yahoologout;
	yc.ext_yahoo_chat_yahooerror = ext_yahoo_chat_yahooerror;
	yc.ext_yahoo_got_webcam_image = ext_yahoo_got_webcam_image;
	yc.ext_yahoo_webcam_invite = ext_yahoo_webcam_invite;
	yc.ext_yahoo_webcam_invite_reply = ext_yahoo_webcam_invite_reply;
	yc.ext_yahoo_webcam_closed = ext_yahoo_webcam_closed;
	yc.ext_yahoo_webcam_viewer = ext_yahoo_webcam_viewer;
	yc.ext_yahoo_webcam_data_request = ext_yahoo_webcam_data_request;
	yc.ext_yahoo_got_file = ext_yahoo_got_file;
	yc.ext_yahoo_got_ft_data = ext_yahoo_got_ft_data;
	yc.ext_yahoo_get_ip_addr = ext_yahoo_get_ip_addr;
	yc.ext_yahoo_file_transfer_done = ext_yahoo_file_transfer_done;
	yc.ext_yahoo_contact_added = ext_yahoo_contact_added;
	yc.ext_yahoo_rejected = ext_yahoo_rejected;
	yc.ext_yahoo_typing_notify = ext_yahoo_typing_notify;
	yc.ext_yahoo_game_notify = ext_yahoo_game_notify;
	yc.ext_yahoo_mail_notify = ext_yahoo_mail_notify;
	yc.ext_yahoo_got_search_result = ext_yahoo_got_search_result;
	yc.ext_yahoo_system_message = ext_yahoo_system_message;
	yc.ext_yahoo_error = ext_yahoo_error;
	yc.ext_yahoo_log = ext_yahoo_log;
	yc.ext_yahoo_add_handler = ext_yahoo_add_handler;
	yc.ext_yahoo_remove_handler = ext_yahoo_remove_handler;
	yc.ext_yahoo_connect = ext_yahoo_connect;
	yc.ext_yahoo_connect_async = ext_yahoo_connect_async;
	yc.ext_yahoo_read = ext_yahoo_read;
	yc.ext_yahoo_write = ext_yahoo_write;
	yc.ext_yahoo_close = ext_yahoo_close;
	yc.ext_yahoo_got_buddyicon = ext_yahoo_got_buddyicon;
	yc.ext_yahoo_got_buddyicon_checksum = ext_yahoo_got_buddyicon_checksum;
	yc.ext_yahoo_buddyicon_uploaded = ext_yahoo_buddyicon_uploaded;
	yc.ext_yahoo_got_buddyicon_request = ext_yahoo_got_buddyicon_request;
	yc.ext_yahoo_got_ping = ext_yahoo_got_ping;
	yc.ext_yahoo_got_buddy_change_group = ext_yahoo_got_buddy_change_group;
	
	yahoo_register_callbacks(&yc);
}
