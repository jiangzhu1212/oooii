// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#pragma once
#ifndef oString_path_h
#define oString_path_h

// Encapsulation of parsing a string representing a file on a local hard drive.

#include <oString/fixed_string.h>
#include <oBase/path_traits.h>
#include <oString/stringize.h>
#include <functional>

namespace ouro {

template<typename charT, typename traitsT = default_posix_path_traits<charT>>
class basic_path
{
public:
	static const size_t Capacity = oMAX_PATH;
	typedef typename traitsT traits;
	typedef typename traits::char_type char_type;
	typedef size_t hash_type;
	typedef size_t size_type;
	typedef path_string string_type;
	typedef std::pair<const char_type*, const char_type*> string_piece_type;

	typedef const char_type(&const_array_ref)[Capacity];

	basic_path() { clear(); }
	basic_path(const char_type* start, const char_type* end) : p(start, end) { parse(); }
	basic_path(const char_type* that) { operator=(that); }
	basic_path(const basic_path& that) { operator=(that); }
	basic_path& operator=(const basic_path& that) { return operator=(that.c_str()); }
	basic_path& operator=(const char_type* that)
	{
		if (that)
		{
			p = that;
			parse();
		}
		else
			clear();
		return *this;
	}

	// modifiers

	void clear()
	{
		p.clear();
		Length = 0;
		RootDirOffset = ParentEndOffset = BasenameOffset = ExtensionOffset = npos;
		HasRootName = EndsWithSep = false;
		Hash = 0;
	}

	basic_path& assign(const char_type* start, const char_type* end) { p.assign(start, end); parse(); return *this; }

	basic_path& append(const char_type* path_element, bool ensure_separator = true)
	{
		if (ensure_separator && !traits::is_sep(*path_element) && !EndsWithSep)
			p.append(traits::generic_sep_str());
		p.append(path_element);
		parse();
		return *this;
	}

	basic_path& append(const char_type* start, const char_type* end, bool ensure_separator = true)
	{
		if (ensure_separator && !traits::is_sep(*start) && !EndsWithSep)
			p.append(traits::generic_sep_str());
		p.append(start, end);
		parse();
		return *this;
	}

	basic_path& append(const string_piece_type& string_piece, bool ensure_separator = true) { return append(string_piece.first, string_piece.second, ensure_separator); }

	void swap(basic_path& that)
	{
		if (this != &that)
		{
			std::swap(p, that.p);
			std::swap(RootDirOffset, that.RootDirOffset);
			std::swap(ParentEndOffset, that.ParentEndOffset);
			std::swap(BasenameOffset, that.BasenameOffset);
			std::swap(ExtensionOffset, that.ExtensionOffset);
			std::swap(HasRootName, that.HasRootName);
			std::swap(EndsWithSep, that.EndsWithSep);
		}
	}

	basic_path operator/(const basic_path& that) const
	{
		basic_path p(*this);
		return p /= that;
	}

	basic_path& operator/=(const char_type* that)
	{
		append(that);
		return *this;
	}

	basic_path& replace_extension(const char_type* new_ext = traits::empty_str())
	{
		if (has_extension()) p[ExtensionOffset] = 0;
		if (new_ext && !traits::is_dot(new_ext[0])) p.append(traits::dot_str());
		p.append(new_ext);
		parse();
		return *this;
	}

	basic_path& replace_extension(const string_type& new_ext)
	{
		return replace_extension(new_ext.c_str());
	}

	basic_path& replace_extension_with_suffix(const char_type* new_ext = traits::empty_str())
	{
		if (has_extension()) p[ExtensionOffset] = 0;
		p.append(new_ext);
		parse();
		return *this;
	}

	basic_path& replace_extension_with_suffix(const string_type& new_ext)
	{
		return replace_extension_with_suffix(new_ext.c_str());
	}

	basic_path& replace_filename(const char_type* new_filename = traits::empty_str())
	{
		if (BasenameOffset != npos)
			p[BasenameOffset] = 0;
		else if (ExtensionOffset != npos)
			p[ExtensionOffset] = 0;
		p.append(new_filename);
		parse();
		return *this;
	}

	basic_path& replace_filename(const string_type& new_filename)
	{
		return replace_filename(new_filename.c_str());
	}

	basic_path& replace_filename(const basic_path& new_filename)
	{
		return replace_filename(new_filename.c_str());
	}

	basic_path& remove_filename()
	{
		if (BasenameOffset != npos)
		{
			p[BasenameOffset] = 0;
			parse();
		}
		else if (EndsWithSep)
		{
			EndsWithSep = false;
			p[p.length()-1] = 0;
			parse();
		}
		return *this;
	}

	// Inserts text after basename and before extension
	basic_path& insert_basename_suffix(const char_type* new_suffix)
	{
		auto tmp = extension();
		if (has_extension())
			p[ExtensionOffset] = 0;
		p.append(new_suffix).append(tmp);
		parse();
		return *this;
	}

	basic_path& insert_basename_suffix(const string_type& new_suffix)
	{
		return insert_basename_suffix(new_suffix.c_str());
	}

