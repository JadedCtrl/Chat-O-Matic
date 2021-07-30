/*
 * Copyright 2010, Pier Luigi Fiorini. All rights reserved.
 * Copyright 2021, Jaidyn Levesque. All rights reserved.
 * Distributed under the terms of the GPL v2 License.
 *
 * Authors:
 *		Pier Luigi Fiorini, pierluigi.fiorini@gmail.com
 *		Jaidyn Levesque, jadedctrl@teknik.io
 */

#include <iostream>

#include <Catalog.h>
#include <Directory.h>
#include <Entry.h>
#include <File.h>
#include <FindDirectory.h>
#include <List.h>
#include <StringList.h>

#include <libsupport/SHA1.h>

#include <ChatProtocolMessages.h>
#include <Flags.h>
#include <Role.h>

#include <gloox/chatstatefilter.h>
#include <gloox/messageeventfilter.h>
#include <gloox/mucroom.h>

#include "JabberHandler.h"


#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "JabberHandler"


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
	while ((e = client->recv(10000000)) == gloox::ConnNoError);

	if (e != gloox::ConnUserDisconnected)
		handler->HandleConnectionError(e);

	return B_OK;
}


JabberHandler::JabberHandler()
	:
	fClient(NULL),
	fVCardManager(NULL),
	fSession(NULL)
{
	fAvatars = new BList();
}


JabberHandler::~JabberHandler()
{
	Shutdown();

	BString* item = NULL;
	for (int32 i = 0; (item = (BString*)fAvatars->ItemAt(i)); i++)
		delete item;
	delete fAvatars;
}


status_t
JabberHandler::Init(ChatProtocolMessengerInterface* messenger)
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
		case IM_SET_OWN_STATUS:
		{
			int32 status = msg->FindInt32("status");
			BString status_msg = msg->FindString("message");

			switch (status) {
				case STATUS_ONLINE:
					// Log in if we still need to
					resume_thread(fRecvThread);
					break;
				case STATUS_OFFLINE:
					kill_thread(fRecvThread);
					break;
				default:
					break;
			}
			break;
		}
		case IM_SEND_MESSAGE:
		{
			const char* id = msg->FindString("chat_id");
			const char* subject = msg->FindString("subject");
			const char* body = msg->FindString("body");
			gloox::MUCRoom* room = fRooms.ValueFor(id);

			if (!id || !body)
				return B_ERROR;

			// Send JabberHandler message
			gloox::Message jm(gloox::Message::Chat, gloox::JID(id),
				body, (subject ? subject : gloox::EmptyString));
			if (room != NULL) {
				room->send(body);
				break;
			}
			else
				fClient->send(jm);

			// If non-MUC, tell Caya we actually sent the message
			// (An MUC should echo the message back to us later, see
			// handleMUCMessage)
			_MessageSent(id, subject, body);
			break;
		}
		case IM_CREATE_CHAT:
		{
			const char* user_id = msg->FindString("user_id");

			// TODO: Contact validation, make sure permssion is granted

			if (!user_id)
				return B_ERROR;

			_EnsureUserChat(user_id);
			_ChatCreatedMsg(user_id);
			break;
		}
		case IM_CREATE_ROOM:
		{
			BString chat_id;
			if (msg->FindString("chat_id", &chat_id) != B_OK)
				break;
			_JoinRoom(chat_id);
			break;
		}
		case IM_JOIN_ROOM:
		{
			BString chat_id;
			if (msg->FindString("chat_id", &chat_id) == B_OK)
				_JoinRoom(chat_id.String());
			break;
		}
		case IM_LEAVE_ROOM:
		{
			BString chat_id = msg->FindString("chat_id");
			gloox::MUCRoom* room = fRooms.ValueFor(chat_id);

			// MUCs are special, one-on-ones we can just drop
			if (room != NULL)
				room->leave();

			// We've gotta let Caya know!
			BMessage left(IM_MESSAGE);
			left.AddInt32("im_what", IM_ROOM_LEFT);
			left.AddString("chat_id", chat_id);
			_SendMessage(&left);
			break;
		}
		case IM_ROOM_INVITE_ACCEPT:
		{
			BString chat_id;
			if (msg->FindString("chat_id", &chat_id) != B_OK)
				break;

			BStringList splitAtPassword;
			chat_id.Split("#", false, splitAtPassword);
			chat_id = splitAtPassword.StringAt(0);

			_JoinRoom(chat_id.String());
			break;
		}
		case IM_ROOM_SEND_INVITE:
		{
			BString chat_id = msg->FindString("chat_id");
			gloox::MUCRoom* room = fRooms.ValueFor(chat_id);
			BString user_id;
			if (room == NULL || msg->FindString("user_id", &user_id) != B_OK)
				break;

			room->invite(gloox::JID(user_id.String()), "");
			break;
		}
		case IM_GET_ROOM_PARTICIPANTS:
		{
			BString chat_id = msg->FindString("chat_id");
			gloox::MUCRoom* room = fRooms.ValueFor(chat_id);

			if (room != NULL)
				room->getRoomItems();
			else if (fUserChats.HasString(chat_id) == true) {
				BMessage users(IM_MESSAGE);
				users.AddInt32("im_what", IM_ROOM_PARTICIPANTS);
				users.AddString("user_id", chat_id);
			}
			break;
		}
		case IM_GET_ROOM_METADATA:
		{
			BString chat_id = msg->FindString("chat_id");
			gloox::MUCRoom* room = fRooms.ValueFor(chat_id);
			if (room != NULL)
				room->getRoomInfo();
			if (fUserChats.HasString(chat_id) == true)
			{
				BMessage metadata(IM_MESSAGE);
				metadata.AddInt32("im_what", IM_ROOM_METADATA);
				metadata.AddString("chat_id", chat_id);
				metadata.AddInt32("room_default_flags", 0 | ROOM_LOG_LOCALLY | ROOM_POPULATE_LOGS);
				metadata.AddInt32("room_disallowed_flags", 0 | ROOM_AUTOJOIN | ROOM_AUTOCREATE);
				_SendMessage(&metadata);
			}
			break;
		}
		case IM_SET_ROOM_SUBJECT:
		{
			BString chat_id = msg->FindString("chat_id");
			gloox::MUCRoom* room = fRooms.ValueFor(chat_id);
			if (room != NULL)
				room->setSubject(msg->GetString("subject", ""));
			break;
		}
		case IM_GET_EXTENDED_CONTACT_INFO:
		{
			BString user_id;
			if (msg->FindString("user_id", &user_id) == B_OK)
				fVCardManager->fetchVCard(gloox::JID(user_id.String()), this);
			break;
		}
		case IM_CONTACT_LIST_ADD_CONTACT:
		{
			BString user_name = msg->FindString("user_name");
			BString user_id;
			if (msg->FindString("user_id", &user_id) != B_OK)
				break;
			fClient->rosterManager()->add(gloox::JID(user_id.String()),
				user_name.String(), gloox::StringList());
			fClient->rosterManager()->subscribe(gloox::JID(user_id.String()),
				user_name.String());
			fClient->rosterManager()->synchronize();
			break;
		}
		case IM_CONTACT_LIST_REMOVE_CONTACT:
		{
			BString user_id;
			if (msg->FindString("user_id", &user_id) != B_OK)
				break;
			fClient->rosterManager()->remove(gloox::JID(user_id.String()));
			fClient->rosterManager()->unsubscribe(gloox::JID(user_id.String()));
			fClient->rosterManager()->synchronize();

			BMessage rm(IM_MESSAGE);
			rm.AddInt32("im_what", IM_CONTACT_LIST_CONTACT_REMOVED);
			rm.AddString("user_id", user_id);
			_SendMessage(&rm);
			break;
		}
		case IM_CONTACT_LIST_EDIT_CONTACT:
		{
			BString user_id;
			BString user_name = msg->FindString("user_name");
			if (msg->FindString("user_id", &user_id) != B_OK)
				break;

			gloox::JID jid(user_id.String());
			gloox::RosterItem* item =
				fClient->rosterManager()->getRosterItem(jid);

			if (item != NULL && user_name.IsEmpty() == false) {
				item->setName(user_name.String());
				fClient->rosterManager()->synchronize();
			}
			break;
		}
		case IM_ROOM_KICK_PARTICIPANT:
		case IM_ROOM_BAN_PARTICIPANT:
		case IM_ROOM_UNBAN_PARTICIPANT:
		case IM_ROOM_MUTE_PARTICIPANT:
		case IM_ROOM_UNMUTE_PARTICIPANT:
			_MUCModeration(msg);
			break;
		default:
			return B_ERROR;
	}

	return B_OK;
}


