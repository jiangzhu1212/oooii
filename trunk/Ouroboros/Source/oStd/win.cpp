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
#include "win.h"

namespace oStd {

	namespace windows {

const char* as_string(const version::value& _Version)
{
	switch (_Version)
	{
		case oStd::windows::version::win2000: return "Windows 2000";
		case oStd::windows::version::xp: return "Windows XP";
		case oStd::windows::version::xp_pro_64bit: return "Windows XP Pro 64-bit";
		case oStd::windows::version::server_2003: return "Windows Server 2003";
		case oStd::windows::version::home_server: return "Windows Home Server";
		case oStd::windows::version::server_2003r2: return "Windows Server 2003R2";
		case oStd::windows::version::vista: return "Windows Vista";
		case oStd::windows::version::server_2008: return "Windows Server 2008";
		case oStd::windows::version::server_2008r2: return "Windows Server 2008R2";
		case oStd::windows::version::win7: return "Windows 7";
		case oStd::windows::version::win7_sp1: return "Windows 7 SP1";
		case oStd::windows::version::win8: return "Windows 8";
		case oStd::windows::version::server_2012: "Windows Server 2012";
		case oStd::windows::version::win8_1: return "Windows 8.1";
		case oStd::windows::version::server_2012_sp1: "Windows Server 2012 SP1";
		case oStd::windows::version::unknown:
		default: break;
	}

	return "?";
}

static errno_t errno_from_hresult(HRESULT _hResult)
{
	switch (_hResult)
	{
		// From WinError.h
		case ERROR_FILE_NOT_FOUND: return ENOENT;
		case ERROR_PATH_NOT_FOUND: return ENOENT;
		case ERROR_TOO_MANY_OPEN_FILES: return EMFILE;
		case ERROR_DIR_NOT_EMPTY : return ENOTEMPTY;
		case ERROR_ACCESS_DENIED: return EACCES;
		case ERROR_INVALID_HANDLE: return ENOTRECOVERABLE;
		case ERROR_ARENA_TRASHED: return ENOTRECOVERABLE;
		case ERROR_NOT_ENOUGH_MEMORY: return ENOMEM;
		case ERROR_INVALID_BLOCK: return ENOTRECOVERABLE;
		case ERROR_BAD_ENVIRONMENT: return ENOTRECOVERABLE;
		case ERROR_BAD_FORMAT: return ENOTRECOVERABLE;
		case ERROR_INVALID_ACCESS: return EACCES;
		case ERROR_INVALID_DATA: return ENOTRECOVERABLE;
		case ERROR_OUTOFMEMORY: return ENOMEM;
		case ERROR_INVALID_DRIVE: return ENOTRECOVERABLE;
		case ERROR_CURRENT_DIRECTORY: return ENOTRECOVERABLE;
		case ERROR_NOT_SAME_DEVICE: return ENOTRECOVERABLE;
		case ERROR_NO_MORE_FILES: return ENOTRECOVERABLE;
		case ERROR_WRITE_PROTECT: return EACCES;
		case ERROR_BAD_UNIT: return ENOTRECOVERABLE;
		case ERROR_NOT_READY: return ENOTRECOVERABLE;
		case ERROR_BAD_COMMAND: return ENOTRECOVERABLE;
		case ERROR_CRC: return ENOTRECOVERABLE;
		case ERROR_BAD_LENGTH: return ENOTRECOVERABLE;
		case ERROR_SEEK: return EIO;
		case ERROR_NOT_DOS_DISK: return ENOTRECOVERABLE;
		case ERROR_WRITE_FAULT: return EFAULT;
		case ERROR_READ_FAULT: return EFAULT;
		case ERROR_SHARING_VIOLATION: return EACCES;
		case ERROR_LOCK_VIOLATION: return EACCES;
		case ERROR_WRONG_DISK: return ENOTRECOVERABLE;
		case E_NOINTERFACE: return ENOSYS;
		case E_OUTOFMEMORY: return ENOMEM;
		case E_FAIL: return ENOTRECOVERABLE;
		case E_ACCESSDENIED: return EACCES;
		case E_INVALIDARG: return EINVAL;
		case E_POINTER: return EINVAL;
		case S_OK: return 0;
		case S_FALSE: return ENOTRECOVERABLE;
		default: break;
	}
	return ENOTRECOVERABLE;
}

const char* as_string_HR(HRESULT _hResult)
{
	switch (_hResult)
	{
		// From WinError.h
		//case ERROR_INVALID_FUNCTION: return "ERROR_INVALID_FUNCTION"; // same value as S_FALSE
		case ERROR_FILE_NOT_FOUND: return "ERROR_FILE_NOT_FOUND";
		case ERROR_PATH_NOT_FOUND: return "ERROR_PATH_NOT_FOUND";
		case ERROR_TOO_MANY_OPEN_FILES: return "ERROR_TOO_MANY_OPEN_FILES";
		case ERROR_DIR_NOT_EMPTY : return "ERROR_DIR_NOT_EMPTY";
		case ERROR_ACCESS_DENIED: return "ERROR_ACCESS_DENIED";
		case ERROR_INVALID_HANDLE: return "ERROR_INVALID_HANDLE";
		case ERROR_ARENA_TRASHED: return "ERROR_ARENA_TRASHED";
		case ERROR_NOT_ENOUGH_MEMORY: return "ERROR_NOT_ENOUGH_MEMORY";
		case ERROR_INVALID_BLOCK: return "ERROR_INVALID_BLOCK";
		case ERROR_BAD_ENVIRONMENT: return "ERROR_BAD_ENVIRONMENT";
		case ERROR_BAD_FORMAT: return "ERROR_BAD_FORMAT";
		case ERROR_INVALID_ACCESS: return "ERROR_INVALID_ACCESS";
		case ERROR_INVALID_DATA: return "ERROR_INVALID_DATA";
		case ERROR_OUTOFMEMORY: return "ERROR_OUTOFMEMORY";
		case ERROR_INVALID_DRIVE: return "ERROR_INVALID_DRIVE";
		case ERROR_CURRENT_DIRECTORY: return "ERROR_CURRENT_DIRECTORY";
		case ERROR_NOT_SAME_DEVICE: return "ERROR_NOT_SAME_DEVICE";
		case ERROR_NO_MORE_FILES: return "ERROR_NO_MORE_FILES";
		case ERROR_WRITE_PROTECT: return "ERROR_WRITE_PROTECT";
		case ERROR_BAD_UNIT: return "ERROR_BAD_UNIT";
		case ERROR_NOT_READY: return "ERROR_NOT_READY";
		case ERROR_BAD_COMMAND: return "ERROR_BAD_COMMAND";
		case ERROR_CRC: return "ERROR_CRC";
		case ERROR_BAD_LENGTH: return "ERROR_BAD_LENGTH";
		case ERROR_SEEK: return "ERROR_SEEK";
		case ERROR_NOT_DOS_DISK: return "ERROR_NOT_DOS_DISK";
		case ERROR_WRITE_FAULT: return "ERROR_WRITE_FAULT";
		case ERROR_READ_FAULT: return "ERROR_READ_FAULT";
		case ERROR_SHARING_VIOLATION: return "ERROR_SHARING_VIOLATION";
		case ERROR_LOCK_VIOLATION: return "ERROR_LOCK_VIOLATION";
		case ERROR_WRONG_DISK: return "ERROR_WRONG_DISK";
		case E_NOINTERFACE: return "E_NOINTERFACE";
		case E_OUTOFMEMORY: return "E_OUTOFMEMORY";
		case E_FAIL: return "E_FAIL";
		case E_ACCESSDENIED: return "E_ACCESSDENIED";
		case E_INVALIDARG: return "E_INVALIDARG";
		case S_OK: return "S_OK";
		case S_FALSE: return "S_FALSE";
		default: return "?";
	}
}

const char* as_string_HR_DXGI(HRESULT _hResult)
{
	switch (_hResult)
	{
		case DXGI_ERROR_INVALID_CALL: return "DXGI_ERROR_INVALID_CALL";
		case DXGI_ERROR_NOT_FOUND: return "DXGI_ERROR_NOT_FOUND";
		case DXGI_ERROR_MORE_DATA: return "DXGI_ERROR_MORE_DATA";
		case DXGI_ERROR_UNSUPPORTED: return "DXGI_ERROR_UNSUPPORTED";
		case DXGI_ERROR_DEVICE_REMOVED: return "DXGI_ERROR_DEVICE_REMOVED";
		case DXGI_ERROR_DEVICE_HUNG: return "DXGI_ERROR_DEVICE_HUNG";
		case DXGI_ERROR_DEVICE_RESET: return "DXGI_ERROR_DEVICE_RESET";
		case DXGI_ERROR_WAS_STILL_DRAWING: return "DXGI_ERROR_WAS_STILL_DRAWING";
		case DXGI_ERROR_FRAME_STATISTICS_DISJOINT: return "DXGI_ERROR_FRAME_STATISTICS_DISJOINT";
		case DXGI_ERROR_GRAPHICS_VIDPN_SOURCE_IN_USE: return "DXGI_ERROR_GRAPHICS_VIDPN_SOURCE_IN_USE";
		case DXGI_ERROR_DRIVER_INTERNAL_ERROR: return "DXGI_ERROR_DRIVER_INTERNAL_ERROR";
		case DXGI_ERROR_NONEXCLUSIVE: return "DXGI_ERROR_NONEXCLUSIVE";
		case DXGI_ERROR_NOT_CURRENTLY_AVAILABLE: return "DXGI_ERROR_NOT_CURRENTLY_AVAILABLE";
		case DXGI_ERROR_REMOTE_CLIENT_DISCONNECTED: return "DXGI_ERROR_REMOTE_CLIENT_DISCONNECTED";
		case DXGI_ERROR_REMOTE_OUTOFMEMORY: return "DXGI_ERROR_REMOTE_OUTOFMEMORY";
		default: break;
	}
	return as_string_HR(_hResult);
}

const char* as_string_HR_DX11(HRESULT _hResult)
{
	switch (_hResult)
	{
		case D3D11_ERROR_FILE_NOT_FOUND: return "D3D11_ERROR_FILE_NOT_FOUND";
		case D3D11_ERROR_TOO_MANY_UNIQUE_STATE_OBJECTS: return "D3D11_ERROR_TOO_MANY_UNIQUE_STATE_OBJECTS";
		case D3D11_ERROR_TOO_MANY_UNIQUE_VIEW_OBJECTS: return "D3D11_ERROR_TOO_MANY_UNIQUE_VIEW_OBJECTS";
		case D3D11_ERROR_DEFERRED_CONTEXT_MAP_WITHOUT_INITIAL_DISCARD: return "D3D11_ERROR_DEFERRED_CONTEXT_MAP_WITHOUT_INITIAL_DISCARD";
		case D3DERR_INVALIDCALL: return "D3DERR_INVALIDCALL";
		case D3DERR_WASSTILLDRAWING: return "D3DERR_WASSTILLDRAWING";
		default: break;
	}
	return as_string_HR_DXGI(_hResult);
}

const char* as_string_HR_VFW(HRESULT _hResult)
{
	switch (_hResult)
	{
		case VFW_E_INVALIDMEDIATYPE: return "VFW_E_INVALIDMEDIATYPE";
		case VFW_E_NOT_CONNECTED: return "VFW_E_NOT_CONNECTED";
		case VFW_E_NOT_STOPPED: return "VFW_E_NOT_STOPPED";
		case VFW_E_WRONG_STATE: return "VFW_E_WRONG_STATE";
		default: break;
	}
	return as_string_HR(_hResult);
}

class scoped_local_alloc
{
public:
	scoped_local_alloc() : p(nullptr) {}
	scoped_local_alloc(void* _Pointer) : p(_Pointer) {}
	~scoped_local_alloc() { if (p) LocalFree(p); }
private:
	void* p;
};

std::string message(HRESULT _hResult)
{
	std::string msg;
	char* pMessage = nullptr;
	FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM, 0, static_cast<DWORD>(_hResult), 0, (LPTSTR)&pMessage, 16, 0);
	scoped_local_alloc Message(pMessage);

