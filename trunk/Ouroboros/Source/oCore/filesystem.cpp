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
#include <oCore/filesystem.h>
#include <oStd/date.h>
#include <oStd/macros.h>
#include "win.h"
#include <io.h>
#include <memory>

namespace oStd
{

const char* as_string(const oCore::filesystem::file_type::value& _Type)
{
	switch (_Type)
	{
		case oCore::filesystem::file_type::block_file: return "block_file";
		case oCore::filesystem::file_type::character_file: return "character_file";
		case oCore::filesystem::file_type::directory_file: return "directory_file";
		case oCore::filesystem::file_type::fifo_file: return "fifo_file";
		case oCore::filesystem::file_type::file_not_found: return "file_not_found";
		case oCore::filesystem::file_type::regular_file: return "regular_file";
		case oCore::filesystem::file_type::socket_file: return "socket_file";
		case oCore::filesystem::file_type::status_unknown: return "status_unknown";
		case oCore::filesystem::file_type::symlink_file: return "symlink_file";
		case oCore::filesystem::file_type::type_unknown: return "type_unknown";
		case oCore::filesystem::file_type::read_only_directory_file: return "read_only_directory_file";
		case oCore::filesystem::file_type::read_only_file: return "read_only_file";
		default: break;
	}
	return "?";
}

} // namespace oStd

using namespace oStd;

#define oFSTHROW0(_ErrCode) throw filesystem_error(make_error_code(std::errc::_ErrCode))
#define oFSTHROW01(_ErrCode, _Path1) throw filesystem_error(_Path1, make_error_code(std::errc::_ErrCode))
#define oFSTHROW02(_ErrCode, _Path1, _Path2) throw filesystem_error(_Path1, _Path2, make_error_code(std::errc::_ErrCode))

