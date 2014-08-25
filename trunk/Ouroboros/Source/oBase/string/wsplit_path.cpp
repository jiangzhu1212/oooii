// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.

#include <string.h>

namespace ouro {

static bool is_sep(wchar_t c, bool _Posix) { return c == L'/' || (!_Posix && c == L'\\'); }
static bool is_unc(const wchar_t* path, bool _Posix) { return path && is_sep(path[0], _Posix) && is_sep(path[1], _Posix) && !is_sep(path[2], _Posix) && path[0] == path[1]; }
static bool has_vol(const wchar_t* path, bool _Posix) { return !_Posix && path && path[0] && path[1] == L':';  }

static const wchar_t* find_parentend(const wchar_t* path, bool _Posix, const wchar_t* _Basename, bool _UNC)
{
	const wchar_t* end = _Basename;
	while (end >= path && is_sep(*end, _Posix)) end--;
	if (!_UNC && end > (path+1)) end--;
	while (end > (path+1) && is_sep(*end, _Posix) && is_sep(*(end-1), _Posix)) end--;
	return end;
}

size_t wsplit_path(const wchar_t* path
	, bool _Posix
	, const wchar_t** out_root
	, const wchar_t** out_path
	, const wchar_t** out_parent_path_end
	, const wchar_t** out_basename
	, const wchar_t** out_ext)
{
	*out_root = *out_path = *out_parent_path_end = *out_basename = *out_ext = nullptr;
	const size_t len = wcslen(path);
	const bool unc = is_unc(path, _Posix);
	const wchar_t* NonUNCPath = path;
	if (unc)
	{
		*out_root = path;
		NonUNCPath = path + 2;
	}

	else if (has_vol(path, _Posix))
		*out_root = path;

	const wchar_t* cur = path + len - 1;
	while (cur >= NonUNCPath)
	{
		if (is_sep(*cur, _Posix))
		{
			// *(cur+1) fails with foo/ because it's a valid null basename
			// cur != path fails with "/" because there is no parent
			if (!*out_basename)
			{
				if (is_sep(NonUNCPath[0], _Posix) && !NonUNCPath[1])
					*out_basename = NonUNCPath;
				else
					*out_basename = cur + 1;
				if (*out_ext)
				{
					*out_parent_path_end = find_parentend(path, _Posix, *out_basename, unc);
					return len;
				}
			}
		}

		else if (*cur == L'.' && !*out_ext)
			*out_ext = cur;

		cur--;
	}

	if (*out_ext && !*out_basename && (!(*out_ext)[1] || (*out_ext)[1] == L'.')) // "." or ".."
		*out_ext = nullptr;

	if (*out_basename)
	{
		*out_parent_path_end = find_parentend(path, _Posix, *out_basename, unc);
		if (!**out_basename)
			*out_basename = nullptr;
	}

	return len;
}

}
