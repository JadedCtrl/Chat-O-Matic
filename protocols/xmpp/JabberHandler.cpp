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
	JabberHandler* handler = (JabberHandler*)data;
	if (!handler)
		return B_BAD_VALUE;

	gloox::Client* client = handler->Client();
	if (!client)
		return B_BAD_VALUE;

	gloox::ConnectionError e;
	while ((e = client->recv(1000)) == gloox::ConnNoError);

	if (e != gloox::ConnUserDisconnected)
		handler->HandleError(e);

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
		B_NORMAL_PRIORITY, (void*)this);
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


gloox::Client*
JabberHandler::Client() const
{
	return fClient;
}


void
JabberHandler::HandleError(gloox::ConnectionError& e)
{
	// Handle error
	BMessage errMsg(IM_ERROR);

	switch (e) {
		case gloox::ConnStreamError:
		{
			gloox::StreamError streamError = fClient->streamError();

			errMsg.AddString("error", "Bad or malformed XML stream.");

			switch (streamError) {
				case gloox::StreamErrorBadFormat:
					errMsg.AddString("detail", "The entity has sent XML that "
						"cannot be processed");
					break;
				case gloox::StreamErrorBadNamespacePrefix:
					errMsg.AddString("detail", "The entity has sent a namespace "
						"prefix which is not supported, or has sent no namespace "
						"prefix on an element that requires such prefix.");
					break;
				case gloox::StreamErrorConflict:
					errMsg.AddString("detail", "The server is closing the active "
						"stream for this entity because a new stream has been "
						"initiated that conflicts with the existing stream.");
					break;
				case gloox::StreamErrorConnectionTimeout:
					errMsg.AddString("detail", "The entity has not generated any "
						"traffic over the stream for some period of time.");
					break;
				case gloox::StreamErrorHostGone:
					errMsg.AddString("detail", "The host initially used corresponds "
						"to a hostname that is no longer hosted by the server.");
					break;
				case gloox::StreamErrorHostUnknown:
					errMsg.AddString("detail", "The host initially used does not "
						"correspond to a hostname that is hosted by the server.");
					break;
				case gloox::StreamErrorImproperAddressing:
					errMsg.AddString("detail", "A stanza sent between two servers "
						"lacks a 'to' or 'from' attribute (or the attribute has no "
						"value.");
					break;
				case gloox::StreamErrorInternalServerError:
					errMsg.AddString("detail", "The Server has experienced a "
						"misconfiguration or an otherwise-undefined internal error "
						"that prevents it from servicing the stream.");
					break;
				case gloox::StreamErrorInvalidFrom:
					errMsg.AddString("detail", "The JID or hostname provided in a "
						"'from' address does not match an authorized JID or validated "
						"domain negotiation between servers via SASL or dialback, or "
						"between a client and a server via authentication and resource "
						"binding.");
					break;
				case gloox::StreamErrorInvalidId:
					errMsg.AddString("detail", "The stream ID or dialback ID is invalid "
						"or does not match and ID previously provdided.");
					break;
				case gloox::StreamErrorInvalidNamespace:
					errMsg.AddString("detail", "The streams namespace name is something "
						"other than \"http://etherx.jabber.org/streams\" or the dialback "
						"namespace name is something other than \"jabber:server:dialback\".");
					break;
				case gloox::StreamErrorInvalidXml:
					errMsg.AddString("detail", "The entity has sent invalid XML over the "
						"stream to a server that performs validation.");
					break;
				case gloox::StreamErrorNotAuthorized:
					errMsg.AddString("detail", "The entity has attempted to send data before "
						"the stream has been authenticated, or otherwise is not authorized to "
						"perform an action related to stream negotiation; the receiving entity "
						"must not process the offending stanza before sending the stream error.");
					break;
				case gloox::StreamErrorPolicyViolation:
					errMsg.AddString("detail", "The entity has violated some local service "
						"policy; the server may choose to specify the policy in the <text/> "
						"element or an application-specific condition element.");
					break;
				case gloox::StreamErrorRemoteConnectionFailed:
					errMsg.AddString("detail", "The server is unable to properly connect to "
						"a remote entit that is required for authentication.");
					break;
				case gloox::StreamErrorResourceConstraint:
					errMsg.AddString("detail", "The server lacks the system resources necessary "
						"to service the stream.");
					break;
				case gloox::StreamErrorRestrictedXml:
					errMsg.AddString("detail", "The entity has attempted to send restricted XML "
						"features such as a comment, processing instruction, DTD, entity reference "
						"or unescaped character.");
					break;
				case gloox::StreamErrorSeeOtherHost:
					errMsg.AddString("detail", "The server will not provide service to the initiating "
						"entity but is redirecting traffic to another host; the server should specify "
						"the alternate hostname or IP address (which MUST be a valid domain identifier) "
						"as the XML characted data of the <see-other-host/> element.");
					break;
				case gloox::StreamErrorSystemShutdown:
					errMsg.AddString("detail", "The server is being shut down and all active streams "
						"are being closed.");
					break;
				case gloox::StreamErrorUndefinedCondition:
					errMsg.AddString("detail", "The error condition is not one of those defined by the "
						"other condition in this list; this error condition should be used only in "
						"conjunction with an application-specific condition.");
					break;
				case gloox::StreamErrorUnsupportedEncoding:
					errMsg.AddString("detail", "The initiating entity has encoded the stream in an "
						"encoding that is not supported by the server.");
					break;
				case gloox::StreamErrorUnsupportedStanzaType:
					errMsg.AddString("detail", "The initiating entity has sent a first-level child "
						"of the stream that is not supported by the server.");
					break;
				case gloox::StreamErrorUnsupportedVersion:
					errMsg.AddString("detail", "The value of the 'version' attribute provided by the "
						"initiating entity in the stream header specifies a version of XMPP that is not "
						"supported by the server; the server may specify the version(s) it supports in "
						"the <text/> element.");
					break;
				case gloox::StreamErrorXmlNotWellFormed:
					errMsg.AddString("detail", "The initiating entity has sent XML that is not "
						"well-formed as defined by XML.");
					break;
				default:
					break;
			}
			break;
		}
		case gloox::ConnStreamVersionError:
			errMsg.AddString("detail", "The incoming stream's version is not "
				"supported.");
			break;
		case gloox::ConnStreamClosed:
			errMsg.AddString("detail", "The stream has been closed by the server.");
			break;
		case gloox::ConnProxyAuthRequired:
			errMsg.AddString("detail", "The HTTP/SOCKS5 proxy requires authentication.");
			break;
		case gloox::ConnProxyAuthFailed:
			errMsg.AddString("detail", "HTTP/SOCKS5 proxy authentication failed.");
			break;
		case gloox::ConnProxyNoSupportedAuth:
			errMsg.AddString("detail", "The HTTP/SOCKS5 proxy requires an unsupported "
				"authentication mechanism.");
			break;
		case gloox::ConnIoError:
			errMsg.AddString("detail", "Input/output error.");
			break;
		case gloox::ConnParseError:
			errMsg.AddString("detail", "A XML parse error occurred.");
			break;
		case gloox::ConnConnectionRefused:
			errMsg.AddString("detail", "The connection was refused by the server "
				"on the socket level.");
			break;
		case gloox::ConnDnsError:
			errMsg.AddString("detail", "Server's hostname resolution failed.");
			break;
		case gloox::ConnOutOfMemory:
			errMsg.AddString("detail", "Out of memory.");
			break;
		case gloox::ConnTlsFailed:
			errMsg.AddString("detail", "The server's certificate could not be verified or "
				"the TLS handshake did not complete successfully.");
			break;
		case gloox::ConnTlsNotAvailable:
			errMsg.AddString("detail", "The server didn't offer TLS while it was set to be "
				"required, or TLS was not compiled in.");
			break;
		case gloox::ConnCompressionFailed:
			errMsg.AddString("detail", "Negotiating or initializing compression failed.");
			break;
		case gloox::ConnAuthenticationFailed:
		{
			gloox::AuthenticationError authError = fClient->authError();

			errMsg.AddString("error", "Authentication failed. Username or password wrong "
				"or account does not exist.");

			switch (authError) {
				case gloox::SaslAborted:
					errMsg.AddString("detail", "The receiving entity acknowledges an <abort/> "
						"element sent by initiating entity; sent in reply to the <abort/> "
						"element.");
					break;
				case gloox::SaslIncorrectEncoding:
					errMsg.AddString("detail", "The data provided by the initiating entity "
						"could not be processed because the base64 encoding is incorrect.");
					break;
				case gloox::SaslInvalidAuthzid:
					errMsg.AddString("detail", "The authid provided by the initiating entity "
						"is invalid, either because it is incorrectly formatted or because "
						"the initiating entity does not have permissions to authorize that ID; "
						"sent in reply to a <response/> element or an <auth/> element with "
						"initial response data.");
					break;
				case gloox::SaslInvalidMechanism:
					errMsg.AddString("detail", "The initiating element did not provide a "
						"mechanism or requested a mechanism that is not supported by the "
						"receiving entity; sent in reply to an <auth/> element.");
					break;
				case gloox::SaslMalformedRequest:
					errMsg.AddString("detail", "The request is malformed (e.g., the <auth/> "
						"element includes an initial response but the mechanism does not "
						"allow that); sent in reply to an <abort/>, <auth/>, <challenge/>, or "
						"<response/> element.");
					break;
				case gloox::SaslMechanismTooWeak:
					errMsg.AddString("detail", "The mechanism requested by the initiating entity "
						"is weaker than server policy permits for that initiating entity; sent in "
						"reply to a <response/> element or an <auth/> element with initial "
						"response data.");
					break;
				case gloox::SaslNotAuthorized:
					errMsg.AddString("detail", "The authentication failed because the initiating "
						"entity did not provide valid credentials (this includes but is not "
						"limited to the case of an unknown username); sent in reply to a "
						"<response/> element or an <auth/> element with initial response data.");
					break;
				case gloox::SaslTemporaryAuthFailure:
					errMsg.AddString("detail", "The authentication failed because of a temporary "
						"error condition within the receiving entity; sent in reply to an "
						"<auth/> element or <response/> element.");
					break;
				case gloox::NonSaslConflict:
					errMsg.AddString("detail", "Resource conflict, see XEP-0078.");
					break;
				case gloox::NonSaslNotAcceptable:
					errMsg.AddString("detail", "Required information not provided, "
						"see XEP-0078.");
					break;
				case gloox::NonSaslNotAuthorized:
					errMsg.AddString("detail", "Incorrect credentials.");
					break;
				default:
					break;
			}
			break;
		}
		case gloox::ConnNotConnected:
			errMsg.AddString("error", "There is no active connection.");
			break;
		default:
			break;
	}

	_SendMessage(&errMsg);
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
JabberHandler::_Notify(notification_type type, const char* title, const char* message)
{
	BMessage msg(IM_MESSAGE);
	msg.AddInt32("im_what", IM_NOTIFICATION);
	msg.AddInt32("type", (int32)type);
	msg.AddString("title", title);
	msg.AddString("message", message);
	_SendMessage(&msg);
}


void
JabberHandler::_NotifyProgress(const char* title, const char* message, float progress)
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
	// We are online
	BMessage msg(IM_MESSAGE);
	msg.AddInt32("im_what", IM_OWN_STATUS_SET);
	msg.AddInt32("status", CAYA_ONLINE);
	_SendMessage(&msg);

	// Notify our online status
	BString content(fUsername);
	content << " has logged in!";
	_Notify(B_INFORMATION_NOTIFICATION, "Connected",
		content.String());

	fVCardManager->fetchVCard(fJid, this);
}


