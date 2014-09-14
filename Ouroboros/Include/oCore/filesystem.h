// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.

// File system abstraction.

#pragma once
#include <oMemory/allocate.h>
#include <oString/path.h>
#include <functional>
#include <cstdio>
#include <memory>
#include <system_error>

namespace ouro { namespace filesystem {

template<typename charT, typename TraitsT = default_posix_path_traits<charT>>
class basic_filesystem_error : public std::system_error
{
public:
	typedef basic_path<charT, TraitsT> path_type;

	basic_filesystem_error() {}
	~basic_filesystem_error() {}
	basic_filesystem_error(const basic_filesystem_error<charT, TraitsT>& _That)
		: std::system_error(_That)
		, Path1(_That.Path1)
		, Path2(_That.Path2)
	{}

	basic_filesystem_error(std::error_code _ErrorCode)
		: system_error(_ErrorCode, _ErrorCode.message())
	{}

	basic_filesystem_error(const path_type& _Path1, std::error_code _ErrorCode)
		: system_error(_ErrorCode, _ErrorCode.message())
		, Path1(_Path1)
	{}

	basic_filesystem_error(const path_type& _Path1, const path_type& _Path2, std::error_code _ErrorCode)
		: system_error(_ErrorCode, _ErrorCode.message())
		, Path1(_Path1)
		, Path2(_Path2)
	{}

	basic_filesystem_error(const char* _Message, std::error_code _ErrorCode)
		: system_error(_ErrorCode, _Message)
	{}

	basic_filesystem_error(const char* _Message, const path_type& _Path1, std::error_code _ErrorCode)
		: system_error(_ErrorCode, _Message)
		, Path1(_Path1)
	{}

	basic_filesystem_error(const char* _Message, const path_type& _Path1, const path_type& _Path2, std::error_code _ErrorCode)
		: system_error(_ErrorCode, _Message)
		, Path1(_Path1)
		, Path2(_Path2)
	{}
	
