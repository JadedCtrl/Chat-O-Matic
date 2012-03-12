/*
 * Copyright 2009-2011, Andrea Anzani. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Andrea Anzani, andrea.anzani@gmail.com
 */

#include "WindowsManager.h"

WindowsManager*	WindowsManager::fInstance = NULL;


WindowsManager::WindowsManager()
{
	fCurrentPoint.Set(50.0f, 50.0f);
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
	fCurrentPoint += BPoint(50.0f, 50.0f);
}
