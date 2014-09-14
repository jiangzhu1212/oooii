// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.

// Encapsulation of parsing a Universal Resource Identifier (URI) that
// should be compliant with specification: http://tools.ietf.org/html/rfc3986

#pragma once
#include <oString/path.h>
#include <oString/stringize.h>
#include <oString/string_codec.h>
#include <oString/uri_traits.h>
#include <regex>

namespace ouro {
	namespace detail { std::regex& uri_regex(); }

template<typename charT, typename TraitsT>
class basic_uri
{
	typedef unsigned short index_type;
	static const index_type npos = ~0u & 0xffff;
	typedef std::pair<index_type, index_type> index_pair;

public:
	typedef TraitsT traits;
	typedef typename traits::path_traits_type path_traits_type;
	typedef typename traits::path_type path_type;
	typedef typename traits::char_type char_type;
	typedef typename traits::size_type size_type;
	static const size_type Capacity = traits::capacity;
	typedef fixed_string<char_type, 512> string_type;
	typedef unsigned long long hash_type;
	typedef typename string_type::string_piece_type string_piece_type;
	typedef const char_type(&const_array_ref)[Capacity];

	static const char_type* remove_flag() { return (const char_type*)(-1); }

	basic_uri() { clear(); }
	basic_uri(const char_type* _URIReference) { operator=(_URIReference); }
	basic_uri(const char_type* _URIBase, const char_type* _URIRelative) { operator=(_URIRelative); make_absolute(_URIBase); }
	basic_uri(const basic_uri& that) { operator=(that); }
	basic_uri(const path& _Path) { operator=(_Path); }
	const basic_uri& operator=(const basic_uri& that)
	{
		if (this != &that)
		{
			URI = that.URI;
			Scheme = that.Scheme;
			Authority = that.Authority;
			Path = that.Path;
			Query = that.Query;
			Fragment = that.Fragment;
			Hash = that.Hash;
		}
		return *this;
	}

	const basic_uri& operator=(const char_type* _URIReference)
	{
		if (_URIReference)
		{
			URI = _URIReference;
			parse();
		}
		else
			clear();
		return *this;
	}

	const basic_uri& operator=(const path& _Path)
	{
		string_type p(traits::file_scheme_prefix_str());
		const char_type *rootname = nullptr, *path = nullptr, *parentend = nullptr, *base = nullptr, *ext = nullptr;
		tsplit_path<char_type>(p, traits::path_traits_type::posix, &rootname, &path, &parentend, &base, &ext);
		if (!rootname)
			p += traits::sep_str();
		p += (_Path.c_str());
		return operator=(p);
	}

	// modifiers
	void clear()
	{
		URI.clear();
		clear(Scheme);
		clear(Authority);
		clear(Path);
		clear(Query);
		clear(Fragment);
		End = 0;
		Hash = 0;
	}

	void swap(basic_uri& that)
	{
		if (this != &that)
		{
			std::swap(Scheme, that.Scheme);
			std::swap(Authority, that.Authority);
			std::swap(Path, that.Path);
			std::swap(Query, that.Query);
			std::swap(Fragment, that.Fragment);
			std::swap(Hash, that.Hash);
		}
	}

