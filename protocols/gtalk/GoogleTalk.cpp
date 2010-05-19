#include <stdio.h>
#include <string>

#include <Entry.h>

#include <libjabber/JabberSSLPlug.h>
#include <libjabber/States.h>

#include "CayaProtocolMessages.h"
#include "GoogleTalk.h"

const char* kProtocolSignature = "gtalk";
const char* kProtocolName = "Google Talk";

int64 idsms = 0;


GoogleTalk::GoogleTalk()
	: JabberHandler("jabberHandler", fPlug = new JabberSSLPlug("talk.google.com", 5223)),
	fUsername(""),
	fServer("gmail.com"),
	fPassword("") 
{
}


GoogleTalk::~GoogleTalk()
{
	Shutdown();
}


status_t
GoogleTalk::Init(CayaProtocolMessengerInterface* msgr)
{
	fServerMsgr = msgr;
	fRostered = false;
	fAgent = false;
	fFullLogged = false;
	fPerc = 0.0;
	fLaterBuddyList = new StrList();

	return B_OK;
}


status_t
GoogleTalk::Shutdown()
{
	LogOff();

	fLaterBuddyList->clear();
	delete fLaterBuddyList;

	thread_id plug = fPlug->Thread();
	BMessenger(fPlug).SendMessage(B_QUIT_REQUESTED);
	fPlug = NULL;

	int32 res = 0;
	wait_for_thread(plug, &res);

	return B_OK;
}


