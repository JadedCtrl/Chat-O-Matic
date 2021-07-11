/*
 * Copyright 2021, Jaidyn Levesque <jadedctrl@teknik.io>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef _PURPLE_DIALOG_H
#define _PURPLE_DIALOG_H

#include <libpurple/purple.h>

#include <ObjectList.h>
#include <Window.h>


const uint32 ACTION_BUTTON = 'PDab';


struct RequestAction
{
	BString name;
	PurpleRequestType type;
	int32 index;

	union {
		PurpleRequestActionCb action;
	} callback;
};


class PurpleDialog : public BWindow {
public:
					// PURPLE_REQUEST_ACTION
					PurpleDialog(const char* title, const char* primary,
						const char* secondary, PurpleAccount* account,
						va_list actions, size_t action_count, void* user_data);

	virtual	void	MessageReceived(BMessage* msg);

private:
			void	_InitActionInterface(const char* label, const char* desc);
			void	_ParseActions(va_list actions, int32 count,
						PurpleRequestType type);

	BObjectList<RequestAction> fActions;
	void* fUserData;
};

#endif // _PURPLE_DIALOG_H
