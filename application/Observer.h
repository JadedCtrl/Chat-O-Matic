/*
 * Copyright 2009, Andrea Anzani. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef Observer_h_
#define Observer_h_

#include <String.h>

class Notifier;

class Observer
{
	public:
	virtual void ObserveString(int32 what, BString str) {};
	virtual void ObserveInteger(int32 what, int32 value) {};
	virtual void ObservePointer(int32 what, void* ptr) {};
};
#endif