status_t
JabberHandler::Shutdown()
{
	if (fVCardManager)
		fVCardManager->cancelVCardOperations(this);

	if (fClient)
		fClient->disposeMessageSession(fSession);

	kill_thread(fRecvThread);

	return B_OK;
}


void
JabberHandler::SetAddOnPath(BPath path)
{
	fPath = path;
}


BPath
JabberHandler::AddOnPath()
{
	return fPath;
}


void
JabberHandler::SetName(const char* name)
{
	fName = name;
}


const char*
JabberHandler::GetName()
{
	return fName.String();
}


BObjectList<BMessage>
JabberHandler::Commands()
{
	return BObjectList<BMessage>();
}


BObjectList<BMessage>
JabberHandler::ChatPopUpItems()
{
	return BObjectList<BMessage>();
}


BObjectList<BMessage>
JabberHandler::UserPopUpItems()
{
	return BObjectList<BMessage>();
}


BObjectList<BMessage>
JabberHandler::MenuBarItems()
{
	return BObjectList<BMessage>();
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
	fNick = username;
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
	fClient = new OurClient(fJid, fPassword.String());
	fClient->setServer(fServer.String());
	if (fPort > 0)
		fClient->setPort(fPort);
	fClient->registerConnectionListener(this);
	fClient->registerMessageSessionHandler(this);
	fClient->registerMUCInvitationHandler(new InviteHandler(fClient, this));
	fClient->rosterManager()->registerRosterListener(this);
	fClient->disco()->setVersion("Cardie", VERSION);
	fClient->disco()->setIdentity("client", "cardie");
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


ChatProtocolMessengerInterface*
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
JabberHandler::HandleConnectionError(gloox::ConnectionError& e)
{
#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "JabberHandler ― Connection errors"

	// Handle error
	BMessage errMsg(IM_ERROR);

	switch (e) {
		case gloox::ConnStreamError:
		{
#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "JabberHandler ― Connection stream errors"
			gloox::StreamError streamError = fClient->streamError();

			errMsg.AddString("error", B_TRANSLATE("Bad or malformed XML stream."));

			switch (streamError) {
				case gloox::StreamErrorBadFormat:
					errMsg.AddString("detail", B_TRANSLATE("The entity has "
							"sent XML that cannot be processed"));
					break;
				case gloox::StreamErrorBadNamespacePrefix:
					errMsg.AddString("detail", B_TRANSLATE("The entity has "
							"sent a namespace prefix which is not supported, "
							"or has sent no namespace prefix on an element "
							"that requires such prefix."));
					break;
				case gloox::StreamErrorConflict:
					errMsg.AddString("detail", B_TRANSLATE("The server is "
							"closing the active stream for this entity "
							"because a new stream has been initiated that "
							"conflicts with the existing stream."));
					break;
				case gloox::StreamErrorConnectionTimeout:
					errMsg.AddString("detail", B_TRANSLATE("The entity has "
							"not generated any traffic over the stream for "
							"some period of time."));
					break;
				case gloox::StreamErrorHostGone:
					errMsg.AddString("detail", B_TRANSLATE("The host initially "
							"used corresponds to a hostname that is no longer "
							"hosted by the server."));
					break;
				case gloox::StreamErrorHostUnknown:
					errMsg.AddString("detail", B_TRANSLATE("The host initially "
							"used does not correspond to a hostname that is "
							"hosted by the server."));
					break;
				case gloox::StreamErrorImproperAddressing:
					errMsg.AddString("detail", B_TRANSLATE("A stanza sent "
							"between two servers lacks a 'to' or 'from' "
							"attribute (or the attribute has no value."));
					break;
				case gloox::StreamErrorInternalServerError:
					errMsg.AddString("detail", B_TRANSLATE("The server has "
							"experienced a misconfiguration or an "
							"otherwise-undefined internal error that prevents "
							"it from servicing the stream."));
					break;
				case gloox::StreamErrorInvalidFrom:
					errMsg.AddString("detail", B_TRANSLATE("The JID or "
							"hostname provided in a 'from' address does not "
							"match an authorized JID or validated domain "
							"negotiation between servers via SASL or dialback, "
							"or between a client and a server via "
							"authentication and resource binding."));
					break;
				case gloox::StreamErrorInvalidId:
					errMsg.AddString("detail", B_TRANSLATE("The stream ID or "
							"dialback ID is invalid or does not match and ID "
							"previously provdided."));
					break;
				case gloox::StreamErrorInvalidNamespace:
					errMsg.AddString("detail", B_TRANSLATE("The streams "
							"namespace name is something other than "
							"\"http://etherx.jabber.org/streams\" or the "
							"dialback namespace name is something other than "
							"\"jabber:server:dialback\"."));
					break;
				case gloox::StreamErrorInvalidXml:
					errMsg.AddString("detail", B_TRANSLATE("The entity has "
							"sent invalid XML over the stream to a server that "
							"performs validation."));
					break;
				case gloox::StreamErrorNotAuthorized:
					errMsg.AddString("detail", B_TRANSLATE("The entity has "
							"attempted to send data before the stream has been "
							"authenticated, or otherwise is not authorized to "
							"perform an action related to stream negotiation; "
							"the receiving entity must not process the "
							"offending stanza before sending the stream "
							"error."));
					break;
				case gloox::StreamErrorPolicyViolation:
					errMsg.AddString("detail", B_TRANSLATE("The entity has "
							"violated some local service policy; the server "
							"may choose to specify the policy in the <text/> "
							"element or an application-specific condition "
							"element."));
					break;
				case gloox::StreamErrorRemoteConnectionFailed:
					errMsg.AddString("detail", B_TRANSLATE("The server is "
							"unable to properly connect to a remote entity "
							"that is required for authentication."));
					break;
				case gloox::StreamErrorResourceConstraint:
					errMsg.AddString("detail", B_TRANSLATE("The server lacks "
							"the system resources necessary to service the "
							"stream."));
					break;
				case gloox::StreamErrorRestrictedXml:
					errMsg.AddString("detail", B_TRANSLATE("The entity has "
							"attempted to send restricted XML features such as "
							"a comment, processing instruction, DTD, entity "
							"reference or unescaped character."));
					break;
				case gloox::StreamErrorSeeOtherHost:
					errMsg.AddString("detail", B_TRANSLATE("The server will "
							"not provide service to the initiating entity but "
							"is redirecting traffic to another host; the "
							"server should specify the alternate hostname or "
							"IP address (which MUST be a valid domain "
							"identifier) as the XML characted data of the "
							"<see-other-host/> element."));
					break;
				case gloox::StreamErrorSystemShutdown:
					errMsg.AddString("detail", B_TRANSLATE("The server is "
							"being shut down and all active streams are being "
							"closed."));
					break;
				case gloox::StreamErrorUndefinedCondition:
					errMsg.AddString("detail", B_TRANSLATE("The error "
							"condition is not one of those defined by the "
							"other condition in this list; this error "
							"condition should be used only in conjunction with "
							"an application-specific condition."));
					break;
				case gloox::StreamErrorUnsupportedEncoding:
					errMsg.AddString("detail", B_TRANSLATE("The initiating "
							"entity has encoded the stream in an encoding "
							"that is not supported by the server."));
					break;
				case gloox::StreamErrorUnsupportedStanzaType:
					errMsg.AddString("detail", B_TRANSLATE("The initiating "
							"entity has sent a first-level child of the stream "
							"that is not supported by the server."));
					break;
				case gloox::StreamErrorUnsupportedVersion:
					errMsg.AddString("detail", B_TRANSLATE("The value of the "
							"'version' attribute provided by the initiating "
							"entity in the stream header specifies a version "
							"of XMPP that is not supported by the server; the "
							"server may specify the version(s) it supports in "
							"the <text/> element."));
					break;
				case gloox::StreamErrorXmlNotWellFormed:
					errMsg.AddString("detail", B_TRANSLATE("The initiating "
							"entity has sent XML that is not well-formed as "
							"defined by XML."));
					break;
				default:
					break;
			}
			break;
		}
		case gloox::ConnStreamVersionError:
			errMsg.AddString("detail", B_TRANSLATE("The incoming stream's "
					"version is not supported."));
			break;
		case gloox::ConnStreamClosed:
			errMsg.AddString("detail", B_TRANSLATE("The stream has been closed "
					"by the server."));
			break;
		case gloox::ConnProxyAuthRequired:
			errMsg.AddString("detail", B_TRANSLATE("The HTTP/SOCKS5 proxy "
					"requires authentication."));
			break;
		case gloox::ConnProxyAuthFailed:
			errMsg.AddString("detail", B_TRANSLATE("HTTP/SOCKS5 proxy "
					"authentication failed."));
			break;
		case gloox::ConnProxyNoSupportedAuth:
			errMsg.AddString("detail", B_TRANSLATE("The HTTP/SOCKS5 proxy "
					"requires an unsupported authentication mechanism."));
			break;
		case gloox::ConnIoError:
			errMsg.AddString("detail", B_TRANSLATE("Input/output error."));
			break;
		case gloox::ConnParseError:
			errMsg.AddString("detail", B_TRANSLATE("A XML parse error "
					"occurred."));
			break;
		case gloox::ConnConnectionRefused:
			errMsg.AddString("detail", B_TRANSLATE("The connection was refused "
					"by the server on the socket level."));
			break;
		case gloox::ConnDnsError:
			errMsg.AddString("detail", B_TRANSLATE("Server's hostname "
					"resolution failed."));
			break;
		case gloox::ConnOutOfMemory:
			errMsg.AddString("detail", B_TRANSLATE("Out of memory."));
			break;
		case gloox::ConnTlsFailed:
			errMsg.AddString("detail", B_TRANSLATE("The server's certificate "
					"could not be verified or the TLS handshake did not "
					"complete successfully."));
			break;
		case gloox::ConnTlsNotAvailable:
			errMsg.AddString("detail", B_TRANSLATE("The server didn't offer "
					"TLS while it was set to be required, or TLS was not "
					"compiled in."));
			break;
		case gloox::ConnCompressionFailed:
			errMsg.AddString("detail", B_TRANSLATE("Negotiating or "
					"initializing compression failed."));
			break;
		case gloox::ConnAuthenticationFailed:
		{
#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "JabberHandler ― Connection authentication errors"
			gloox::AuthenticationError authError = fClient->authError();

			errMsg.AddString("error", B_TRANSLATE("Authentication failed. "
						"Username or password wrong or account does not "
						"exist."));

			switch (authError) {
				case gloox::SaslAborted:
					errMsg.AddString("detail", B_TRANSLATE("The receiving "
							"entity acknowledges an <abort/> element sent by "
							"initiating entity; sent in reply to the <abort/> "
							"element."));
					break;
				case gloox::SaslIncorrectEncoding:
					errMsg.AddString("detail", B_TRANSLATE("The data provided "
							"by the initiating entity could not be processed "
							"because the base64 encoding is incorrect."));
					break;
				case gloox::SaslInvalidAuthzid:
					errMsg.AddString("detail", B_TRANSLATE("The authid "
							"provided by the initiating entity is invalid, "
							"either because it is incorrectly formatted or "
							"because the initiating entity does not have "
							"permissions to authorize that ID; sent in reply "
							"to a <response/> element or an <auth/> element "
							"with initial response data."));
					break;
				case gloox::SaslInvalidMechanism:
					errMsg.AddString("detail", B_TRANSLATE("The initiating "
							"element did not provide a mechanism or requested "
							"a mechanism that is not supported by the "
							"receiving entity; sent in reply to an <auth/> "
							"element."));
					break;
				case gloox::SaslMalformedRequest:
					errMsg.AddString("detail", B_TRANSLATE("The request is "
							"malformed (e.g., the <auth/> element includes an "
							"initial response but the mechanism does not "
							"allow that); sent in reply to an <abort/>, "
							"<auth/>, <challenge/>, or <response/> element."));
					break;
				case gloox::SaslMechanismTooWeak:
					errMsg.AddString("detail", B_TRANSLATE("The mechanism "
							"requested by the initiating entity is weaker "
							"than server policy permits for that initiating "
							"entity; sent in reply to a <response/> element or "
							"an <auth/> element with initial response data."));
					break;
				case gloox::SaslNotAuthorized:
					errMsg.AddString("detail", B_TRANSLATE("The authentication "
							"failed because the initiating entity did not "
							"provide valid credentials (this includes but is "
							"not limited to the case of an unknown username); "
							"sent in reply to a <response/> element or an "
							"<auth/> element with initial response data."));
					break;
				case gloox::SaslTemporaryAuthFailure:
					errMsg.AddString("detail", B_TRANSLATE("The authentication "
							"failed because of a temporary error condition "
							"within the receiving entity; sent in reply to an "
							"<auth/> element or <response/> element."));
					break;
				case gloox::NonSaslConflict:
					errMsg.AddString("detail", B_TRANSLATE("Resource conflict, "
							"see XEP-0078."));
					break;
				case gloox::NonSaslNotAcceptable:
					errMsg.AddString("detail", B_TRANSLATE("Required "
							"information not provided, see XEP-0078."));
					break;
				case gloox::NonSaslNotAuthorized:
					errMsg.AddString("detail", B_TRANSLATE("Incorrect "
							"credentials."));
					break;
				default:
					break;
			}
			break;
		}
		case gloox::ConnNotConnected: {
			BMessage disconnect(IM_MESSAGE);
			disconnect.AddInt32("im_what", IM_PROTOCOL_DISABLE);
			_SendMessage(&disconnect);
			break;
		}
		default:
			break;
	}

	_SendMessage(&errMsg);
}


void
JabberHandler::HandleStanzaError(gloox::StanzaError error)
{
#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "JabberHandler ― Stanza errors"

	BMessage errMsg(IM_ERROR);
	errMsg.AddString("error", B_TRANSLATE("Stanza-related error"));
	BString detail;

	switch (error)
	{
		case gloox::StanzaErrorBadRequest:
			detail = B_TRANSLATE("The sender has sent XML that is malformed or "
				"that cannot be processed.");
			break;
		case gloox::StanzaErrorConflict:
			detail = B_TRANSLATE("Access cannot be granted because an existing "
				"resource or session exists with the same name or address.");
			break;
		case gloox::StanzaErrorFeatureNotImplemented:
			detail = B_TRANSLATE("This feature hasn't been implemented by the "
				"recipient or by the server.");
			break;
		case gloox::StanzaErrorForbidden:
			detail = B_TRANSLATE("You don't have permssion to do this.");
			break;
		case gloox::StanzaErrorGone:
			detail = B_TRANSLATE("The recipient or server can no longer be "
				"contacted at this address. Try again later, or with a "
				"different address.");
			break;
		case gloox::StanzaErrorInternalServerError:
			detail = B_TRANSLATE("The server could not process the stanza "
				"because of a misconfiguration or an otherwise-undefined "
				"internal server error.");
			break;
		case gloox::StanzaErrorItemNotFound:
			detail = B_TRANSLATE("The addressed JID or item requested cannot "
				"be found.");
			break;
		case gloox::StanzaErrorJidMalformed:
			detail = B_TRANSLATE("An invalid XMPP address or identifier was "
				"given. If you can, please try a different one.");
			break;
		case gloox::StanzaErrorNotAcceptable:
			detail = B_TRANSLATE("The server or user refuses to accept this, "
				"because some criteria hasn't been met (e.g., a local policy "
				"regarding acceptable words in messages).");
			break;
		case gloox::StanzaErrorNotAllowed:
			detail = B_TRANSLATE("You aren't allowed to do this by the server "
				"or recepient.");
			break;
		case gloox::StanzaErrorNotAuthorized:
			detail = B_TRANSLATE("You need to be properily authenticated "
				"before doing this.");
		case gloox::StanzaErrorNotModified:
			detail = B_TRANSLATE("The item requested has not changed since it "
				"was last requested.");
		case gloox::StanzaErrorPaymentRequired:
			detail = B_TRANSLATE("The server refuses to offer service, because "
				"payment is required.");
			break;
		case gloox::StanzaErrorRecipientUnavailable:
			detail = B_TRANSLATE("The recipient is temporarily unavailable.");
			break;
		case gloox::StanzaErrorRedirect:
			detail = B_TRANSLATE("The recipient or server is redirecting "
				"requests for this information to another entity, usually "
				"temporarily.");
			break;
		case gloox::StanzaErrorRegistrationRequired:
			detail = B_TRANSLATE("You can't do this before registration! Be "
				"sure your finished registering for your account.");
			break;
		case gloox::StanzaErrorRemoteServerNotFound:
			detail = B_TRANSLATE("That user's server doesn't exist.");
			break;
		case gloox::StanzaErrorRemoteServerTimeout:
			detail = B_TRANSLATE("Connection to that user's server has timed "
				"out.");
			break;
		case gloox::StanzaErrorResourceConstraint:
			detail = B_TRANSLATE("The server or recipient are too busy right "
				"now; try again later.");
			break;
		case gloox::StanzaErrorServiceUnavailable:
			detail = B_TRANSLATE("The server or recipient don't provide this "
				"service.");
			break;
		case gloox::StanzaErrorSubscribtionRequired:
			detail = B_TRANSLATE("You can't access this unless you are "
				"subscribed.");
			break;
	}

	errMsg.AddString("detail", detail);
	_SendMessage(&errMsg);
}


void
JabberHandler::_SendMessage(BMessage* msg)
{
	// Skip invalid messages
	if (!msg)
		return;

	msg->AddString("protocol", Signature());
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
	msg.AddString("user_id", fJid.bare().c_str());
	msg.AddString("chat_id", id);
	msg.AddString("subject", subject);
	msg.AddString("body", body);
	_SendMessage(&msg);
}


void
JabberHandler::_ChatCreatedMsg(const char* id)
{
	BMessage msg(IM_MESSAGE);
	msg.AddInt32("im_what", IM_CHAT_CREATED);
	msg.AddString("chat_id", id);
	msg.AddString("user_id", id);
	_SendMessage(&msg);
}


void
JabberHandler::_RoleChangedMsg(BString chat_id, BString user_id,
							gloox::MUCRoomRole role, gloox::MUCRoomAffiliation aff)
{
	BMessage roleMsg(IM_MESSAGE);
	roleMsg.AddInt32("im_what", IM_ROOM_ROLECHANGED);
	roleMsg.AddString("user_id", user_id);
	roleMsg.AddString("chat_id", chat_id);
	roleMsg.AddString("role_title", _RoleTitle(role, aff));
	roleMsg.AddInt32("role_perms", _RolePerms(role, aff));
	roleMsg.AddInt32("role_priority", _RolePriority(role, aff));
	_SendMessage(&roleMsg);
}


void
JabberHandler::_UserLeftMsg(BString chat_id, gloox::MUCRoomParticipant participant)
{
	BString user_id;
	const char* nick = participant.nick->resource().c_str();
	bool isSelf = _MUCUserId(chat_id, nick, &user_id);

	if (user_id.IsEmpty() == true)
		return;

	int32 im_what = IM_ROOM_PARTICIPANT_LEFT;
	int flags = participant.flags;

	if (flags & gloox::UserBanned)
		im_what = IM_ROOM_PARTICIPANT_BANNED;
	else if (flags & gloox::UserKicked)
		im_what = IM_ROOM_PARTICIPANT_KICKED;

	BMessage leftMsg(IM_MESSAGE);
	leftMsg.AddInt32("im_what", im_what);
	leftMsg.AddString("chat_id", chat_id);
	leftMsg.AddString("user_id", user_id);
	leftMsg.AddString("user_name", nick);
	if (participant.reason.empty() == false)
		leftMsg.AddString("body", participant.reason.c_str());
	_SendMessage(&leftMsg);
}


void
JabberHandler::_StatusSetMsg(const char* user_id, gloox::Presence::PresenceType type,
						  const char* message, const char* resource)
{
	BMessage msg(IM_MESSAGE);
	msg.AddInt32("im_what", IM_STATUS_SET);
	msg.AddString("user_id", user_id);
	msg.AddInt32("status", _GlooxStatusToApp(type));

	if (BString(resource).IsEmpty() == false)
		msg.AddString("resource", resource);
	if (BString(message).IsEmpty() == false)
		msg.AddString("message", message);

	_SendMessage(&msg);
}


void
JabberHandler::_EnsureUserChat(const char* chat_id)
{
	if (fUserChats.HasString(BString(chat_id)) == false)
		fUserChats.Add(BString(chat_id));
}


status_t
JabberHandler::_SetupAvatarCache()
{
	if (fAvatarCachePath.InitCheck() == B_OK)
		return B_OK;

	BPath path;

	if (find_directory(B_USER_SETTINGS_DIRECTORY, &path) != B_OK)
		return B_ERROR;

	path.Append("Cardie/Cache/Accounts");
	path.Append(GetName());

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

	// Do we need to notify the app?
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
		msg.AddString("user_id", id);
	}
	msg.AddRef("ref", &ref);
	_SendMessage(&msg);
}


