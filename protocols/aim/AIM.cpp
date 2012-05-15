/*
 * Copyright 2010, Alexander Botero-Lowry. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#include <stdio.h>
#include <errno.h>
#include <string>

#include <Entry.h>

#include "AIM.h"
#include "CayaProtocolMessages.h"

const char* kProtocolSignature = "aim";
const char* kProtocolName = "AOL Instant Messenger";

CayaProtocolMessengerInterface* gServerMsgr;


AIMProtocol::AIMProtocol()
{
}


AIMProtocol::~AIMProtocol()
{
	Shutdown();
}


status_t
AIMProtocol::Init(CayaProtocolMessengerInterface* msgr)
{
	gServerMsgr = msgr;

	fIMCommHandle = imcomm_create_handle();
	fOnline = false;

	// Register callbacks
	imcomm_register_callback(fIMCommHandle, IMCOMM_IM_INCOMING, (void (*)())GotMessage);
	imcomm_register_callback(fIMCommHandle, IMCOMM_IM_SIGNON, (void (*)())BuddyOnline);
	imcomm_register_callback(fIMCommHandle, IMCOMM_IM_SIGNOFF, (void (*)())BuddyOffline);
	imcomm_register_callback(fIMCommHandle, IMCOMM_IM_BUDDYAWAY, (void (*)())BuddyAway);
	imcomm_register_callback(fIMCommHandle, IMCOMM_IM_BUDDYUNAWAY, (void (*)())BuddyBack);
	imcomm_register_callback(fIMCommHandle, IMCOMM_IM_AWAYMSG, (void (*)())BuddyAwayMsg);
	imcomm_register_callback(fIMCommHandle, IMCOMM_IM_IDLEINFO, (void (*)())BuddyIdle);
	imcomm_register_callback(fIMCommHandle, IMCOMM_IM_PROFILE, (void (*)())BuddyProfile);

	// TODO: Missing IMCOMM_ERROR, IMCOMM_FORMATTED_SN, IMCOMM_HANDLE_DELETED

	return B_OK;
}


status_t
AIMProtocol::Shutdown()
{
    LogOff();
    return B_OK;
}


void
AIMProtocol::_NotifyProgress(const char* title, const char* message, float progress)
{
	BMessage msg(IM_MESSAGE);
	msg.AddInt32("im_what", IM_PROGRESS);
	msg.AddString("title", title);
	msg.AddString("message", message);
	msg.AddFloat("progress", progress);
	_SendMessage(&msg);
}


void
AIMProtocol::_Notify(notification_type type, const char* title, const char* message)
{
	BMessage msg(IM_MESSAGE);
	msg.AddInt32("im_what", IM_NOTIFICATION);
	msg.AddInt32("type", (int32)type);
	msg.AddString("title", title);
	msg.AddString("message", message);
	_SendMessage(&msg);
}


void
AIMProtocol::_SendMessage(BMessage* msg)
{
	// Skip invalid messages
	if (!msg)
		return;

	msg->AddString("protocol", kProtocolSignature);
	gServerMsgr->SendMessage(msg);
}


status_t
AIMProtocol::Process(BMessage* msg)
{
	switch (msg->what) {
		case IM_MESSAGE: {
			int32 im_what = 0;
 
			msg->FindInt32("im_what", &im_what);

			switch (im_what) {
				case IM_SET_OWN_STATUS: {
					int32 status = msg->FindInt32("status");

					BString status_msg("");
					msg->FindString("message", &status_msg);
					char* smsg = strdup(status_msg.String());

					switch (status) {
						case CAYA_ONLINE:
							if (!fOnline)
								A_LogOn();
							else
								imcomm_set_unaway(fIMCommHandle);
							break;
						case CAYA_AWAY:
							imcomm_set_away(fIMCommHandle, smsg);
							break;
						case CAYA_CUSTOM_STATUS:
							//imcomm_set_away(fIMCommHandle, smsg);
							//UnsupportedOperation(); ?
							break;
						case CAYA_DO_NOT_DISTURB:
							imcomm_set_away(fIMCommHandle, smsg);
							//UnsupportedOperation(); ?
							break;
						case CAYA_OFFLINE:
							LogOff();
							break;
						default:
							break;
					}

					free(smsg);

					BMessage msg(IM_MESSAGE);
					msg.AddInt32("im_what", IM_STATUS_SET);
					msg.AddString("protocol", kProtocolSignature);
					msg.AddInt32("status", status);
					gServerMsgr->SendMessage(&msg);
					break;
				}
				case IM_SET_NICKNAME:
					UnsupportedOperation();
					break;
				case IM_SEND_MESSAGE: {
					const char* buddy = msg->FindString("id");
					const char* sms = msg->FindString("body");
					imcomm_im_send_message(fIMCommHandle, buddy, sms, 0);

					// XXX send a message to let caya know we did it
					BMessage msg(IM_MESSAGE);
					msg.AddInt32("im_what", IM_MESSAGE_SENT);
					msg.AddString("protocol", kProtocolSignature);
					msg.AddString("id", buddy);
					msg.AddString("body", sms);

					gServerMsgr->SendMessage(&msg);
					break;
				}
				case IM_REGISTER_CONTACTS: {
					const char* buddy = NULL;
					char *buddy_copy;

					for (int32 i = 0; msg->FindString("id", i, &buddy) == B_OK; i++) {
						buddy_copy = strdup(buddy);
						imcomm_im_add_buddy(fIMCommHandle, buddy_copy);
						free(buddy_copy);
					}
					break;
				}
				case IM_UNREGISTER_CONTACTS: {
					const char* buddy = NULL;

					for (int32 i = 0; msg->FindString("id", i, &buddy) == B_OK; i++)
						imcomm_im_remove_buddy(fIMCommHandle, buddy);
					break;
				}
				case IM_USER_STARTED_TYPING:
					//UnsupportedOperation();
					break;
				case IM_USER_STOPPED_TYPING:
					//UnsupportedOperation();
					break;
				case IM_GET_CONTACT_INFO:
					UnsupportedOperation();
					break;
				case IM_ASK_AUTHORIZATION:
				case IM_AUTHORIZATION_RECEIVED:
				case IM_AUTHORIZATION_REQUEST:
				case IM_AUTHORIZATION_RESPONSE:
				case IM_CONTACT_AUTHORIZED:
					UnsupportedOperation();
					break;
				case IM_SPECIAL_TO_PROTOCOL:
						UnsupportedOperation();
					break;
				default:
					return B_ERROR;
			}
			break;
		}
		default:
			// We don't handle this what code
			return B_ERROR;
	}

    return B_OK;
}


void
AIMProtocol::UnsupportedOperation()
{
}


const char*
AIMProtocol::Signature() const
{
	return kProtocolSignature;
}


const char*
AIMProtocol::FriendlySignature() const
{
	return kProtocolName;
}


status_t
AIMProtocol::UpdateSettings(BMessage* msg)
{
	const char* username = NULL;
	const char* password = NULL;

	msg->FindString("username", &username);
	msg->FindString("password", &password);
	//msg->FindString("server", &server);
	//msg->FindInt32("port", &server);

	if ((username == NULL) || (password == NULL)) {
		printf("Invalid settings");
		return B_ERROR;
	}

	if ((fUsername != username) || (fPassword != password)) {
		fUsername = username;
		fPassword = password;
		// XXX kill the handle and sign back in?
	}

	return B_OK;
}


uint32
AIMProtocol::GetEncoding()
{
	return 0xffff; // No conversion, AIMProtocol handles UTF-8 ???
}


CayaProtocolMessengerInterface*
AIMProtocol::MessengerInterface() const
{
	return gServerMsgr;
}


uint32
AIMProtocol::Version() const
{
	return CAYA_VERSION_1_PRE_ALPHA_1;
}


status_t
AIMProtocol::A_LogOn()
{
    int ret = imcomm_im_signon(fIMCommHandle, fUsername, fPassword);
    if (ret == IMCOMM_RET_OK) {
		fIMCommThread = spawn_thread(WaitForData, "imcomm receiver",
			B_LOW_PRIORITY, this);
		resume_thread(fIMCommThread);

		BString content(fUsername);
		content << " has logged in!";
		_Notify(B_INFORMATION_NOTIFICATION, "Connected",
		content.String());

		fOnline = true;

		return B_OK;
	}

	return B_ERROR;
}


status_t
AIMProtocol::LogOff()
{
	fOnline = false;
	imcomm_delete_handle_now(fIMCommHandle);
	BString content(fUsername);
	content << " has logged out!";
	_Notify(B_INFORMATION_NOTIFICATION, "Disconnected",
			content.String());

	return B_OK;
}


int32
AIMProtocol::WaitForData(void* aimProtocol)
{
	// XXX No need for aimProtocol since imcomm does its own callback thing
	fd_set readset;
	struct timeval tm;

	while (1) {
		FD_ZERO(&readset);
		tm.tv_sec = 2;
		tm.tv_usec = 500000;

		IMCOMM_RET ret = imcomm_select(1, &readset, NULL, NULL, &tm);
		if (ret != IMCOMM_RET_OK && errno == EINTR)
			continue;
	}

	return 0;
}


void
AIMProtocol::GotMessage(void* imcomm, char* who, int aut, char* recvmsg)
{
	BMessage msg(IM_MESSAGE);
	msg.AddInt32("im_what", IM_MESSAGE_RECEIVED);
	msg.AddString("protocol", kProtocolSignature);
	msg.AddString("id", who);
	msg.AddString("body", strip_html(recvmsg));

	gServerMsgr->SendMessage(&msg);
}


void
AIMProtocol::BuddyOnline(void* imcomm, char* who)
{
	BMessage msg(IM_MESSAGE);
	msg.AddInt32("im_what", IM_STATUS_SET);
	msg.AddString("protocol", kProtocolSignature);
	msg.AddString("id", who);
	msg.AddInt32("status", CAYA_ONLINE);

	gServerMsgr->SendMessage(&msg);
}


void
AIMProtocol::BuddyOffline(void* imcomm, char* who)
{
	BMessage msg(IM_MESSAGE);
	msg.AddInt32("im_what", IM_STATUS_SET);
	msg.AddString("protocol", kProtocolSignature);
	msg.AddString("id", who);
	msg.AddInt32("status", CAYA_OFFLINE);

	gServerMsgr->SendMessage(&msg);
}


void
AIMProtocol::BuddyAway(void* imcomm, char* who)
{
	imcomm_request_awaymsg(imcomm, who);
}


void
AIMProtocol::BuddyBack(void* imcomm, char* who)
{
	BMessage msg(IM_MESSAGE);
	msg.AddInt32("im_what", IM_STATUS_SET);
	msg.AddString("protocol", kProtocolSignature);
	msg.AddString("id", who);
	msg.AddInt32("status", CAYA_ONLINE);

	gServerMsgr->SendMessage(&msg);
}


void
AIMProtocol::BuddyAwayMsg(void* imcomm, char* who, char* awaymsg)
{
	BMessage msg(IM_MESSAGE);
	msg.AddInt32("im_what", IM_STATUS_SET);
	msg.AddString("protocol", kProtocolSignature);
	msg.AddString("id", who);
	msg.AddInt32("status", CAYA_AWAY);
	msg.AddString("message", strip_html(awaymsg));

	gServerMsgr->SendMessage(&msg);
}


void
AIMProtocol::BuddyIdle(void* imcomm, char* who, long idletime)
{
	BMessage msg(IM_MESSAGE);
	msg.AddInt32("im_what", IM_STATUS_SET);
	msg.AddString("protocol", kProtocolSignature);
	msg.AddString("id", who);
	msg.AddInt32("status", CAYA_ONLINE);

	gServerMsgr->SendMessage(&msg);
}


void
AIMProtocol::BuddyProfile(void* handle, char* who, char* profile)
{
	// XXX vcard is probably an issue
}


char *
AIMProtocol::strip_html(const char* message)
{
	char* temp;
	uint32 x, xnot, y, count, inhtml;
	size_t len = strlen(message);

	temp = (char*)malloc(len + 1);
	for (x = 0, count = 0, inhtml = 0; x < len; x++) {
		if (message[x] == '<' && inhtml == 0) {
			if (x + 10 < len) {
				// Convert links into [http://url] link text
				if (strncasecmp(message + x, "<a href=\"", 9) == 0) {
					xnot = x + 9;

					for (y = xnot; y < len; y++) {
						if (message[y] == '\"') {
							// We don't have to worry about the buffer size,
							// because it's guaranteed to be bigger
							memcpy(temp + count, "[", 1);
							memcpy(temp + count + 1, message + xnot,
							       y - xnot);
							memcpy(temp + count + 1 + (y - xnot), "] ", 2);
							count += y - xnot + 3;
							x = y;
							break;
						}
					}
				}
			}

			if (x + 3 < len) {
				if (strncasecmp(message + x, "<br>", 4) == 0) {
					temp[count] = '\n';
					count++;
					x += 3;
					continue;
				}
			}

			inhtml = 1;
			continue;
		}

		if (inhtml) {
			if (message[x] == '>')
				inhtml = 0;

			continue;
		}

		if (message[x] == '&') {
			if (x + 4 < len) {
				if (strncmp(message + x, "&amp;", 5) == 0) {
					temp[count] = '&';
					count++;
					x += 4;
					continue;
				}
			}

			if (x + 5 < len) {
				if (strncmp(message + x, "&quot;", 6) == 0) {
					temp[count] = '\"';
					count++;
					x += 5;
					continue;
				}

				if (strncmp(message + x, "&nbsp;", 6) == 0) {
					temp[count] = ' ';
					count++;
					x += 5;
					continue;
				}
			}

			if (x + 3 < len) {
				if (strncmp(message + x, "&lt;", 4) == 0) {
					temp[count] = '<';
					count++;
					x += 3;
					continue;
				}
			}

			if (x + 3 < len) {
				if (strncmp(message + x, "&gt;", 4) == 0) {
					temp[count] = '>';
					count++;
					x += 3;
					continue;
				}
			}
		}

		if (message[x] == '\n' || message[x] == '\r')
			continue;
		else
			temp[count] = message[x];
		count++;
	}

	temp = (char*)realloc(temp, count + 1);
	temp[count] = 0;
	return temp;
}
