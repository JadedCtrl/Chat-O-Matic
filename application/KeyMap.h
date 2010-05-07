/*
 * Copyright 2009, Andrea Anzani. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _KEY_MAP_H
#define _KEY_MAP_H

#include <map>

#include <List.h>
#include <SupportDefs.h>

template<class KEY, class TYPE>
class KeyMap {
public:
	uint32 	CountItems();

	void  	AddItem(KEY k, TYPE t);

	TYPE	ValueFor(KEY, bool* found = NULL);

	void	RemoveItemAt(int32 position);
	void	RemoveItemFor(KEY);

	TYPE	ValueAt(int32 position);
	KEY		KeyAt(int32 position);

	BList*	Items();

private:
	std::map<KEY,TYPE> fMap;
	typedef typename std::map<KEY,TYPE>::iterator fIter;
};


template<class KEY, class TYPE>
uint32 KeyMap<KEY, TYPE>::CountItems()
{
	return fMap.size();
}


template<class KEY, class TYPE>
void KeyMap<KEY, TYPE>::AddItem(KEY k, TYPE t)
{
	fMap[k] = t;
}


template<class KEY, class TYPE>
TYPE KeyMap<KEY, TYPE>::ValueFor(KEY k, bool* found)
{
	fIter i = fMap.find(k);

	if (found) {
		if (i == fMap.end())
			*found = false;
		else
			*found = true;
	}

	if (i == fMap.end())
		return NULL;
	return i->second;
}


template<class KEY, class TYPE>
void KeyMap<KEY, TYPE>::RemoveItemAt(int32 position)
{
	fIter i = fMap.begin();
	std::advance(i, position);
	fMap.erase(i->first);
}


template<class KEY, class TYPE>
void KeyMap<KEY, TYPE>::RemoveItemFor(KEY k)
{
	fMap.erase(k);
}


template<class KEY, class TYPE>
TYPE KeyMap<KEY, TYPE>::ValueAt(int32 position)
{
	fIter i = fMap.begin();
	std::advance(i, position); 	
	if (i == fMap.end())
		return NULL;
	return i->second;
}


template<class KEY, class TYPE>
KEY KeyMap<KEY, TYPE>::KeyAt(int32 position)
{
	fIter i = fMap.begin();
	std::advance(i, position); 	
	if (i == fMap.end())
		return NULL;
	return i->first;
}


template<class KEY, class TYPE>
BList* KeyMap<KEY, TYPE>::Items()
{
	BList* list = new BList();

	for (fIter i = fMap.begin(); i != fMap.end(); ++i)
		list->AddItem(i->second);

	return list;
}

#endif	// _KEY_MAP_H
