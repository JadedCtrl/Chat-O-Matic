/*
 * Copyright 2009-2010, Andrea Anzani. All rights reserved.
 * Copyright 2009-2010, Pier Luigi Fiorini. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Andrea Anzani, andrea.anzani@gmail.com
 *		Pier Luigi Fiorini, pierluigi.fiorini@gmail.com
 */

#include "Singleton.h"

template<typename T> T* Singleton<T>::fInstance = 0;


template<typename T>
Singleton<T>::Singleton()
{
}


template<typename T>
T*
Singleton<T>::Get()
{
	if (!fInstance)
		fInstance = new T();
	return fInstance;
}
