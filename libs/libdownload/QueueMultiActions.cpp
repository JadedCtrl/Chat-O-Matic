/*
 * Copyright 2010, Andrea Anzani. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Andrea Anzani, andrea.anzani@gmail.com
 */

#include "QueueMultiActions.h"

#include <Message.h>
#include <stdio.h>


#define CHKLOCK if(!IsLocked()) debugger("\nQueueMultiActions must me locked!\n");

QueueMultiActions::QueueMultiActions(const char* name, int count)
{

	fCount = count;
	if (count > MAX_MULTI)
		debugger("wrong number of multi download settings");

	for (int j = 0; j < MAX_MULTI; j++) {
		fCurrentAction[j] = NULL;
		fID[j] = 0;
	}

	fName.SetTo(name);

	for (int i = 0; i < fCount; i++) {
		BString n(name);
		n << "'s enemy #" << i;
		token* tok = new token;
		tok->qa = this;
		tok->index = i;
		fID[i] = spawn_thread(QueueMultiActions::ManageTheQueue, n.String(), B_NORMAL_PRIORITY, tok);
		//printf("Thread ready [%s] %ld\n",n.String(),fID[i]);

	}



	fLock = create_sem(1, name);
	fLocked = false;

}

QueueMultiActions::~QueueMultiActions()
{
	if (Lock()) {

		for (int i = 0; i < fCount; i++) {
			suspend_thread(fID[i]);
			exit_thread(fID[i]);
		}
		delete_sem(fLock);
	}
}


bool
QueueMultiActions::Lock()
{
	status_t value = acquire_sem(fLock) ;
	if (value == B_NO_ERROR) {
		fLocked = true;
	}
	return (value == B_NO_ERROR);
}


void
QueueMultiActions::Unlock()
{
	fLocked = false;
	release_sem(fLock);
}


void
QueueMultiActions::SetDownloadCount(int fProposedCount)
{

	if (Lock()) {


		if (fProposedCount == fCount || fProposedCount <= 0 || fProposedCount > MAX_MULTI) {
			Unlock();
			return;
		}

		if (fProposedCount > fCount) {

			for (int i = fCount; i < fProposedCount; i++) {
				if (	fID[i] == 0) {
					BString n = fName;
					n << "'s enemy #" << i;
					token* tok = new token;
					tok->qa = this;
					tok->index = i;
					fID[i] = spawn_thread(QueueMultiActions::ManageTheQueue, n.String(), B_NORMAL_PRIORITY, tok);
					//printf("Thread ready [%s] %ld\n",n.String(),fID[i]);
				}
			}
		} else {
			//uhm what to do?
			for (int i = fProposedCount; i < fCount; i++) {
				SuppressAction(fCurrentAction[i]);
			}

		}

		fCount = fProposedCount;
		//NotifyALL

		for (int i = 0; i < fCount; i++)
			resume_thread(fID[i]);

		Unlock();
	}
}


void
QueueMultiActions::AddAction(Action* a)
{

	//FIX!
	//esiste già un azione con lo stesso ID???

	if (Lock()) {
		//printf("adding %ld - Action name %s\n",fID,a->GetDescription().String());
		fList.AddItem(a);
		if (fList.CountItems() <= fCount) { //Auto-start thread
			for (int i = 0; i < fCount; i++)
				resume_thread(fID[i]);
		}
		Unlock();
	}
}


Action*
QueueMultiActions::CurrentAction(int index)
{
	CHKLOCK;
	return fCurrentAction[index];
}


void
QueueMultiActions::SetCurrentAction(Action* action, int index)
{
	CHKLOCK;
	fCurrentAction[index] = action;
}


int32
QueueMultiActions::CountActions()
{
	CHKLOCK;
	return fList.CountItems();
}


Action*
QueueMultiActions::ActionAt(int32 pos)
{
	CHKLOCK;
	return fList.ItemAt(pos);
}


void
QueueMultiActions::RemoveActionAt(int32 pos)
{
	CHKLOCK;
	fList.RemoveItemAt(pos);
}


bool
QueueMultiActions::IsLocked()
{
	return fLocked;
}


void
QueueMultiActions::KIllThread(int id)
{
	fID[id] = 0;
}


int32
QueueMultiActions::ManageTheQueue(void* data)
{

	token*	tok  = (token*)data;
	QueueMultiActions* qa = tok->qa;
	int index = tok->index;

	//printf("Thread started %ld\n",qa->fID[index]);

	while (true) {

		Action* last = NULL;

		if (qa->Lock()) {

			//printf("Thread executing PID %ld Count %ld\n",qa->fID[index],qa->fList.CountItems());
			if (qa->fList.CountItems() > 0) {
				// remove and delete the action.
				last = qa->fList.ItemAt(0);
				qa->fList.RemoveItemAt(0);
				qa->SetCurrentAction(last, index);

			} else {

				last = NULL;
				qa->SetCurrentAction(last, index);
			}

			qa->Unlock();
		}

		if (last) {
			// pop the action
			qa->ActionReadyToPerform(last);
			// perform the action
			BMessage err;
			status_t status = last->Perform(&err);
			// do post-perform!

			qa->ActionPerformed(last, status, &err);

			if (qa->Lock()) {
				qa->SetCurrentAction(NULL, index);
				delete last;
				qa->Unlock();
			}

		} else {

			//printf("Thread suspend PID %ld Count %ld\n",qa->fID[index],qa->fList.CountItems());
			suspend_thread(qa->fID[index]);
		}

		//Check my life?
		if (qa->Lock()) {
			if ( index >= qa->fCount) {
				qa->KIllThread(index);
				qa->Unlock();
				//printf("Thread died PID %ld\n",qa->fID[index]);
				return 0; //bye-bye world!
			}
			qa->Unlock();
		}

	} //while

}
