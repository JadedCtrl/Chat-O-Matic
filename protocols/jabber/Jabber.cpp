/*
 * Copyright 2010, Pier Luigi Fiorini. All rights reserved.
 * Distributed under the terms of the GPL v2 License.
 *
 * Authors:
 *		Pier Luigi Fiorini, pierluigi.fiorini@gmail.com
 */

//#include <libsupport/Base64.h>

#include "CayaProtocolMessages.h"
#include "Jabber.h"

const char* kProtocolSignature = "jabber";
const char* kProtocolName = "Jabber";


static status_t
connect_thread(void* data)
{
	gloox::Client* client = (gloox::Client*)data;
	if (!client)
		return B_BAD_VALUE;

#if 1
	client->connect();
#else
	client->recv(-1);
#endif

	return B_OK;
}


Jabber::Jabber()
	: fClient(NULL),
	fVCardManager(NULL)
{
}


Jabber::~Jabber()
{
	fVCardManager->cancelVCardOperations(this);
	Shutdown();
}


status_t
Jabber::Init(CayaProtocolMessengerInterface* messenger)
{
	fServerMessenger = messenger;

	return B_OK;
}


status_t
Jabber::Process(BMessage* msg)
{
	if (msg->what != IM_MESSAGE)
		return B_ERROR;

	int32 im_what = 0;

	msg->FindInt32("im_what", &im_what);

	switch (im_what) {
		case IM_SET_OWN_STATUS: {
			int32 status = msg->FindInt32("status");
			BString status_msg = msg->FindString("message");

			switch (status) {
				case CAYA_ONLINE:
					// Log in if we still need to
					if (!fClient->authed())
						fClient->login();
					break;
				default:
					break;
			}
			break;
		}
		case IM_SEND_MESSAGE: {
			const char* id = msg->FindString("id");
			const char* subject = msg->FindString("subject");
			const char* body = msg->FindString("body");

			if (!id || !body)
				return B_ERROR;

			// Send Jabber message
			gloox::Message jm(gloox::Message::Chat, gloox::JID(id),
				body, (subject ? subject : gloox::EmptyString));
			fClient->send(jm);

			// Tell Caya we actually sent the message
			MessageSent(id, subject, body);
			break;
		}
		default:
			return B_ERROR;
	}

	return B_OK;
}


status_t
Jabber::Shutdown()
{
	if (fClient)
		fClient->disconnect();
	return B_OK;
}


const char*
Jabber::Signature() const
{
	return kProtocolSignature;
}


const char*
Jabber::FriendlySignature() const
{
	return kProtocolName;
}


status_t
Jabber::UpdateSettings(BMessage* msg)
{
	const char* username = msg->FindString("username");
	const char* password = msg->FindString("password");
	const char* server = msg->FindString("server");
	const char* resource = msg->FindString("resource");

	// Sanity check
	if (!username || !password || !server || !resource)
		return B_ERROR;

	// Store settings
	fUsername = username;
	fPassword = password;
	fServer = server;
	fResource = resource;

	// Compose jid
	BString jidString(fUsername);
	jidString << "@" << fServer << "/" << fResource;
	gloox::JID jid(jidString.String());

	// Register our client
	fClient = new gloox::Client(jid, fPassword.String());
#if 0
	fConnection = new gloox::ConnectionTCPClient(fClient, fClient->logInstance(),
		fServer.String(), 5222);
	fClient->setConnectionImpl(fConnection);
#endif
	fClient->registerConnectionListener(this);
	fClient->registerMessageHandler(this);
	fClient->rosterManager()->registerRosterListener(this);
	fClient->disco()->setVersion("Caya", VERSION);
	fClient->disco()->setIdentity("client", "caya");
	fClient->logInstance().registerLogHandler(gloox::LogLevelDebug,
		gloox::LogAreaAll, this);

	// Register vCard manager
	fVCardManager = new gloox::VCardManager(fClient);

#if 0
	// Connect to the server
	fClient->connect(false);
#endif

	// Read data from another thread
	// FIXME: Maybe get data using select() and recv() on the socket
	thread_id tid = spawn_thread(connect_thread, "connect_thread",
		B_NORMAL_PRIORITY, (void*)fClient);
	return resume_thread(tid);
}