	// If a parameter is not null, it will replace that part of the current 
	// basic_uri. If a parameter is remove_flag(), { that part will be removed.
	// (An empty string sets that part to a valid empty string part.)
	basic_uri& replace(
		const char_type* _Scheme = nullptr
		, const char_type* _Authority = nullptr
		, const char_type* _Path = nullptr
		, const char_type* _Query = nullptr
		, const char_type* _Fragment = nullptr)
	{
		bool reparse = false;
		string_type s;
		if (_Scheme == remove_flag()) { reparse = true; }
		else if (_Scheme)
		{
			s += _Scheme;
			to_lower(std::begin(s), std::end(s));
			s += traits::scheme_str();
		}
		else if (has_scheme())
		{
			s += convert(Scheme);
			s += traits::scheme_str();
		}

		if (traits::is_file_scheme(s))
		{
			s += traits::double_sep_str();

			if (_Authority == remove_flag())
				;
			else if (_Authority)
				s += _Authority;
			else if (has_authority())
				s += convert(Authority);

			if (_Path)
			{
				if (!path_traits_type::is_sep(_Path[0]))
					s += traits::sep_str();
			}
			else if (has_path() && !path_traits_type::is_sep(*(URI.c_str() + Path.first)))
				s += traits::sep_str();
		}
		else if (_Authority == remove_flag()) { reparse = true; }
		else if (_Authority)
		{
			s += traits::double_sep_str();
			s += _Authority;
		}
		else if (has_authority())
		{
			s += traits::double_sep_str();
			s += convert(Authority);
		}

		if (_Path == remove_flag()) { reparse = true; }
		else if (_Path)
		{
			if (!s.empty() && _Path && !path_traits_type::is_sep(*_Path))
				s += traits::sep_str();

			path_type::string_type p;
			clean_path(p, _Path);
			size_t len = s.length();
			percent_encode(s.c_str() + len, s.capacity() - len, p, " ");
		}
		else if (has_path())
			s.append(convert(Path));

		if (_Query == remove_flag()) { reparse = true; }
		else if (_Query)
		{
			s += traits::query_str();
			s += _Query;
		}
		else if (has_query())
		{
			s += traits::query_str();
			s.append(convert(Query));
		}
		
		if (_Fragment == remove_flag()) { reparse = true; }
		else if (_Fragment)
		{
			s += traits::fragment_str();
			s += _Fragment;
		}
		else if (has_fragment())
		{
			s += traits::fragment_str();
			s.append(convert(Fragment));
		}

		if (reparse || !s.empty())
			operator=(s);

		return *this;
	}

	basic_uri& replace_scheme(const char_type* _Scheme = traits::empty_str()) { return replace(_Scheme); }
	basic_uri& replace_authority(const char_type* _Authority = traits::empty_str()) { return replace(nullptr, _Authority); }
	basic_uri& replace_path(const char_type* _Path = traits::empty_str()) { return replace(nullptr, nullptr, _Path); }
	basic_uri& replace_query(const char_type* _Query = traits::empty_str()) { return replace(nullptr, nullptr, nullptr, _Query); }
	basic_uri& replace_fragment(const char_type* _Fragment = traits::empty_str()) { return replace(nullptr, nullptr, nullptr, nullptr, _Fragment); }

	basic_uri& replace_extension(const char_type* _NewExtension = traits::empty_str())
	{
		return replace_path(path().replace_extension(_NewExtension));
	}

	basic_uri& replace_extension_with_suffix(const char_type* _NewSuffix)
	{
		return replace_path(path().replace_extension_with_suffix(_NewSuffix));
	}

	basic_uri& replace_filename(const char_type* _NewFilename = traits::empty_str())
	{
		return replace_path(path().replace_filename(_NewFilename));
	}

	// Returns a basic_uri reference relative to the specified _URIBase. If this basic_uri 
	// does not begin with the base, the returned basic_uri is empty.
	basic_uri& make_absolute(const basic_uri& _Base)
	{
		bool usebasescheme = false;
		bool usebaseauthority = false;
		bool usemergedpath = false;
		bool usebasequery = false;
		path_type mergedpath;

		if (!has_scheme())
		{
			if (!has_authority())
			{
				if (empty(Path))
				{
					mergedpath = _Base.path();
					usemergedpath = true;
					usebasequery = !has_query();
				}
				
				else
				{
					if (!path_traits_type::is_sep(*(URI.c_str() + Path.first)))
					{
						mergedpath = _Base.path();
						mergedpath.remove_filename();
						mergedpath.append(convert(Path));
						usemergedpath = true;
					}
				}
				usebaseauthority = true;
			}
			usebasescheme = true;
		}

		return replace(
			usebasescheme ? (_Base.has_scheme() ? _Base.scheme().c_str() : remove_flag()) : nullptr
			, usebaseauthority ? (_Base.has_authority() ? _Base.authority().c_str() : remove_flag()) : nullptr
			, usemergedpath ? mergedpath.c_str() : nullptr
			, usebasequery ? (_Base.has_query() ? _Base.query().c_str() : remove_flag()) : nullptr);
	}

