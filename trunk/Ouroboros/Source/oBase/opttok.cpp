/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2014 Antony Arciuolo.                                    *
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
#include <oBase/opttok.h>
#include <oBase/compiler_config.h>
#include <oBase/string.h>

namespace ouro {

static inline bool isopt(const char* arg) { return *arg == '-'; }
static inline bool islongopt(const char* arg) { return *arg == '-' && *(arg+1) == '-'; }

char opttok(const char** _ppValue, int _Argc, const char* _Argv[], const option* _pOptions, size_t _NumOptions)
{
	oTHREAD_LOCAL static const char** local_argv = 0;
	oTHREAD_LOCAL static int local_argc = 0;
	oTHREAD_LOCAL static const option* local_options = 0;
	oTHREAD_LOCAL static size_t num_options = 0;

	oTHREAD_LOCAL static int i = 0;

	*_ppValue = "";

	if (_Argv)
	{
		local_argv = _Argv;
		local_argc = _Argc;
		local_options = _pOptions;
		num_options = _NumOptions;
		i = 1; // skip exe name
	}

	else if (i >= local_argc)
		return 0;

	const char* currarg = local_argv[i];
	const char* nextarg = local_argv[i+1];

	if (!currarg)
		return 0;

	const option* oend = local_options + num_options;

	if (islongopt(currarg))
	{
		currarg += 2;
		for (const option* o = local_options; o < oend; o++)
		{
			if (!strcmp(o->fullname, currarg))
			{
				if (o->argname)
				{
					if (i == _Argc-1 || isopt(nextarg))
					{
						i++;
						*_ppValue = (const char*)i;
						return ':';
					}

					*_ppValue = nextarg;
				}

				else
					*_ppValue = currarg;

				i++;
				if (o->argname)
					i++;

				return o->abbrev;
			}

			o++;
		}

		i++; // skip unrecognized opt
		*_ppValue = currarg;
		return '?';
	}

	else if (isopt(local_argv[i]))
	{
		currarg++;
		for (const option* o = local_options; o < oend; o++)
		{
			if (*currarg == o->abbrev)
			{
				if (o->argname)
				{
					if (*(currarg+1) == ' ' || *(currarg+1) == 0)
					{
						if (i == _Argc-1 || isopt(nextarg))
						{
							i++;
							return ':';
						}

						*_ppValue = nextarg;
						i += 2;
					}

					else
					{
						*_ppValue = currarg + 1;
						i++;
					}
				}

				else
				{
					*_ppValue = currarg;
					i++;
				}

				return o->abbrev;
			}
		}

		i++; // skip unrecognized opt
		*_ppValue = currarg;
		return '?';
	}

	*_ppValue = local_argv[i++]; // skip to next arg
	return ' ';
}

char* optdoc(char* _StrDestination, size_t _SizeofStrDestination, const char* _AppName, const option* _pOptions, size_t _NumOptions, const char* _LooseParameters)
{
	ellipsize(_StrDestination, _SizeofStrDestination);
	char* dst = _StrDestination;

	int w = snprintf(dst, _SizeofStrDestination, "%s [options] ", _AppName);
	if (w == -1)
		return _StrDestination;
	dst += w;
	_SizeofStrDestination -= w;

	const option* oend = _pOptions + _NumOptions;
	for (const option* o = _pOptions; o < oend; o++)
	{
		w = snprintf(dst, _SizeofStrDestination, "-%c%s%s ", o->abbrev, o->argname ? " " : "", o->argname ? o->argname : "");
		if (w == -1) return _StrDestination;
		dst += w;
		_SizeofStrDestination -= w;
	}

	if (_LooseParameters && *_LooseParameters)
	{
		w = snprintf(dst, _SizeofStrDestination, "%s", _LooseParameters);
		dst += w;
		_SizeofStrDestination -= w;
	}

	w = snprintf(dst, _SizeofStrDestination, "\n\n");
	if (w == -1) return _StrDestination;
	dst += w;
	_SizeofStrDestination -= w;

	size_t maxLenFullname = 0;
	size_t maxLenArgname = 0;
	for (const option* o = _pOptions; o < oend && _SizeofStrDestination > 0; o++)
	{
		maxLenFullname = __max(maxLenFullname, strlen(o->fullname ? o->fullname : ""));
		maxLenArgname = __max(maxLenArgname, strlen(o->argname ? o->argname : ""));
	}

	static const size_t MaxLen = 80;
	const size_t descStartCol = 3 + 1 + maxLenFullname + 5 + maxLenArgname + 4;

	for (const option* o = _pOptions; o < oend && _SizeofStrDestination > 0; o++)
	{
		char fullname[64];
		*fullname = '\0';
		if (o->fullname)
			snprintf(fullname, " [--%s]", o->fullname);

		const char* odesc = o->desc ? o->desc : "";
		size_t newline = strcspn(odesc, oNEWLINE);
		char descline[128];
		if (newline > sizeof(descline))
			return nullptr;
		
		strlcpy(descline, odesc);
		bool doMoreLines = descline[newline] != '\0';
		descline[newline] = '\0';

		w = snprintf(dst, _SizeofStrDestination, "  -%c%- *s %- *s : %s\n"
			, o->abbrev
			, maxLenFullname + 5
			, fullname
			, maxLenArgname
			, o->argname ? o->argname : ""
			, descline);
		if (w == -1)
			return _StrDestination;
		dst += w;
		_SizeofStrDestination -= w;

		while (doMoreLines)
		{
			odesc += newline + 1;
			newline = strcspn(odesc, oNEWLINE);
			if (newline > sizeof(descline))
				return nullptr;
		
			strlcpy(descline, odesc);
			doMoreLines = descline[newline] != '\0';
			descline[newline] = '\0';

			w = snprintf(dst, _SizeofStrDestination, "% *s%s\n"
				, descStartCol
				, " "
				, descline);
			if (w == -1)
				return _StrDestination;
			dst += w;
			_SizeofStrDestination -= w;
		}
	}

	return _StrDestination;
}

}