	basic_path& remove_basename_suffix(const char_type* suffix)
	{
		if (has_basename() && suffix)
		{
			string_type s = suffix;
			char_type* ext = has_extension() ? &p[ExtensionOffset] : nullptr;
			if (ext) s += ext;
			char_type* suf = ext ? ext : &p[BasenameOffset];
			while (*suf && suf < ext) suf++; // move to end of basename
			suf -= strlen(suffix);
			if (suf >= p.c_str())
			{
				if (!traits::compare(suf, s))
				{
					*suf = 0;
					p.append(ext);
					parse();
				}
			}
		}
	
		return *this;
	}

	basic_path& remove_basename_suffix(const string_type& suffix)
	{
		return remove_basename_suffix(suffix.c_str());
	}
	
	// decomposition
	const_array_ref c_str() const { return (const_array_ref)p; }
	operator const char_type*() const { return p; }
	operator string_type() const { return p; }

	#define NULL_STR_PIECE string_piece_type(p.c_str(), p.c_str())
	string_piece_type string_piece() const { return string_piece_type(p.c_str(), p.c_str() + Length); }
	string_piece_type root_path_piece() const { return has_root_name() ? (has_root_directory() ? string_piece_type(p.c_str(), p.c_str() + RootDirOffset + 1) : string_piece()) : (traits::has_vol(p) ? NULL_STR_PIECE : root_directory_piece()); }
	string_piece_type root_name_piece() const { return HasRootName ? (has_root_directory() ? string_piece_type(p.c_str(), p.c_str() + RootDirOffset) : string_piece()) : NULL_STR_PIECE; }
	string_piece_type root_directory_piece() const { return has_root_directory() ? string_piece_type(p.c_str() + RootDirOffset, p.c_str() + RootDirOffset + 1) : NULL_STR_PIECE; }
	string_piece_type relative_path_piece() const { const char* first = after_seps(p.c_str() + (has_root_directory() ? RootDirOffset + 1 : 0)); return string_piece_type(first, first + Length); }
	string_piece_type parent_path_piece() const { return has_parent_path() ? string_piece_type(p.c_str(), p.c_str() + ParentEndOffset) : NULL_STR_PIECE; }
	string_piece_type basename_piece() const { return has_basename() ? (string_piece_type(p.c_str() + BasenameOffset, has_extension() ? (p.c_str() + ExtensionOffset) : (p.c_str() + Length))) : NULL_STR_PIECE; }
	string_piece_type filename_piece() const { return EndsWithSep ? string_piece_type(traits::dot_str(), traits::dot_str() + 1) : (BasenameOffset != npos ? string_piece_type(p.c_str() + BasenameOffset, p.c_str() + Length) : string_piece()); }
	string_piece_type extension_piece() const { return has_extension() ? string_piece_type(p.c_str() + ExtensionOffset, p.c_str() + Length) : NULL_STR_PIECE; }
	#undef NULL_STR_PIECE

	#define oPATH_API_BY_COPY(fn) basic_path fn() const { string_piece_type sp = fn##_piece(); return basic_path(sp.first, sp.second); }
	oPATH_API_BY_COPY(string)
	oPATH_API_BY_COPY(root_path)
	oPATH_API_BY_COPY(root_name)
	oPATH_API_BY_COPY(root_directory)
	oPATH_API_BY_COPY(relative_path)
	oPATH_API_BY_COPY(parent_path)
	oPATH_API_BY_COPY(basename)
	oPATH_API_BY_COPY(filename)
	oPATH_API_BY_COPY(extension)
	#undef oPATH_API_BY_COPY

	// All elements up to extension
	string_type stem() const { return has_extension() ? (has_basename() ? string_type(p.c_str(), p.c_str() + ExtensionOffset) : string_type()) : p; }
	
	// All elements up to and including the left-most colon
	string_type prefix() const { return traits::has_vol(p) ? string_type(p.c_str(), p.c_str() + 2) : string_type(); }

	// query
	/* constexpr */ size_type capacity() const { return Capacity; }
	bool empty() const { return p.empty(); }
	bool has_root_path() const { return has_root_directory() || has_root_name(); }
	bool has_root_name() const { return HasRootName; }
	bool has_root_directory() const { return RootDirOffset != npos; }
	bool has_relative_path() const { return !p.empty() && has_root_directory() && !traits::is_sep(p[0]) && !p[1]; }
	bool has_parent_path() const { return ParentEndOffset != 0 && ParentEndOffset != npos; }
	bool has_filename() const { return has_basename() || has_extension(); }

	bool is_absolute() const { return (has_root_directory() && !traits::posix && has_root_name());  }
	bool is_windows_absolute() const { return is_absolute() || base_path_traits<char_type, false>::has_vol(p);  } // it is too annoying to live without this
	bool is_relative() const { return !is_absolute(); }
	bool is_windows_relative() const { return !is_windows_absolute(); }
	bool is_unc() const { return traits::is_unc(p); }
	bool is_windows_unc() const { return base_path_traits<char_type, false>::is_unc(p); }
	bool has_basename() const { return BasenameOffset != npos; }
	bool has_extension() const { return ExtensionOffset != npos; }
	bool has_extension(const char_type* _Extension) const { return _Extension && traits::is_dot(*_Extension) && has_extension() ? !traits::compare(&p[ExtensionOffset], _Extension) : false; }

