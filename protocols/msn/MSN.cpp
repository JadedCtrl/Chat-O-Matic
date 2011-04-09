/*
 * Copyright 2010, Dario Casalinuovo. All rights reserved.
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
 */

#include <stdio.h>
#include <string>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <syslog.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netdb.h>
#include <openssl/ssl.h>

#include <Entry.h>

#include <msn.h>

#include "MSN.h"

extern const char* kProtocolSignature = "msn";
extern const char* kProtocolName = "MSN Protocol";

struct pollfd *kPollSockets = NULL;
struct ssl {
	bool isSSL;
	bool isConnected;
	SSL *ssl;
	SSL_CTX *ctx;
} *kSocketsSsl = NULL;

int kSocketsCount = 0;
int kSocketsAvailable = 0;
int32 StartPollThread(void* punt)
{
	MSNP* mainClass = (MSNP*) punt;

	MSN::NotificationServerConnection* mainConnection = mainClass->GetConnection();
	mainConnection->connect("messenger.hotmail.com", 1863);
	while (1) {
		if (kPollSockets == NULL)
			continue;

		poll(kPollSockets, kSocketsCount, -1);

		for (int i = 0; i < kSocketsCount; i++) {
			if (kPollSockets[i].fd == -1) {
				continue;
			}
			if (kPollSockets[i].revents & POLLHUP) {
				kPollSockets[i].revents = 0;
				continue;
			}
			if (kPollSockets[i].revents & (POLLIN | POLLOUT | POLLPRI)) {
				MSN::Connection *c;

				c = mainConnection->connectionWithSocket((void*)kPollSockets[i].fd);

				if (c != NULL) {
					// TODO make the ssl code more styled and less bugged
					if (kSocketsSsl[i].isSSL && !kSocketsSsl[i].isConnected) {
						BIO *bio_socket_new;
						SSL_METHOD *meth=NULL;
						meth=const_cast<SSL_METHOD*>(SSLv23_client_method());
						SSL_library_init();
						kSocketsSsl[i].ctx = SSL_CTX_new(meth);
						kSocketsSsl[i].ssl = SSL_new(kSocketsSsl[i].ctx);
						bio_socket_new = BIO_new_socket(kPollSockets[i].fd, BIO_CLOSE);
						if (!kSocketsSsl[i].ssl)
							break;
						BIO_set_nbio(bio_socket_new, 0);
						SSL_set_bio(kSocketsSsl[i].ssl, bio_socket_new, bio_socket_new);
						SSL_set_mode(kSocketsSsl[i].ssl, SSL_MODE_AUTO_RETRY);

						// TODO - fix-me - not async and buggy
						// and handle errors
						/*int ret =*/ SSL_connect(kSocketsSsl[i].ssl);
						kSocketsSsl[i].isConnected = true;
					}

					if (c->isConnected() == false)
						c->socketConnectionCompleted();

					if (kPollSockets[i].revents & POLLIN) {
						if (kSocketsSsl[i].isSSL && kSocketsSsl[i].isConnected) {
							 if (SSL_want_read(kSocketsSsl[i].ssl)) {
								 kPollSockets[i].revents = 0;
								 continue;
							 }
						}
						c->dataArrivedOnSocket();
					}

					if (kPollSockets[i].revents & POLLOUT) {
						c->socketIsWritable();
					}
				}
			}

			if (kPollSockets[i].revents & (POLLERR | POLLNVAL)) {
				MSN::Connection *c;

				c = mainConnection->connectionWithSocket((void*)kPollSockets[i].fd);

				if (c != NULL) {
					delete c;
				}
				kPollSockets[i].fd = -1;
				kPollSockets[i].revents = 0;
				continue;
			}
		}
		
		if (kPollSockets[0].revents & POLLIN) {
			kPollSockets[0].revents = 0;
		}
	}
}


MSNP::MSNP()
	: fUsername(""),
	fServer("messenger.hotmail.com"),
	fPassword(""),
	fMainConnection(NULL),
	fSettings(false)
{

}


MSNP::~MSNP()
{
	Shutdown();
}


status_t
MSNP::Init(CayaProtocolMessengerInterface* msgr)
{
	fServerMsgr = msgr;
	fLogged = false;

	fClientID = 0;
	fClientID += MSN::MSNC7;
	fClientID += MSN::MSNC6;
	fClientID += MSN::MSNC5;
	fClientID += MSN::MSNC4;
	fClientID += MSN::MSNC3;
	fClientID += MSN::MSNC2;
	fClientID += MSN::MSNC1;
	fClientID += MSN::SupportWinks;
	fClientID += MSN::VoiceClips;
	fClientID += MSN::InkGifSupport;
	fClientID += MSN::SIPInvitations;
	fClientID += MSN::SupportMultiPacketMessaging;

	return B_OK;
}


