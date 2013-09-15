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
// File system abstraction.
#pragma once
#ifndef oCore_filesystem_h
#define oCore_filesystem_h

#include <oCore/filesystem_error.h>
#include <stdio.h>

namespace oCore {
	namespace filesystem {

/* enum class */ namespace open_option
{ enum value {

	binary_read,
	binary_write,
	binary_append,
	text_read,
	text_write,
	text_append,

};}

/* enum class */ namespace save_option
{ enum value {

	text_write,
	text_append,
	binary_write, 
	binary_append,

};}

/* enum class */ namespace seek_origin
{ enum value {

	set,
	cur,
	end, 

};}

/* enum class */ namespace copy_option
{ enum value {

	fail_if_exists,
	overwrite_if_exists,

};}

/* enum class */ namespace file_type
{ enum value {

	block_file,
	character_file,
	directory_file,
	fifo_file,
	file_not_found,
	regular_file,
	socket_file,
	status_unknown,
	symlink_file,
	type_unknown,
	read_only_directory_file,
	read_only_file,

};}

/* enum class */ namespace symlink_option
{ enum value {
	none,
	no_recurse = none,
	recurse,
};}

struct space_info
{
	unsigned long long available;
	unsigned long long capacity;
	unsigned long long free;
};

class file_status
{
public:
	explicit file_status(file_type::value _FileType = file_type::status_unknown)
		: FileType(_FileType)
	{}

