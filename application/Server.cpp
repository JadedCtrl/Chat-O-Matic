/*
 * Copyright 2009, Andrea Anzani. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Andrea Anzani, andrea.anzani@gmail.com
 *		Pier Luigi Fiorini, pierluigi.fiorini@gmail.com
 */

#include <Application.h>
#include <Entry.h>
#include <Path.h>
#include <TranslationUtils.h>

#include "AccountManager.h"
#include "ImageCache.h"
#include "LooperCayaProtocol.h"
#include "ProtocolManager.h"
#include "Server.h"
#include "MainWindow.h"
#include "RosterItem.h"
#include "ChatWindow.h"
#include <Debug.h>
#include "Account.h"

Server::Server(MainWindow* mainWindow)
	: BMessageFilter(B_ANY_DELIVERY, B_ANY_SOURCE)
{
	CayaProtocol* pp = ProtocolManager::Get()->GetProtocol("gtalk");
	if (!pp)
		debugger("something wrong");

	//FIXME: here just a first draft of the final design:
	
	Account*	gtalkAccount = new Account(mainWindow);		

	pp->Init(gtalkAccount);

	fMainWindow = mainWindow;
	fProtocol = new LooperCayaProtocol(pp);
}


void 
Server::Quit()
{
	ContactLinker* linker = NULL;	
	while ((linker = fRosterMap.ValueAt(0))) {
		linker->DeleteWindow();
		linker->DeletePopUp();
		fRosterMap.RemoveItemAt(0);
	}
}


void
Server::Login()
{
	BMessage* msg = new BMessage(IM_MESSAGE);
	msg->AddInt32("im_what", IM_SET_STATUS);
	msg->AddInt32("status", CAYA_ONLINE);
	fProtocol->PostMessage(msg);
}


void
Server::SendProtocolMessage(BMessage* msg)
{
	if (msg != NULL)
		fProtocol->PostMessage(msg);
}


void
Server::UpdateSettings(BMessage settings)
{
	fProtocol->Protocol()->UpdateSettings(settings);
}


filter_result
Server::Filter(BMessage* message, BHandler **target)
{
	filter_result result = B_DISPATCH_MESSAGE;

	switch(message->what) {
		case IM_SERVER_BASED_CONTACT_LIST:
		{
			int i = 0;
			BString id;
			while (message->FindString("id", i++, &id) == B_OK) {
				bool found = false;
				ContactLinker* item = fRosterMap.ValueFor(id, &found);

				if (!found) {
					item = new ContactLinker(id.String(), Looper());
					fRosterMap.AddItem(id, item);
				}
			}
			result = B_SKIP_MESSAGE;
			break;
		}
		case OPEN_WINDOW:
		{
			int index = message->FindInt32("index");
			RosterItem* ritem = fMainWindow->ItemAt(index);
			if (ritem != NULL)
				ritem->GetContactLinker()->ShowWindow();
			result = B_SKIP_MESSAGE;			
			break;
		}
		case CLOSE_WINDOW:
		{
			BString id = message->FindString("id");
			if (id != "") {
				bool found = false;
				ContactLinker *item = fRosterMap.ValueFor(id, &found);

				if (found)
					item->HideWindow();	
			}
			result = B_SKIP_MESSAGE;
			break;
		}
		case IM_MESSAGE:
			result = ImMessage(message);
			break;
	}

	return result;	
}


RosterMap
Server::RosterItems() const
{
	return fRosterMap;
}


RosterItem*		
Server::RosterItemForId(BString id)
{
	bool found = false;
	ContactLinker* item = fRosterMap.ValueFor(id, &found);
	return item ? item->GetRosterItem() : NULL;			
}


filter_result
Server::ImMessage(BMessage* msg)
{	
	filter_result result = B_DISPATCH_MESSAGE;
	int32 im_what = msg->FindInt32("im_what");

	switch (im_what) {
		case IM_STATUS_SET:
		{
			int32 status;
			const char* protocol;

			if (msg->FindInt32("status", &status) != B_OK)
				return B_SKIP_MESSAGE;
			if (msg->FindString("protocol", &protocol) != B_OK)
				return B_SKIP_MESSAGE;

			AccountManager* accountManager = AccountManager::Get();
			accountManager->SetStatus((CayaStatus)status);
			break;
		}
		case IM_STATUS_CHANGED:
		{
			int32 status;

			if (msg->FindInt32("status", &status) != B_OK)
				return B_SKIP_MESSAGE;

			ContactLinker* linker = EnsureContactLinker(msg->FindString("id"));
			linker->SetNotifyStatus((CayaStatus)status);
			linker->SetNotifyPersonalStatus(msg->FindString("message"));
			break;
		}
		case IM_CONTACT_INFO:		
		{
			ContactLinker* linker = EnsureContactLinker(msg->FindString("id"));
			BString fullName = msg->FindString("nick");
			if (fullName != "")
				linker->SetNotifyName(fullName);	
			break;
		}
		case IM_AVATAR_CHANGED:
		{
			ContactLinker* linker = EnsureContactLinker(msg->FindString("id"));
			entry_ref ref;
			if (linker) {
				if (msg->FindRef("ref", &ref) == B_OK) {
					//BPath fullPath(&ref);
					//BBitmap *bitmap = ImageCache::GetImage(BString(fullPath.Path()), BString(fullPath.Path()));										
					BBitmap *bitmap = BTranslationUtils::GetBitmap(&ref);
					linker->SetNotifyAvatarBitmap(bitmap);
				} else
					linker->SetNotifyAvatarBitmap(NULL);
			}
			break;
		}
		case IM_SEND_MESSAGE:
			fProtocol->PostMessage(msg);
			break;
		case IM_MESSAGE_RECEIVED:
		{
			BString id = msg->FindString("id");
			if (id != "") {
				bool found = false;
				ContactLinker* item = fRosterMap.ValueFor(id, &found);
				if (found) {
					ChatWindow* win = item->GetChatWindow();
					item->ShowWindow();
					win->PostMessage(msg);
				}
			}
			result = B_SKIP_MESSAGE;
			break;
		}
		default:
			msg->PrintToStream();
			break;
	}

	return result;
}


ContactLinker*	
Server::EnsureContactLinker(BString id)
{
	ContactLinker* item = NULL;
	if (id != "") {
		bool found = false;
		item = fRosterMap.ValueFor(id, &found);

		if (!found) {
			item = new ContactLinker(id.String(), Looper());
			fRosterMap.AddItem(id, item);
		}
	}

	return item;
}

ContactLinker*	
Server::GetOwnContact()
{
	return fMySelf;
}
