/*
 * Copyright 2010, Pier Luigi Fiorini. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _SINGLETON_H
#define _SINGLETON_H

template<typename T>
class Singleton {
public:
	static	T*		Get();

protected:
	static	T*		fInstance;

					Singleton();
};

#endif	// _SINGLETON_H