UserStatus
JabberHandler::_GlooxStatusToApp(gloox::Presence::PresenceType type)
{
	switch (type) {
		case gloox::Presence::Available:
		case gloox::Presence::Chat:
			return STATUS_ONLINE;
		case gloox::Presence::Away:
			return STATUS_AWAY;
		case gloox::Presence::XA:
			return STATUS_CUSTOM_STATUS;
		case gloox::Presence::DND:
			return STATUS_DO_NOT_DISTURB;
		case gloox::Presence::Unavailable:
			return STATUS_OFFLINE;
		default:
			break;
	}

	return STATUS_OFFLINE;
}


BString
JabberHandler::_MUCChatId(gloox::MUCRoom* room)
{
	BString chat_id(room->name().c_str());
	chat_id << "@" << room->service().c_str();

	BStringList parts;
	chat_id.Split("/", false, parts);

	return parts.StringAt(0);
}


bool
JabberHandler::_MUCUserId(BString chat_id, const char* nick, BString* id)
{
	BString chat(chat_id);
	chat << "/" << nick;

	// If sent from own user, use normal ID
	if (nick == fNick) {
		*id = fJid.bare().c_str();
		return true;
	}

	*id = chat;
	return false;
}


void
JabberHandler::_JoinRoom(const char* chat_id)
{
	BString join_id(chat_id);
	join_id << "/" << fNick;

	gloox::MUCRoom* room = fRooms.ValueFor(chat_id);
	if (room == NULL)
		room = new gloox::MUCRoom(fClient, gloox::JID(join_id.String()), this, this);

	room->join();
	fRooms.AddItem(BString(chat_id), room);
}


