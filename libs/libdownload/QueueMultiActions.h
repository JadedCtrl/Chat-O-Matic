/*
 * Copyright 2010, Andrea Anzani. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef  QueueMultiActions_H_
#define QueueMultiActions_H_

#include <Looper.h>
#include <ObjectList.h>
#include <OS.h>


#include "Action.h"

#define MAX_MULTI		5

class QueueMultiActions
{

public:

	QueueMultiActions(const char* name, int count);
	virtual ~QueueMultiActions();

	void		AddAction(Action* );

	//BEFORE CALLING CurrentAction, you MUST Lock() the Queue!
	Action*	CurrentAction(int index);

	int32		CountActions();
	int			CountThreads() {
		return fCount;
	}

	Action*	ActionAt(int32 pos);
	void	RemoveActionAt(int32 pos);


	void	SetDownloadCount(int count);


	bool	Lock();
	void	Unlock();
	bool	IsLocked();

protected:

	static	int32	ManageTheQueue(void*);

	virtual	void	ActionReadyToPerform(Action*) = 0;
	virtual	void	ActionPerformed(Action*, status_t, BMessage*) = 0;
	virtual	void	SuppressAction(Action*) = 0;


private:
	struct token {
		int								index;
		QueueMultiActions*		qa;
	};


	void		SetCurrentAction(Action*, int index);
	void		KIllThread(int id);

	Action*						fCurrentAction[MAX_MULTI];
	thread_id 					fID[MAX_MULTI];
	sem_id						fLock;
	BObjectList<Action>	fList;
	bool							fLocked;
	int								fCount;
	BString						fName;
};
#endif