#define oFSTHROW(_ErrCode, _Format, ...) do \
{	lstring msg; vsnprintf(msg, _Format, # __VA_ARGS__ ); \
	throw filesystem_error(msg, make_error_code(std::errc::_ErrCode)); \
} while (false)

#define oFSTHROW1(_ErrCode, _Path1, _Format, ...) do \
{	lstring msg; vsnprintf(msg, _Format, # __VA_ARGS__ ); \
	throw filesystem_error(msg, _Path1, make_error_code(std::errc::_ErrCode)); \
} while (false)

#define oFSTHROW2(_ErrCode, _Path1, _Path2, _Format, ...) do \
{	lstring msg; vsnprintf(msg, _Format, # __VA_ARGS__ ); \
	throw filesystem_error(msg, _Path1, _Path2, make_error_code(std::errc::_ErrCode)); \
} while (false)

#define oFSTHROWLAST() throw windows::error()
#define oFSTHROWLAST1(_Path1) throw windows::error()
#define oFSTHROWLAST2(_Path1, _Path2) throw windows::error()

#define oFSTHROW_FOPEN(err, _Path) do \
{	char strerr[256]; \
	strerror_s(strerr, err); \
	throw filesystem_error(strerr, _Path, make_error_code((std::errc::errc)err)); \
} while (false)

#define oFSTHROW_FOPEN0() do \
{	errno_t err = 0; \
	_get_errno(&err); \
	char strerr[256]; \
	strerror_s(strerr, err); \
	throw filesystem_error(strerr, make_error_code((std::errc::errc)err)); \
} while (false)

namespace oCore {
	namespace filesystem {

static bool is_dot(const char* _Filename)
{
	return (!strcmp("..", _Filename) || !strcmp(".", _Filename));
}

path app_path(bool _IncludeFilename)
{
	char Path[MAX_PATH];
	DWORD len = GetModuleFileNameA(GetModuleHandle(nullptr), Path, oCOUNTOF(Path));
	if (!len)
	{
		if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
			oTHROW0(no_buffer_space);
		oTHROW0(operation_not_permitted);
	}

	if (!_IncludeFilename)
	{
		char* p = Path + len - 1;
		while (*p != '\\' && p >= Path)
			p--;
		if (p < Path)
			oTHROW0(operation_not_permitted);
		*(++p) = '\0';
	}

	return Path;
}

path temp_path(bool _IncludeFilename)
{
	char Path[MAX_PATH+1];
	if (!GetTempPathA(oCOUNTOF(Path), Path))
		oTHROW0(operation_not_permitted);
	if (_IncludeFilename && !GetTempFileNameA(Path, "tmp", 0, Path))
			oFSTHROWLAST();
	return Path;
}

path desktop_path()
{
	char Path[MAX_PATH];
	if (!SHGetSpecialFolderPathA(nullptr, Path, CSIDL_DESKTOPDIRECTORY/*CSIDL_COMMON_DESKTOPDIRECTORY*/, FALSE))
		oTHROW0(operation_not_permitted);
	if (strlcat(Path, "\\") >= oCOUNTOF(Path))
		oTHROW0(no_buffer_space);
	return Path;
}

path system_path()
{
	char Path[MAX_PATH];
	UINT len = GetSystemDirectoryA(Path, oCOUNTOF(Path));
	if (!len)
		oTHROW0(operation_not_permitted);
	if (len > (MAX_PATH-2))
		oTHROW0(no_buffer_space);
	Path[len] = '\\';
	Path[len+1] = '\0';
	return Path;
}

path os_path()
{
	char Path[MAX_PATH];
	UINT len = GetWindowsDirectoryA(Path, oCOUNTOF(Path));
	if (!len)
		oTHROW0(operation_not_permitted);
	if (len > (MAX_PATH-2))
		oTHROW0(no_buffer_space);
	Path[len] = '\\';
	Path[len+1] = '\0';
	return Path;
}

path dev_path()
{
	path Root = app_path();
	while (1)
	{
		path leaf = Root.filename();
		if (!_stricmp("bin", leaf))
			return Root.remove_filename();
		Root.remove_filename();
		if (Root.empty())
			oTHROW0(no_such_file_or_directory );
	} while (!Root.empty());
}

path data_path()
{
	path Data;
	try { Data = dev_path(); }
	catch (filesystem_error&) { Data = app_path(); }
	Data /= "data/";

	return Data;
}

path current_path()
{
	char Path[MAX_PATH];
	if (!GetCurrentDirectoryA(oCOUNTOF(Path), Path))
		oTHROW0(operation_not_permitted);
	
	return Path;
}

void current_path(const path& _Path)
{
	if (!SetCurrentDirectory(_Path))
		oFSTHROWLAST1(_Path);
}

path resolve(const path& _RelativePath)
{
	#define APPEND() \
		AbsolutePath /= _RelativePath; \
		if (exists(AbsolutePath)) return AbsolutePath;

	path AbsolutePath = desktop_path();
	APPEND();
	AbsolutePath = temp_path();
	APPEND();
	AbsolutePath = system_path();
	APPEND();
	AbsolutePath = os_path();
	APPEND();
	AbsolutePath = data_path();
	APPEND();
	AbsolutePath = app_path();
	APPEND();

	oFSTHROW01(no_such_file_or_directory, _RelativePath);
}

static file_status status_internal(DWORD _dwFileAttributes)
{
	if (_dwFileAttributes == INVALID_FILE_ATTRIBUTES)
		return file_status(file_type::file_not_found);

	if (_dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
	{
		if (_dwFileAttributes & FILE_ATTRIBUTE_READONLY)
			return file_status(file_type::read_only_directory_file);
		return file_status(file_type::directory_file);
	}
	
	if (_dwFileAttributes & FILE_ATTRIBUTE_READONLY)
		return file_status(file_type::read_only_file);
	
	// todo: add determination if a symlink_file
	// what are block/character file-s on Windows?

	return file_status(file_type::regular_file);
}

file_status status(const path& _Path)
{
	return status_internal(GetFileAttributesA(_Path));
}

bool exists(const path& _Path)
{
	return INVALID_FILE_ATTRIBUTES != GetFileAttributesA(_Path);
}

void read_only(const path& _Path, bool _ReadOnly)
{
	DWORD dwFileAttributes = GetFileAttributesA(_Path);
	if (dwFileAttributes == INVALID_FILE_ATTRIBUTES)
		oFSTHROW01(no_such_file_or_directory, _Path);

	if (_ReadOnly) dwFileAttributes |= FILE_ATTRIBUTE_READONLY;
	else dwFileAttributes &=~ FILE_ATTRIBUTE_READONLY;

	if (!SetFileAttributesA(_Path, dwFileAttributes))
		oFSTHROWLAST1(_Path);
}

template<typename WIN32_TYPE> static unsigned long long file_size_internal(const WIN32_TYPE& _FileData)
{
	return (_FileData.nFileSizeHigh * (static_cast<unsigned long long>(MAXDWORD) + 1)) 
		+ _FileData.nFileSizeLow;
}

unsigned long long file_size(const path& _Path)
{
	WIN32_FILE_ATTRIBUTE_DATA fd;
	if (!GetFileAttributesExA(_Path, GetFileExInfoStandard, &fd))
		oFSTHROWLAST1(_Path);
	return file_size_internal(fd);
}

time_t last_write_time(const path& _Path)
{
	WIN32_FILE_ATTRIBUTE_DATA fad;
	if (0 == GetFileAttributesExA(_Path, GetFileExInfoStandard, &fad))
		oFSTHROWLAST1(_Path);
	return date_cast<time_t>(fad.ftLastWriteTime);
}

void last_write_time(const path& _Path, time_t _Time)
{
	HANDLE hFile = CreateFileA(_Path, FILE_WRITE_ATTRIBUTES, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (hFile == INVALID_HANDLE_VALUE)
		oFSTHROWLAST1(_Path);
	finally CloseFile([&] { CloseHandle(hFile); });
	FILETIME time = date_cast<FILETIME>(_Time);
	if (!SetFileTime(hFile, nullptr, nullptr, &time))
		oFSTHROWLAST1(_Path);
}

bool remove_filename(const path& _Path)
{
	return !!DeleteFileA(_Path);
}

bool remove_directory(const path& _Path)
{
	int Retried = 0;
Retry:

	if (!RemoveDirectory(_Path))
	{
		HRESULT hr = GetLastError();
		switch (hr)
		{
			case ERROR_DIR_NOT_EMPTY:
			{
				// if a file explorer is open, this may be a false positive
				this_thread::sleep_for(chrono::milliseconds(100));
				if (Retried > 1)
					oFSTHROW01(directory_not_empty, _Path);
				Retried++;
				goto Retry;
			}
			case ERROR_SHARING_VIOLATION: oFSTHROW01(device_or_resource_busy, _Path);
			default: break;
		}
		oFSTHROWLAST1(_Path);
	}

	return true;
}

unsigned int remove_all(const path& _Path)
{
	if (is_directory(_Path))
	{
		path wildcard(_Path);
		wildcard /= "*";
		unsigned int removed = 0;
		enumerate(wildcard, [&](const path& _FullPath, const file_status& _Status, unsigned long long _Size)->bool
		{
			if (!is_directory(_Status))
				removed += remove_filename(_FullPath) ? 1 : 0;
			return true;
		});

		enumerate(wildcard, [&](const path& _FullPath, const file_status& _Status, unsigned long long _Size)->bool
		{
			if (is_directory(_Status))
				removed += remove_all(_FullPath);
			return true;
		});

		removed += remove_directory(_Path) ? 1 : 0;
		return removed;
	}

	return remove_filename(_Path) ? 1 : 0;
}

void copy_file(const path& _From, const path& _To, copy_option::value _Option)
{
	if (!CopyFileA(_From, _To, _Option == copy_option::fail_if_exists))
		oFSTHROWLAST2(_From, _To);
}

unsigned int copy_all(const path& _From, const path& _To, copy_option::value _Option)
{
	if (is_directory(_From))
	{
		unsigned int copied = 0;
		file_status ToStatus = status(_To);

		if (!exists(ToStatus))
			create_directory(_To);

		else if (!is_directory(ToStatus))
			oFSTHROW02(is_a_directory, _From, _To);

		path wildcard(_From);
		wildcard /= "*";
		path FullTo(_To);
		FullTo /= "*"; // put a "filename" so it can be replaced below
		enumerate(wildcard, [&](const path& _FullPath, const file_status& _Status, unsigned long long _Size)->bool
		{
			FullTo.replace_filename(_FullPath.filename());

			if (is_directory(_Status))
			{
				copied += copy_all(_FullPath, FullTo, _Option);
			}

			else
			{
				copy_file(_FullPath, FullTo, _Option);
				copied++;
			}
			
			return true;
		});

		return copied;
	}
	copy_file(_From, _To, _Option);
	return 1;
}

void rename(const path& _From, const path& _To, copy_option::value _Option)
{
	if (!exists(_From))
		oFSTHROW01(no_such_file_or_directory, _From);

	if (_Option == copy_option::overwrite_if_exists && exists(_To))
		remove_all(_To);

	MoveFileA(_From, _To);
}

bool create_directory(const path& _Path)
{
	if (!CreateDirectoryA(_Path, nullptr))
	{
		HRESULT hr = GetLastError();
		switch (hr)
		{
			case ERROR_ALREADY_EXISTS: return false;
			case ERROR_PATH_NOT_FOUND: oFSTHROW01(no_such_file_or_directory, _Path); break;
			default: oFSTHROWLAST();
		}
	}

	return true;
}

static bool create_directories_internal(const path& _Path)
{
	if (!CreateDirectoryA(_Path, nullptr))
	{
		HRESULT hr = GetLastError();
		switch (hr)
		{
			case ERROR_ALREADY_EXISTS: return false;
			case ERROR_PATH_NOT_FOUND:
			{
				path parent = _Path.parent_path();

				if (!create_directories_internal(parent))
					return false; // pass thru error message

				// Now try again
				return create_directory(_Path);
			}
			default: oFSTHROWLAST1(_Path);
		}
	}

	return true;
}

bool create_directories(const path& _Path)
{
	return create_directories_internal(_Path);
}

// This returns information for the volume containing _Path
space_info space(const path& _Path)
{
	space_info s;
	if (!GetDiskFreeSpaceEx(_Path, (PULARGE_INTEGER)&s.available, (PULARGE_INTEGER)&s.capacity, (PULARGE_INTEGER)&s.free))
		oFSTHROWLAST1(_Path);
	return s;
}

// Enumerates all entries matching _WildcardPath
void enumerate(const path& _WildcardPath
	, const std::function<bool(const path& _FullPath
		, const file_status& _Status
		, unsigned long long _Size)>& _Enumerator)
{
	WIN32_FIND_DATA fd;
	HANDLE hFind = FindFirstFile(_WildcardPath, &fd);
	finally CloseSearch([&](){ if (hFind != INVALID_HANDLE_VALUE) FindClose(hFind); });

	if (hFind != INVALID_HANDLE_VALUE)
	{
		path resolved(_WildcardPath);

		while (true)
		{
			if (!is_dot(fd.cFileName))
			{
				file_status status = status_internal(fd.dwFileAttributes);
				resolved.replace_filename(fd.cFileName);
				if (!_Enumerator(resolved, status, file_size_internal(fd)))
					break;
			}

			if (!FindNextFile(hFind, &fd))
				break;
		}
	}
}

void* map(const path& _Path
	, bool _ReadOnly
	, unsigned long long _Offset
	, unsigned long long _Size)
{
	HANDLE hFile = CreateFileA(_Path, _ReadOnly ? GENERIC_READ : (GENERIC_READ|GENERIC_WRITE), 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (hFile == INVALID_HANDLE_VALUE)
		oFSTHROWLAST();
	finally CloseFile([&] { CloseHandle(hFile); });

	SYSTEM_INFO si;
	GetSystemInfo(&si);
	byte_swizzle64 alignedOffset;
	alignedOffset.as_unsigned_long_long = byte_align_down(_Offset, si.dwAllocationGranularity);
	unsigned long long offsetPadding = _Offset - alignedOffset.as_unsigned_long_long;
	unsigned long long alignedSize = _Size + offsetPadding;

	DWORD fProtect = _ReadOnly ? PAGE_READONLY : PAGE_READWRITE;
	HANDLE hMapped = CreateFileMapping(hFile, nullptr, fProtect, 0, 0, nullptr);
	if (!hMapped)
		oFSTHROWLAST();
	finally CloseMapped([&] { CloseHandle(hMapped); });

	oCHECK_SIZE(SIZE_T, alignedSize);
	void* p = MapViewOfFile(hMapped, fProtect == PAGE_READONLY ? FILE_MAP_READ : FILE_MAP_WRITE, alignedOffset.as_unsigned_int[1], alignedOffset.as_unsigned_int[0], static_cast<SIZE_T>(alignedSize));
	if (!p)
		oFSTHROWLAST();

	// Exit with a ref count of 1 on the underlying HANDLE, held by MapViewOfFile
	// so the file is fully closed when unmap is called. Right now the count is at 
	// 3, but the finallies will remove 2.
	oCHECK_SIZE(size_t, alignedSize);
	return byte_add(p, static_cast<size_t>(offsetPadding));
}

void unmap(void* _MappedPointer)
{
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	void* p = byte_align_down(_MappedPointer, si.dwAllocationGranularity);
	if (!UnmapViewOfFile(p))
		oFSTHROWLAST();
}

file_handle open(const path& _Path, open_option::value _OpenOption)
{
	static char* opt = "rwa";

	char open[3];
	open[0] = opt[_OpenOption % 3];
	open[1] = _OpenOption >= open_option::text_read ? 't' : 'b';
	open[2] = '\0';

	file_handle hfile = nullptr;
	errno_t err = fopen_s((FILE**)&hfile, _Path, open);
	if (err)
		oFSTHROW_FOPEN(err, _Path);
	return hfile;
}

void close(file_handle _hFile)
{
	if (!_hFile)
		throw filesystem_error(make_error_code(std::errc::invalid_argument));
	if (0 != fclose((FILE*)_hFile))
		oFSTHROW_FOPEN0();
}

bool at_end(file_handle _hFile)
{
	return !!feof((FILE*)_hFile);
}

void last_write_time(file_handle _hFile, time_t _Time)
{
	HANDLE hFile = (HANDLE)_get_osfhandle(_fileno(_hFile));
	if (hFile == INVALID_HANDLE_VALUE)
		oTHROW0(no_such_file_or_directory);
	FILETIME time = date_cast<FILETIME>(_Time);
	oVB(SetFileTime(hFile, 0, 0, &time));
}

void seek(file_handle _hFile, long long _Offset, seek_origin::value _Origin)
{ 
	if (-1 == _fseeki64((FILE*)_hFile, _Offset, (int)_Origin))
		oFSTHROW_FOPEN0();
}

unsigned long long file_size(file_handle _hFile)
{
	HANDLE hFile = (HANDLE)_get_osfhandle(_fileno(_hFile));
	if (hFile == INVALID_HANDLE_VALUE)
		oTHROW0(no_such_file_or_directory);
	LARGE_INTEGER fsize;
	oVB(GetFileSizeEx(hFile, &fsize));
	return fsize.QuadPart;
}

unsigned long long tell(file_handle _hFile)
{
	unsigned long long offset = _ftelli64((FILE*)_hFile);
	if (offset == 1L)
		oFSTHROW_FOPEN0();
	return offset;
}

unsigned long long read(file_handle _hFile, void* _pDestination, unsigned long long _SizeofDestination, unsigned long long _ReadSize)
{
	oCHECK_SIZE(size_t, _ReadSize);
	oCHECK_SIZE(size_t, _SizeofDestination);
	size_t bytesRead = fread_s(_pDestination, static_cast<size_t>(_SizeofDestination), 1, static_cast<size_t>(_ReadSize), (FILE*)_hFile);
	if (static_cast<size_t>(_ReadSize) != bytesRead && !at_end(_hFile))
		oFSTHROW_FOPEN0();
	return bytesRead;
}

unsigned long long write(file_handle _hFile, const void* _pSource, unsigned long long _WriteSize, bool _Flush)
{
	oCHECK_SIZE(size_t, _WriteSize);
	size_t bytesWritten = fwrite(_pSource, 1, static_cast<size_t>(_WriteSize), (FILE*)_hFile);
	if (static_cast<size_t>(_WriteSize) != bytesWritten)
		oFSTHROW_FOPEN0();
	if (_Flush)
		fflush((FILE*)_hFile);
	return bytesWritten;
}

void save(const path& _Path, const void* _pSource, size_t _SizeofSource, save_option::value _SaveOption)
{
	open_option::value OpenOption = open_option::binary_read;
	switch (_SaveOption)
	{
		case save_option::text_write: OpenOption = open_option::text_write; break;
		case save_option::text_append: OpenOption = open_option::text_append; break;
		case save_option::binary_write: OpenOption = open_option::binary_write; break;
		case save_option::binary_append: OpenOption = open_option::binary_append; break;
		oNODEFAULT;
	}

	file_handle f = open(_Path, OpenOption);
	finally CloseFile([&] { close(f); });
	write(f, _pSource, _SizeofSource);
}

void delete_buffer(char* _pBuffer)
{
	delete [] _pBuffer;
}

std::shared_ptr<char> load(const path& _Path, size_t* _pSize, load_option::value _LoadOption)
{
	unsigned long long FileSize = file_size(_Path);

	// in case we need a UTF32 nul terminator
	oCHECK_SIZE(size_t, FileSize);
	size_t AllocSize = static_cast<size_t>(FileSize) + (_LoadOption == load_option::text_read ? 4 : 0);

	char* p = new char[AllocSize];
	std::shared_ptr<char> buffer(p, delete_buffer);

	if (_LoadOption == load_option::text_read)
	{
		// put enough nul terminators for a UTF32 or 16 or 8
		p[FileSize+0] = p[FileSize+1] = p[FileSize+2] = p[FileSize+3] = '\0';
	}

	{
		file_handle f = open(_Path, _LoadOption == load_option::text_read ? open_option::text_read : open_option::binary_read);
		finally CloseFile([&] { close(f); });
		if (FileSize != read(f, p, AllocSize, FileSize))
			oTHROW(io_error, "read failed: %s", _Path.c_str());
	}

	// record honest size (tools like FXC crash if any larger size is given)
	if (_pSize)
	{
		oCHECK_SIZE(size_t, FileSize);
		*_pSize = static_cast<size_t>(FileSize);

		if (_LoadOption == load_option::text_read)
		{
			utf_type::value type = utfcmp(p, __min(static_cast<size_t>(FileSize), 512));
			switch (type)
			{
				case utf_type::utf32be: case utf_type::utf32le: *_pSize += 4; break;
				case utf_type::utf16be: case utf_type::utf16le: *_pSize += 2; break;
				case utf_type::ascii: *_pSize += 1; break;
			}
		}
	}

	return std::move(buffer);
}

std::shared_ptr<char> load(const path& _Path, load_option::value _LoadOption)
{
	return std::move(load(_Path, nullptr, _LoadOption));
}

	} // namespace filesystem
} // namespace oCore