	hash_type hash() const { return Hash; }

	// Returns similar to strcmp/stricmp depending on path_traits. NOTE: A clean
	// and unclean path will not compare to be the same.
	int compare(const basic_path& that) const { return traits::compare(this->c_str(), that.c_str()); }
	int compare(const char_type* that) const { return traits::compare(this->c_str(), that); }

	const basic_path& make_relative(const char* root)
	{
		if (root && *root)
		{
			basic_path relPath;

			size_t CommonLen = cmnroot(p, root);
			if (CommonLen)
			{
				size_t nSeperators = 0;
				size_t index = CommonLen - 1;
				while (root[index])
				{
					if (traits::is_sep(root[index]) && 0 != root[index + 1])
						nSeperators++;
					index++;
				}

				clear();
				for (size_t i = 0; i < nSeperators; i++)
					relPath /= traits::dotdot_str();

				relPath /= (*this).c_str() + CommonLen;
			}
		}
		return *this;
	}

private:
	typedef unsigned short index_type;
	static const index_type npos = ~0u & 0xffff;
	unsigned short idx(const char_type* _PtrIntoP) { return _PtrIntoP ? static_cast<unsigned short>(std::distance((const char_type*)p, _PtrIntoP)) : npos; }

	hash_type Hash;
	string_type p;
	index_type RootDirOffset; // index of first non-root-name slash
	index_type ParentEndOffset; // index of first right-most slash backed up past any redundant slashes
	index_type BasenameOffset; // index of just after right-most slash
	index_type ExtensionOffset; // index of right-most dot
	index_type Length; // or offset of nul terminator
	bool HasRootName;
	bool EndsWithSep;

	static const char_type* after_seps(const char_type* p) { while (*p && traits::is_sep(*p)) p++; return p; }
	static const char_type* to_sep(const char_type* p) { while (*p && !traits::is_sep(*p)) p++; return p; }
	static const char_type* find_root_directory(const char_type* path, const char_type* root_name)
	{
		const char_type* dir = root_name;
		if (dir)
		{
			dir = after_seps(dir); // move past unc seps
			dir = to_sep(dir); // move past unc name
			if (!*dir) dir = nullptr; // if after-unc sep, then no rootdir
		}

		else if (traits::is_sep(path[0])) // rootdir is has_root_name() || is_sep(p[0])
			dir = path;
		return dir;
	}

	static size_t common_base_length(const char* path1, const char* path2)
	{
		size_t last_sep = 0;
		size_t len = 0;
		if (path1 && *path1 && path2 && *path2)
		{
			while (path1[len] && path2[len])
			{
				char_type a = tolower(path1[len]);
				char_type b = tolower(path2[len]);

				if (a != b || traits::is_sep(a) != traits::is_sep(b))
					break;

				if (traits::is_sep(a) || traits::is_sep(b))
					last_sep = len;
			
				len++;
			}
		}

		return last_sep;
	}

	void parse()
	{
		if (traits::always_clean) clean_path(p, p);
		const char_type *rootname = nullptr, *path = nullptr, *parentend = nullptr, *base = nullptr, *ext = nullptr;
		Length = static_cast<index_type>(tsplit_path<char_type>(p, traits::posix, &rootname, &path, &parentend, &base, &ext));
		HasRootName = !!rootname;
		BasenameOffset = idx(base);
		ExtensionOffset = idx(ext);
		RootDirOffset = idx(find_root_directory(p, rootname));
		ParentEndOffset = idx(parentend);
		EndsWithSep = Length > 1 && traits::is_sep(p[Length-1]);
		Hash = fnv1a<hash_type>(p);
	}
};

typedef basic_path<char> posix_path;
typedef basic_path<wchar_t> posix_wpath;

typedef basic_path<char, default_windows_path_traits<char>> windows_path;
typedef basic_path<wchar_t, default_windows_path_traits<wchar_t>> windows_wpath;

typedef posix_path path;
typedef posix_wpath wpath;

template<typename charT, typename traitsT>
char* to_string(char* dst, size_t dst_size, const basic_path<charT, traitsT>& value)
{
	try { ((const basic_path<charT, traitsT>::string_type&)value).copy_to(dst, dst_size); }
	catch (std::exception&) { return nullptr; }
	return dst;
}

template<typename charT, typename traitsT>
bool from_string(basic_path<charT, traitsT>* out_value, const char* src)
{
	try { *out_value = src; }
	catch (std::exception&) { return false; }
	return true;
}

}

namespace std {

template<typename charT, typename traitsT>
struct hash<ouro::basic_path<charT, traitsT>> { std::size_t operator()(const ouro::basic_path<charT, traitsT>& path) const { return path.hash(); } };

} // namespace std

#endif