	basic_uri& make_relative(const basic_uri& _URIBase)
	{
		auto s = scheme(), bs = _URIBase.scheme();
		auto a = authority(), ba = _URIBase.authority();

		// Must have scheme and authority the same to be made relative.
		if (traits::compare(s, bs) || traits::compare(a, ba))
			return *this;

		string_type relpath;
		auto p = path(), bp = _URIBase.path();
		if (!relativize_path(relpath, bp, p))
			return *this;

		return replace(remove_flag(), remove_flag(), relpath);
	}

	// decomposition
	const_array_ref c_str() const { return (const_array_ref)URI; }
	operator const char_type*() const { return URI; }
	operator string_type() const { return URI; }

	string_piece_type scheme_piece() const { return convert(Scheme); }
	string_piece_type authority_piece() const { return convert(Authority); }
	string_piece_type path_piece() const { return convert(Path); }
	string_piece_type query_piece() const { return convert(Query); }
	string_piece_type fragment_piece() const { return convert(Fragment); }

	string_type scheme() const { return string_type(convert(Scheme)); }
	string_type authority() const { return string_type(convert(Authority)); }
	string_type path_string() const { string_type s(convert(Path)); percent_decode(s, s.capacity(), s); return s; }
	string_type path_encoded() const { return string_type(convert(Path)); }
	path_type path() const { return path_type(path_string()); }
	string_type query() const { return string_type(convert(Query)); }
	string_type fragment() const { return string_type(convert(Fragment)); }

	// culls the fragment
	basic_uri document() const { return has_fragment() ? basic_uri(URI.c_str(), URI.c_str() + Fragment.first - 1) : *this; }

	// todo: implement user_info, host, port

	// query
	/* constexpr */ size_type capacity() const { return Capacity; }
	bool empty() const { return URI.empty(); }
	hash_type hash() const { return Hash; }
	bool valid() const { return has_scheme() || has_authority() || has_path() || has_query() || has_fragment(); }
	bool relative() const { return !absolute(); }
	bool absolute() const { return has_scheme() && empty(Fragment); }
	operator bool() const { return valid(); }
	operator size_type() const { return static_cast<size_type>(hash()); }
	bool has_scheme() const { return !empty(Scheme); }
	bool has_authority() const { return !empty(Authority); }
	bool has_path() const { return !empty(Path); }
	bool has_query() const { return !empty(Query); }
	bool has_fragment() const { return !empty(Fragment); }

	// Compares the scheme, authority, path and query, (ignores fragment) NOTE: 
	// a.is_same_docment(b) != b.is_same_document(a) The "larger" more explicit 
	// document should be the parameter and this should hold the smaller/relative
	// part. http://tools.ietf.org/html/rfc3986#section-4.4
	bool is_same_document(const basic_uri& that) const
	{
		if (empty() || that.empty() || URI[0] == *traits::fragment_str() || that.URI[0] == *traits::fragment_str())
			return true;
		uri tmp(*this, that);
		return (!traits::compare(tmp.scheme(), that.scheme())
			&& !traits::compare(tmp.authority(), that.authority())
			&& !traits::compare(tmp.path(), that.path())
			&& !traits::compare(tmp.query(), that.query()));
	}

	int compare(const basic_uri& that) const
	{
		if (Hash == that.Hash)
		{
			#ifdef _DEBUG
				if (traits::compare(URI, that.URI))
				{
					char msg[oMAX_URI*2 + 64];
					snprintf(msg, "has collision between \"%s\" and \"%s\"", URI.c_str(), that.URI.c_str());
					throw std::logic_error(msg);
				}
			#endif
			return 0;
		}
		return traits::compare(URI, that.URI);
	}

	bool operator==(const basic_uri& that) const { return compare(that) == 0; }
	bool operator!=(const basic_uri& that) const { return !(*this == that); }
	bool operator<(const basic_uri& that) const { return compare(that) < 0; }

private:

	fixed_string<char_type, Capacity> URI;
	index_pair Scheme, Authority, Path, Query, Fragment;
	index_type End;
	hash_type Hash;