void
JabberHandler::_MUCModeration(BMessage* msg)
{
	BString chat_id = msg->FindString("chat_id");
	BString user_id;
	BString body = msg->FindString("body");
	gloox::MUCRoom* room = fRooms.ValueFor(chat_id);

	if (room == NULL || msg->FindString("user_id", &user_id) != B_OK)
		return;

	std::string nick = gloox::JID(user_id.String()).resource();

	switch (msg->FindInt32("im_what"))
	{
		case IM_ROOM_KICK_PARTICIPANT:
			room->kick(nick, body.String());
			break;
		case IM_ROOM_BAN_PARTICIPANT:
			room->ban(nick, body.String());
			break;
		case IM_ROOM_MUTE_PARTICIPANT: 
			room->revokeVoice(nick, body.String());
			break;
		case IM_ROOM_UNMUTE_PARTICIPANT:
			room->grantVoice(nick, body.String());
			break;
	}
}


const char*
JabberHandler::_RoleTitle(gloox::MUCRoomRole role, gloox::MUCRoomAffiliation aff)
{
#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "JabberHandler ― User roles"

	switch (role)
	{
		case gloox::RoleVisitor:
			return B_TRANSLATE("Visitor");
		case gloox::RoleParticipant:
			return B_TRANSLATE("Member");
		case gloox::RoleModerator:
			if (aff == gloox::AffiliationOwner)
				return B_TRANSLATE("Owner");
			return B_TRANSLATE("Moderator");
	}
	return B_TRANSLATE("Invalid");
}


