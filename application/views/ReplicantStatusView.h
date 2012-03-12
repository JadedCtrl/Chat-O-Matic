/*
 * Copyright 2011, Dario Casalinuovo. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Dario Casalinuovo
 */
#ifndef _REPLICANT_STATUS_VIEW_H
#define _REPLICANT_STATUS_VIEW_H

#include <Handler.h>
#include <Messenger.h>
#include <Resources.h>
#include <View.h>

#include "CayaConstants.h"
#include "CayaResources.h"

class BPopUpMenu;
class BMenuField;

class BitmapView;
class NicknameTextControl;

class ReplicantHandler;

class ReplicantStatusView : public BView {
public:
							ReplicantStatusView();
							ReplicantStatusView(BMessage* archive);
							~ReplicantStatusView();

	virtual	void			MessageReceived(BMessage* msg);
	virtual void			AttachedToWindow();
	virtual void			DetachedFromWindow();

	virtual void			Draw(BRect rect);

			void			SetStatus(CayaStatus status);

	virtual status_t		Archive(BMessage* archive, bool deep) const;

	static	ReplicantStatusView* Instantiate(BMessage* archive);

			void			MouseDown(BPoint point);


	static status_t			InstallReplicant();
	static status_t			RemoveReplicant();
private:
			void			_Init();
			void			_BuildMenu();
			void			_ShowMenu(BPoint point);
			BBitmap*		_GetIcon(const uint32 id);

			BBitmap*		fIcon;

			BBitmap*		fConnectingIcon;
			BBitmap*		fCayaIcon;
			BBitmap*		fOfflineIcon;
			BBitmap*		fBusyIcon;
			BBitmap*		fAwayIcon;
			BBitmap*		fExitMenuIcon;
			BBitmap*		fPreferencesIcon;

			BResources*		fResources;

			BPopUpMenu*		fReplicantMenu;

			bool			fIsInstalled;
			BMessenger*		fCayaMsg;

			ReplicantHandler* fReplicantHandler;
};

#endif
