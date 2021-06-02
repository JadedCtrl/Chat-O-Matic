/*
 * Copyright 2010, Pier Luigi Fiorini. All rights reserved.
 * Distributed under the terms of the GPL v2 License.
 */
#ifndef _JABBER_HANDLER_H
#define _JABBER_HANDLER_H

#include <Notification.h>
#include <Path.h>
#include <String.h>

#include <gloox/client.h>
#include <gloox/chatstatehandler.h>
#include <gloox/connectionlistener.h>
#include <gloox/connectiontcpclient.h>
#include <gloox/discohandler.h>
#include <gloox/disco.h>
#include <gloox/rostermanager.h>
#include <gloox/loghandler.h>
#include <gloox/logsink.h>
#include <gloox/message.h>
#include <gloox/messagehandler.h>
#include <gloox/messagesession.h>
#include <gloox/messagesessionhandler.h>
#include <gloox/messageeventhandler.h>
#include <gloox/mucroomhandler.h>
#include <gloox/mucroomconfighandler.h>
#include <gloox/presence.h>
#include <gloox/vcardhandler.h>
#include <gloox/vcardmanager.h>

#include <CayaProtocol.h>
#include <CayaConstants.h>
#include <libsupport/KeyMap.h>

class BList;


typedef KeyMap<BString, gloox::MUCRoom*> RoomMap;


class JabberHandler : public CayaProtocol, gloox::RosterListener, gloox::ConnectionListener,
								gloox::LogHandler, gloox::MessageSessionHandler,
								gloox::MessageHandler, gloox::MessageEventHandler,
								gloox::ChatStateHandler, gloox::VCardHandler,
								gloox::MUCRoomHandler, gloox::MUCRoomConfigHandler {
public:
									JabberHandler();
	virtual							~JabberHandler();

			// CayaProtocol inheritance
	virtual	status_t				Init(CayaProtocolMessengerInterface*);

	virtual	status_t				Process(BMessage* msg);

	virtual	status_t				Shutdown();

	virtual	const char*				Signature() const = 0;
	virtual	const char*				FriendlySignature() const = 0;

	virtual void					SetAddOnPath(BPath path);
	virtual BPath					AddOnPath();

	virtual void					SetName(const char* name);
	virtual const char*				GetName();

	virtual	status_t				UpdateSettings(BMessage* msg);

	virtual	uint32					GetEncoding();

	virtual bool					SaveLogs() const { return true; }

	virtual CayaProtocolMessengerInterface*
									MessengerInterface() const;

			// Functions for gloox
			gloox::Client*			Client() const;
			void					HandleConnectionError(gloox::ConnectionError& e);
			void					HandleStanzaError(gloox::StanzaError error);

			// Callbacks for protocols
	virtual	void					OverrideSettings() = 0;
	virtual	BString					ComposeJID() const = 0;

protected:
			BString					fUsername;
			BString					fPassword;
			BString					fServer;
			BString					fResource;
			uint16					fPort;

			BPath					fPath;
			BString					fName;

			BMessage				_SettingsTemplate(const char* username, bool serverOption);
private:
			CayaProtocolMessengerInterface*
									fServerMessenger;

			gloox::Client*			fClient;
			gloox::ConnectionTCPClient*
									fConnection;
			gloox::VCardManager*	fVCardManager;
			gloox::MessageSession*	fSession;

			gloox::JID				fJid;
			thread_id				fRecvThread;
			RoomMap					fRooms;

			BPath					fCachePath;
			BPath					fAvatarCachePath;
			BMessage				fAvatarCache;
			BList*					fAvatars;

			void					_SendMessage(BMessage* msg);
			void					_MessageSent(const char* id, const char* subject,
												const char* body);

			void					_ChatCreated(const char* id);


			void					_Notify(notification_type type, const char* title, const char* message);
			void					_NotifyProgress(const char* title, const char* message, float progress);

			status_t				_SetupAvatarCache();
			status_t				_SaveAvatarCache();
			void					_CacheAvatar(const char* id, const char* binval, size_t length);
			void					_AvatarChanged(const char*id, const char* filename);

			CayaStatus				_GlooxStatusToCaya(gloox::Presence::PresenceType type);

	virtual	void					onConnect();
	virtual	void					onDisconnect(gloox::ConnectionError);
	virtual	bool					onTLSConnect(const gloox::CertInfo&);
	virtual	void					onResourceBindError(const gloox::Error*);
	virtual	void					handleRoster(const gloox::Roster&);
	virtual	void					handleMessageSession(gloox::MessageSession* session);
	virtual	void					handleMessage(const gloox::Message& m, gloox::MessageSession*);
	virtual	void					handleMessageEvent(const gloox::JID& from, gloox::MessageEventType event);
	virtual	void					handleChatState(const gloox::JID& from, gloox::ChatStateType state);

	virtual void					handleMUCParticipantPresence(gloox::MUCRoom* room,
																 const gloox::MUCRoomParticipant participant,
																 const gloox::Presence &presence);
	virtual void					handleMUCMessage(gloox::MUCRoom* room, const gloox::Message &m, bool priv);
	virtual bool					handleMUCRoomCreation(gloox::MUCRoom* room);
	virtual void 					handleMUCSubject(gloox::MUCRoom* room, const std::string &nick,
													 const std::string &subject);
	virtual void					handleMUCInviteDecline(gloox::MUCRoom* room, const gloox::JID &invitee,
														   const std::string &reason);
	virtual void					handleMUCError(gloox::MUCRoom* room, gloox::StanzaError error);
	virtual void					handleMUCInfo(gloox::MUCRoom* room, int features, const std::string &name,
												  const gloox::DataForm* infoForm);
	virtual void					handleMUCItems(gloox::MUCRoom* room, const gloox::Disco::ItemList &items);

	virtual void					handleMUCConfigList(gloox::MUCRoom* room,
														const gloox::MUCListItemList &items,
														gloox::MUCOperation operation);
	virtual void 					handleMUCConfigForm(gloox::MUCRoom* room, const gloox::DataForm &form);
	virtual void 					handleMUCConfigResult(gloox::MUCRoom* room, bool success,
														  gloox::MUCOperation operation);
	virtual void				 	handleMUCRequest(gloox::MUCRoom* room, const gloox::DataForm &form);

	virtual	void					handleItemAdded(const gloox::JID&);
	virtual	void					handleItemSubscribed(const gloox::JID&);
	virtual	void					handleItemUnsubscribed(const gloox::JID&);
	virtual	void					handleItemRemoved(const gloox::JID&);
	virtual	void					handleItemUpdated(const gloox::JID&);
	virtual	void					handleRosterPresence(const gloox::RosterItem&,
														 const std::string&, gloox::Presence::PresenceType,
														 const std::string&);
	virtual	void					handleSelfPresence(const gloox::RosterItem&, const std::string&,
													   gloox::Presence::PresenceType, const std::string&);
	virtual	bool					handleSubscriptionRequest(const gloox::JID&, const std::string&);
	virtual	bool					handleUnsubscriptionRequest(const gloox::JID&, const std::string&);
	virtual	void					handleNonrosterPresence(const gloox::Presence&);
	virtual	void					handleRosterError(const gloox::IQ&);
	virtual	void					handleLog(gloox::LogLevel, gloox::LogArea, const std::string&);
	virtual	void					handleVCard(const gloox::JID&, const gloox::VCard*);
	virtual	void					handleVCardResult(gloox::VCardHandler::VCardContext,
													  const gloox::JID&, gloox::StanzaError);
};

#endif	// _JABBER_HANDLER_H
