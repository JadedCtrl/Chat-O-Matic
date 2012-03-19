/*
 * This is a plugin ported from im_kit to Caya,
 * the code was updated to support libyahoo2.
 *
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
#include <DataIO.h>

#include <CayaProtocol.h>
#include <CayaConstants.h>
#include <CayaProtocolMessages.h>

#include "Yahoo.h"
#include "YahooConnection.h"

#define LOG printf

const char *  kProtocolSignature = "yahoo";
const char *  kProtocolName = "Yahoo";

extern "C" {
	CayaProtocol* protocol();
	const char* signature();
	const char* friendly_signature();
	void register_callbacks();
}

CayaProtocol*
protocol()
{
	return (CayaProtocol*)new Yahoo();
}


const char*
signature()
{
	return kProtocolSignature;
}


const char*
friendly_signature()
{
	return kProtocolName;
}


Yahoo::Yahoo()
	:
	fYahooID(""),
	fPassword(""),
	fYahoo(NULL)
{
	register_callbacks();
}


Yahoo::~Yahoo()
{
	if (fYahoo)
		delete fYahoo;
}


status_t
Yahoo::Init( CayaProtocolMessengerInterface* msgr )
{
	fServerMsgr = msgr;

	return B_OK;
}

void
Yahoo::_NotifyProgress(const char* title, const char* message, float progress)
{
	BMessage msg(IM_MESSAGE);
	msg.AddInt32("im_what", IM_PROGRESS);
	msg.AddString("title", title);
	msg.AddString("message", message);
	msg.AddFloat("progress", progress);
	_SendMessage(&msg);
}


void
Yahoo::_Notify(notification_type type, const char* title, const char* message)
{
	BMessage msg(IM_MESSAGE);
	msg.AddInt32("im_what", IM_NOTIFICATION);
	msg.AddInt32("type", (int32)type);
	msg.AddString("title", title);
	msg.AddString("message", message);
	_SendMessage(&msg);
}


void
Yahoo::_SendMessage(BMessage* msg)
{
	// Skip invalid messages
	if (!msg)
		return;

	msg->AddString("protocol", kProtocolSignature);
	fServerMsgr->SendMessage(msg);
}



status_t
Yahoo::Shutdown()
{
	if (fYahoo) {
		YahooConnection * oldYahoo = fYahoo;
		fYahoo = NULL;
		delete oldYahoo;
	}

	return B_OK;
}


uint32
Yahoo::Version() const
{
	return CAYA_VERSION_1_PRE_ALPHA_1;
}


status_t
Yahoo::Process(BMessage * msg)
{
	LOG("Yahoo: Process()\n");
	msg->PrintToStream();
	switch (msg->what)
	{
		case IM_MESSAGE:
		{
			int32 im_what = 0;

			msg->FindInt32("im_what", &im_what);

			switch (im_what)
			{
				case IM_SET_OWN_STATUS:
				{
					LOG("Yahoo: Set own status\n");
					int32 status = msg->FindInt32("status");
					BString status_msg("");
					msg->FindString("message", &status_msg);
					//LOG(kProtocolSignature, liMedium, "Set status to %s", status);

					switch (status) {
						case CAYA_ONLINE:
						LOG("Yahoo: Caya online\n");
							if (!fYahoo) {
								if (fYahooID != "") {
									Progress("Yahoo Login", "Yahoo: Connecting..", 0.50);

									fYahoo = new YahooConnection(
										this,
										fYahooID.String(), 
										fPassword.String()
										);
								}
							} else {
								fYahoo->SetAway(YAHOO_STATUS_AVAILABLE, NULL);
							}
							break;
						case CAYA_AWAY:
							if (fYahoo) {
								fYahoo->SetAway(YAHOO_STATUS_NOTATDESK, NULL);
							}
							break;
						case CAYA_EXTENDED_AWAY:

							break;
						case CAYA_DO_NOT_DISTURB:
							if (fYahoo) {
								fYahoo->SetAway(YAHOO_STATUS_BUSY, NULL);
							}
							break;
						case CAYA_OFFLINE:
							if (fYahoo) {
								YahooConnection * oldYahoo = fYahoo;
								fYahoo = NULL;
								delete oldYahoo;
							}
							break;
						default:
//							Error("Invalid", NULL);
//							Log("caya msn plugin :
							break;
					}
					break;
				}
				break;

				case IM_SEND_MESSAGE:
					if (fYahoo)
						fYahoo->Message( msg->FindString("id"), msg->FindString("body") );
					else
						return B_ERROR;
					break;
				/*case IM::REGISTER_CONTACTS:
					if ( fYahoo ) {
						const char * buddy=NULL;

						for (int i=0; msg->FindString("id", i, &buddy) == B_OK; i++)
							fYahoo->AddBuddy( buddy );
					} else {
						return B_ERROR;
					}
					break;
				case IM::UNREGISTER_CONTACTS:
					if ( fYahoo ) {
						const char * buddy=NULL;

						for ( int i=0; 
							msg->FindString("id", i, &buddy) == B_OK; i++ ) {
							fYahoo->RemoveBuddy( buddy );
						}
					} else
						return B_ERROR;
					break;*/

				case IM_USER_STARTED_TYPING: 
				if (fYahoo) {
					const char *id = msg->FindString("id");
					if (!id) return B_ERROR;
				
					fYahoo->Typing(id, 1);
				}
				break;

				case IM_USER_STOPPED_TYPING: 
				if (fYahoo) {
					const char *id = msg->FindString("id");
					if (!id) return B_ERROR;
					
					fYahoo->Typing(id, 0);
				}
				break;

				/*case IM_SET_OWN_AVATAR:
				if ( fYahoo ) {

				}
				break;*/

				default:
					// we don't handle this im_what code
					return B_ERROR;
			}
		}	break;

		default:
			return B_ERROR;
	}

	return B_OK;
}


