// $(header)
#include <oooii/oSTL.h>
#include <oooii/oErrno.h>

errno_t oRegexCopy(char *_StrDestination, size_t _SizeofStrDestination, const std::tr1::cmatch& _Matches, size_t _NthMatch)
{
	if (!_StrDestination || _NthMatch >= _Matches.size()) return EINVAL;
	errno_t err = 0;
	size_t size = _Matches[_NthMatch].length();
	if (size == 0)
		err = ENOENT;
	else if (size >= _SizeofStrDestination)
	{
		err = ERANGE;	
		size = _SizeofStrDestination-1;
	}

	memcpy(_StrDestination, _Matches[_NthMatch].first, size);
	_StrDestination[size] = 0;
	return err;
}

const char* oRegexGetError(std::tr1::regex_constants::error_type _RegexError)
{
	switch (_RegexError)
	{
		case std::tr1::regex_constants::error_badbrace: return "the expression contained an invalid count in a { } expression";
		case std::tr1::regex_constants::error_badrepeat: return "a repeat expression (one of '*', '?', '+', '{' in most contexts) was not preceded by an expression";
		case std::tr1::regex_constants::error_brace: return "the expression contained an unmatched '{' or '}'";
		case std::tr1::regex_constants::error_brack: return "the expression contained an unmatched '[' or ']'";
		case std::tr1::regex_constants::error_collate: return "the expression contained an invalid collating element name";
		case std::tr1::regex_constants::error_complexity: return "an attempted match failed because it was too complex";
		case std::tr1::regex_constants::error_ctype: return "the expression contained an invalid character class name";
		case std::tr1::regex_constants::error_escape: return "the expression contained an invalid escape sequence";
		case std::tr1::regex_constants::error_paren: return "the expression contained an unmatched '(' or ')'";
		case std::tr1::regex_constants::error_range: return "the expression contained an invalid character range specifier";
		case std::tr1::regex_constants::error_space: return "parsing a regular expression failed because there were not enough resources available";
		case std::tr1::regex_constants::error_stack: return "an attempted match failed because there was not enough memory available";
		case std::tr1::regex_constants::error_backref: return "the expression contained an invalid back reference";
		default: break;
	}

	return "unidentified regex error";
}

bool oTryCompilelRegex(std::tr1::regex& _OutRegex, char* _StrError, size_t _SizeofStrError, const char* _StrRegex, std::tr1::regex_constants::syntax_option_type _Flags)
{
	try { _OutRegex = std::tr1::regex(_StrRegex, _Flags); }
	catch (const std::tr1::regex_error& err)
	{
		if (_StrError)
			sprintf_s(_StrError, _SizeofStrError, "%s in \"%s\"", oRegexGetError(err.code()), _StrRegex);
		return false;
	}

	return true;
}