	if (!*pMessage || !memcmp(pMessage, "???", 3))
	{
		const char* s = as_string_HR_DX11(_hResult);
		if (*s == '?') s = as_string_HR_VFW(_hResult);
		if (*s == '?')
		{
			#if oDXVER >= oDXVER_9a
				msg = DXGetErrorStringA(_hResult);
			#else
				msg = std::move(oStd::formatf("unrecognized error code 0x%08x", _hResult));
			#endif
		}

		else
			msg = pMessage;
	}
	else
		msg = pMessage;

	return std::move(msg);
}

const char* as_string_display_code(UINT _DISPCode)
{
	switch (_DISPCode)
	{
		case DISP_CHANGE_BADDUALVIEW: return "DISP_CHANGE_BADDUALVIEW";
		case DISP_CHANGE_BADFLAGS: return "DISP_CHANGE_BADFLAGS";
		case DISP_CHANGE_BADMODE: return "DISP_CHANGE_BADMODE";
		case DISP_CHANGE_BADPARAM: return "DISP_CHANGE_BADPARAM";
		case DISP_CHANGE_FAILED: return "DISP_CHANGE_FAILED";
		case DISP_CHANGE_NOTUPDATED: return "DISP_CHANGE_NOTUPDATED";
		case DISP_CHANGE_RESTART: return "DISP_CHANGE_RESTART";
		case DISP_CHANGE_SUCCESSFUL: return "DISP_CHANGE_SUCCESSFUL";
		default: return "?";
	}
}

version::value get_version()
{
	OSVERSIONINFOEX osvi;
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	if (GetVersionEx((OSVERSIONINFO*)&osvi))
	{
		if (osvi.dwMajorVersion == 6)
		{
			if (osvi.dwMinorVersion == 2)
			{
				if (osvi.wServicePackMajor == 1)
					return osvi.wProductType == VER_NT_WORKSTATION ? version::win8_1 : version::server_2012_sp1;
				else if (osvi.wServicePackMajor == 0)
					return osvi.wProductType == VER_NT_WORKSTATION ? version::win8 : version::server_2012;
			}

			else if (osvi.dwMinorVersion == 1)
			{
				if (osvi.wServicePackMajor == 0)
					return osvi.wProductType == VER_NT_WORKSTATION ? version::win7 : version::server_2008r2;
				else if (osvi.wServicePackMajor == 1)
					return osvi.wProductType == VER_NT_WORKSTATION ? version::win7_sp1 : version::server_2008r2_sp1;
			}
			else if (osvi.dwMinorVersion == 0)
			{
				if (osvi.wServicePackMajor == 2)
					return osvi.wProductType == VER_NT_WORKSTATION ? version::vista_sp2 : version::server_2008_sp2;
				else if (osvi.wServicePackMajor == 1)
					return osvi.wProductType == VER_NT_WORKSTATION ? version::vista_sp1 : version::server_2008_sp1;
				else if (osvi.wServicePackMajor == 0)
					return osvi.wProductType == VER_NT_WORKSTATION ? version::vista : version::server_2008;
			}
		}

		else if (osvi.dwMajorVersion == 5)
		{
			if (osvi.dwMinorVersion == 2)
			{
				SYSTEM_INFO si;
				GetSystemInfo(&si);
				if ((osvi.wProductType == VER_NT_WORKSTATION) && (si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_AMD64))
					return version::xp_pro_64bit;
				else if (osvi.wSuiteMask & 0x00008000 /*VER_SUITE_WH_SERVER*/)
					return version::home_server;
				else
					return GetSystemMetrics(SM_SERVERR2) ? version::server_2003r2 : version::server_2003;
			}

			else if (osvi.dwMinorVersion == 1)
				return version::xp;
			else if (osvi.dwMinorVersion == 0)
				return version::win2000;
		}
	}

	return version::unknown;
}

class category_impl : public std::error_category
{
public:
	const char* name() const { return "windows"; }
	std::string message(value_type _ErrCode) const
	{
		errno_t e = windows::errno_from_hresult((HRESULT)_ErrCode);
		if (e && e != ENOTRECOVERABLE)
			return std::generic_category().message(e);
		return std::move(oStd::windows::message((HRESULT)_ErrCode));
	}
};

const std::error_category& category()
{
	static windows::category_impl sSingleton;
	return sSingleton;
}

	} // namespace windows
} // namespace oStd