const char*
Yahoo::Signature() const
{
	return kProtocolSignature;
}


const char*
Yahoo::FriendlySignature() const
{
	return kProtocolName;
}


status_t
Yahoo::UpdateSettings( BMessage* msg )
{
	msg->PrintToStream();
	const char *yahooID = NULL;
	const char *password = NULL;

	msg->FindString("yahooID", &yahooID);
	msg->FindString("password", &password);
	LOG("%s %s \n", yahooID, password);

	if ((yahooID == NULL) || (password == NULL)) {
		Error("Yahoo Protocol: Invalid settings!", NULL);
		return B_ERROR;
	}

	fYahooID = yahooID;
	fPassword = password;

	BMessage mess(IM_MESSAGE);
	mess.AddInt32("im_what", IM_OWN_CONTACT_INFO);
	mess.AddString("protocol", kProtocolSignature);
	mess.AddString("id", yahooID);
	mess.AddString("name", yahooID);
	fServerMsgr->SendMessage(&mess);

	return B_OK;
}


uint32
Yahoo::GetEncoding()
{
	return 0xffff; // No conversion, Yahoo handles UTF-8
}

// YahooManager stuff

void
Yahoo::Error( const char * message, const char * who )
{
	BMessage msg(IM_ERROR);
	msg.AddString("protocol", kProtocolName);
	if (who)
		msg.AddString("id", who);
	msg.AddString("error", message);

	fServerMsgr->SendMessage( &msg );
}


void
Yahoo::GotMessage( const char * from, const char * message )
{
	LOG("Yahoo::GotMessage()\n");

	BMessage msg(IM_MESSAGE);
	msg.AddInt32("im_what", IM_MESSAGE_RECEIVED);
	msg.AddString("protocol", kProtocolSignature);
	msg.AddString("id", from);
	msg.AddString("body", message);
	fServerMsgr->SendMessage( &msg );
}


void
Yahoo::MessageSent( const char * to, const char * message )
{
	LOG("Yahoo::MessageSent()\n");

	BMessage msg(IM_MESSAGE);
	msg.AddInt32("im_what", IM_MESSAGE_SENT);
	msg.AddString("protocol", kProtocolSignature);
	msg.AddString("id", to);
	msg.AddString("body", message);

	fServerMsgr->SendMessage( &msg );
}


void
Yahoo::LoggedIn()
{
	LOG("Yahoo::LoggedIn()\n");

	Progress("Yahoo Login", "Yahoo: Logged in!", 1.00);

	BMessage msg(IM_MESSAGE);
	msg.AddInt32("im_what", IM_OWN_STATUS_SET);
	msg.AddString("protocol", kProtocolSignature);
	msg.AddInt32("status", CAYA_ONLINE);
	BString content(fYahooID);
	content << " has logged in!";
	_Notify(B_INFORMATION_NOTIFICATION, "Connected",
		content.String());


	fServerMsgr->SendMessage( &msg );
}


void
Yahoo::SetAway(bool away)
{
	LOG("Yahoo::SetAway()\n");

	BMessage msg(IM_MESSAGE);
	msg.AddInt32("im_what", IM_OWN_STATUS_SET);
	msg.AddString("protocol", kProtocolSignature);
	if ( away )
		msg.AddInt32("status", CAYA_AWAY);
	else
		msg.AddInt32("status", CAYA_ONLINE);

	fServerMsgr->SendMessage( &msg );
}


void
Yahoo::LoggedOut()
{
	LOG("Yahoo::LoggedOut()\n");

	BMessage msg(IM_MESSAGE);
	msg.AddInt32("im_what", IM_OWN_STATUS_SET);
	msg.AddString("protocol", kProtocolSignature);
	msg.AddInt32("status", CAYA_OFFLINE);

	fServerMsgr->SendMessage(&msg);

	if (fYahoo) {
		YahooConnection * oldYahoo = fYahoo;
		fYahoo = NULL;
		delete oldYahoo;
	}
	BString content(fYahooID);
	content << " has logged out!";
	_Notify(B_INFORMATION_NOTIFICATION, "Disconnected",
			content.String());

}


