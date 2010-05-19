/*
 * Copyright 2009, Andrea Anzani. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Andrea Anzani, andrea.anzani@gmail.com
 */

#include "WindowsManager.h"

WindowsManager*	WindowsManager::fInstance = NULL;


WindowsManager::WindowsManager()
{
	fCurrentPoint.Set(40.0f, 40.0f);
}


WindowsManager*
WindowsManager::Get()
{
	if (!fInstance)
		fInstance = new WindowsManager();

	return fInstance;	
}


void	
WindowsManager::RelocateWindow(BWindow*	window)
{
	window->SetWorkspaces(B_CURRENT_WORKSPACE);
	window->MoveTo(fCurrentPoint);
	fCurrentPoint += BPoint(40.0f, 40.0f);
}
