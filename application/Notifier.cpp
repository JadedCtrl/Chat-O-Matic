/*
 * Copyright 2009, Andrea Anzani. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Andrea Anzani, andrea.anzani@gmail.com
 */

#include "Notifier.h"
#include "Observer.h"


void	
Notifier::RegisterObserver(Observer* obs)
{
	if (!fObserverList.HasItem(obs))
		fObserverList.AddItem(obs);
}


void
Notifier::UnregisterObserver(Observer* obs)
{
	if (fObserverList.HasItem(obs))
		fObserverList.RemoveItem(obs, false);
}


void 
Notifier::NotifyString(int32 what, BString str)
{
	for (int i = 0; i < fObserverList.CountItems(); i++)
		fObserverList.ItemAt(i)->ObserveString(what, str);
}


void 
Notifier::NotifyInteger(int32 what, int32 value)
{
	for (int i = 0; i < fObserverList.CountItems(); i++)
		fObserverList.ItemAt(i)->ObserveInteger(what, value);
}

void 
Notifier::NotifyPointer(int32 what, void* ptr)
{
	for (int i = 0; i < fObserverList.CountItems(); i++)
		fObserverList.ItemAt(i)->ObservePointer(what, ptr);
}