uint32
Jabber::GetEncoding()
{
	return 0xffff;
}


CayaProtocolMessengerInterface*
Jabber::MessengerInterface() const
{
	return fServerMessenger;
}


void
Jabber::MessageSent(const char* id, const char* subject,
					const char* body)
{
	BMessage msg(IM_MESSAGE);
	msg.AddInt32("im_what", IM_MESSAGE_SENT);
	msg.AddString("protocol", kProtocolSignature);
	msg.AddString("id", id);
	msg.AddString("subject", subject);
	msg.AddString("body", body);
	fServerMessenger->SendMessage(&msg);
}


CayaStatus
Jabber::GlooxStatusToCaya(gloox::Presence::PresenceType type)
{
	switch (type) {
		case gloox::Presence::Available:
		case gloox::Presence::Chat:
			return CAYA_ONLINE;
		case gloox::Presence::Away:
			return CAYA_AWAY;
		case gloox::Presence::XA:
			return CAYA_EXTENDED_AWAY;
		case gloox::Presence::DND:
			return CAYA_DO_NOT_DISTURB;
		case gloox::Presence::Unavailable:
			return CAYA_OFFLINE;
		default:
			break;
	}

	return CAYA_OFFLINE;
}


/***********************************************************************
 * gloox callbacks
 **********************************************************************/


void
Jabber::onConnect()
{
	BMessage msg(IM_MESSAGE);
	msg.AddInt32("im_what", IM_OWN_STATUS_SET);
	msg.AddString("protocol", kProtocolSignature);
	msg.AddInt32("status", CAYA_ONLINE);
	fServerMessenger->SendMessage(&msg);
}


void
Jabber::onDisconnect(gloox::ConnectionError e)
{
	BMessage msg(IM_MESSAGE);
	msg.AddInt32("im_what", IM_OWN_STATUS_SET);
	msg.AddString("protocol", kProtocolSignature);
	msg.AddInt32("status", CAYA_OFFLINE);
	fServerMessenger->SendMessage(&msg);
}


bool
Jabber::onTLSConnect(const gloox::CertInfo& info)
{
	return true;
}


void
Jabber::onResourceBindError(const gloox::Error* error)
{
}


void
Jabber::handleRoster(const gloox::Roster& roster)
{
	std::list<BMessage> msgs;

	BMessage contactListMsg(IM_MESSAGE);
	contactListMsg.AddInt32("im_what", IM_CONTACT_LIST);

	gloox::Roster::const_iterator it = roster.begin();
	for (; it != roster.end(); ++it) {
		const char* jid = (*it).second->jid().c_str();
		const char* name = (*it).second->name().c_str();
		int32 subscription = (*it).second->subscription();

		// Add jid to the server based contact list message
		contactListMsg.AddString("id", jid);

		// Contact information message
		BMessage infoMsg(IM_MESSAGE);
		infoMsg.AddInt32("im_what", IM_CONTACT_INFO);
		infoMsg.AddString("protocol", kProtocolSignature);
		infoMsg.AddString("id", jid);
		infoMsg.AddString("name", name);
		infoMsg.AddInt32("subscription", subscription);
		infoMsg.AddInt32("status", CAYA_OFFLINE);

		// Groups
		gloox::StringList g = (*it).second->groups();
		gloox::StringList::const_iterator it_g = g.begin();
		for (; it_g != g.end(); ++it_g)
			infoMsg.AddString("group", (*it_g).c_str());

		// Resources
		gloox::RosterItem::ResourceMap::const_iterator rit
			= (*it).second->resources().begin();
		for (; rit != (*it).second->resources().end(); ++rit)
			infoMsg.AddString("resource", (*rit).first.c_str());

		// Store contact info message to be sent later
		msgs.push_back(infoMsg);
	}

	// Send server based contact list
	contactListMsg.AddString("protocol", kProtocolSignature);
	fServerMessenger->SendMessage(&contactListMsg);

	// Contact list and vCard request
	std::list<BMessage>::iterator msgsIt;
	for (msgsIt = msgs.begin(); msgsIt != msgs.end(); ++msgsIt) {
		BMessage msg = (*msgsIt);
		const char* jid = msg.FindString("id");
		fServerMessenger->SendMessage(&msg);
		fVCardManager->fetchVCard(gloox::JID(jid), this);
	}
}


