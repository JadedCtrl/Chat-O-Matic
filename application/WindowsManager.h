/*
 * Copyright 2009, Andrea Anzani. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _WINDOWS_MANAGER_H_
#define _WINDOWS_MANAGER_H_

#include <Window.h>
#include <Point.h>

class WindowsManager
{
public:
	static	WindowsManager*	Get();
		
	void					RelocateWindow(BWindow*	window);

private:
							WindowsManager();

	static	WindowsManager*	fInstance;
			BPoint			fCurrentPoint;
};

#endif	// _WINDOWS_MANAGER_H_
