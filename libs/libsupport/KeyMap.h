/*
 * Copyright 2009-2010, Andrea Anzani. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _KEY_MAP_H
#define _KEY_MAP_H

#include <map>

#include "List.h"

template<class KEY, class TYPE>
class KeyMap {
public:
	uint32 		CountItems() const;

	void  		AddItem(KEY k, TYPE t);

	TYPE		ValueFor(KEY, bool* found = NULL) const;

	TYPE		RemoveItemAt(int32 position);
	TYPE		RemoveItemFor(KEY);

	KEY			KeyAt(uint32 position) const;
	TYPE		ValueAt(uint32 position) const;

	void		AddList(KeyMap<KEY, TYPE> appendList);

	List<TYPE>	Values() const;

private:
	std::map<KEY, TYPE> fMap;
	typedef typename std::map<KEY, TYPE>::iterator fIter;
	typedef typename std::map<KEY, TYPE>::const_iterator fConstIter;
};


template<class KEY, class TYPE>
inline uint32
KeyMap<KEY, TYPE>::CountItems() const
{
	return fMap.size();
}


template<class KEY, class TYPE>
inline void
KeyMap<KEY, TYPE>::AddItem(KEY k, TYPE t)
{
	fMap[k] = t;
}


template<class KEY, class TYPE>
inline TYPE
KeyMap<KEY, TYPE>::ValueFor(KEY k, bool* found) const
{
	fConstIter i = fMap.find(k);

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
inline TYPE
KeyMap<KEY, TYPE>::RemoveItemAt(int32 position)
{
	TYPE value = ValueAt(position);
	fIter i = fMap.begin();
	std::advance(i, position);
	fMap.erase(i->first);
	return value;
}


template<class KEY, class TYPE>
inline TYPE
KeyMap<KEY, TYPE>::RemoveItemFor(KEY k)
{
	TYPE value = ValueFor(k);
	fMap.erase(k);
	return value;
}


template<class KEY, class TYPE>
inline KEY
KeyMap<KEY, TYPE>::KeyAt(uint32 position) const
{
	fConstIter i = fMap.begin();
	std::advance(i, position); 	
	if (i == fMap.end())
		return NULL;
	return i->first;
}


template<class KEY, class TYPE>
inline TYPE
KeyMap<KEY, TYPE>::ValueAt(uint32 position) const
{
	fConstIter i = fMap.begin();
	std::advance(i, position); 	
	if (i == fMap.end())
		return NULL;
	return i->second;
}


template<class KEY, class TYPE>
inline void
KeyMap<KEY, TYPE>::AddList(KeyMap<KEY, TYPE> appendList)
{
	if (appendList.CountItems() == 0)
		return;
	for (int i = 0; i < appendList.CountItems(); i++)
		AddItem(appendList.KeyAt(i), appendList.ValueAt(i));
}


template<class KEY, class TYPE>
inline List<TYPE>
KeyMap<KEY, TYPE>::Values() const
{
	List<TYPE> list;

	for (fIter i = fMap.begin(); i != fMap.end(); ++i)
		list.AddItem(i->second);

	return list;
}

#endif	// _KEY_MAP_H
