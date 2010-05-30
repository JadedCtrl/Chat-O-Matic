/*
 * Copyright 2010, Pier Luigi Fiorini. All rights reserved.
 * Distributed under the terms of the GPL v2 License.
 *
 * Authors:
 *		Pier Luigi Fiorini, pierluigi.fiorini@gmail.com
 */

#include <Directory.h>
#include <Entry.h>
#include <File.h>
#include <FindDirectory.h>
#include <List.h>

#include <CayaProtocolMessages.h>

#include <libsupport/SHA1.h>

#include "JabberHandler.h"


static status_t
connect_thread(void* data)
{
	gloox::Client* client = (gloox::Client*)data;
	if (!client)
		return B_BAD_VALUE;

	while (client->recv(1000) == gloox::ConnNoError);

	return B_OK;
}


JabberHandler::JabberHandler()
	:
	fClient(NULL),
	fVCardManager(NULL)
{
	fAvatars = new BList();
}


JabberHandler::~JabberHandler()
{
	fVCardManager->cancelVCardOperations(this);
	Shutdown();

	BString* item = NULL;
	for (int32 i = 0; (item = (BString*)fAvatars->ItemAt(i)); i++)
		delete item;
	delete fAvatars;
}


status_t
JabberHandler::Init(CayaProtocolMessengerInterface* messenger)
{
	fServerMessenger = messenger;

	return B_OK;
}


status_t
JabberHandler::Process(BMessage* msg)
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
					resume_thread(fRecvThread);
					break;
				case CAYA_OFFLINE:
					kill_thread(fRecvThread);
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

			// Send JabberHandler message
			gloox::Message jm(gloox::Message::Chat, gloox::JID(id),
				body, (subject ? subject : gloox::EmptyString));
			fClient->send(jm);

			// Tell Caya we actually sent the message
			_MessageSent(id, subject, body);
			break;
		}
		default:
			return B_ERROR;
	}

	return B_OK;
}


status_t
JabberHandler::Shutdown()
{
	if (fClient)
		fClient->disconnect();
	return B_OK;
}


const char*
JabberHandler::Signature() const
{
	return kProtocolSignature;
}


const char*
JabberHandler::FriendlySignature() const
{
	return kProtocolName;
}


status_t
JabberHandler::UpdateSettings(BMessage* msg)
{
	const char* username = msg->FindString("username");
	const char* password = msg->FindString("password");
	const char* server = msg->FindString("server");
	const char* resource = msg->FindString("resource");

	// Sanity check
	if (!username || !password || !resource)
		return B_ERROR;

	// Store settings
	fUsername = username;
	fPassword = password;
	fServer = server;
	fResource = resource;
	fPort = 5222;

	// Give the add-on a change to override settings
	OverrideSettings();

	// Compose JID
	BString jid = ComposeJID();
	fJid.setJID(jid.String());

	// Register our client
	fClient = new gloox::Client(fJid, fPassword.String());
	fClient->setServer(fServer.String());
	if (fPort > 0)
		fClient->setPort(fPort);
	fClient->registerConnectionListener(this);
	fClient->registerMessageHandler(this);
	fClient->rosterManager()->registerRosterListener(this);
	fClient->disco()->setVersion("Caya", VERSION);
	fClient->disco()->setIdentity("client", "caya");
	fClient->logInstance().registerLogHandler(gloox::LogLevelDebug,
		gloox::LogAreaAll, this);

	// Register vCard manager
	fVCardManager = new gloox::VCardManager(fClient);

	// Connect to the server
	fClient->connect(false);

	// Read data from another thread
	fRecvThread = spawn_thread(connect_thread, "connect_thread",
		B_NORMAL_PRIORITY, (void*)fClient);
	if (fRecvThread < B_OK)
		return B_ERROR;
	return B_OK;
}


uint32
JabberHandler::GetEncoding()
{
	return 0xffff;
}


CayaProtocolMessengerInterface*
JabberHandler::MessengerInterface() const
{
	return fServerMessenger;
}


void
JabberHandler::_SendMessage(BMessage* msg)
{
	// Skip invalid messages
	if (!msg)
		return;

	msg->AddString("protocol", kProtocolSignature);
	fServerMessenger->SendMessage(msg);
}


void
JabberHandler::_Progress(const char* title, const char* message, float progress)
{
	BMessage msg(IM_MESSAGE);
	msg.AddInt32("im_what", IM_PROGRESS);
	msg.AddString("title", title);
	msg.AddString("message", message);
	msg.AddFloat("progress", progress);
	_SendMessage(&msg);
}