void
Jabber::handleMessage(const gloox::Message& m, gloox::MessageSession*)
{
	// Only chat messages are handled now
	if (m.subtype() != gloox::Message::Chat)
		return;

	if (m.body() == "") {
		// TODO: Started and stopped typing
	} else {
		BMessage msg(IM_MESSAGE);
		msg.AddInt32("im_what", IM_MESSAGE_RECEIVED);
		msg.AddString("protocol", kProtocolSignature);
		msg.AddString("id", m.from().bare().c_str());
		msg.AddString("subject", m.subject().c_str());
		msg.AddString("body", m.body().c_str());
		fServerMessenger->SendMessage(&msg);
	}
}


void
Jabber::handleItemAdded(const gloox::JID&)
{
}


void
Jabber::handleItemSubscribed(const gloox::JID&)
{
}


void
Jabber::handleItemUnsubscribed(const gloox::JID&)
{
}


void
Jabber::handleItemRemoved(const gloox::JID&)
{
}


void
Jabber::handleItemUpdated(const gloox::JID&)
{
}


void
Jabber::handleRosterPresence(const gloox::RosterItem& item,
									const std::string&,
									gloox::Presence::PresenceType type,
									const std::string& presenceMsg)
{
	BMessage msg(IM_MESSAGE);
	msg.AddInt32("im_what", IM_STATUS_SET);
	msg.AddString("protocol", kProtocolSignature);
	msg.AddString("id", item.jid().c_str());
	msg.AddInt32("status", GlooxStatusToCaya(type));
	msg.AddString("message", presenceMsg.c_str());
	fServerMessenger->SendMessage(&msg);
}


void
Jabber::handleSelfPresence(const gloox::RosterItem& item, const std::string&,
								  gloox::Presence::PresenceType type,
								  const std::string& presenceMsg)
{
	BMessage msg(IM_MESSAGE);
	msg.AddInt32("im_what", IM_OWN_CONTACT_INFO);
	msg.AddString("protocol", kProtocolSignature);
	msg.AddString("id", item.jid().c_str());
	msg.AddString("name", item.name().c_str());
	msg.AddInt32("subscription", item.subscription());
//resource
//groups
	msg.AddInt32("status", GlooxStatusToCaya(type));
	msg.AddString("message", presenceMsg.c_str());
	fServerMessenger->SendMessage(&msg);
}


bool
Jabber::handleSubscriptionRequest(const gloox::JID&, const std::string&)
{
	return true;
}


bool
Jabber::handleUnsubscriptionRequest(const gloox::JID&, const std::string&)
{
	return true;
}


void
Jabber::handleNonrosterPresence(const gloox::Presence&)
{
}


void
Jabber::handleRosterError(const gloox::IQ&)
{
}


void
Jabber::handleLog(gloox::LogLevel level, gloox::LogArea,
						 const std::string& msg)
{
	if (level >= gloox::LogLevelWarning)
		printf("%s\n", msg.c_str());
}


void
Jabber::handleVCard(const gloox::JID& jid, const gloox::VCard* card)
{
	if (!card)
		return;

	gloox::VCard::Name name = card->name();
	gloox::VCard::Photo photo = card->photo();

	std::string fullName = name.family + " " + name.given;

#if 0
	BString decoded
		= Base64::Decode(BString(photo.binval.c_str()));
#endif

	BMessage msg(IM_MESSAGE);
	msg.AddInt32("im_what", IM_EXTENDED_CONTACT_INFO);
	msg.AddString("protocol", kProtocolSignature);
	msg.AddString("id", jid.bare().c_str());
	msg.AddString("nick", card->nickname().c_str());
	msg.AddString("family name", name.family.c_str());
	msg.AddString("given name", name.given.c_str());
	msg.AddString("middle name", name.middle.c_str());
	msg.AddString("prefix", name.prefix.c_str());
	msg.AddString("suffix", name.suffix.c_str());
	msg.AddString("full name", fullName.c_str());
	fServerMessenger->SendMessage(&msg);
}


void
Jabber::handleVCardResult(gloox::VCardHandler::VCardContext context,
								 const gloox::JID& jid,
								 gloox::StanzaError)
{
	//if (context == gloox::VCardHandler::FetchVCard)
	//else
}