status_t
MSNP::Shutdown()
{
//	LogOut();

	return B_OK;
}


status_t
MSNP::Process(BMessage* msg)
{
//	printf("Process()\n");
//	msg->PrintToStream();

	switch (msg->what) {
		case IM_MESSAGE:
		{
			int32 im_what = 0;

			msg->FindInt32("im_what", &im_what);

			switch (im_what) {
				case IM_SET_OWN_NICKNAME:
				{
					BString nick;

					if (msg->FindString("nick", &nick) == B_OK) {
						if (fLogged) {
							fMainConnection->setFriendlyName(nick.String(), true);
							fNickname = nick.String();
						} else {
							fNickname = nick.String();
						}
					}
					break;
				}
				case IM_SET_OWN_STATUS:
				{
					int32 status = msg->FindInt32("status");
					BString status_msg("");
					msg->FindString("message", &status_msg);

					switch (status) {
						case CAYA_ONLINE:
							if (fLogged) {
								fMainConnection->setState(MSN::STATUS_AVAILABLE, fClientID);
							} else if (fSettings) {
								if (fMainConnection != NULL) {
									break;
								}
								if (fUsername == "")
									Error("Empty Username!", NULL);
								if (fPassword == "")
									Error("Empty Password!",NULL);
								Progress("MSN Protocol: Login", "MSNP: Connecting...", 0.0f);
								MSN::Passport username;
								try {
									username = MSN::Passport(fUsername.c_str());
								} catch (MSN::InvalidPassport & e) {
									Error("MSN Protocol: Invalid Passport!", "Error!");
									return B_ERROR;
								}
								MSN::Callbacks* cb = dynamic_cast<MSN::Callbacks*> (this);
								fMainConnection = new MSN::NotificationServerConnection(username,
									fPassword.String(), *cb);

								resume_thread(fPollThread);
							} else {
								Error("MSN Protocol: Settings Error", NULL);
							}
							break;
						case CAYA_AWAY:
							if (fLogged) {
								fMainConnection->setState(MSN::STATUS_AWAY, fClientID);
							} else {
								Error("MSN Protocol: You are not logged!", NULL);
							}
							break;
						case CAYA_EXTENDED_AWAY:

							break;
						case CAYA_DO_NOT_DISTURB:
							if (fLogged) {
								fMainConnection->setState(MSN::STATUS_IDLE, fClientID);
							} else {
								Error("MSN Protocol: You are not logged!", NULL);
							}
							break;
						case CAYA_OFFLINE:
							if (fLogged) {
								fMainConnection->disconnect();
								delete fMainConnection;
								int end = fBuddyList.CountItems();
								int x;
								for (x=0; x != end; x++) {
									MSN::Buddy contact = fBuddyList.ItemAt(x);
									if (contact.lists & MSN::LST_AL ) {
										BMessage msg(IM_MESSAGE);
										msg.AddInt32("im_what", IM_STATUS_SET);
										msg.AddString("protocol", kProtocolSignature);
										msg.AddString("id", contact.userName.c_str());
										msg.AddInt32("status", CAYA_OFFLINE);
										fServerMsgr->SendMessage(&msg);
									}
								}
								fLogged = false;
								fSettings = true;
								suspend_thread(fPollThread);
							}
							break;
						default:
//							Error("Invalid", NULL);
//							Log("caya msn plugin :
							break;
					}
					break;
				}
				case IM_SEND_MESSAGE:
				{
					const char* buddy = msg->FindString("id");
					const char* sms = msg->FindString("body");
					const string rcpt_ = buddy;
					const string msg_ = sms;
					int x, y;
					bool nouveau = true;
					int count = 0;
					count = fSwitchboardList.CountItems();
					if (count != 0) {
						for (x=0; x < count; x++) {
							if (fSwitchboardList.ItemAt(x)->first == rcpt_) {
								if (fSwitchboardList.ItemAt(x)->second->isConnected()) {
									nouveau = false;
									y = x;
								} else {
									delete fSwitchboardList.ItemAt(x)->second;
									//delete fSwitchboardList.ItemAt(x)->first;
									fSwitchboardList.RemoveItemAt(x);
								}
								break;
							}
						}
					}

					if (nouveau) {
						const pair<string, string> *ctx
							= new pair<string, string>(rcpt_, msg_);
						// request a new switchboard connection
						fMainConnection->requestSwitchboardConnection(ctx);

					} else {
						MSN::SwitchboardServerConnection* conn = fSwitchboardList.ItemAt(y)->second;
						conn->sendTypingNotification();
						conn->sendMessage(msg_);

						BMessage msg(IM_MESSAGE);
						msg.AddInt32("im_what", IM_MESSAGE_SENT);
						msg.AddString("protocol", kProtocolSignature);
						msg.AddString("id", buddy);
						msg.AddString("body", sms);
						fServerMsgr->SendMessage(&msg);
					}
					break;
				}
				case IM_REGISTER_CONTACTS:
				{

					break;
				}
				case IM_UNREGISTER_CONTACTS:
				{

					break;
				}
				case IM_USER_STARTED_TYPING:
				{
					const char* id = msg->FindString("id");

					int x;
					int count = 0;
					count = fSwitchboardList.CountItems();
					if (count != 0) {
						for (x=0; x < count; x++) {
							if (fSwitchboardList.ItemAt(x)->first == id) {
								fSwitchboardList.ItemAt(x)->second->sendTypingNotification();
								break;
							}
						}
					}
					break;
				}
				case IM_USER_STOPPED_TYPING:
				{

					break;
				}
				case IM_GET_CONTACT_INFO:

					break;
				default:
					// We don't handle this im_what code
					//LOG(kProtocolName, liDebug, "Got unhandled message: %ld", im_what);
					//msg->PrintToStream();
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


const char*
MSNP::Signature() const
{
	return kProtocolSignature;
}


const char*
MSNP::FriendlySignature() const
{
	return kProtocolName;
}


status_t
MSNP::UpdateSettings(BMessage* msg)
{
	printf("updatesettings\n");
	const char* usernm = NULL;
	const char* password = NULL;
//	const char* res = NULL;

	msg->FindString("username", &usernm);
	msg->FindString("password", &password);
//	msg->FindString("resource", &res);

	if ((usernm == NULL) || (password == NULL)) {
		Error("Error: Username or Password Empty",NULL);
		return B_ERROR;
	}

	fUsername = usernm;
	fPassword.SetTo(password);

	fSettings = true;

	fPollThread = spawn_thread(StartPollThread,
		"MSNP-PollThread", B_NORMAL_PRIORITY, this);

	return B_OK;
}


uint32
MSNP::GetEncoding()
{
return 0xffff;
}

// other stuff

void
MSNP::Error(const char* message, const char* who)
{
	BMessage msg(IM_ERROR);
	msg.AddString("protocol", kProtocolName);
	if (who)
		msg.AddString("id", who);
	msg.AddString("error", message);

	fServerMsgr->SendMessage( &msg );
}


void
MSNP::Progress(const char* id, const char* message, float progress)
{
	BMessage msg(IM_MESSAGE);
	msg.AddInt32("im_what", IM_PROGRESS );
	msg.AddString("protocol", kProtocolName);
	msg.AddString("progressID", id);
	msg.AddString("message", message);
	msg.AddFloat("progress", progress);
	msg.AddInt32("state", 11); //IM_impsConnecting );

	fServerMsgr->SendMessage(&msg);
}

uint32
MSNP::Version() const
{
	return CAYA_VERSION_1_PRE_ALPHA_1;
}

void
MSNP::SendContactInfo(MSN::Buddy* buddy)
{
	int32 what = IM_CONTACT_INFO;
	BMessage msg(IM_MESSAGE);
	msg.AddInt32("im_what", what);
	msg.AddString("protocol", kProtocolSignature);
	msg.AddString("id", buddy->userName.c_str());
	msg.AddString("name", buddy->friendlyName.c_str());
	msg.AddString("email", buddy->userName.c_str());
	// Send contact information
	fServerMsgr->SendMessage(&msg);
}


void
MSNP::MessageFromBuddy(const char* mess, const char* id)
{
	BMessage msg(IM_MESSAGE);
	msg.AddInt32("im_what", IM_MESSAGE_RECEIVED);
	msg.AddString("protocol", kProtocolSignature);
	msg.AddString("id", id);
	msg.AddString("body", mess);
	fServerMsgr->SendMessage(&msg);
}


/********************/
/* libmsn callbacks */
/********************/

void MSNP::registerSocket(void *s, int reading, int writing, bool isSSL)
{
//	printf("MSNP::registerSocket %d %d %d\n", reading, writing, isSSL);
	int x = 0;
	if (kSocketsCount == kSocketsAvailable) {

		kPollSockets = (struct pollfd*) realloc(kPollSockets, (kSocketsAvailable + 10) * sizeof(struct pollfd));
		if (kPollSockets == NULL) {
			Error("Memory Error!!\n", NULL);
			return;
		}

		kSocketsSsl = (struct ssl*) realloc(kSocketsSsl, (kSocketsAvailable + 10) * sizeof(struct ssl));
		if (kSocketsSsl == NULL) {
			Error("Memory Error!!\n", NULL);
			return;
		}
		kSocketsAvailable += 10;	
		for (x=kSocketsCount; x < kSocketsAvailable; x++) {
			kPollSockets[x].fd = -1;
			kPollSockets[x].events = 0;
			kPollSockets[x].revents = 0;
			kSocketsSsl[x].isSSL = false;
			kSocketsSsl[x].isConnected = false;
			kSocketsSsl[x].ctx = NULL;
			kSocketsSsl[x].ssl = NULL;
		}
		x=kSocketsCount;
		kPollSockets[x].fd = getSocketFileDescriptor(s);
		kPollSockets[x].revents = 0;
		kPollSockets[x].events = 0;

		if (reading)
			kPollSockets[x].events |= POLLIN;
		if (writing)
			kPollSockets[x].events |= POLLOUT;
		if (isSSL)
			kSocketsSsl[x].isSSL = true;

		kSocketsCount++;
		return;
	}

	if (kPollSockets != NULL) {
		for (x = 0; x < kSocketsCount+kSocketsAvailable; x++) {
			if (getSocketFileDescriptor(s) == kPollSockets[x].fd || kPollSockets[x].fd == -1) {
				if(kPollSockets[x].fd == -1)
					kSocketsCount++;

				kPollSockets[x].fd = getSocketFileDescriptor(s);

				kPollSockets[x].events = 0;
				
				if (reading)
					kPollSockets[x].events |= POLLIN;
				if (writing)
					kPollSockets[x].events |= POLLOUT;
				if (isSSL)
					kSocketsSsl[x].isSSL = true;
				return;
			}
		}
	}
}


int
MSNP::getSocketFileDescriptor (void *sock)
{
	long a = (long)sock;
	return (int)a;
}


void MSNP::closeSocket(void *s)
{
//printf("MSNP::closeSocket\n");
	int x;
	int i;
	for (x = 0; x < kSocketsCount; x++) {
		if (getSocketFileDescriptor(s) == kPollSockets[x].fd) {
			close(getSocketFileDescriptor(s));
			if (kSocketsSsl[x].isSSL) {
				if (kSocketsSsl[x].ssl) {
					SSL_set_shutdown(kSocketsSsl[x].ssl,
						SSL_SENT_SHUTDOWN|SSL_RECEIVED_SHUTDOWN);
					SSL_free(kSocketsSsl[x].ssl);

					if (kSocketsSsl[x].ctx)
						SSL_CTX_free(kSocketsSsl[x].ctx);
				}
			}
			for (i = x; i < kSocketsCount; i++) {
				kPollSockets[i] = kPollSockets[i+1];		
				kSocketsSsl[i] = kSocketsSsl[i+1];
			}
			i = kSocketsCount;
			kPollSockets[i].fd = -1;
			kPollSockets[i].revents = 0;
			kPollSockets[i].events = 0;
			kSocketsSsl[i].isConnected = false;
			kSocketsSsl[i].isSSL = false;
			kSocketsSsl[i].ctx = NULL;
			kSocketsSsl[i].ssl = NULL;
		}
	}
	kSocketsCount--;
}


void MSNP::unregisterSocket(void *s)
{
//	printf("MSNP::unregisterSocket");
	for (int x = 0; x < kSocketsCount; x++) {
		if (kPollSockets[x].fd == getSocketFileDescriptor(s))
			kPollSockets[x].events = 0;
	}
}


void MSNP::gotFriendlyName(MSN::NotificationServerConnection * conn, std::string friendlyname)
{
	BMessage msg(IM_MESSAGE);
	msg.AddInt32("im_what", IM_OWN_CONTACT_INFO);
	msg.AddString("protocol", kProtocolSignature);
	msg.AddString("id", fUsername.c_str());
	msg.AddString("name", friendlyname.c_str());
	fServerMsgr->SendMessage(&msg);

}


void MSNP::gotBuddyListInfo(MSN::NotificationServerConnection* conn, MSN::ListSyncInfo* info)
{
	std::map<std::string, MSN::Buddy *>::iterator i = info->contactList.begin();
	std::map<std::string, int> allContacts;
	//Progress("MSN Protocol Login", "MSN Protocol: Logged in!", 0.90);
	for (; i != info->contactList.end(); i++) {
		MSN::Buddy *contact = (*i).second;
		if (contact->lists & MSN::LST_AB ) {
			allContacts[contact->userName.c_str()]=0;
			allContacts[contact->userName.c_str()] |= MSN::LST_AB;
		}
		if (contact->lists & MSN::LST_AL) {
			allContacts[contact->userName.c_str()] |= MSN::LST_AL;
			printf("-AL %s \n", contact->userName.c_str());
			fBuddyList.AddItem(*contact);
		}

		if (contact->lists & MSN::LST_BL) {
			allContacts[contact->userName.c_str()] |= MSN::LST_BL;
		}

		if (contact->lists & MSN::LST_RL) {
			printf("-RL %s \n", contact->userName.c_str());
		}
		if (contact->lists & MSN::LST_PL) {
			printf("-PL %s \n", contact->userName.c_str());
		}
	}
	conn->completeConnection(allContacts,info);
}


void MSNP::gotLatestListSerial(MSN::NotificationServerConnection * conn, std::string lastChange)
{

}


void MSNP::gotGTC(MSN::NotificationServerConnection * conn, char c)
{

}


void MSNP::gotOIMDeleteConfirmation(MSN::NotificationServerConnection * conn, bool success, std::string id)
{

}


void MSNP::gotOIMSendConfirmation(MSN::NotificationServerConnection * conn, bool success, int id)
{

}


void MSNP::gotOIM(MSN::NotificationServerConnection * conn, bool success, std::string id, std::string message)
{

}


void MSNP::gotOIMList(MSN::NotificationServerConnection * conn, std::vector<MSN::eachOIM> OIMs)
{

}


void MSNP::connectionReady(MSN::Connection * conn)
{
	fLogged = true;
	BMessage serverBased(IM_MESSAGE);
	serverBased.AddInt32("im_what", IM_CONTACT_LIST);
	serverBased.AddString("protocol", kProtocolSignature);

	int end = fBuddyList.CountItems();
	int x;
	for (x=0; x != end; x++) {
		MSN::Buddy contact = fBuddyList.ItemAt(x);
		if (contact.lists & MSN::LST_AL ) {
			serverBased.AddString("id", contact.userName.c_str());
		}
	}
	fServerMsgr->SendMessage(&serverBased);

	for (x=0; x != end; x++) {
		MSN::Buddy contact = fBuddyList.ItemAt(x);
		if (contact.lists & MSN::LST_AL ) {
			SendContactInfo(&contact);
		}
	}

	BMessage msg(IM_MESSAGE);
	msg.AddInt32("im_what", IM_OWN_STATUS_SET);
	msg.AddString("protocol", kProtocolSignature);
	msg.AddInt32("status", CAYA_ONLINE);
	fServerMsgr->SendMessage(&msg);
	Progress("MSN Protocol Login", "MSN Protocol: Logged in!", 1.00);
	fMainConnection->setState(MSN::STATUS_AVAILABLE, fClientID);
}


void MSNP::gotBLP(MSN::NotificationServerConnection * conn, char c)
{

}


void MSNP::addedListEntry(MSN::NotificationServerConnection * conn, MSN::ContactList list, MSN::Passport username, std::string friendlyname)
{

}


void MSNP::removedListEntry(MSN::NotificationServerConnection * conn, MSN::ContactList list, MSN::Passport username)
{

}


void MSNP::addedGroup(MSN::NotificationServerConnection * conn, bool added, std::string groupName, std::string groupID)
{

}


void MSNP::removedGroup(MSN::NotificationServerConnection * conn, bool removed, std::string groupID)
{

}


void MSNP::renamedGroup(MSN::NotificationServerConnection * conn, bool renamed, std::string newGroupName, std::string groupID)
{

}


void MSNP::showError(MSN::Connection * conn, std::string msg)
{
//	printf("MSNP::showError: %s", msg.c_str());
}


void MSNP::buddyChangedStatus(MSN::NotificationServerConnection * conn, MSN::Passport buddy, std::string friendlyname, MSN::BuddyStatus status, unsigned int clientID, std::string msnobject)
{
	BMessage msg(IM_MESSAGE);
	msg.AddInt32("im_what", IM_STATUS_SET);
	msg.AddString("protocol", kProtocolSignature);
	msg.AddString("id", buddy.c_str());
	switch(status) {
		case MSN::STATUS_AVAILABLE :
			msg.AddInt32("status", CAYA_ONLINE);
			break;
		case MSN::STATUS_AWAY :
			msg.AddInt32("status", CAYA_AWAY);
			break;
		case MSN::STATUS_IDLE :
			msg.AddInt32("status", CAYA_DO_NOT_DISTURB);
			break;
		case MSN::STATUS_BUSY :
			msg.AddInt32("status", CAYA_DO_NOT_DISTURB);
			break;
		case MSN::STATUS_BERIGHTBACK :
			msg.AddInt32("status", CAYA_AWAY);
			break;
		case MSN::STATUS_ONTHEPHONE :
			msg.AddInt32("status", CAYA_DO_NOT_DISTURB);
			break;
		case MSN::STATUS_OUTTOLUNCH :
			msg.AddInt32("status", CAYA_AWAY);
			break;
		default :
			msg.AddInt32("status", CAYA_OFFLINE);
		break;
	}
	fServerMsgr->SendMessage(&msg);

	// updating the user informations
	BMessage mess(IM_MESSAGE);
	mess.AddInt32("im_what", IM_CONTACT_INFO);
	mess.AddString("protocol", kProtocolSignature);
	msg.AddString("id", buddy.c_str());
	msg.AddString("name", friendlyname.c_str());
	fServerMsgr->SendMessage(&msg);
}


void MSNP::buddyOffline(MSN::NotificationServerConnection * conn, MSN::Passport buddy)
{
	int x;
	int count = 0;
	count = fSwitchboardList.CountItems();
	if (count != 0) {
		for (x = 0; x < count; x++) {
			if (fSwitchboardList.ItemAt(x)->first == buddy) {
				delete fSwitchboardList.ItemAt(x)->second;
				fSwitchboardList.RemoveItemAt(x);
				break;
			}
		}
	}
	BMessage msg(IM_MESSAGE);
	msg.AddInt32("im_what", IM_STATUS_SET);
	msg.AddString("protocol", kProtocolSignature);
	msg.AddString("id", buddy.c_str());
	msg.AddInt32("status", CAYA_OFFLINE);
	fServerMsgr->SendMessage(&msg);
}


void MSNP::gotSwitchboard(MSN::SwitchboardServerConnection * conn, const void * tag)
{
	if (tag) {
		const pair<string, string> *ctx = static_cast<const pair<string, string> *>(tag);
		conn->inviteUser(ctx->first);
	}
}


void MSNP::buddyJoinedConversation(MSN::SwitchboardServerConnection * conn, MSN::Passport username, std::string friendlyname, int is_initial)
{
	if (conn->auth.tag) {
		const pair<string, string> *ctx = static_cast<const pair<string, string> *>(conn->auth.tag);
		conn->sendTypingNotification();
		/*int trid = */conn->sendMessage(ctx->second);
		fSwitchboardList.AddItem(new pair<string,
			MSN::SwitchboardServerConnection*>(username, conn));
		delete ctx;
		conn->auth.tag = NULL;
	}
}


void MSNP::buddyLeftConversation(MSN::SwitchboardServerConnection * conn, MSN::Passport username)
{
//	printf("MSNP::buddyLeftConversation\n");
	int x;
	int count = 0;
	count = fSwitchboardList.CountItems();
	if (count != 0) {
		for (x=0; x < count; x++) {
			if (fSwitchboardList.ItemAt(x)->second == conn) {;
				break;
			}
		}
	}
}


void MSNP::gotInstantMessage(MSN::SwitchboardServerConnection * conn, MSN::Passport username, std::string friendlyname, MSN::Message * msg)
{
	MessageFromBuddy(msg->getBody().c_str(), username.c_str());
}


void MSNP::gotEmoticonNotification(MSN::SwitchboardServerConnection * conn, MSN::Passport username, std::string alias, std::string msnobject)
{
//	printf("MSNP::gotEmoticonNotification\n");
}


void MSNP::failedSendingMessage(MSN::Connection * conn)
{
//TODO implement it
}


void MSNP::buddyTyping(MSN::SwitchboardServerConnection * conn, MSN::Passport username, std::string friendlyname)
{
	BMessage msg(IM_MESSAGE);
	msg.AddInt32("im_what", IM_CONTACT_STARTED_TYPING);
	msg.AddString("protocol", kProtocolSignature);
	msg.AddString("id", username.c_str());
	fServerMsgr->SendMessage(&msg);
}


void MSNP::gotNudge(MSN::SwitchboardServerConnection * conn, MSN::Passport username)
{
	MessageFromBuddy("** Nudge!!!", username.c_str());
}


void MSNP::gotVoiceClipNotification(MSN::SwitchboardServerConnection * conn, MSN::Passport username, std::string msnobject)
{
	MessageFromBuddy("** Sent you a Voice Clip, but Caya cannot yet display it", username.c_str());
}


void MSNP::gotWinkNotification(MSN::SwitchboardServerConnection * conn, MSN::Passport username, std::string msnobject)
{
	MessageFromBuddy("** Sent you a Wink, but Caya cannot yet display it", username.c_str());
}


void MSNP::gotInk(MSN::SwitchboardServerConnection * conn, MSN::Passport username, std::string image)
{
	MessageFromBuddy("** Sent you an Ink draw, but Caya cannot yet display it", username.c_str());
}


void MSNP::gotContactDisplayPicture(MSN::SwitchboardServerConnection * conn, MSN::Passport passport, std::string filename )
{
	printf("MSNP::gotContactDisplayPicture %s\n", filename.c_str());
}


void MSNP::gotActionMessage(MSN::SwitchboardServerConnection * conn, MSN::Passport username, std::string message)
{
//	printf("MSNP::gotActionMessage\n");
}


void MSNP::gotInitialEmailNotification(MSN::NotificationServerConnection * conn, int msgs_inbox, int unread_inbox, int msgs_folders, int unread_folders)
{

}


void MSNP::gotNewEmailNotification(MSN::NotificationServerConnection * conn, std::string from, std::string subject)
{

}


void MSNP::fileTransferInviteResponse(MSN::SwitchboardServerConnection * conn, unsigned int sessionID, bool response)
{

}


void MSNP::fileTransferProgress(MSN::SwitchboardServerConnection * conn, unsigned int sessionID, unsigned long long transferred, unsigned long long total)
{

}


void MSNP::fileTransferFailed(MSN::SwitchboardServerConnection * conn, unsigned int sessionID, MSN::fileTransferError error)
{

}


void MSNP::fileTransferSucceeded(MSN::SwitchboardServerConnection * conn, unsigned int sessionID)
{

}


void MSNP::gotNewConnection(MSN::Connection * conn)
{
	// this is at least what the method should do.
	if (dynamic_cast<MSN::NotificationServerConnection *>(conn))
		dynamic_cast<MSN::NotificationServerConnection *>(conn)->synchronizeContactList();
	// we call synchronizeContactList and the callback GotBuddyListInfo is called
	// from libmsn if all goes well
}


void MSNP::buddyChangedPersonalInfo(MSN::NotificationServerConnection * conn, MSN::Passport fromPassport, MSN::personalInfo pInfo)
{
}


void MSNP::closingConnection(MSN::Connection * conn)
{
//	printf ("MSNP::closingConnection : connection with socket %d\n", (int)conn->sock);
}


void MSNP::changedStatus(MSN::NotificationServerConnection * conn, MSN::BuddyStatus state)
{
	MSN::personalInfo pInfo;
//	pInfo.PSM="From Caya at Haiku OS - www.haiku-os.org";
	conn->setPersonalStatus(pInfo);
}


void * MSNP::connectToServer(std::string hostname, int port, bool *connected, bool isSSL)
{
//	printf("MSNP::connectToServer\n");
	struct sockaddr_in sa;
	struct hostent *hp;
	int s;

	if ((hp = gethostbyname(hostname.c_str())) == NULL) {
		errno = ECONNREFUSED;
		return (void*)-1;
	}

	memset(&sa,0,sizeof(sa));
	memcpy((char *)&sa.sin_addr,hp->h_addr,hp->h_length);     /* set address */
	sa.sin_family = hp->h_addrtype;
	sa.sin_port = htons((u_short)port);

	if ((s = socket(hp->h_addrtype,SOCK_STREAM,0)) < 0)     /* get socket */
		return (void*)-1;

	int oldfdArgs = fcntl(s, F_GETFL, 0);
	fcntl(s, F_SETFL, oldfdArgs | O_NONBLOCK);

	if (connect(s,(struct sockaddr *)&sa,sizeof sa) < 0) {
		if (errno != EINPROGRESS) {
			close(s);
			return (void*)-1;
		}
		*connected = false;
	} else {
		*connected = true;
	}
	return (void*)s;
}


int MSNP::listenOnPort(int port)
{
//	printf("MSNP::listenOnPort\n");
	int s;
	struct sockaddr_in addr;

	if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		return -1;
	}

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons((uint16)port);

	if (bind(s, (sockaddr *)(&addr), sizeof(addr)) < 0 || listen(s, 1) < 0)	{
		close(s);
		return -1;
	}
	return s;
}


std::string MSNP::getOurIP(void)
{
	struct hostent* hn;
	char buf2[1024];

	gethostname(buf2, 1024);
	hn = gethostbyname(buf2);

	return inet_ntoa( *((struct in_addr*)hn->h_addr));
}


void MSNP::log(int i, const char *s)
{
//	printf("MSNP::log %s\n", s);
}


std::string MSNP::getSecureHTTPProxy()
{
	return "";
}


void MSNP::gotMessageSentACK(MSN::SwitchboardServerConnection * conn, int trID)
{
}


void MSNP::askFileTransfer(MSN::SwitchboardServerConnection * conn, MSN::fileTransferInvite ft)
{

}


void MSNP::addedContactToGroup(MSN::NotificationServerConnection * conn, bool added, std::string groupId, std::string contactId)
{

}


void MSNP::removedContactFromGroup(MSN::NotificationServerConnection * conn, bool removed, std::string groupId, std::string contactId)
{

}


void MSNP::addedContactToAddressBook(MSN::NotificationServerConnection * conn, bool added, std::string passport, std::string displayName, std::string guid)
{

}


void MSNP::removedContactFromAddressBook(MSN::NotificationServerConnection * conn, bool removed, std::string contactId, std::string passport)
{

}


void MSNP::enabledContactOnAddressBook(MSN::NotificationServerConnection * conn, bool enabled, std::string contactId, std::string passport)
{

}


void MSNP::disabledContactOnAddressBook(MSN::NotificationServerConnection * conn, bool disabled, std::string contactId)
{

}


size_t MSNP::getDataFromSocket (void* sock, char* data, size_t size)
{
//	printf("MSNP::getDataFromSocket\n");
	int idx=0;
	bool ssl_done = false;
	int newAmountRead = 0;
	for (int i = 0; i < kSocketsCount; i++) {
		if (kPollSockets[i].fd == getSocketFileDescriptor(sock)) {
			idx = i;
			break;
		}
	}

	if (!kSocketsSsl[idx].isSSL) {
		int newWritten = ::recv(getSocketFileDescriptor(sock), data, size, 0);
		if (errno == EAGAIN)
		   return -1;
		return newWritten;
	}

	if (!kSocketsSsl[idx].isConnected)
		return 0;

	while (!ssl_done) {
		if (!kSocketsSsl[idx].ssl) break;
		newAmountRead = SSL_read(kSocketsSsl[idx].ssl, data, size);
		switch(SSL_get_error(kSocketsSsl[idx].ssl, newAmountRead))
		{
			case SSL_ERROR_NONE:
				return newAmountRead;
			case SSL_ERROR_ZERO_RETURN:
				return 0;
			case SSL_ERROR_WANT_READ:
			case SSL_ERROR_WANT_WRITE:
				ssl_done=false;
				break;
			case SSL_ERROR_SSL:
				return 0;
				break;
			case SSL_ERROR_SYSCALL:
				return 0;
				break;
			default:
				return newAmountRead;
		}
	}
	return 0;
}


size_t MSNP::writeDataToSocket (void* sock, char* data, size_t size)
{
//	printf("MSNP::writeDataToSocket\n");
	size_t written = 0;
	int idx;
	for (int i = 0; i < kSocketsCount; i++) {
		if (kPollSockets[i].fd == getSocketFileDescriptor(sock)) {
			idx = i;
			break;
		}
	}

	while (written < size) {
		int newWritten;
		if (kSocketsSsl[idx].isSSL) {
			if (!kSocketsSsl[idx].isConnected)
				return 0;
			newWritten = SSL_write(kSocketsSsl[idx].ssl, data, (int) (size - written));
			int error = SSL_get_error(kSocketsSsl[idx].ssl,newWritten);
			switch (error) {
				case SSL_ERROR_NONE:
					written += newWritten;
					data+=newWritten;
					break;
				case SSL_ERROR_WANT_WRITE:
				case SSL_ERROR_WANT_READ:
					continue;
				case SSL_ERROR_ZERO_RETURN:
					break;
				case SSL_ERROR_SYSCALL:
				default:
					break;
			}
		} else {
			newWritten = ::send(getSocketFileDescriptor(sock), data, (int)(size - written), 0);

			if (newWritten <= 0) {
				if (errno == EAGAIN) {
					continue;
				} else {
					break;
				}
			}
			written += newWritten;
			data+=newWritten;
		}
	}
	if (written != size) {
		showError(NULL, "Error on socket");
		unregisterSocket(sock);
		closeSocket(sock);
		return written;
	}
	return written;
}


void MSNP::gotVoiceClipFile(MSN::SwitchboardServerConnection* conn, unsigned int sessionID, std::string file)
{

}


void MSNP::gotEmoticonFile(MSN::SwitchboardServerConnection* conn, unsigned int sessionID, std::string alias, std::string file)
{

}


void MSNP::gotWinkFile(MSN::SwitchboardServerConnection* conn, unsigned int sessionID, std::string file)
{

}


void MSNP::gotInboxUrl(MSN::NotificationServerConnection* conn, MSN::hotmailInfo info)
{

}