	template<typename BidirIt>
	index_pair convert(const std::sub_match<BidirIt>& _Submatch) const
	{
		return index_pair(
			static_cast<index_type>(_Submatch.first - URI.c_str())
			, static_cast<index_type>(_Submatch.second - URI.c_str()));
	}

	static bool empty(const index_pair& _Pair) { return _Pair.first == _Pair.second; }

	string_piece_type convert(const index_pair& _Pair) const
	{
		return string_piece_type(URI.c_str() + _Pair.first, URI.c_str() + _Pair.second);
	}

	void clear(index_pair& _Pair) { _Pair.first = _Pair.second = 0; }

	void parse()
	{
		const std::regex& reURI = detail::uri_regex();
		std::match_results<const char_type*> matches;
		std::regex_search(URI.c_str(), matches, reURI);
		if (matches.empty())
			throw std::invalid_argument("invalid basic_uri");

		// apply good practices from http://www.textuality.com/tag/uri-comp-2.html
		bool hadprefix = matches[2].matched || matches[4].matched;
		bool hasprefix = hadprefix;
		// Clean path of non-leading . and .. and scrub separators
		if (matches[5].length() && !path_traits_type::is_dot(URI[0]))
		{
			// const-cast of the template type so path cleaning can be done in-place
			std::match_results<char_type*>::value_type& path_match = 
				*(std::match_results<char_type*>::value_type*)&matches[5];

			char_type c = *path_match.second;
			*path_match.second = 0;
			ouro::path_string p;
			clean_path(path_match.first, path_match.length() + 1, path_match.first); // length() + 1 is for nul
			size_t newLen = string_type::traits::length(path_match.first);
			*path_match.second = c;
			if (newLen != static_cast<size_t>(path_match.length()))
			{
				memmove(path_match.first + newLen, path_match.second, std::distance(matches[5].second, matches[0].second));
				std::regex_search(URI.c_str(), matches, reURI);
				hasprefix = matches[2].matched || matches[4].matched;
			}
		}

		// http://tools.ietf.org/html/rfc3986#appendix-B
		// if there was a prefix change due to path cleaning, it means there was
		// never a prefix
		if (hadprefix != hasprefix)
		{
			Scheme = index_pair();
			Authority = index_pair();
		}
		else
		{
			Scheme = convert(matches[2]);
			Authority = convert(matches[4]);
		}

		Path = convert(matches[5]);
		Query = convert(matches[7]);
		Fragment = convert(matches[9]);
		End = static_cast<index_type>(matches[0].second - matches[0].first);

		// const-cast of the template type so to-lower can be done in-place
		// for now do the whole URI, but at least the scheme and percent encodings
		// should be forced to lower-case.
		std::match_results<char_type*>::value_type& to_lower_range = 
			*(std::match_results<char_type*>::value_type*)&matches[2];

		to_lower(to_lower_range.first, to_lower_range.second);

		// If the whole path isn't made lower-case for case-insensitive purposes, 
		// the percent values at least should be made lower-case (i.e. this should 
		// be true: &7A == &7a)
		percent_to_lower(URI, URI.capacity(), URI);

		Hash = fnv1a<hash_type>(URI);
	}
};

typedef basic_uri<char, uri_traits<char, unsigned long long, default_posix_path_traits<char>>> uri;
typedef basic_uri<wchar_t, uri_traits<wchar_t, unsigned long long, default_posix_path_traits<wchar_t>>> wuri;

template<typename charT, typename TraitsT>
char* to_string(char* _StrDestination, size_t _SizeofStrDestination, const basic_uri<charT, TraitsT>& value)
{
	try { ((const basic_uri<charT, TraitsT>::string_type&)value).copy_to(_StrDestination, _SizeofStrDestination); }
	catch (std::exception&) { return nullptr; }
	return _StrDestination;
}

template<typename charT, typename TraitsT>
bool from_string(basic_uri<charT, TraitsT>* _pValue, const char* _StrSource)
{
	try { *_pValue = _StrSource; }
	catch (std::exception&) { return false; }
	return true;
}

}

namespace std {

template<typename charT, typename TraitsT>
struct hash<ouro::basic_uri<charT, TraitsT>> { std::size_t operator()(const ouro::basic_uri<charT, TraitsT>& _URI) const { return _URI.hash(); } };

}
