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
#ifndef oCore_win_util_h
#define oCore_win_util_h

#include <oCore/windows/win_error.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <comutil.h>

// intrusive_ptr support
inline ULONG ref_count(IUnknown* unk) { ULONG r = unk->AddRef()-1; unk->Release(); return r; }
inline void intrusive_ptr_add_ref(IUnknown* unk) { unk->AddRef(); }
inline void intrusive_ptr_release(IUnknown* unk) { unk->Release(); }

namespace ouro {
	namespace windows {

class scoped_handle
{
	scoped_handle(scoped_handle&);
	const scoped_handle& operator=(scoped_handle&);

public:
	scoped_handle() : h(nullptr) {}
	scoped_handle(HANDLE _Handle) : h(_Handle) { oVB(h != INVALID_HANDLE_VALUE); }
	scoped_handle(scoped_handle&& _That) { operator=(std::move(_That)); }
	const scoped_handle& operator=(scoped_handle&& _That)
	{
		if (this != &_That)
		{
			close();
			h = _That.h;
			_That.h = nullptr;
		}
		return *this;
	}

	const scoped_handle& operator=(HANDLE _That)
	{
		close();
		h = _That;
	}

	~scoped_handle() { close(); }

	operator HANDLE() { return h; }

private:
	HANDLE h;
	void close() { if (h && h != INVALID_HANDLE_VALUE) { ::CloseHandle(h); h = nullptr; } }
};

	} // namespace windows
} // namespace ouro

// primarily intended for id classes
template<typename T> DWORD asdword(const T& _ID) { return *((DWORD*)&_ID); }
template<> inline DWORD asdword(const std::thread::id& _ID) { return ((_Thrd_t*)&_ID)->_Id; }
inline std::thread::id astid(DWORD _ID)
{
	_Thrd_t tid; 
	ouro::windows::scoped_handle h(OpenThread(THREAD_QUERY_LIMITED_INFORMATION, FALSE, _ID));
	tid._Hnd = h;
	tid._Id = _ID;
	return *(std::thread::id*)&tid;
}

#endif
