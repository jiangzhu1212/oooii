/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2013 Antony Arciuolo.                                    *
 * arciuolo@gmail.com                                                     *
 *                                                                        *
 * Permission is hereby granted, free of charge, to any person obtaining  *
 * a copy of this software and associated documentation files (the        *
 * "Software"), to deal in the Software without restriction, including    *
 * without limitation the rights to use, copy, modify, merge, publish,    *
 * distribute, sublicense, and/or sell copies of the Software, and to     *
 * permit persons to whom the Software is furnished to do so, subject to  *
 * the following conditions:                                              *
 *                                                                        *
 * The above copyright notice and this permission notice shall be         *
 * included in all copies or substantial portions of the Software.        *
 *                                                                        *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        *
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     *
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND                  *
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE *
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION *
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION  *
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.        *
 **************************************************************************/
#pragma once
#ifndef about_ouroboros_h
#define about_ouroboros_h

#include <Ouroboros_externals.h>
#include <oGUI/about.h>

#define OUROBOROS_WEBSITE "http://code.google.com/p/oooii/"
#define OUROBOROS_SUPPORT "http://code.google.com/p/oooii/issues/list"

#																																								define oOUROBOROS_EXTERNAL(_StrName, _StrVersion, _UrlHome, _UrlLicense, _StrDesc) _StrName "|"
static const char* OUROBOROS_COMPONENTS = oOUROBOROS_EXTERNALS;
#																																								undef oOUROBOROS_EXTERNAL
#																																								define oOUROBOROS_EXTERNAL(_StrName, _StrVersion, _UrlHome, _UrlLicense, _StrDesc) "<a href=\"" _UrlHome "\">homepage</a>  <a href=\"" _UrlLicense "\">license</a>\r\n" _StrVersion "\r\n" _StrDesc "|"
static const char* OUROBOROS_COMPONENT_COMMENTS = oOUROBOROS_EXTERNALS;
#																																								undef oOUROBOROS_EXTERNAL

#define oDECLARE_ABOUT_INFO(_AboutInfoVariable, _Icon) \
	ouro::about::info _AboutInfoVariable; \
	_AboutInfoVariable.icon = (ouro::icon_handle)_Icon; \
	_AboutInfoVariable.website = OUROBOROS_WEBSITE; \
	_AboutInfoVariable.issue_site = OUROBOROS_SUPPORT; \
	_AboutInfoVariable.components = OUROBOROS_COMPONENTS; \
	_AboutInfoVariable.component_comments = OUROBOROS_COMPONENT_COMMENTS;

#endif