	inline const path_type& path1() const { return Path1; }
	inline const path_type& path2() const { return Path2; }

private:
	path_type Path1;
	path_type Path2;
};

typedef basic_filesystem_error<char> filesystem_error;
typedef basic_filesystem_error<wchar_t> wfilesystem_error;

typedef basic_filesystem_error<char, default_windows_path_traits<char>> windows_filesystem_error;
typedef basic_filesystem_error<wchar_t, default_windows_path_traits<wchar_t>> windows_wfilesystem_error;

const std::error_category& filesystem_category();

/*constexpr*/ inline std::error_code make_error_code(std::errc::errc _Errc) { return std::error_code(static_cast<int>(_Errc), filesystem_category()); }
/*constexpr*/ inline std::error_condition make_error_condition(std::errc::errc _Errc) { return std::error_condition(static_cast<int>(_Errc), filesystem_category()); }

/* enum class */ namespace open_option
{ enum value {

	binary_read,
	binary_write,
	binary_append,
	text_read,
	text_write,
	text_append,

};}

/* enum class */ namespace load_option
{ enum value {

	text_read,
	binary_read,

};}

/* enum class */ namespace save_option
{ enum value {

	text_write,
	text_append,
	binary_write, 
	binary_append,

};}

/* enum class */ namespace map_option
{ enum value {

	binary_read,
	binary_write,

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
path app_path(bool _IncludeFilename = false); // includes executable name
path temp_path(bool _IncludeFilename = false); // includes a uniquely named file
path log_path(bool _IncludeFilename = false, const char* _ExeSuffix = nullptr); // includes a name based on the app name, date and process id
path desktop_path();
path system_path();
path os_path();
path dev_path(); // development root path
path data_path();
path current_path();
void current_path(const path& _Path);

// calls one of the above functions based on _RootName being:
// [app temp log desktop system os dev data current]
path root_path(const char* _RootName);

// Returns an absolute path by resolving against system path bases in the 
// following order: 
// an authority matching one of: [app current system os desktop data temp]
// if no above match, then search the environment variable PATH.
path resolve(const path& _RelativePath);

// Returns the type of the file
file_status status(const path& _Path);

// marks a file read-only or read-write
void read_only(const path& _Path, bool _ReadOnly = true);

inline bool is_directory(const file_status& _Status) { return _Status.type() == file_type::directory_file || _Status.type() == file_type::read_only_directory_file; }
inline bool is_directory(const path& _Path) { return is_directory(status(_Path)); }

inline bool is_read_only_directory(file_status _Status) { return _Status.type() == file_type::read_only_directory_file; }
inline bool is_read_only_directory(const path& _Path) { return is_read_only_directory(status(_Path)); }

inline bool is_regular(file_status _Status) { return _Status.type() == file_type::regular_file || _Status.type() == file_type::read_only_file; }
inline bool is_regular(const path& _Path) { return is_regular(status(_Path)); }

inline bool is_read_only_regular(file_status _Status) { return _Status.type() == file_type::read_only_file; }
inline bool is_read_only_regular(const path& _Path) { return is_read_only_regular(status(_Path)); }

inline bool is_read_only(file_status _Status) { return is_read_only_directory(_Status) || is_read_only_regular(_Status); }
inline bool is_read_only(const path& _Path) { return is_read_only(status(_Path)); }

inline bool is_symlink(file_status _Status) { return _Status.type() == file_type::symlink_file; }
inline bool is_symlink(const path& _Path) { return is_symlink(status(_Path)); }

bool exists(const path& _Path);
inline bool exists(file_status _Status) { return _Status.type() != file_type::status_unknown && _Status.type() != file_type::file_not_found; }

inline bool is_other(file_status _Status) { return exists(_Status) && !is_regular(_Status) && is_read_only_regular(_Status) && !is_directory(_Status) && !is_symlink(_Status); }
inline bool is_other(const path& _Path) { return is_other(status(_Path)); }

unsigned long long file_size(const path& _Path);

time_t last_write_time(const path& _Path);
void last_write_time(const path& _Path, time_t _Time);

bool remove_filename(const path& _Path);
inline bool remove(const path& _Path) { return remove_filename(_Path); }
bool remove_directory(const path& _Path);

// Supports wildcards, returns number of items removed
unsigned int remove_all(const path& _Path);

void copy_file(const path& _From, const path& _To, copy_option::value _Option = copy_option::fail_if_exists);

// for either files or directories. Returns number of items copied.
unsigned int copy_all(const path& _From, const path& _To, copy_option::value _Option = copy_option::fail_if_exists);

// for either files or directories
void rename(const path& _From, const path& _To, copy_option::value _Option = copy_option::fail_if_exists);

// Creates the specified directory. This returns false if the directory already 
// exists and throws if this fails for any other reason, such as prerequisite 
// directories missing.
bool create_directory(const path& _Path);

// Creates any and all intermediate paths to make the final specified path
bool create_directories(const path& _Path);

// This returns information for the volume containing _Path
space_info space(const path& _Path);

// Enumerates all files matching _WildcardPath
void enumerate(const path& _WildcardPath
	, const std::function<bool(const path& _FullPath
		, const file_status& _Status
		, unsigned long long _Size)>& _Enumerator);

// Enumerates all entries matching _WildcardPath and recurses into 
// sub-directories.
inline void enumerate_recursively(const path& _WildcardPath
	, const std::function<bool(const path& _FullPath
		, const file_status& _Status
		, unsigned long long _Size)>& _Enumerator)
{
	enumerate(_WildcardPath, [&](const path& _Path
		, const file_status& _Status
		, unsigned long long _Size)->bool
	{
		if (is_directory(_Status))
		{
			path wildcard(_Path);
			wildcard /= _WildcardPath.filename();
			enumerate_recursively(wildcard, _Enumerator);
			return true;
		}

		return _Enumerator(_Path, _Status, _Size);
	});
}

// Memory-maps the specified file, similar to mmap or MapViewOfFile, but with a 
// set of policy constraints to keep its usage simple. The specified path is 
// opened and exposed solely as the mapped pointer and the file is closed when 
// unmapped.
void* map(const path& _Path
	, map_option::value _MapOption
	, unsigned long long _Offset
	, unsigned long long _Size);

// Unmap a memory pointer returned from map().
void unmap(void* _MappedPointer);

// fopen style API
typedef FILE* file_handle;
typedef void* native_file_handle;

// Converts a standard C file handle from fopen to an OS-native handle
// (HANDLE on Windows)
native_file_handle get_native(file_handle _hFile);

file_handle open(const path& _Path, open_option::value _OpenOption);
void close(file_handle _hFile);
bool at_end(file_handle _hFile);
path get_path(file_handle _hFile);
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

class scoped_file
{
public:
	scoped_file() : h(nullptr) {}
	scoped_file(const path& _Path, open_option::value _OpenOption) : h(nullptr) { if (!_Path.empty()) h = open(_Path, _OpenOption); }
	scoped_file(file_handle _hFile) : h(_hFile) {}
	scoped_file(scoped_file&& _That) { operator=(std::move(_That)); }
	scoped_file& operator=(scoped_file&& _That)
	{
		if (this != &_That) { if (h) close(h); h = _That.h; _That.h = nullptr; }
		return *this;
	}

	~scoped_file() { if (h) close(h); }

	operator bool() const { return !!h; }
	operator file_handle() const { return h; }

private:
	file_handle h;

	scoped_file(const scoped_file&);
	const scoped_file& operator=(const scoped_file&);
};

// Writes the entire buffer to the specified file. Open is specified to allow 
// text and write v. append specification.
void save(const path& _Path, const void* _pSource, size_t _SizeofSource, save_option::value _SaveOption = save_option::binary_write);

// Allocates the size of the file reads it into memory and returns that buffer.
// If loaded as text, the allocation will be padded and a nul terminator will be
// added so the buffer can immediately be used as a string.
scoped_allocation load(const path& _Path, load_option::value _LoadOption = load_option::binary_read, const allocator& _Allocator = default_allocator);
inline scoped_allocation load(const path& _Path, const allocator& _Allocator) { return load(_Path, load_option::binary_read, _Allocator); }

// Similar to load, this will load the complete file into an allocated buffer but
// the open, allocate, read and close occurs in another thread as well as the 
// _OnComplete so ensure its implementation is thread safe. The scoped_allocation
// can either be moved in the _OnComplete or left alone where the system will
// auto-free it when the _OnCompletion goes out of scope. In success cases _pError 
// will be nullptr, but if it is valid then _Buffer is invalid.
void load_async(const path& _Path
								, const std::function<void(const path& _Path, scoped_allocation& _Buffer, const std::system_error* _pError)>& _OnComplete
								, load_option::value _LoadOption = load_option::binary_read
								, const allocator& _Allocator = default_allocator);

// Similar to save, this will save the specified buffer call _OnComplete. The open
// write and close will occur on another thread so ensure _OnComplete's implementation
// is thread safe. The scoped_allocation must be moved initially, but will resurface
// in _OnComplete at which point the application can grab it again or allow it to 
// go out of scope and free itself. In success cases _pError is nullptr but if it is 
// valid then _Buffer is invalid.
void save_async(const path& _Path
								, scoped_allocation&& _Buffer
								, const std::function<void(const path& _Path, scoped_allocation& _Buffer, const std::system_error* _pError)>& _OnComplete
								, save_option::value _SaveOption = save_option::binary_write);

// Waits until all async operations have completed
void wait();
bool wait_for(unsigned int _TimeoutMS);

// returns if IO threads are running
bool joinable();

// waits for all associated IO operations to complete then joins all IO threads. If load_async is used
// this must be called before the application ends.
void join();

}}
