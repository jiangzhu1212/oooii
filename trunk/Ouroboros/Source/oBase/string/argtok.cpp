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
#include <oBase/string.h>

#ifndef PATH_MAX
	#define PATH_MAX 512
#endif

namespace ouro {

const char** argtok(void* (*_Allocate)(size_t), const char* _AppPath, const char* _CommandLine, int* _pNumArgs)
{
	/** <citation
		usage="Implementation" 
		reason="Need an ASCII version of CommandLineToArgvW" 
		author="Alexander A. Telyatnikov"
		description="http://alter.org.ua/docs/win/args/"
		license="*** Assumed Public Domain ***"
		licenseurl="http://alter.org.ua/docs/win/args/"
		modification="made less Windowsy, changes to get it to compile, add exe path as arg 0"
	/>*/
	// $(CitedCodeBegin)

	char** argv;
	char* _argv;
	size_t len;
	int argc;
	char a;
	size_t i, j;

	bool in_QM;
	bool in_TEXT;
	bool in_SPACE;

	size_t path_size = 0;

	len = (size_t)strlen(_CommandLine);
	if (_AppPath) // @oooii
	{
		path_size = strlen(_AppPath) + 1;
		len += path_size;
	}

	i = ((len+2)/2)*sizeof(void*) + sizeof(void*);

	if (_AppPath) // @oooii
		i += sizeof(void*);
	
	if (!_Allocate)
		_Allocate = malloc;

	size_t full_size = i + len + 2;

	argv = (char**)_Allocate(full_size);
	memset(argv, 0, full_size);

	_argv = (char*)(((unsigned char*)argv)+i);

	argc = 0;
	argv[argc] = _argv;
	in_QM = false;
	in_TEXT = false;
	in_SPACE = true;
	i = 0;
	j = 0;

	// optionally insert exe path so this is exactly like argc/argv
	if (_AppPath)
	{
		clean_path(argv[argc], path_size, _AppPath, '\\');
		j = (size_t)strlen(argv[argc])+1;
		argc++;
		argv[argc] = _argv+j;
	}

	while( (a = _CommandLine[i]) != 0 ) {
		if(in_QM) {
			if(a == '\"') {
				in_QM = false;
			} else {
				_argv[j] = a;
				j++;
			}
		} else {
			switch(a) {
			case '\"':
				in_QM = true;
				in_TEXT = true;
				if(in_SPACE) {
					argv[argc] = _argv+j;
					argc++;
				}
				in_SPACE = false;
				break;
			case ' ':
			case '\t':
			case '\n':
			case '\r':
				if(in_TEXT) {
					_argv[j] = '\0';
					j++;
				}
				in_TEXT = false;
				in_SPACE = true;
				break;
			default:
				in_TEXT = true;
				if(in_SPACE) {
					argv[argc] = _argv+j;
					argc++;
				}
				_argv[j] = a;
				j++;
				in_SPACE = false;
				break;
			}
		}
		i++;
	}
	_argv[j] = '\0';
	argv[argc] = nullptr;

	*_pNumArgs = argc;
	return (const char**)argv;
}

}
