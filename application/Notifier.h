/*
 * Copyright 2009-2011, Andrea Anzani. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef NOTIFIER_H
#define NOTIFIER_H

#include <String.h>
#include <ObjectList.h>

#include "Observer.h"


class Notifier
{
	public:
		void	RegisterObserver(Observer*);
		void	UnregisterObserver(Observer*);
		
		void NotifyString(int32 what, BString str);
		void NotifyInteger(int32 what, int32 value);
		void NotifyPointer(int32 what, void* ptr);
		
	private:
		BObjectList<Observer>	fObserverList;	
};


#endif // NOTIFIER_H

