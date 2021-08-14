This file isn't comprehensive, and is only quick overview. It's probably
only useful in conjunction with skimming the appropriate headers
(`application/ChatProtocol.h`, `application/ChatProtocolMessages.h`) and one of
the included add-ons.
This file's mostly here mostly here to clear up some behaviors that might not be
immediately clear.


# Add-on structure
An add-on should export a few functions, and offer at least one class inheriting
ChatProtocol.

Each add-on must export the following functions;
	ChatProtocol* protocol_at(int32 i)
	int32 protocol_count()
	const char* signature()
	const char* friendly_signature()
	uint32 version()

A single add-on can support multiple protocols (the Purple add-on being the
only current example of this)― but generally returning a "1" from
protocol_count() and only returning a protocol from protocol_at(0) should be
all you need.

For a full description of the ChatProtocol object, look through
`application/ChatProtocol.h`.

Each ChatProtocol can be treated as either an account's instance, or solely for
retrieving general metadata from the Icon(), Signature(), etc. methods― so
please don't start the connection from the constructor.


## Connection details
A ChatProtocol's UpdateSettings() method should be used to receive settings from
the program and declare a separate connection thread, but it generally shouldn't
actually start the connection nor this thread.

The connection should be started when the user's status is set to STATUS_ONLINE―
and correspondingly, the connection should be paused or terminated when it is
STATUS_OFFLINE. The user should be able to easily toggle the connection this way
without any real consequences. If this is impossible, then the add-on can send
IM_PROTOCOL_DISABLE just as the user-status is set to offline.

STATUS_OFFLINE is for a momentary pause, i.e., the server is down or the user
toggled the connection. The ChatProtocol will remain existent and active.

If the status is set to offline by the protocol (and not the user!) Cardie will
automatically attempt a reconnect after some time by trying to set the status
to STATUS_ONLINE, which should toggle the connection.

IM_PROTOCOL_DISABLE deletes the ChatProtocol, and should only be sent to the app
when an irrecoverable error requiring user intervention has happened, i.e., a
configuration error, incorrect password, etc. If possible, it's preferable to
send the user a notification (with IM_ERROR) telling them about the problem,
just before sending IM_PROTOCOL_DISABLE.


## Templates
Each ChatProtocol has to provide UI "templates" for some important dialogues
through the SettingsTemplate() method. In order of importance, they are
"account", "create_room", "join_room", and "roster".

"account" is used for accounts settings (seen by the user when configuring their
account), "create_room"/"join_room" for creating or joining a room respectively,
and "roster" for adding/editing a contact on the roster.

Here's a shorter version of the XMPP add-on's "account" settings:

BMessage('IMst') {
        setting[0] = BMessage(0x0) {
                name = string("username")
                description = string("Jabber identifier:")
                error = string("You can't log into an account without a username.")
                type = int32(0x43535452 or 1129534546)
        }
        setting[1] = BMessage(0x0) {
                name = string("password")
                description = string("Password:")
                error = string("You can't log into an account without a password.")
                type = int32(0x43535452 or 1129534546)
                is_secret = bool(true)
        }
        setting[2] = BMessage(0x0) {
                name = string("server")
                description = string("Server:")
                error = string("You can't add an account without a server.")
                type = int32(0x43535452 or 1129534546)
        }
}

The template is a BMessage with sub-messages named "setting", each with, at
least, an internal "name" (the slot used by Cardie in the message parameter of
UpdateSettings()), a user-friendly label ("description"), and the slot type
in "type"― currently B_INT32_TYPE, B_STRING_TYPE, and B_BOOL_TYPE are accepted.

By default, slots are not required, and it's accepted for the user to skip them.
To make a slot required, put an error message into "error", warning the user
to change their ways.

Also optionally accepted are "is_secret" to determine if entered text is
visible and "default" for a default value.

The internal names of "settings" in these templates determine the values you
should expect to receive for some messages from the app, like IM_JOIN_ROOM.



# Dealing with messages
For documentation of each API message, look through
`application/ChatProtocolMessages.h`. For an example add-on, take a look at
`protocols/xmpp/`― though it might not be simple, it is feature-complete.

When messages are received from or to the app, it will generally be a message
of IM_MESSAGE with an int32 named "im_what" containing the value.

"im_what" values (along with comprehensive descriptions of their meanings) 
can be found in `application/ChatProtocolMessages.h`.

## Slots
Here are standard message slots that are frequently used or required, along
with their meanings:
	* chat_id	Unique identifier for a chatroom. (e.g., room address)
	* chat_name	Display-name for a chatroom. Uniqueness not required.
	* user_id	Unique identifier for a user. (e.g., JID, Matrix username)
	* user_name	Nick-name or display name for a user. Uniqueness not required.
	* body		Used for message-text, or explanation of an action (inviting or banning a user, etc)

user_names and chat_names can be changed at will
(through IM_CONTACT_INFO/IM_USER_SET_NAME/IM_ROOM_PARTICIPANTS and
IM_ROOM_DATA/IM_ROOM_NAME_SET respectively), but user_ids and chat_ids cannot
be changed.

If you have to, you can "change" a chat or user's ID, by faking the user leaving
and re-joining the room. This should be avoided if possible, since it breaks
continuity a bit.



# Joining a room
The basic structure for joining a room should be like this, with each line being
a subsequent response to the previous message:
	* Cardie (`IM_JOIN_ROOM`/`IM_CREATE_CHAT`) → Protocol
	* Protocol (`IM_ROOM_JOINED`/`IM_CHAT_CREATED`) → Cardie 
	* Cardie (`IM_GET_ROOM_METADATA` & `IM_GET_ROOM_PARTICIPANTS`) → Protocol
	* Protocol (`IM_ROOM_METADATA` & `IM_ROOM_PARTICIPANTS`) → Cardie

Preferably, IM_ROOM_METADATA and IM_ROOM_PARTICIPANTS should only be used as
above (in response to a request from Cardie) since they are silent and don't
explicitly tell the user what's happened― whereas messages like
IM_ROOM_PARTICIPANT_JOINED and IM_ROOM_SUBJECT_SET will inform the user of the
change.

You can send IM_ROOM_PARTICIPANTS multiple times in a row― users
not mentioned in subsequent mentions are not implicitly removed from the
user-list, you must send a separate IM_ROOM_PARTICIPANT_LEFT for each parting
user.
