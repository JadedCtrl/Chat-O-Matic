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

void            buddy_online(void *h, const char *who);
void            buddy_offline(void *h, const char *who);
void            idleinfo(void *, const char *, unsigned long);
void            error(void *h, int errornum);
void            incoming_im(void *, const char *, const int, const char *);
char           *strip_html(char *message);

void
incoming_im(void *handle, const char *who, const int automessage,
	    const char *message)
{
	char           *msg;
	msg = strip_html((char *) message);
	printf("[%s] %s\n", who, msg);

	if (strcmp(msg, "quit now") == 0) {
#ifdef MACINTOSH_CLASSIC
		mactcp_close(handle);
		CloseResolver();
#endif
		free(msg);
		exit(0);
	}
	free(msg);
}

int
main(int argc, char *argv[])
{
	void           *handle;

#ifndef MACINTOSH_CLASSIC
	fd_set          readfs;
	struct timeval  tm;

	tm.tv_sec = 2;
	tm.tv_usec = 150000;
#endif

	handle = imcomm_create_handle();
	imcomm_register_callback(handle, IMCOMM_IM_SIGNON, buddy_online);
	imcomm_register_callback(handle, IMCOMM_IM_SIGNOFF, buddy_offline);
	imcomm_register_callback(handle, IMCOMM_IM_IDLEINFO, idleinfo);
	imcomm_register_callback(handle, IMCOMM_ERROR, error);
	imcomm_register_callback(handle, IMCOMM_IM_INCOMING, incoming_im);
	imcomm_im_signon(handle, argv[1], argv[2]);
	printf("Starting IMComm test program...\n");

	while (1) {
#ifdef MACINTOSH_CLASSIC
		imcomm_select(NULL, NULL, NULL, NULL, NULL);
#else
		FD_ZERO(&readfs);
#if 0
		FD_SET(fileno(stdin), &readfs);
#endif
		imcomm_select(0, &readfs, NULL, NULL, &tm);
		fflush(stdout);
#endif
	}

	return 0;
}

void
buddy_online(void *h, const char *who)
{
	printf(">>> %s is now online\n", who);
	return;
}

void
buddy_offline(void *h, const char *who)
{
	printf("<<< %s is now offline\n", who);
	return;
}

void
idleinfo(void *h, const char *who, unsigned long time)
{
	printf("*** Idle: %s, %ld\n", who, time);
	return;
}

void
error(void *h, int errornum)
{
	printf("*** ");

	switch (errornum) {
	case IMCOMM_STATUS_CONNECTED:
		printf("Connected.\n");
		break;
	case IMCOMM_ERROR_INVALID_LOGIN:
		printf("Login failed.\n");
		break;
	case IMCOMM_ERROR_DISCONNECTED:
		printf("Disconnected.\n");
		break;
	case IMCOMM_ERROR_OTHER_SIGNON:
		printf
			("You've been disconnected because you signed on at a different location.\n");
		break;
	case IMCOMM_STATUS_AUTHDONE:
		printf("Authentication succeeded.\n");
		break;
	default:
		printf("Unknown error type.\n");
		break;
	}
}

char           *
strip_html(char *message)
{
	char           *temp;
	int             x, count, inhtml;

	temp = malloc(strlen(message) + 1);
	for (x = 0, count = 0, inhtml = 0; x < strlen(message); x++) {
		if (message[x] == '&') {
			if (x + 4 < strlen(message)) {
				if (strncmp(message + x, "&amp;", 5) == 0) {
					temp[count] = '&';
					count++;
					x += 4;
					continue;
				}
			}
			if (x + 5 < strlen(message)) {
				if (strncmp(message + x, "&quot;", 6) == 0) {
					temp[count] = '\"';
					count++;
					x += 5;
					continue;
				}
			}
			if (x + 3 < strlen(message)) {
				if (strncmp(message + x, "&lt;", 4) == 0) {
					temp[count] = '<';
					count++;
					x += 3;
					continue;
				}
			}
			if (x + 3 < strlen(message)) {
				if (strncmp(message + x, "&gt;", 4) == 0) {
					temp[count] = '>';
					count++;
					x += 3;
					continue;
				}
			}
		}
		if (message[x] == '<')
			inhtml = 1;
		if (inhtml) {
			if (message[x] == '>')
				inhtml = 0;
			continue;
		}
		if (message[x] == '\n' || message[x] == '\r')
			temp[count] = ' ';
		else
			temp[count] = message[x];
		count++;
	}

	temp = realloc(temp, count + 1);
	temp[count] = 0;
	return temp;
}