void
JabberHandler::_MessageSent(const char* id, const char* subject,
					const char* body)
{
	BMessage msg(IM_MESSAGE);
	msg.AddInt32("im_what", IM_MESSAGE_SENT);
	msg.AddString("id", id);
	msg.AddString("subject", subject);
	msg.AddString("body", body);
	_SendMessage(&msg);
}


status_t
JabberHandler::_SetupAvatarCache()
{
	if (fAvatarCachePath.InitCheck() == B_OK)
		return B_OK;

	BPath path;

	if (find_directory(B_USER_SETTINGS_DIRECTORY, &path) != B_OK)
		return B_ERROR;

	path.Append("Caya");
	path.Append("Cache");
	path.Append(kProtocolSignature);

	if (create_directory(path.Path(), 0755) != B_OK)
		return B_ERROR;

	fCachePath = path;

	path.Append("avatar-cache");

	BFile file(path.Path(), B_READ_ONLY);
	fAvatarCache.Unflatten(&file);

	fAvatarCachePath = path;

	return B_OK;
}


status_t
JabberHandler::_SaveAvatarCache()
{
	if (fAvatarCachePath.InitCheck() != B_OK)
		return B_ERROR;

	BFile file(fAvatarCachePath.Path(), B_CREATE_FILE | B_WRITE_ONLY | B_ERASE_FILE);
	return fAvatarCache.Flatten(&file);
}


void
JabberHandler::_CacheAvatar(const char* id, const char* binimage, size_t length)
{
	if (!id || !binimage || length <= 0)
		return;

	// Calculate avatar hash
	CSHA1 s1;
	char hash[256];
	s1.Reset();
	s1.Update((uchar*)binimage, length);
	s1.Final();
	s1.ReportHash(hash, CSHA1::REPORT_HEX);

	BString sha1;
	sha1.SetTo(hash, 256);

	BString oldSha1;
	if (fAvatarCache.FindString(id, &oldSha1) != B_OK || oldSha1 == "" || sha1 != oldSha1) {
		// Replace old hash and save cache
		fAvatarCache.RemoveName(id);
		fAvatarCache.AddString(id, sha1);
		_SaveAvatarCache();

		if (oldSha1 != "") {
			BPath path(fCachePath);
			path.Append(oldSha1);

			// Remove old image file
			BEntry entry(path.Path());
			entry.Remove();

			// Remove old hash from the list
			BString* item = NULL;
			for (int32 i = 0; (item = (BString*)fAvatars->ItemAt(i)); i++) {
				if (item->Compare(oldSha1) == 0) {
					fAvatars->RemoveItem(item);
					break;
				}
			}
		}
	}

	// Determine file path
	BPath path(fCachePath);
	path.Append(sha1);

	BEntry entry(path.Path());
	if (!entry.Exists()) {
		// Save to file
		BFile file(path.Path(), B_CREATE_FILE | B_WRITE_ONLY | B_ERASE_FILE);
		file.Write(binimage, length);
	}

	// Do we need to notify Caya?
	bool found = false;
	BString* item = NULL;
	for (int32 i = 0; (item = (BString*)fAvatars->ItemAt(i)); i++) {
		if (item->Compare(sha1) == 0) {
			found = true;
			break;
		}
	}

	if (!found) {
		// Add new hash to the list if needed
		fAvatars->AddItem(new BString(sha1));

		// Notify Caya that the avatar has changed
		_AvatarChanged(id, path.Path());
	}
}


void
JabberHandler::_AvatarChanged(const char* id, const char* filename)
{
	entry_ref ref;
	if (get_ref_for_path(filename, &ref) != B_OK)
		return;

	BMessage msg(IM_MESSAGE);
	if (fJid.bare() == id)
		msg.AddInt32("im_what", IM_OWN_AVATAR_SET);
	else {
		msg.AddInt32("im_what", IM_AVATAR_SET);
		msg.AddString("id", id);
	}
	msg.AddRef("ref", &ref);
	_SendMessage(&msg);
}


CayaStatus
JabberHandler::_GlooxStatusToCaya(gloox::Presence::PresenceType type)
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
JabberHandler::onConnect()
{
	BMessage msg(IM_MESSAGE);
	msg.AddInt32("im_what", IM_OWN_STATUS_SET);
	msg.AddInt32("status", CAYA_ONLINE);
	_SendMessage(&msg);

	BString content(fUsername);
	content << " has logged in!";
	_Progress("Connected", content.String(), 1.0f);

	fVCardManager->fetchVCard(fJid, this);
}


void
JabberHandler::onDisconnect(gloox::ConnectionError e)
{
	BMessage msg(IM_MESSAGE);
	msg.AddInt32("im_what", IM_OWN_STATUS_SET);
	msg.AddInt32("status", CAYA_OFFLINE);
	_SendMessage(&msg);

	BString content(fUsername);
	content << " has logged out!";
	_Progress("Disconnected", content.String(), 1.0f);
}