void
JabberHandler::onDisconnect(gloox::ConnectionError e)
{
	// We are offline
	BMessage msg(IM_MESSAGE);
	msg.AddInt32("im_what", IM_OWN_STATUS_SET);
	msg.AddInt32("status", CAYA_OFFLINE);
	_SendMessage(&msg);

	if (e == gloox::ConnNoError) {
		// Notify our offline status
		BString content(fUsername);
		content << " has logged out!";
		_Notify(B_INFORMATION_NOTIFICATION, "Disconnected",
			content.String());
		return;
	}

	// Handle error
	HandleError(e);
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
									const std::string& resource,
									gloox::Presence::PresenceType type,
									const std::string& presenceMsg)
{
	BMessage msg(IM_MESSAGE);
	msg.AddInt32("im_what", IM_STATUS_SET);
	msg.AddString("id", item.jid().c_str());
	msg.AddInt32("status", _GlooxStatusToCaya(type));
	msg.AddString("resource", resource.c_str());
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
	msg.AddInt32("status", _GlooxStatusToCaya(type));
	msg.AddString("message", presenceMsg.c_str());

	// Groups
	gloox::StringList g = item.groups();
	gloox::StringList::const_iterator it_g = g.begin();
	for (; it_g != g.end(); ++it_g)
		msg.AddString("group", (*it_g).c_str());

	// Resources
	gloox::RosterItem::ResourceMap::const_iterator rit
		= item.resources().begin();
	for (; rit != item.resources().end(); ++rit)
		msg.AddString("resource", (*rit).first.c_str());

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