int32
JabberHandler::_RolePerms(gloox::MUCRoomRole role, gloox::MUCRoomAffiliation aff)
{
	switch (role)
	{
		case gloox::RoleVisitor:
			return 0 | PERM_READ | PERM_NICK;
		case gloox::RoleParticipant:
			return 0 | PERM_READ | PERM_WRITE | PERM_ROOM_SUBJECT;
		case gloox::RoleModerator: {
			int32 perm = 0 | PERM_READ | PERM_WRITE | PERM_ROOM_SUBJECT
						   | PERM_KICK | PERM_MUTE;
			if (aff == gloox::AffiliationOwner)
				perm = perm | PERM_ROLECHANGE | PERM_BAN;
			return perm;
		}
	}
	return 0;
}


int32
JabberHandler::_RolePriority(gloox::MUCRoomRole role, gloox::MUCRoomAffiliation aff)
{
	switch (role)
	{
		case gloox::RoleParticipant:
			return 1;
		case gloox::RoleModerator:
			if (aff == gloox::AffiliationOwner)
				return 3;
			return 2;
	}
	return 0;
}


BMessage
JabberHandler::_SettingsTemplate(const char* username, bool serverOption)
{
#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "JabberHandler ― Settings template"

	BMessage stemplate('IMst');

	BMessage usernameText;
	usernameText.AddString("name", "username");
	usernameText.AddString("description", username);
	usernameText.AddString("error", B_TRANSLATE("You can't log into an account "
			"without a username.\nPlease fill in your username for the given "
			"server."));
	usernameText.AddInt32("type", 'CSTR');
	stemplate.AddMessage("setting", &usernameText);

	BMessage passwordText;
	passwordText.AddString("name", "password");
	passwordText.AddString("description", B_TRANSLATE("Password:"));
	passwordText.AddString("error", B_TRANSLATE("You can't log into an account "
			"without a password.\nPlease fill in your password for the given "
			"account."));
	passwordText.AddInt32("type", 'CSTR');
	passwordText.AddBool("is_secret", true);
	stemplate.AddMessage("setting", &passwordText);

	BMessage serverText;
	serverText.AddString("name", "server");
	serverText.AddString("description", B_TRANSLATE("Server:"));
	serverText.AddString("error", B_TRANSLATE("You can't add an account "
			"without a server.\nPlease add a valid XMPP server."));
	serverText.AddInt32("type", 'CSTR');
	if (serverOption == true)
		stemplate.AddMessage("setting", &serverText);

	BMessage resourceText;
	resourceText.AddString("name", "resource");
	resourceText.AddString("description", B_TRANSLATE("Resource:"));
	resourceText.AddInt32("type", 'CSTR');
	resourceText.AddString("default", "Cardie");
	resourceText.AddString("error", B_TRANSLATE("You can't add an account "
			"without a resource.\nDon't worry― it can be whatever string you "
			"want."));
	stemplate.AddMessage("setting", &resourceText);

	return stemplate;
}


