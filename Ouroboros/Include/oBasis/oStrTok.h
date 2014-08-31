// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#pragma once
#ifndef oStrTok_h
#define oStrTok_h

#include <functional>

// _____________________________________________________________________________
// String tokenization

// Like strtok_s, except you can additionally specify open and close scope chars 
// to ignore. For example:
// char* ctx;
// const char* s = "myparams1(0, 1), myparams2(2, 3), myparams3(4, 5)"
// const char* token = oStrTok(s, ",", &ctx, "(", ")");
// while (token)
// {
//		token = oStrTok(0, ",", &ctx, "(", ")");
//		printf("%s\n", token);
// }
// Yields:
// myparams1(0, 1)
// myparams2(2, 3)
// myparams3(4, 5)
char* oStrTok(const char* _Token, const char* _Delimiter, char** _ppContext, const char* _OpenScopeChars = "", const char* _CloseScopeChars = "");

// Use this to clean up a oStrTok iteration if iterating through all tokens is 
// unnecessary. This is automatically called when oStrTok returns 0.
void oStrTokClose(char** _ppContext);

// Open and close pairings might be mismatched, in which case oStrTok will 
// return 0 early, call oStrTokClose automatically, but leave the context in a 
// state that can be queried with this function. The context is not really valid 
// (i.e. all memory has been freed).
bool oStrTokFinishedSuccessfully(char** _ppContext);

// This is a helper function to skip _Count number of tokens delimited by 
// _pDelimiters in _pString
const char* oStrTokSkip(const char* _pString, const char* _pDelimiters, int _Count, bool _SkipDelimiters=true);

// A more convenient form of oStrTok
void oStrParse(const char* _StrSource, const char* _Delimiter, const std::function<void(const char* _Value)>& _Enumerator);

#endif
