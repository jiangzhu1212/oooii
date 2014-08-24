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
// Provide a simple, clean way of handling command line parameters.
#pragma once
#ifndef oBase_opttok_h
#define oBase_opttok_h

namespace ouro {

struct option
{
	// single-character version of command line switch. If '\0' then only fullname
	// can be used.
	char abbrev; 
	
	// fullname version of command line switch. If nullptr then only abbrev can be 
	// used.
	const char* fullname;

	// name of argument to pass to switch. If nullptr, then the switch takes no 
	// argument.
	const char* argname;

	// description displayed by optdoc
	const char* desc; 
};

// Similar to strtok, first call should specify argv and argc from main(), and
// subsequent calls should pass nullptr to those values. This searches through 
// the specified command line arguments and matches either "--fullname" or 
// "-abbrev" and fills *value with the value for the arg (or if argname is 
// nullptr, meaning there is no arg, then *value gets the fullname or nullptr if 
// the options does not exist.)
// returns:
// if there is a match, the abbrev value
// 0 for no more matches
// ' ' for regular arguments (non-option values)
// '?' for an unrecognized option (error condition)
// ':' if there was a missing option value (error condition)
char opttok(const char** _ppValue, int _Argc, const char* _Argv[], const option* _pOptions, size_t _NumOptions);
inline char opttok(const char** _ppValue) { return opttok(_ppValue, 0, nullptr, nullptr, 0); }
template<size_t size> char opttok(const char** _ppValue, int _Argc, const char* _Argv[], const option (&_pOptions)[size]) { return opttok(_ppValue, _Argc, _Argv, _pOptions, size); }

// Prints documentation for the specified options to the specified buffer.
char* optdoc(char* _StrDestination, size_t _SizeofStrDestination, const char* _AppName, const option* _pOptions, size_t _NumOptions, const char* _LooseParameters = "");
template<size_t size> char* optdoc(char* _StrDestination, size_t _SizeofStrDestination, const char* _AppName, const option (&_pOptions)[size], const char* _LooseParameters = "") { return optdoc(_StrDestination, _SizeofStrDestination, _AppName, _pOptions, size, _LooseParameters); }
template<size_t capacity> char* optdoc(char (&_StrDestination)[capacity], const char* _AppName, const option* _pOptions, size_t _NumOptions, const char* _LooseParameters = "") { return optdoc(_StrDestination, capacity, _AppName, _pOptions, _NumOptions, _LooseParameters); }
template<size_t capacity, size_t size> char* optdoc(char (&_StrDestination)[capacity], const char* _AppName, const option (&_pOptions)[size], const char* _LooseParameters = "") { return optdoc(_StrDestination, capacity, _AppName, _pOptions, size, _LooseParameters); }

}

#endif