BMessage
JabberHandler::_RoomTemplate()
{
#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "JabberHandler ― Room template"

	BMessage stemplate('IMst');
	BMessage roomIdentifier;
	roomIdentifier.AddString("name", "chat_id");
	roomIdentifier.AddString("description", B_TRANSLATE("Room ID:"));
	roomIdentifier.AddString("error", B_TRANSLATE("You can't have a room "
			"without a JID!\nUse the \"name@server\" format."));
	roomIdentifier.AddInt32("type", 'CSTR');
	stemplate.AddMessage("setting", &roomIdentifier);

	return stemplate;
}


BMessage
JabberHandler::_RosterTemplate()
{
#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "JabberHandler ― Roster template"

	BMessage stemplate('IMst');

	BMessage user_id;
	user_id.AddString("name", "user_id");
	user_id.AddString("description", B_TRANSLATE("User ID:"));
	user_id.AddString("error", B_TRANSLATE("You can't befriend an IDless "
			"miscreant!\nPlease use the \"name@server\" format."));
	user_id.AddInt32("type", 'CSTR');
	stemplate.AddMessage("setting", &user_id);

	BMessage user_name;
	user_name.AddString("name", "user_name");
	user_name.AddString("description", B_TRANSLATE("Nickname:"));
	user_name.AddInt32("type", 'CSTR');
	stemplate.AddMessage("setting", &user_name);

	return stemplate;
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
	msg.AddInt32("status", STATUS_ONLINE);
	_SendMessage(&msg);

	fVCardManager->fetchVCard(fJid, this);
}


void
JabberHandler::onDisconnect(gloox::ConnectionError e)
{
	// We are offline
	BMessage msg(IM_MESSAGE);
	msg.AddInt32("im_what", IM_OWN_STATUS_SET);
	msg.AddInt32("status", STATUS_OFFLINE);
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
	HandleConnectionError(e);
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
		const char* jid = (*it).second->jidJID().full().c_str();
		const char* name = (*it).second->name().c_str();
		int32 subscription = (*it).second->subscription();

		// Add jid to the server based contact list message
		contactListMsg.AddString("user_id", jid);

		// Contact information message
		BMessage infoMsg(IM_MESSAGE);
		infoMsg.AddInt32("im_what", IM_CONTACT_INFO);
		infoMsg.AddString("user_id", jid);
		infoMsg.AddString("user_name", name);
		infoMsg.AddInt32("status", STATUS_OFFLINE);

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
		const char* jid = msg.FindString("user_id");
		_SendMessage(&msg);
		fVCardManager->fetchVCard(gloox::JID(jid), this);
	}

	// This is a safe spot to say "READY", since this is called immediately
	// after logging in, whether the user has friends in the roster or not―
	// especially since loading all the users from the roster can take a while.
	BMessage readyMsg(IM_MESSAGE);
	readyMsg.AddInt32("im_what", IM_PROTOCOL_READY);
	_SendMessage(&readyMsg);
}