status_t
GoogleTalk::Process(BMessage* msg)
{
	switch (msg->what) {
		case IM_MESSAGE:
		{
			int32 im_what = 0;

			msg->FindInt32("im_what", &im_what);

			switch (im_what) {
				case IM_SET_NICKNAME:
				{
					BString nick;

					if (msg->FindString("nick", &nick) == B_OK)
						SetOwnNickname(nick);
					break;
				}
				case IM_SET_OWN_STATUS:
				{
					int32 status = msg->FindInt32("status");
					BString status_msg("");
					msg->FindString("message", &status_msg);
					
					switch (status) {
						case CAYA_ONLINE:
							if (!IsAuthorized()) {
								if (fServer == "")
									Error("Empty Server!", NULL);
								if (fUsername == "")
									Error("Empty Username!", NULL);
								if (fPassword == "")
									Error("Empty Password!",NULL);

								Progress("GoogleTalk Login", "GoogleTalk: Connecting...", 0.0f);
								SetStatus(S_ONLINE, "");
								RequestVCard(GetJid()); //by default we ask for our own vCard.
							} else {
								SetStatus(S_ONLINE, "");
								SetAway(false);
							}
							break;
						case CAYA_AWAY:
							if (IsAuthorized()) {
						 		SetStatus(S_AWAY, status_msg);
								SetAway(true);
							}
							break;
						case CAYA_EXTENDED_AWAY:
							if (IsAuthorized()) {
								SetStatus(S_XA, status_msg);
								SetAway(true);
							}
							break;
						case CAYA_DO_NOT_DISTURB:
							if (IsAuthorized()) {
								SetStatus(S_DND, status_msg);
							}
							break;
						case CAYA_OFFLINE:
							SetStatus(S_OFFLINE, "");
							break;
						default:
							Error("Invalid", NULL);
							break;
					}
					break;
				}
				case IM_SEND_MESSAGE:
				{
					const char* buddy = msg->FindString("id");
					const char* sms = msg->FindString("message");

					JabberMessage jm;
					jm.SetTo(buddy);
					jm.SetFrom(GetJid());
					jm.SetBody(sms);
					TimeStamp(jm);

					// Not the right place.. see Jabber::Message
					JabberContact* contact = getContact(buddy);

					//tmp: new mess id!
					BString messid("caya");
					messid << idsms;
					idsms++;

					if (contact)
						jm.SetID(messid);

					SendMessage(jm);
					MessageSent(buddy,sms);
					break;
				}
				case IM_REGISTER_CONTACTS:
				{
					type_code garbage;
					int32 count = 0;
					msg->GetInfo("id", &garbage, &count);

					if (count > 0) {
						for (int i = 0; msg->FindString("id", i); i++) {
							const char* id = msg->FindString("id", i);
							JabberContact* contact = getContact(id);
							if (contact)
								  BuddyStatusChanged(contact);
							else {
								// Are we on-line?
								// send auth req?
								if (fFullLogged) { 			
									AddContact(id, id, "");
									BuddyStatusChanged(id, CAYA_OFFLINE);
								} else {
									// we add to a temp list.
									// when logged in we will register the new buddy...
									fLaterBuddyList->push_back(BString(id));
								}
							}
						}
					} else
						return B_ERROR;
					break;
				}
				case IM_UNREGISTER_CONTACTS:
				{
					const char* buddy = NULL;

					for (int i = 0; msg->FindString("id", i, &buddy) == B_OK; i++) {
						//LOG(kProtocolSignature, liDebug, "Unregister Contact: '%s'", buddy);

						if (!fFullLogged)
							BuddyStatusChanged(buddy, CAYA_OFFLINE);
						else {
							//LOG(kProtocolSignature, liDebug, "Unregister Contact DOING IT");
						 	JabberContact* contact = getContact(buddy);
						 	if (contact)
								RemoveContact(contact);
						}
					}
					break;
				}
				case IM_USER_STARTED_TYPING:
				{
					const char* id = NULL;

					if (msg->FindString("id", &id) == B_OK) {
						JabberContact* contact=getContact(id);
						if (contact)
							StartComposingMessage(contact);
					}
					break;
				}
				case IM_USER_STOPPED_TYPING:
				{
					const char* id = NULL;

					if (msg->FindString("id", &id) == B_OK) {
						JabberContact* contact = getContact(id);
						if (contact && (contact->GetLastMessageID().ICompare("") != 0)) {
							StopComposingMessage(contact);
							contact->SetLastMessageID("");
						}
					}
					break;
				}
				case IM_GET_CONTACT_INFO:
					SendContactInfo(msg->FindString("id"));
					break;
				case IM_ASK_AUTHORIZATION:
				{
					if (!IsAuthorized())
						return B_ERROR;

					const char* id = msg->FindString("id");
					int32 button = msg->FindInt32("which");

					if (button == 0) {
						// Authorization granted
						AcceptSubscription(id);
						BMessage im_msg(IM_MESSAGE);
						im_msg.AddInt32("im_what", IM_CONTACT_AUTHORIZED);
						im_msg.AddString("protocol", kProtocolSignature);
						im_msg.AddString("id", id);
						im_msg.AddString("message", "");
						fServerMsgr->SendMessage(&im_msg);

						// Now we want to see you! ;)
						AddContact(id, id, "");														
					} else {
						// Authorization rejected
						Error("Authorization rejected!",id);
					}
					break;
				}
				case IM_SPECIAL_TO_PROTOCOL:
					Send(msg->FindString("direct_data"));		
					break;
				default:
					// We don't handle this im_what code
					//LOG(kProtocolSignature, liDebug, "Got unhandled message: %ld", im_what);
					msg->PrintToStream();
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
GoogleTalk::Signature() const
{
	return kProtocolSignature;
}


const char*
GoogleTalk::FriendlySignature() const
{
	return kProtocolName;
}


status_t
GoogleTalk::UpdateSettings(BMessage* msg)
{
	const char* username = NULL;
	const char* password = NULL;
	const char* res = NULL;

	msg->FindString("username", &username);
	msg->FindString("password", &password);
	msg->FindString("resource", &res);

	if ((username == NULL) || (password == NULL)) {
		//LOG( kProtocolSignature, liHigh, "Invalid settings!");
		printf("Invalid settings");
		return B_ERROR;
	}

	fUsername = username;
	int32 atpos=fUsername.FindLast("@");
	if (atpos> 0) {
		BString server;
		fUsername.CopyInto(server,atpos + 1,fUsername.Length()-atpos);
		fUsername.Remove(atpos,fUsername.Length()-atpos);
		fServer = server;		
	} else
		fServer.SetTo("gmail.com");

	fPassword = password;
	
	SetUsername(fUsername);
	SetHost(fServer);
	SetPassword(fPassword);

	if (strlen(res)==0)
		SetResource("caya");
	else
		SetResource(res);

	SetPriority(5);
	SetPort(5223);

	return B_OK;
}


uint32
GoogleTalk::GetEncoding()
{
	return 0xffff; // No conversion, GoogleTalk handles UTF-8 ???
}


// JabberManager stuff


void
GoogleTalk::Error(const char* message, const char* who)
{
	//LOG("GoogleTalk", liDebug, "GoogleTalk::Error(%s,%s)", message, who);

	BMessage msg(IM_ERROR);
	msg.AddString("protocol", kProtocolSignature);
	if (who)
		msg.AddString("id", who);
	msg.AddString("error", message);

	fServerMsgr->SendMessage( &msg );
}


void
GoogleTalk::GotMessage(const char* from, const char* message)
{
	//LOG("GoogleTalk", liDebug, "GoogleTalk::GotMessage()");

	BMessage msg(IM_MESSAGE);
	msg.AddInt32("im_what", IM_MESSAGE_RECEIVED);
	msg.AddString("protocol", kProtocolSignature);
	msg.AddString("id", from);
	msg.AddString("message", message);

	fServerMsgr->SendMessage( &msg );
}


void
GoogleTalk::MessageSent(const char* to, const char* message)
{
	//LOG("GoogleTalk", liDebug, "GoogleTalk::GotMessage()");

	BMessage msg(IM_MESSAGE);
	msg.AddInt32("im_what", IM_MESSAGE_SENT);
	msg.AddString("protocol", kProtocolSignature);
	msg.AddString("id", to);
	msg.AddString("message", message);

	fServerMsgr->SendMessage( &msg );
}


void
GoogleTalk::LoggedIn()
{
	Progress("GoogleTalk Login", "GoogleTalk: Logged in!", 1.00);

	BMessage msg(IM_MESSAGE);
	msg.AddInt32("im_what", IM_STATUS_SET);
	msg.AddString("protocol", kProtocolSignature);
	msg.AddInt32("status", CAYA_ONLINE);

	fServerMsgr->SendMessage(&msg);

	fFullLogged = true;



	while (fLaterBuddyList->size() != 0) {
		BString id = *(fLaterBuddyList->begin());
		fLaterBuddyList->pop_front();	// removes first item
		JabberContact* contact=getContact(id.String());
		if (!contact) {
			AddContact(id.String(),id.String(),"");
			BuddyStatusChanged(id.String(), CAYA_OFFLINE);
		}	
	}

}


void
GoogleTalk::SetAway(bool away)
{
	//LOG("GoogleTalk", liDebug, "GoogleTalk::SetAway()");

	BMessage msg(IM_MESSAGE);
	msg.AddInt32("im_what", IM_STATUS_SET);
	msg.AddString("protocol", kProtocolSignature);
	if ( away )
		msg.AddInt32("status", CAYA_AWAY);
	else
		msg.AddInt32("status", CAYA_ONLINE);

	fServerMsgr->SendMessage( &msg );
}


void
GoogleTalk::LoggedOut()
{
	//LOG("GoogleTalk", liDebug, "GoogleTalk::LoggedOut()");

	BMessage msg(IM_MESSAGE);
	msg.AddInt32("im_what", IM_STATUS_SET);
	msg.AddString("protocol", kProtocolSignature);
	msg.AddInt32("status", CAYA_OFFLINE);
	fServerMsgr->SendMessage(&msg);
	fFullLogged = false;
	fRostered = false;
	fAgent = false;
	fPerc = 0.0;
}


void
GoogleTalk::BuddyStatusChanged(JabberContact* who)
{
	BuddyStatusChanged(who->GetPresence());
}


void
GoogleTalk::BuddyStatusChanged(JabberPresence* jp)
{
	//LOG("GoogleTalk", liDebug, "GoogleTalk::BuddyStatusChanged(%s)",jp->GetJid().String());

	//avoid a receiving self status changes or empty status:
	if (jp->GetJid() == "" || jp->GetJid().ICompare(GetJid()) == 0)
		return;

	BMessage msg(IM_MESSAGE);
	msg.AddInt32("im_what", IM_STATUS_SET);
	msg.AddString("protocol", kProtocolSignature);
	msg.AddString("id", jp->GetJid());
	msg.AddString("resource", jp->GetResource());

	AddStatusString(jp, &msg);
	fServerMsgr->SendMessage(&msg);
}


void
GoogleTalk::AddStatusString(JabberPresence* jp, BMessage* msg)
{
	int32 show = jp->GetShow();
	switch (show) {
		case S_XA:
			msg->AddInt32("status", CAYA_EXTENDED_AWAY);
			break;
		case S_AWAY:
			msg->AddInt32("status", CAYA_AWAY);
			break;
		case S_ONLINE:
			msg->AddInt32("status", CAYA_ONLINE);
			break;
		case S_DND:
			msg->AddInt32("status", CAYA_DO_NOT_DISTURB);
			break;
		case S_CHAT:
			msg->AddInt32("status", CAYA_ONLINE);
			break;
		case S_SEND:
			msg->AddInt32("status", CAYA_ONLINE);
			break;
		default:
			msg->AddInt32("status", CAYA_OFFLINE);
			break;
	}

	if (jp->GetType().ICompare("unavailable") == 0)
			msg->AddInt32("status", CAYA_OFFLINE);

	msg->AddString("message", jp->GetStatus());
}


void
GoogleTalk::BuddyStatusChanged(const char* who, CayaStatus status)
{
	//LOG("GoogleTalk", liDebug, "GoogleTalk::BuddyStatusChanged(%s,%s)",who,status);

	BMessage msg(IM_MESSAGE);
	msg.AddInt32("im_what", IM_STATUS_SET);
	msg.AddString("protocol", kProtocolSignature);
	msg.AddString("id", who);
	msg.AddInt32("status", status);

	fServerMsgr->SendMessage( &msg );
}


void
GoogleTalk::Progress(const char* id, const char* message, float progress)
{
	BMessage msg(IM_MESSAGE);
	msg.AddInt32("im_what", IM_PROGRESS );
	msg.AddString("protocol", kProtocolSignature);
	msg.AddString("progressID", id);
	msg.AddString("message", message);
	msg.AddFloat("progress", progress);
	msg.AddInt32("state", 11); //IM_impsConnecting );

	fServerMsgr->SendMessage(&msg);
}


JabberContact*
GoogleTalk::getContact(const char* id)
{
	RosterList *rl = getRosterList();
	JabberContact* contact = NULL;
	//LOG(kProtocolSignature, liDebug, "getContact %s", id);

	for(int32 i = 0; i < rl->CountItems(); i++) {
		contact = reinterpret_cast<JabberContact*>(getRosterList()->ItemAt(i));
		//LOG(kProtocolSignature, liDebug, "getContact [%3d] GetJID %s", i,contact->GetJid().String());

		if (contact->GetJid().ICompare(id) == 0) {
			//LOG(kProtocolSignature, liDebug, "getContact found!");
			return contact;								
		}								
	}

	return NULL;
}

void			
GoogleTalk::SendContactInfo(const JabberContact* jid)
{
	int32 what = IM_CONTACT_INFO;
	BMessage msg(IM_MESSAGE);
	msg.AddInt32("im_what", what);
	msg.AddString("protocol", kProtocolSignature);
	msg.AddString("id", jid->GetJid());
	msg.AddString("nick", jid->GetName());

	// vCard information
	JabberVCard* vCard = jid->GetVCard();
	if (vCard) {
		msg.AddString("full name", vCard->GetFullName());
		msg.AddString("first name", vCard->GetGivenName());
		msg.AddString("middle name", vCard->GetMiddleName());
		msg.AddString("last name", vCard->GetFamilyName());
		msg.AddString("email", vCard->GetEmail());
		msg.AddString("birthday", vCard->GetBirthday());
		msg.AddString("url", vCard->GetURL());

		entry_ref ref;	
		if (get_ref_for_path(vCard->GetCachedPhotoFile().String(), &ref) == B_OK)
			msg.AddRef("ref", &ref);
	}

	// Send contact information
	fServerMsgr->SendMessage(&msg);
}

void			
GoogleTalk::SendContactInfo(const char* id)
{
	JabberContact* jid = getContact(id);
	if (!jid)
		return;
	
	SendContactInfo(jid);
}


void
GoogleTalk::SendBuddyIcon(const char* id)
{
	JabberContact* jid = getContact(id);
	if (!jid)
		return;

	// vCard information
	JabberVCard* vCard = jid->GetVCard();
	if (vCard) {
		BString data = vCard->GetPhotoContent();

		BMessage msg(IM_MESSAGE);
		msg.AddInt32("im_what", IM_SET_OWN_AVATAR);
		msg.AddString("protocol", kProtocolSignature);
		msg.AddString("id", id);
		msg.AddData("icondata", B_RAW_TYPE, data.String(), data.Length());
		fServerMsgr->SendMessage(&msg);
	}
}


// Callbacks

void
GoogleTalk::Authorized()
{
	SetAway(false);

	fPerc +=0.3333f;

	Progress("GoogleTalk Login", "GoogleTalk: Authorized", fPerc);
	//LOG(kProtocolSignature, liDebug, "GoogleTalk:Login %f - Authorized",fPerc) ;
	CheckLoginStatus();

	JabberHandler::Authorized();
}


void
GoogleTalk::Message(JabberMessage* message)
{
	// We have something to tell
	if (message->GetBody() != "")
		GotMessage(message->GetFrom().String(), message->GetBody().String());

	// Not a nice situation..
	if(message->GetError() != "") {
		Error(message->GetError().String(),message->GetFrom().String());
		return;
	}

	//LOG(kProtocolSignature, liHigh, "GETX: '%s'",message->GetX().String()) ;

	if (message->GetX().ICompare("composing") == 0) {
		// Someone send a composing event...
		if (message->GetBody() == "") {
			 //LOG(kProtocolSignature, liHigh,"CONTACT_STARTED_TYPING");
			 BMessage im_msg(IM_MESSAGE);
			 im_msg.AddInt32("im_what", IM_CONTACT_STARTED_TYPING);
			 im_msg.AddString("protocol", kProtocolSignature);
			 im_msg.AddString("id", message->GetFrom());
			 fServerMsgr->SendMessage(&im_msg);	
			} else {
				//	where we put the last messge id? on the contact (is it the right place?)
				//	maybe we should make an hash table? a BMesage..
				JabberContact* contact = getContact(message->GetFrom().String());
				if(contact)
					contact->SetLastMessageID(message->GetID());
			}
		} else if (message->GetX().ICompare("jabber:x:event") == 0) {
			//not define event this maybe due to:
			// 	unkown event.
			// 	no event (means stop all)

			//LOG(kProtocolSignature, liHigh,"CONTACT_STOPPED_TYPING");

			BMessage im_msg(IM_MESSAGE);
			im_msg.AddInt32("im_what", IM_CONTACT_STOPPED_TYPING);
			im_msg.AddString("protocol", kProtocolSignature);
			im_msg.AddString("id", message->GetFrom());
			fServerMsgr->SendMessage(&im_msg);
		}
}


void
GoogleTalk::Presence(JabberPresence* presence)
{
	BuddyStatusChanged(presence);
}


void
GoogleTalk::Roster(RosterList* roster)
{
	// Fix me! (Roster message can arrive at different times)
	BMessage serverBased(IM_MESSAGE);
	serverBased.AddInt32("im_what", IM_CONTACT_LIST);
	serverBased.AddString("protocol", kProtocolSignature);
	JabberContact* contact;
	int size = roster->CountItems();
	
	for(int32 i = 0; i < size; i++) {
		contact = reinterpret_cast<JabberContact*>(roster->ItemAt(i));
		serverBased.AddString("id", contact->GetJid());
	}

	fServerMsgr->SendMessage(&serverBased);

	for (int32 i=0; i < size; i++) {
		contact = reinterpret_cast<JabberContact*>(roster->ItemAt(i));
		SendContactInfo(contact);
	}

	// Here the case when more than one roster message has arrived!
	if(!fRostered) {
		fPerc += 0.3333f;
		fRostered = true;
		Progress("GoogleTalk Login", "GoogleTalk: Roster", fPerc);
	}

	//LOG(kProtocolSignature, liDebug, "GoogleTalk:Login %f - Rostered",fPerc) ;
	CheckLoginStatus();	
}


void
GoogleTalk::Agents(AgentList* agents)
{
	fPerc +=0.3333f;
	fAgent = true;
	Progress("GoogleTalk Login", "GoogleTalk: Agents", fPerc);
	//LOG(kProtocolSignature, liDebug, "GoogleTalk:Login %f - Agents",fPerc) ;
	CheckLoginStatus();
}


void
GoogleTalk::Disconnected(const BString& reason)
{
	LoggedOut();
	
	if (reason == "")
		return;

	Error(reason.String(),NULL);
}


void
GoogleTalk::SubscriptionRequest(JabberPresence* presence)
{
	BMessage im_msg(IM_MESSAGE);
	im_msg.AddInt32("im_what", IM_AUTHORIZATION_REQUEST);
	im_msg.AddString("protocol", kProtocolSignature);
	im_msg.AddString("id", presence->GetJid());
	im_msg.AddString("message", presence->GetStatus());

	fServerMsgr->SendMessage(&im_msg);	
}


void
GoogleTalk::Unsubscribe(JabberPresence* presence)
{	
	// What should we do when a people unsubscrive from us?
	//debugger("Unsubscribe");
	//LOG("GoogleTalk", liDebug, "GoogleTalk::Unsubscribe()");

	BMessage msg(IM_MESSAGE);
	msg.AddInt32("im_what", IM_STATUS_SET);
	msg.AddString("protocol", kProtocolSignature);
	msg.AddString("id", presence->GetJid());
	msg.AddInt32("status", CAYA_OFFLINE);
	fServerMsgr->SendMessage(&msg);
}


void
GoogleTalk::OwnContactInfo(JabberContact* contact)
{
	int32 what = IM_OWN_CONTACT_INFO;

	BMessage msg(IM_MESSAGE);
	msg.AddInt32("im_what", what);
	msg.AddString("protocol", kProtocolSignature);
	msg.AddString("id", contact->GetJid());
	msg.AddString("nick", contact->GetName());

	// vCard information
	JabberVCard* vCard = contact->GetVCard();
	if (vCard) {
		msg.AddString("full name", vCard->GetFullName());
		msg.AddString("first name", vCard->GetGivenName());
		msg.AddString("middle name", vCard->GetMiddleName());
		msg.AddString("last name", vCard->GetFamilyName());
		msg.AddString("email", vCard->GetEmail());
		msg.AddString("birthday", vCard->GetBirthday());
		msg.AddString("url", vCard->GetURL());

		entry_ref ref;	
		if (get_ref_for_path(vCard->GetCachedPhotoFile().String(), &ref) == B_OK)
			msg.AddRef("ref", &ref);
	}

	// Send information
	fServerMsgr->SendMessage(&msg);
}


void
GoogleTalk::GotBuddyPhoto(const BString& jid, const BString& imagePath)
{
	BMessage msg(IM_MESSAGE);

	msg.AddInt32("im_what", IM_AVATAR_SET);
	msg.AddString("protocol", kProtocolSignature);
	msg.AddString("id", jid);

	entry_ref ref;	
	if (get_ref_for_path(imagePath.String(), &ref) == B_OK)
		msg.AddRef("ref", &ref);

	fServerMsgr->SendMessage(&msg);
}


void
GoogleTalk::Registration(JabberRegistration* registration)
{
	// Just created a new account ?
	// or we have ack of a registration? ack of registartion!
	registration->PrintToStream();
	debugger("Registration");
}


void
GoogleTalk::CheckLoginStatus()
{
	if (fRostered &&  fAgent && !fFullLogged) 
		LoggedIn();	
}
