/*
 * Copyright 2009-2010, Pier Luigi Fiorini. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _TEMPLATE_VIEW_H
#define _TEMPLATE_VIEW_H

#include <View.h>


const uint32 kChanged      = 'CHGD';


class TemplateView : public BView {
public:
					TemplateView(const char* name);

	virtual	void	AttachedToWindow();
};

#endif	// _TEMPLATE_VIEW_H