void
JabberHandler::handleMessageSession(gloox::MessageSession* session)
{
	// Delete previous session
	if (fSession)
		fClient->disposeMessageSession(fSession);
	fSession = session;

	// Register message handler
	fSession->registerMessageHandler(this);

	// Message event filter
	gloox::MessageEventFilter* messageFilter
		= new gloox::MessageEventFilter(fSession);
	messageFilter->registerMessageEventHandler(this);

	// Chat state filter
	gloox::ChatStateFilter* chatFilter
		= new gloox::ChatStateFilter(fSession);
	chatFilter->registerChatStateHandler(this);
}


void
JabberHandler::handleMessage(const gloox::Message& m, gloox::MessageSession*)
{
	// Only chat messages are handled now
	if (m.subtype() != gloox::Message::Chat)
		return;

	// We need a body
	if (m.body() == "")
		return;

	_EnsureUserChat(m.from().bare().c_str());

	// Notify that a chat message was received
	BMessage msg(IM_MESSAGE);
	msg.AddString("user_id", m.from().bare().c_str());
	msg.AddString("chat_id", m.from().bare().c_str());
	msg.AddInt32("im_what", IM_MESSAGE_RECEIVED);
		if (m.subject() != "")
		msg.AddString("subject", m.subject().c_str());
	msg.AddString("body", m.body().c_str());
	_SendMessage(&msg);
}


void
JabberHandler::handleMessageEvent(const gloox::JID& from, gloox::MessageEventType event)
{
}


void
JabberHandler::handleChatState(const gloox::JID& from, gloox::ChatStateType state)
{
	// We're interested only in some states
	if (state == gloox::ChatStateActive || state == gloox::ChatStateInvalid)
		return;

	_EnsureUserChat(from.bare().c_str());

	BMessage msg(IM_MESSAGE);
	msg.AddString("user_id", from.bare().c_str());
	msg.AddString("chat_id", from.bare().c_str());

	switch (state) {
		case gloox::ChatStateComposing:
			msg.AddInt32("im_what", IM_USER_STARTED_TYPING);
			break;
		case gloox::ChatStatePaused:
			msg.AddInt32("im_what", IM_USER_STOPPED_TYPING);
			break;
		case gloox::ChatStateGone:
			// TODO
			break;
		default:
			break;
	}

	_SendMessage(&msg);
}


void
JabberHandler::handleMUCParticipantPresence(gloox::MUCRoom *room,
											const gloox::MUCRoomParticipant participant,
											const gloox::Presence &presence)
{
	// participant.flags, particpiant.role
	// 0-0 (left), 0-* (joined/rolechange)

	gloox::MUCRoomRole role = participant.role;
	gloox::MUCRoomAffiliation aff = participant.affiliation;

	const char* nick = participant.nick->resource().c_str();

	BString user_id;
	BString chat_id = _MUCChatId(room);
	bool isSelf = _MUCUserId(chat_id, nick, &user_id);

	if (chat_id.IsEmpty() == true || user_id.IsEmpty() == true)
		return;

	if (isSelf == true) {
		int im_what = IM_ROOM_JOINED;
		if (presence.presence() == 5)
			im_what = IM_ROOM_LEFT;

		BMessage joinedMsg(IM_MESSAGE);
		joinedMsg.AddInt32("im_what", im_what);
		joinedMsg.AddString("chat_id", chat_id);
		_SendMessage(&joinedMsg);
		_RoleChangedMsg(chat_id, user_id, role, aff);
		return;
	}

	_StatusSetMsg(user_id.String(), presence.presence(), presence.status().c_str(), "");

	// If unavailable (disconnected/left chat)
	if (presence.presence() == 5) {
		_UserLeftMsg(chat_id, participant);
		return;
	}

	BMessage joinMsg(IM_MESSAGE);
	joinMsg.AddInt32("im_what", IM_ROOM_PARTICIPANT_JOINED);
	joinMsg.AddString("user_id", user_id);
	joinMsg.AddString("user_name", nick);
	joinMsg.AddString("chat_id", chat_id);
	_SendMessage(&joinMsg);

	_RoleChangedMsg(chat_id, user_id, role, aff);
}


void
JabberHandler::handleMUCMessage(gloox::MUCRoom *room, const gloox::Message &m,
								bool priv)
{
	BString user_id;
	BString chat_id = _MUCChatId(room);
	bool isSelf = _MUCUserId(chat_id, m.from().resource().c_str(), &user_id);

	if (chat_id.IsEmpty() == true || user_id.IsEmpty() == true)
		return;

	int32 im_what = IM_MESSAGE_RECEIVED;

	// We need a body
	if (m.body() == "")
		return;


	// If sent from own user, then IM_MESSAGE_SENT
	if (isSelf == true)
		im_what = IM_MESSAGE_SENT;

	// when() is only nonzero when sending backdated messages (logs)
	if (m.when() != 0)
		im_what = IM_LOGS_RECEIVED;

	// Notify that a chat message was received
	BMessage msg(IM_MESSAGE);
	msg.AddInt32("im_what", im_what);
	msg.AddString("user_id", user_id);
	msg.AddString("chat_id", chat_id);
		if (m.subject() != "")
		msg.AddString("subject", m.subject().c_str());
	msg.AddString("body", m.body().c_str());
	_SendMessage(&msg);
}


bool
JabberHandler::handleMUCRoomCreation(gloox::MUCRoom* room)
{
	BMessage msg(IM_MESSAGE);
	msg.AddInt32("im_what", IM_ROOM_CREATED);
	msg.AddString("chat_id", _MUCChatId(room));
	_SendMessage(&msg);
	return true;
}


