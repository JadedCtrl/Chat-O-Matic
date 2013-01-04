/*
 * Copyright 2010, Andrea Anzani. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _ACTION_H
#define _ACTION_H

#include <Message.h>
#include <String.h>

#define ActionID bigtime_t

class Action
{
public:
				Action() { fActionID = 0; }
	virtual			~Action() {}

	virtual	status_t	Perform(BMessage* errors) = 0;
	virtual	BString		GetDescription() = 0;

	void			SetActionID(const ActionID id) { fActionID = id; }
	ActionID		GetActionID() const { return fActionID; }

private:
	ActionID	fActionID;
};

#endif	//_ACTION_H