void
Yahoo::GotBuddyList(std::list<std::string>& buddies)
{
	LOG("Yahoo::GotBuddyList()\n");
	std::list<std::string>::iterator i;

	BMessage msg(IM_MESSAGE);
	msg.AddInt32("im_what", IM_CONTACT_LIST);
	msg.AddString("protocol", kProtocolSignature);

	for (i = buddies.begin(); i != buddies.end(); i++) {
		LOG("Got server side buddy %s", i->c_str());
		msg.AddString("id", i->c_str());
	};

	fServerMsgr->SendMessage(&msg);
}


void
Yahoo::GotContactsInfo(std::list<struct yahoo_buddy> & yabs)
{
	LOG("Yahoo::GotContactsInfo()\n");
	std::list<struct yahoo_buddy>::iterator i;

	for (i = yabs.begin(); i != yabs.end(); i++) {
		BMessage msg(IM_MESSAGE);
		msg.AddInt32("im_what", IM_CONTACT_INFO);
		msg.AddString("protocol", kProtocolSignature);
		msg.AddString("id", i->id);
		msg.AddString("name", i->real_name);
		
		fServerMsgr->SendMessage(&msg);
	};
}


void
Yahoo::GotBuddyIcon(const char* who, long length, const char *icon)
{
	/*
	BMessage iconMsg(IM_MESSAGE);
	iconMsg.AddInt32("im_what", IM_AVATAR_SET);
	iconMsg.AddString("protocol", kProtocolSignature);
	iconMsg.AddString("id", who);
	//iconMsg.AddData("icondata", B_RAW_TYPE, icon, length);
	iconMsg.AddData("avatar", NULL, icon, length);

	fServerMsgr->SendMessage(&iconMsg);*/
}


void
Yahoo::BuddyStatusChanged(const char* who, int status, const char* mess, int away, int idle)
{
	LOG("Yahoo::BuddyStatusChanged()\n");
	BMessage msg(IM_MESSAGE);
	msg.AddInt32("im_what", IM_STATUS_SET);
	msg.AddString("protocol", kProtocolSignature);
	msg.AddString("id", who);

	switch(status) {
		case YAHOO_STATUS_CUSTOM:
			msg.AddString("message", mess);
			if (away)
				msg.AddInt32("status", CAYA_AWAY);
			else
				msg.AddInt32("status", CAYA_ONLINE);
			break;
		case YAHOO_STATUS_NOTATDESK:
			msg.AddInt32("status", CAYA_AWAY);
			break;
		case YAHOO_STATUS_NOTINOFFICE:
			msg.AddInt32("status", CAYA_AWAY);
			break;
		case YAHOO_STATUS_ONVACATION:
			msg.AddInt32("status", CAYA_AWAY);
			break;
		case YAHOO_STATUS_BRB:
			msg.AddInt32("status", CAYA_AWAY);
			break;
		case YAHOO_STATUS_OUTTOLUNCH:
			msg.AddInt32("status", CAYA_AWAY);
			break;
		case YAHOO_STATUS_NOTATHOME:
			msg.AddInt32("status", CAYA_AWAY);
			break;
		case YAHOO_STATUS_STEPPEDOUT:
			msg.AddInt32("status", CAYA_AWAY);
			break;
		case YAHOO_STATUS_IDLE:
			msg.AddInt32("status", CAYA_AWAY);
			break;
		case YAHOO_STATUS_ONPHONE :
			msg.AddInt32("status", CAYA_AWAY);
			break;
		case YAHOO_STATUS_OFFLINE :
			msg.AddInt32("status", CAYA_OFFLINE);
			break;
		case YAHOO_STATUS_AVAILABLE :
			msg.AddInt32("status", CAYA_ONLINE);
			break;
		default:
			msg.AddInt32("status", CAYA_ONLINE);			
		break;
	}
	fServerMsgr->SendMessage(&msg);
}


void
Yahoo::Progress(const char* id, const char* message, float progress)
{
	BMessage msg(IM_MESSAGE);
	msg.AddInt32("im_what", IM_PROGRESS );
	msg.AddString("protocol", kProtocolSignature);
	msg.AddString("progressID", id);
	msg.AddString("message", message);
	msg.AddFloat("progress", progress);
	msg.AddInt32("state", 11);

	fServerMsgr->SendMessage(&msg);
}


void
Yahoo::TypeNotify(const char * who, int state)
{
	
	BMessage msg(IM_MESSAGE);
	msg.AddInt32("im_what", state ? IM_CONTACT_STARTED_TYPING : IM_CONTACT_STOPPED_TYPING);
	msg.AddString("protocol", kProtocolSignature);
	msg.AddString("id", who);
	
	fServerMsgr->SendMessage( &msg );
}
