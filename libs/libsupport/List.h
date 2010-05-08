/*
 * Copyright 2009-2010, Pier Luigi Fiorini. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _LIST_H
#define _LIST_H

#include <list>

#include <SupportDefs.h>

template<class T>
class List {
public:
	uint32 	CountItems() const;

	void  	AddItem(T type);

	void	RemoveItemAt(uint32 position);

	T		ItemAt(uint32 position);

private:
	std::list<T> fList;
	typedef typename std::list<T>::iterator fIter;
};


template<class T>
uint32 List<T>::CountItems() const
{
	return fList.size();
}


template<class T>
void List<T>::AddItem(T type)
{
	fList.push_back(type);
}


template<class T>
void List<T>::RemoveItemAt(uint32 position)
{
	fIter i = fMap.begin();
	std::advance(i, position);
	fList.erase(i);
}


template<class T>
T List<T>::ItemAt(uint32 position)
{
	fIter i = fList.begin();
	std::advance(i, position); 	
	return *i;
}

#endif	// _LIST_H