bool
JabberHandler::onTLSConnect(const gloox::CertInfo& info)
{
	return true;
}


void
JabberHandler::onResourceBindError(const gloox::Error* error)
{
}


void
JabberHandler::handleRoster(const gloox::Roster& roster)
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
	_SendMessage(&contactListMsg);

	// Contact list and vCard request
	std::list<BMessage>::iterator msgsIt;
	for (msgsIt = msgs.begin(); msgsIt != msgs.end(); ++msgsIt) {
		BMessage msg = (*msgsIt);
		const char* jid = msg.FindString("id");
		_SendMessage(&msg);
		fVCardManager->fetchVCard(gloox::JID(jid), this);
	}
}


void
JabberHandler::handleMessage(const gloox::Message& m, gloox::MessageSession*)
{
	// Only chat messages are handled now
	if (m.subtype() != gloox::Message::Chat)
		return;

	if (m.body() == "") {
		// TODO: Started and stopped typing
	} else {
		BMessage msg(IM_MESSAGE);
		msg.AddInt32("im_what", IM_MESSAGE_RECEIVED);
		msg.AddString("id", m.from().bare().c_str());
		msg.AddString("subject", m.subject().c_str());
		msg.AddString("body", m.body().c_str());
		_SendMessage(&msg);
	}
}


void
JabberHandler::handleItemAdded(const gloox::JID&)
{
}


void
JabberHandler::handleItemSubscribed(const gloox::JID&)
{
}


void
JabberHandler::handleItemUnsubscribed(const gloox::JID&)
{
}


void
JabberHandler::handleItemRemoved(const gloox::JID&)
{
}


void
JabberHandler::handleItemUpdated(const gloox::JID&)
{
}


void
JabberHandler::handleRosterPresence(const gloox::RosterItem& item,
									const std::string&,
									gloox::Presence::PresenceType type,
									const std::string& presenceMsg)
{
	BMessage msg(IM_MESSAGE);
	msg.AddInt32("im_what", IM_STATUS_SET);
	msg.AddString("id", item.jid().c_str());
	msg.AddInt32("status", _GlooxStatusToCaya(type));
	msg.AddString("message", presenceMsg.c_str());
	_SendMessage(&msg);
}


void
JabberHandler::handleSelfPresence(const gloox::RosterItem& item, const std::string&,
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
	msg.AddInt32("status", _GlooxStatusToCaya(type));
	msg.AddString("message", presenceMsg.c_str());
	_SendMessage(&msg);
}


bool
JabberHandler::handleSubscriptionRequest(const gloox::JID&, const std::string&)
{
	return true;
}


bool
JabberHandler::handleUnsubscriptionRequest(const gloox::JID&, const std::string&)
{
	return true;
}


void
JabberHandler::handleNonrosterPresence(const gloox::Presence&)
{
}


void
JabberHandler::handleRosterError(const gloox::IQ&)
{
}


void
JabberHandler::handleLog(gloox::LogLevel level, gloox::LogArea,
						 const std::string& msg)
{
	if (level >= gloox::LogLevelWarning)
		printf("%s\n", msg.c_str());
}


void
JabberHandler::handleVCard(const gloox::JID& jid, const gloox::VCard* card)
{
	if (!card)
		return;

	gloox::VCard::Name name = card->name();
	gloox::VCard::Photo photo = card->photo();

	std::string fullName = name.family + " " + name.given;

	BMessage msg(IM_MESSAGE);
	msg.AddInt32("im_what", IM_EXTENDED_CONTACT_INFO);
	msg.AddString("id", jid.bare().c_str());
	msg.AddString("nick", card->nickname().c_str());
	msg.AddString("family name", name.family.c_str());
	msg.AddString("given name", name.given.c_str());
	msg.AddString("middle name", name.middle.c_str());
	msg.AddString("prefix", name.prefix.c_str());
	msg.AddString("suffix", name.suffix.c_str());
	msg.AddString("full name", fullName.c_str());
	_SendMessage(&msg);

	// Return if there's no avatar icon
	if (!photo.binval.c_str())
		return;

	if (_SetupAvatarCache() == B_OK)
		// Cache avatar icon and notify the change
		_CacheAvatar(jid.bare().c_str(), photo.binval.c_str(),
			photo.binval.length());
}


void
JabberHandler::handleVCardResult(gloox::VCardHandler::VCardContext context,
								 const gloox::JID& jid,
								 gloox::StanzaError)
{
	//if (context == gloox::VCardHandler::FetchVCard)
	//else
}