	inline file_type::value type() const { return FileType; }
	inline void type(file_type::value _FileType) { FileType = _FileType; }

private:
	file_type::value FileType;
};

// All paths to folders end with a separator
oStd::path app_path(bool _IncludeFilename = false); // includes executable name
oStd::path temp_path(bool _IncludeFilename = false); // includes a uniquely named file
oStd::path desktop_path();
oStd::path system_path();
oStd::path os_path();
oStd::path dev_path(); // development root path
oStd::path data_path();
oStd::path current_path();
void current_path(const oStd::path& _Path);

// Returns an absolute path by resolving against system path bases in the 
// following order: 
// an authority matching one of: [desktop, temp, system, os, data, app]
// app path
// operating system (os) path
// current path
// paths listed in the env var %PATH%
oStd::path resolve(const oStd::path& _RelativePath);

// Returns the type of the file
file_status status(const oStd::path& _Path);

// marks a file read-only or read-write
void read_only(const oStd::path& _Path, bool _ReadOnly = true);

inline bool is_directory(const file_status& _Status) { return _Status.type() == file_type::directory_file || _Status.type() == file_type::read_only_directory_file; }
inline bool is_directory(const oStd::path& _Path) { return is_directory(status(_Path)); }

inline bool is_read_only_directory(file_status _Status) { return _Status.type() == file_type::read_only_directory_file; }
inline bool is_read_only_directory(const oStd::path& _Path) { return is_read_only_directory(status(_Path)); }

inline bool is_regular(file_status _Status) { return _Status.type() == file_type::regular_file || _Status.type() == file_type::read_only_file; }
inline bool is_regular(const oStd::path& _Path) { return is_regular(status(_Path)); }

inline bool is_read_only_regular(file_status _Status) { return _Status.type() == file_type::read_only_file; }
inline bool is_read_only_regular(const oStd::path& _Path) { return is_read_only_regular(status(_Path)); }

inline bool is_read_only(file_status _Status) { return is_read_only_directory(_Status) || is_read_only_regular(_Status); }
inline bool is_read_only(const oStd::path& _Path) { return is_read_only(status(_Path)); }

inline bool is_symlink(file_status _Status) { return _Status.type() == file_type::symlink_file; }
inline bool is_symlink(const oStd::path& _Path) { return is_symlink(status(_Path)); }

bool exists(const oStd::path& _Path);
inline bool exists(file_status _Status) { return _Status.type() != file_type::status_unknown && _Status.type() != file_type::file_not_found; }

inline bool is_other(file_status _Status) { return exists(_Status) && !is_regular(_Status) && is_read_only_regular(_Status) && !is_directory(_Status) && !is_symlink(_Status); }
inline bool is_other(const oStd::path& _Path) { return is_other(status(_Path)); }

unsigned long long file_size(const oStd::path& _Path);

time_t last_write_time(const oStd::path& _Path);
void last_write_time(const oStd::path& _Path, time_t _Time);

bool remove_filename(const oStd::path& _Path);
inline bool remove(const oStd::path& _Path) { return remove_filename(_Path); }
bool remove_directory(const oStd::path& _Path);

// Supports wildcards, returns number of items removed
unsigned int remove_all(const oStd::path& _Path);

void copy_file(const oStd::path& _From, const oStd::path& _To, copy_option::value _Option = copy_option::fail_if_exists);

// for either files or directories. Returns number of items copied.
unsigned int copy_all(const oStd::path& _From, const oStd::path& _To, copy_option::value _Option = copy_option::fail_if_exists);

// for either files or directories
void rename(const oStd::path& _From, const oStd::path& _To, copy_option::value _Option = copy_option::fail_if_exists);

// Creates the specified directory. This returns false if the directory already 
// exists and throws if this fails for any other reason, such as prerequisite 
// directories missing.
bool create_directory(const oStd::path& _Path);

// Creates any and all intermediate paths to make the final specified path
bool create_directories(const oStd::path& _Path);

// This returns information for the volume containing _Path
space_info space(const oStd::path& _Path);

// Enumerates all files matching _WildcardPath
void enumerate(const oStd::path& _WildcardPath
	, const std::function<bool(const oStd::path& _FullPath
		, const file_status& _Status
		, unsigned long long _Size)>& _Enumerator);

// Enumerates all entries matching _WildcardPath and recurses into 
// sub-directories.
inline void enumerate_recursively(const oStd::path& _WildcardPath
	, const std::function<bool(const oStd::path& _FullPath
		, const file_status& _Status
		, unsigned long long _Size)>& _Enumerator)
{
	enumerate(_WildcardPath, [&](const oStd::path& _Path
		, const file_status& _Status
		, unsigned long long _Size)->bool
	{
		if (is_directory(_Status))
		{
			oStd::path wildcard(_Path);
			wildcard /= _WildcardPath.filename();
			enumerate_recursively(wildcard, _Enumerator);
			return true;
		}

		return _Enumerator(_Path, _Status, _Size);
	});
}

// Memory-maps the specified file, similar to mmap or MapViewOfFile, but with a 
// set of policy constraints to keep its usage simple. Because oFile doesn't
// expose file handles, the specified file is opened and exposed exclusively as 
// the mapped pointer and the file is closed when unmapped. If _ReadOnly is 
// false, the pointer will be mapped write only.
void* map(const oStd::path& _Path
	, bool _ReadOnly
	, unsigned long long _Offset
	, unsigned long long _Size);

// Unmap a memory pointer returned from map().
void unmap(void* _MappedPointer);

// fopen style API
typedef FILE* file_handle;

file_handle open(const oStd::path& _Path, open_option::value _OpenOption);
void close(file_handle _hFile);
bool at_end(file_handle _hFile);
void last_write_time(file_handle _hFile, time_t _Time);
void seek(file_handle _hFile, long long _Offset, seek_origin::value _Origin);
unsigned long long tell(file_handle _hFile);
unsigned long long file_size(file_handle _hFile);

unsigned long long read(file_handle _hFile
	, void* _pDestination
	, unsigned long long _SizeofDestination
	, unsigned long long _ReadSize);

unsigned long long write(file_handle _hFile
	, const void* _pSource
	, unsigned long long _WriteSize
	, bool _Flush = false);

// Writes the entire buffer to the specified file. Open is specified to allow 
// text and write v. append specification.
void save(const oStd::path& _Path, const void* _pSource, size_t _SizeofSource, save_option::value _SaveOption = save_option::text_write);

	} // namespace filesystem
} // namespace oCore

#endif