void
JabberHandler::handleMUCSubject(gloox::MUCRoom *room, const std::string &nick,
								const std::string &subject)
{
	BString user_id;
	BString chat_id = _MUCChatId(room);
	bool isSelf = _MUCUserId(chat_id, nick.c_str(), &user_id);

	if (chat_id.IsEmpty() == true)
		return;

	BMessage msg(IM_MESSAGE);
	msg.AddInt32("im_what", IM_ROOM_SUBJECT_SET);
	msg.AddString("subject", subject.c_str());
	msg.AddString("chat_id", chat_id);
	if (user_id.IsEmpty() == false)
		msg.AddString("user_id", user_id);
	_SendMessage(&msg);
}


void
JabberHandler::handleMUCInviteDecline(gloox::MUCRoom *room, const gloox::JID &invitee,
									  const std::string &reason)
{
}




void
JabberHandler::handleMUCError(gloox::MUCRoom *room, gloox::StanzaError error)
{
	HandleStanzaError(error);
}


void
JabberHandler::handleMUCInfo(gloox::MUCRoom *room, int features,
							 const std::string &name, const gloox::DataForm *infoForm)
{
	BString chat_id = _MUCChatId(room);

	BMessage metadata(IM_MESSAGE);
	metadata.AddInt32("im_what", IM_ROOM_METADATA);
	metadata.AddString("chat_id", chat_id);
	metadata.AddString("chat_name", name.c_str());
	metadata.AddInt32("room_default_flags",
		0 | ROOM_AUTOJOIN | ROOM_LOG_LOCALLY | ROOM_POPULATE_LOGS);
	metadata.AddInt32("room_disallowed_flags", 0 | ROOM_AUTOCREATE);
	_SendMessage(&metadata);
}


void
JabberHandler::handleMUCItems(gloox::MUCRoom *room, const gloox::Disco::ItemList &items)
{
	BString chat_id = _MUCChatId(room);
	BStringList nicks;
	BStringList ids;

	if (chat_id.IsEmpty() == true)
		return;

	for (auto item: items) {
		BString nick = item->jid().resource().c_str();
		nicks.Add(nick);

		// Unless it's the user, derive ID from room resource
		if (fNick != nick) {
			BString user_id(chat_id);
			user_id << "/" << nick;
			ids.Add(user_id);
		}
		else
			ids.Add(BString(fJid.bare().c_str()));
	}

	BMessage msg(IM_MESSAGE);
	msg.AddStrings("user_id", ids);
	msg.AddStrings("user_name", nicks);
	msg.AddString("chat_id", chat_id);
	msg.AddInt32("im_what", IM_ROOM_PARTICIPANTS);

	_SendMessage(&msg);
}


void
JabberHandler::handleMUCConfigList(gloox::MUCRoom* room, const gloox::MUCListItemList &items,
								   gloox::MUCOperation operation)
{
}


void
JabberHandler::handleMUCConfigForm(gloox::MUCRoom* room, const gloox::DataForm &form)
{
}


void
JabberHandler::handleMUCConfigResult(gloox::MUCRoom* room, bool success,
									 gloox::MUCOperation operation)
{
}


void
JabberHandler::handleMUCRequest(gloox::MUCRoom* room, const gloox::DataForm &form)
{
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
	_StatusSetMsg(item.jidJID().full().c_str(), type, presenceMsg.c_str(),
		resource.c_str());
}


void
JabberHandler::handleSelfPresence(const gloox::RosterItem& item, const std::string&,
								  gloox::Presence::PresenceType type,
								  const std::string& presenceMsg)
{
	BMessage msg(IM_MESSAGE);
	msg.AddInt32("im_what", IM_OWN_CONTACT_INFO);
	msg.AddString("protocol", Signature());
	msg.AddString("user_id", item.jidJID().full().c_str());
	msg.AddString("user_name", item.name().c_str());
	msg.AddInt32("subscription", item.subscription());
	msg.AddInt32("status", _GlooxStatusToApp(type));
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
		std::cout << msg << std::endl;
}


void
JabberHandler::handleVCard(const gloox::JID& jid, const gloox::VCard* card)
{
	if (!card)
		return;

	gloox::VCard::Name name = card->name();
	gloox::VCard::Photo photo = card->photo();
	BString nick(card->nickname().c_str());

	gloox::RosterItem* item = fClient->rosterManager()->getRosterItem(jid);
	if (item != NULL)
		nick = item->name().c_str();

	std::string fullName = name.family + " " + name.given;

	BMessage msg(IM_MESSAGE);
	msg.AddInt32("im_what", IM_EXTENDED_CONTACT_INFO);
	msg.AddString("user_id", jid.bare().c_str());
	msg.AddString("user_name", nick);
	msg.AddString("family_name", name.family.c_str());
	msg.AddString("given_name", name.given.c_str());
	msg.AddString("middle_name", name.middle.c_str());
	msg.AddString("prefix", name.prefix.c_str());
	msg.AddString("suffix", name.suffix.c_str());
	msg.AddString("full_name", fullName.c_str());
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


OurClient::OurClient(const gloox::JID jid, const char* password)
	:
	gloox::Client(jid, password)
{
}


void
OurClient::handleTag(gloox::Tag* tag)
{
	if (DEBUG_ENABLED)
		std::cerr << "Tag\t" << tag->xml() << std::endl;
	gloox::Client::handleTag(tag);
}


InviteHandler::InviteHandler(gloox::ClientBase* client, JabberHandler* handler)
	:
	gloox::MUCInvitationHandler(client),
	fHandler(handler)
{
}


void
InviteHandler::handleMUCInvitation(const gloox::JID& room, const gloox::JID& from,
								   const std::string& reason, const std::string& body,
								   const std::string& password, bool cont,
								   const std::string& thread)
{
	std::string chat_name = room.resource().c_str();
	BString chat_id = room.full().c_str();

	if (chat_name.empty() == true)
		chat_name = chat_id.String();
	if (password.empty() == false)
		chat_id << "#" << password.c_str();

	BMessage invite(IM_MESSAGE);
	invite.AddInt32("im_what", IM_ROOM_INVITE_RECEIVED);
	invite.AddString("chat_id", chat_id);
	invite.AddString("chat_name", chat_name.c_str());
	invite.AddString("user_id", from.bare().c_str());
	if (reason.empty() == false)
		invite.AddString("body", reason.c_str());
	invite.AddString("protocol", fHandler->Signature());

	fHandler->MessengerInterface()->SendMessage(new BMessage(invite));
}


