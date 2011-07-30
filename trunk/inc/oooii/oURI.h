// $(header)
// URI parsing
#pragma once
#ifndef oURI_h
#define oURI_h

#include <stdlib.h> // _MAX_PATH
#include <string>

namespace oURI {

	static const size_t MAX_SCHEME = 32;
	static const size_t MAX_URI = 512;
	static const size_t MAX_URIREF = 128;

	struct Decomposition
	{
		Decomposition();
		Decomposition(const Decomposition& _Other);
		Decomposition(const char* _Scheme, const char* _Authority = 0, const char* _Path = 0, const char* _Query = 0, const char* _Fragment = 0);
		const Decomposition& operator=(const Decomposition& _Other);
		bool operator==(const Decomposition& _Other);
		inline bool operator!=(const Decomposition& _Other) { return !(*this == _Other); }

		char Scheme[MAX_SCHEME];
		char Authority[MAX_URI];
		char Path[_MAX_PATH];
		char Query[MAX_URI];
		char Fragment[MAX_URI];
	};

	errno_t Decompose(const char* _pURIReference, std::basic_string<char>* _pScheme = NULL, std::basic_string<char>* _pAuthority = NULL, std::basic_string<char>* _pPath = NULL, std::basic_string<char>* _pQuery = NULL, std::basic_string<char>* _pFragment = NULL);
	errno_t Decompose(const char* _URIReference, char* _Scheme, size_t _SizeofScheme, char* _Authority, size_t _SizeofAuthority, char* _Path, size_t _SizeofPath, char* _Query, size_t _SizeofQuery, char* _Fragment, size_t _SizeofFragment);
	errno_t Normalize(char* _NormalizedURI, size_t _SizeofNormalizedURI, const char* _SourceURI);
	errno_t NormalizeBase(char* _NormalizedURIBase, size_t _SizeofNormalizedURIBase, const char* _SourceURIBase); // ensures a trailing separator
	errno_t GetScheme(char* _Scheme, size_t _SizeofScheme, const char* _SourceURI);
	errno_t Recompose(char* _URIReference, size_t _SizeofURIReference, const char* _Scheme, const char* _Authority, const char* _Path, const char* _Query, const char* _Fragment);
	errno_t EnsureFileExtension(char* _URIReferenceWithExtension, size_t _SizeofURIReferenceWithExtension, const char* _SourceURIReference, const char* _Extension);

	errno_t URIFromAbsolutePath(char* _URI, size_t _SizeofURI, const char* _AbsolutePath);
	errno_t URIFromRelativePath(char* _URIReference, size_t _SizeofURIReference, const char* _RelativePath);
	errno_t URIToPath(char* _Path, size_t _SizeofPath, const char* _URI);
	errno_t MakeURIRelativeToURIBase(char* _URIReference, size_t _SizeofURIReference, const char* _URIBase, const char* _URI);
	errno_t MakeURIAbsoluteFromURIBase(char* _URIReference, size_t _SizeofURIReference, const char* _URIBase, const char* _URI);

	// Templated-on-size versions of the above functions
	template<size_t schemeSize, size_t authoritySize, size_t pathSize, size_t querySize, size_t fragmentSize> inline errno_t Decompose(const char* _URIReference, char (&_Scheme)[schemeSize], char (&_Authority)[authoritySize], char (&_Path)[pathSize], char (&_Query)[querySize], char (&_Fragment)[fragmentSize]) { return Decompose(_URIReference, _Scheme, schemeSize, _Authority, authoritySize, _Path, pathSize, _Query, querySize, _Fragment, fragmentSize); }
	template<size_t size> inline errno_t Normalize(char (&_NormalizedURI)[size], const char* _SourceURI) { return Normalize(_NormalizedURI, size, _SourceURI); }
	template<size_t size> inline errno_t NormalizeBase(char (&_NormalizedURIBase)[size], const char* _SourceURIBase) { return errno_t NormalizeBase(_NormalizedURIBase, size, _SourceURIBase); }
	template<size_t size> inline errno_t GetScheme(char (&_Scheme)[size], const char* _SourceURI) { return GetScheme(_Scheme, size, _SourceURI); }
	template<size_t size> inline errno_t Recompose(char(&_URIReference)[size], const char* _Scheme, const char* _Authority, const char* _Path, const char* _Query, const char* _Fragment) { return Recompose(_URIReference, size, _Scheme, _Authority, _Path, _Query, _Fragment); }
	template<size_t size> inline errno_t EnsureFileExtension(char (&_URIReferenceWithExtension)[size], const char* _SourceURIReference, const char* _Extension) { return EnsureFileExtension(_URIReferenceWithExtension, size, _SourceURIReference, _Extension); }
	template<size_t size> inline errno_t URIFromAbsolutePath(char (&_URI)[size], const char* _AbsolutePath) { return URIFromAbsolutePath(_URI, size, _AbsolutePath); }
	template<size_t size> inline errno_t URIFromRelativePath(char (&_URIReference)[size], const char* _RelativePath) { return URIFromRelativePath(_URIReference, size, _RelativePath); }
	template<size_t size> inline errno_t URIToPath(char (&_Path)[size], const char* _URI) { return URIToPath(_Path, size, _URI); }
	template<size_t size> inline errno_t MakeURIRelativeToURIBase(char (&_URIReference)[size], const char* _URIBase, const char* _URI) { return MakeURIRelativeToURIBase(_URIReference, size, _URIBase, _URI); }
	template<size_t size> inline errno_t MakeURIAbsoluteFromURIBase(char (&_URIReference)[size], const char* _URIBase, const char* _URI) { return MakeURIAbsoluteFromURIBase(_URIReference, size, _URIBase, _URI); }

	inline errno_t Decompose(const char* _URIReference, Decomposition& _Decomposition) { return Decompose(_URIReference, _Decomposition.Scheme, _Decomposition.Authority, _Decomposition.Path, _Decomposition.Query, _Decomposition.Fragment); }
	inline errno_t Recompose(char* _URIReference, size_t _SizeofURIReference, const Decomposition& _Decomposition) { return Recompose(_URIReference, _SizeofURIReference, _Decomposition.Scheme, _Decomposition.Authority, _Decomposition.Path, _Decomposition.Query, _Decomposition.Fragment); }
	template<size_t size> inline errno_t Recompose(char (&_URIReference)[size], const Decomposition& _Decomposition) { return Recompose(_URIReference, size, _Decomposition); }

} // namespace oURI

#endif
