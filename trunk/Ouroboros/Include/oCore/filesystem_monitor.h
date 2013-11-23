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
// Object that redirects file lifetime events to a functor.
#pragma once
#ifndef oCore_filesystem_monitor_h
#define oCore_filesystem_monitor_h

#include <oBase/path.h>

namespace ouro {
	namespace filesystem {

/* enum class */ namespace file_event
{	enum value {

	unsupported,
	added,
	removed,
	modified,
	accessible,

};}

class monitor
{
public:
	// A file can get an added or modified event before all work on the file is 
	// complete, so there is a polling to check if the file has settled and is 
	// ready for access by another client system.
  struct info
  {
		info()
			: accessibility_poll_rate_ms(2000)
			, accessibility_timeout_ms(5000)
		{}

    unsigned int accessibility_poll_rate_ms;
    unsigned int accessibility_timeout_ms;
  };

  static std::shared_ptr<monitor> make(const info& _Info, const std::function<void(file_event::value _Event, const path& _Path)>& _OnEvent);

  virtual info get_info() const = 0;
  
  // The specified path can be a wildcard. _BufferSize is how much watch data
	// to buffer between sampling calls. This size is doubled to enable double-
	// buffering and better concurrency.
  virtual void watch(const path& _Path, size_t _BufferSize, bool _Recursive) = 0; 
  
  // If a parent to the specified path is recursively watching, it will 
  // continue watching. This only removes entries that were previously
  // registered with watch() or watch_recursive().
  virtual void unwatch(const path& _Path) = 0;
};

	} // namespace filesystem
} // namespace ouro

#endif
