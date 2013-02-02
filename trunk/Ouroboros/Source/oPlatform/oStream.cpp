/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2013 OOOii.                                              *
 * antony.arciuolo@oooii.com                                              *
 * kevin.myers@oooii.com                                                  *
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
#include <oPlatform/oStream.h> // @oooii-tony: honestly only in oPlatform for oSingleton usage
#include <oPlatform/oSingleton.h>

#include <oBasis/oAlgorithm.h>
#include <oBasis/oFor.h>
#include <oBasis/oLockThis.h>
#include <oBasis/oMutex.h>
#include <oBasis/oRef.h>
#include <oBasis/oRefCount.h>
#include <vector>

#include <oPlatform/oFileSchemeHandler.h> // @oooii-tony: hacky, see ctor comment below

const char* oAsString(const oSTREAM_EVENT& _Event)
{
	switch (_Event)
	{
		case oSTREAM_UNSUPPORTED: return "oSTREAM_UNSUPPORTED";
		case oSTREAM_ADDED: return "oSTREAM_ADDED";
		case oSTREAM_REMOVED: return "oSTREAM_REMOVED";
		case oSTREAM_MODIFIED: return "oSTREAM_MODIFIED";
		case oSTREAM_ACCESSIBLE: return "oSTREAM_ACCESSIBLE";
		oNODEFAULT;
	}
};

struct oStreamContext : public oProcessSingleton<oStreamContext>
{
	static const oGUID GUID;

	bool QueryInterface(const oGUID& _InterfaceID, threadsafe void** _ppInterface) threadsafe override;

	oStreamContext()
	{
		{
			// @oooii-tony: HACK!!! I wish self-registration can occur even in a static 
			// lib, but until we can solve that issue, how can we guarantee we at least
			// have a file system?
			oRef<threadsafe oFileSchemeHandler> SHFile;
			oVERIFY(oFileSchemeHandlerCreate(&SHFile));
			oVERIFY(oStreamContext::RegisterSchemeHandler(SHFile));
		}
	}

	bool SetURIBaseSearchPath(const char* _URIBaseSearchPath) threadsafe;
	char* GetURIBaseSearchPath(char* _StrDestination, size_t _SizeofStrDestination) const threadsafe;
	bool RegisterSchemeHandler(threadsafe oSchemeHandler* _pSchemeHandler) threadsafe;
	void UnregisterSchemeHandler(threadsafe oSchemeHandler* _pSchemeHandler) threadsafe;
	bool FindSchemeHandler(const char* _Scheme, threadsafe oSchemeHandler** _ppSchemeHandler) threadsafe;

	bool GetDesc(const char* _URIReference, oSTREAM_DESC* _pDesc, oURIParts* _pResolvedURIParts = nullptr) threadsafe;
	bool CreateStreamReader(const char* _URIReference, threadsafe oStreamReader** _ppReader) threadsafe;
	bool CreateStreamWriter(const char* _URI, bool _SupportAsyncWrites, threadsafe oStreamWriter** _ppWriter) threadsafe;
	bool CreateStreamMonitor(const char* _URIReference, const oSTREAM_ON_EVENT& _OnEvent, threadsafe oStreamMonitor** _ppMonitor) threadsafe;
	bool Copy(const char* _SourceURIReference, const char* _DestinationURI, bool _Recursive) threadsafe;
	bool Move(const char* _SourceURIReference, const char* _DestinationURI, bool _OverwriteDestination) threadsafe;
	bool Delete(const char* _URI) threadsafe;

protected:

	// Utility functions that don't include mutex locking because that is done at 
	// a higher level, so these aren't threadsafe per-sae
	threadsafe oSchemeHandler* GetSchemeHandler(const char* _Scheme);
	bool VisitURIReferenceInternal(const char* _URIReference, const oFUNCTION<bool(threadsafe oSchemeHandler* _pSchemeHandler, const oURIParts& _URIParts)>& _URIVisitor);
	bool VisitURIReference(const char* _URIReference, const oFUNCTION<bool(threadsafe oSchemeHandler* _pSchemeHandler, const oURIParts& _URIParts)>& _URIVisitor) threadsafe;

	oSharedMutex URIBasesMutex;
	oSharedMutex SchemeHandlersMutex;

	std::vector<oRef<threadsafe oSchemeHandler>> SchemeHandlers;
	std::vector<oStringURI> URIBases;
};

// {463EF9A3-3CBE-40B9-9658-A6160CE058BA}
const oGUID oStreamContext::GUID = { 0x463ef9a3, 0x3cbe, 0x40b9, { 0x96, 0x58, 0xa6, 0x16, 0xc, 0xe0, 0x58, 0xba } };
oSINGLETON_REGISTER(oStreamContext);

const oGUID& oGetGUID(threadsafe const oSchemeHandler* threadsafe const *)
{
	// {DCA57E7E-A75F-4E77-85FC-E41C959FEFC7}
	static const oGUID guid = { 0xdca57e7e, 0xa75f, 0x4e77, { 0x85, 0xfc, 0xe4, 0x1c, 0x95, 0x9f, 0xef, 0xc7 } };
	return guid;
}

bool oStreamContext::QueryInterface(const oGUID& _InterfaceID, threadsafe void** _ppInterface) threadsafe
{
	*_ppInterface = nullptr;
	if (_InterfaceID == oGetGUID<oInterface>() || _InterfaceID == GUID)
	{
		Reference();
		*_ppInterface = this;
	}

	else
	{
		auto pThis = oLockSharedThis(SchemeHandlersMutex);
		oFOR(auto& sh, pThis->SchemeHandlers)
		{
			if (sh->QueryInterface(_InterfaceID, _ppInterface))
				break;
		}
	}

	return !!*_ppInterface;
}

bool oStreamContext::SetURIBaseSearchPath(const char* _URIBaseSearchPath) threadsafe
{
	auto pThis = oLockThis(URIBasesMutex);
	auto& bases = pThis->URIBases;

	bases.reserve(16);
	bases.clear();

	char* ctx = nullptr;
	char* tok = oStrTok(_URIBaseSearchPath, ";", &ctx);
	while (tok)
	{
		bases.resize(bases.size() + 1);
		if (!oURINormalize(bases.back(), tok))
			return oErrorSetLast(oERROR_CORRUPT, "Poorly formed search path. It should be ';' delimited valid URI bases. String:\n%s", _URIBaseSearchPath);
		tok = oStrTok(nullptr, ";", &ctx);
	}

	return true;
};

char* oStreamContext::GetURIBaseSearchPath(char* _StrDestination, size_t _SizeofStrDestination) const threadsafe
{
	bool once = false;
	*_StrDestination = 0;
	auto pThis = oLockSharedThis(URIBasesMutex);
	oFOR(auto& p, pThis->URIBases)
	{
		oStrAppendf(_StrDestination, _SizeofStrDestination, "%s%s", once ? ";" : "");
		once = true;
	}

	return _StrDestination;
}

bool oStreamContext::RegisterSchemeHandler(threadsafe oSchemeHandler* _pSchemeHandler) threadsafe
{
	auto pThis = oLockThis(SchemeHandlersMutex);

	// First check that we're the one and only scheme handler for a scheme and 
	// that our sort order number is also unique
	oFOR(auto& sh, pThis->SchemeHandlers)
	{
		if (!oStricmp(sh->GetScheme(), _pSchemeHandler->GetScheme()))
			return oErrorSetLast(oERROR_REDUNDANT, "There is already a scheme handler for scheme %s", sh->GetScheme());

		if (sh->GetOrder() == _pSchemeHandler->GetOrder())
			return oErrorSetLast(oERROR_REDUNDANT, "The %s scheme handler is currently at order %d, so inserting the %s scheme handler failed", sh->GetScheme(), sh->GetOrder(), _pSchemeHandler->GetScheme());
	}

	for (auto it = pThis->SchemeHandlers.begin(); it != pThis->SchemeHandlers.end(); ++it)
	{
		if ((*it)->GetOrder() > _pSchemeHandler->GetOrder())
		{
			pThis->SchemeHandlers.insert(it, _pSchemeHandler);
			return true;
		}
	}

	// if here, either the list was empty or the order is higher than any in the 
	// list, so append
	pThis->SchemeHandlers.push_back(_pSchemeHandler);
	oTRACE("Registered %s scheme handler", _pSchemeHandler->GetScheme());
	return true;
}

void oStreamContext::UnregisterSchemeHandler(threadsafe oSchemeHandler* _pSchemeHandler) threadsafe
{
	auto pThis = oLockThis(SchemeHandlersMutex);
	oFindAndErase(pThis->SchemeHandlers, oRef<threadsafe oSchemeHandler>(_pSchemeHandler));
}

bool oStreamContext::FindSchemeHandler(const char* _Scheme, threadsafe oSchemeHandler** _ppSchemeHandler) threadsafe
{
	*_ppSchemeHandler = nullptr;
	auto pThis = oLockThis(SchemeHandlersMutex);
	auto it = oStdFindIf(pThis->SchemeHandlers, [&](oRef<threadsafe oSchemeHandler>& _SchemeHandler)->bool { return (!oStricmp(_SchemeHandler->GetScheme(), _Scheme)); });
	if (it != pThis->SchemeHandlers.end())
	{
		*_ppSchemeHandler = *it;
		(*_ppSchemeHandler)->Reference();
		return true;
	}
	return oErrorSetLast(oERROR_NOT_FOUND, "No handler for scheme \"%s\" is registered", oSAFESTRN(_Scheme));
}

threadsafe oSchemeHandler* oStreamContext::GetSchemeHandler(const char* _Scheme)
{
	oFOR(auto& sh, SchemeHandlers)
		if (!oStricmp(_Scheme, sh->GetScheme()))
			return sh;
	return nullptr;
}

bool oStreamContext::VisitURIReferenceInternal(const char* _URIReference, const oFUNCTION<bool(threadsafe oSchemeHandler* _pSchemeHandler, const oURIParts& _URIParts)>& _URIVisitor)
{
	oURIParts URIParts;
	threadsafe oSchemeHandler* sh = nullptr;

	bool DoDecompose = false;

	if (oURIIsAbsolute(_URIReference))
		oVERIFY(oURIDecompose(_URIReference, &URIParts));

	else if (oIsUNCPath(_URIReference) || oIsFullPath(_URIReference))
	{
		oStringURI URI;
		if (!oURIFromAbsolutePath(URI, _URIReference))
			return false;

		oVERIFY(oURIDecompose(URI, &URIParts));
	}

	sh = GetSchemeHandler(URIParts.Scheme);
	if (!sh)
		return oErrorSetLast(oERROR_NOT_FOUND, "no '%s' scheme handler could be found", URIParts.Scheme.empty() ? "(null)" : URIParts.Scheme.c_str());

	return _URIVisitor(sh, URIParts);
}

bool oStreamContext::VisitURIReference(const char* _URIReference, const oFUNCTION<bool(threadsafe oSchemeHandler* _pSchemeHandler, const oURIParts& _URIParts)>& _URIVisitor) threadsafe
{
	oStringURI URI;
	auto pThis = oLockThis(SchemeHandlersMutex);

	// try the URI reference as a full URI
	if (pThis->VisitURIReferenceInternal(_URIReference, _URIVisitor))
		return true;

	oERROR LastErr = oErrorGetLast();
	oStringL LastErrString = oErrorGetLastString();

	// If not, try the search paths/bases
	if (oErrorGetLast() == oERROR_NOT_FOUND)
	{
		auto pThis2 = oLockThis(URIBasesMutex);
		oFOR(auto& base, pThis->URIBases)
		{
			if (0 > oPrintf(URI, "%s/%s", base.c_str(), _URIReference))
				oASSERT(false, "Error in oPrintf (URI too long?)");

			bool success = pThis->VisitURIReferenceInternal(URI, _URIVisitor);
			if (success)
				return true;

			if (oErrorGetLast() != oERROR_NOT_FOUND && oErrorGetLast() != oERROR_INVALID_PARAMETER)
				return false; // pass through error

			// else keep looking
		}
	}
	
	// Lower level VisitURIReferenceInternal should have set the error, but since
	// it does a blind prepend, there may be some invalid URI references formed,
	// so reset it to the original error...
	if (oErrorGetLast() == oERROR_INVALID_PARAMETER)
		oErrorSetLast(LastErr, "%s", LastErrString.c_str());
	
	return false; 
}

bool oStreamContext::GetDesc(const char* _URIReference, oSTREAM_DESC* _pDesc, oURIParts* _pResolvedURIParts) threadsafe
{
	return VisitURIReference(_URIReference, [&](threadsafe oSchemeHandler* _pSchemeHandler, const oURIParts& _URIParts) -> bool
	{
		bool success = _pSchemeHandler->GetDesc(_URIParts, _pDesc);
		if (success && _pResolvedURIParts)
			*_pResolvedURIParts = _URIParts;
		return success;
	});
}

bool oStreamContext::CreateStreamReader(const char* _URIReference, threadsafe oStreamReader** _ppReader) threadsafe
{
	return VisitURIReference(_URIReference, [&](threadsafe oSchemeHandler* _pSchemeHandler, const oURIParts& _URIParts) -> bool
	{
		return _pSchemeHandler->CreateStreamReader(_URIParts, _ppReader);
	});
}

bool oStreamContext::CreateStreamWriter(const char* _URI, bool _SupportAsyncWrites, threadsafe oStreamWriter** _ppWriter) threadsafe
{
	auto pThis = oLockThis(SchemeHandlersMutex);
	return pThis->VisitURIReferenceInternal(_URI, [&](threadsafe oSchemeHandler* _pSchemeHandler, const oURIParts& _URIParts) -> bool
	{
		return _pSchemeHandler->CreateStreamWriter(_URIParts, _SupportAsyncWrites, _ppWriter);
	});
}

bool oStreamContext::CreateStreamMonitor(const char* _URIReference, const oSTREAM_ON_EVENT& _OnEvent, threadsafe oStreamMonitor** _ppMonitor) threadsafe
{
	return VisitURIReference(_URIReference, [&](threadsafe oSchemeHandler* _pSchemeHandler, const oURIParts& _URIParts) -> bool
	{
		return _pSchemeHandler->CreateStreamMonitor(_URIParts, _OnEvent, _ppMonitor);
	});
}

bool oStreamContext::Copy(const char* _SourceURIReference, const char* _DestinationURI, bool _Recursive) threadsafe
{
	oURIParts DestURIParts;
	if (!oURIDecompose(_DestinationURI, &DestURIParts))
		return false; // pass through error

	return VisitURIReference(_SourceURIReference, [&](threadsafe oSchemeHandler* _pSchemeHandler, const oURIParts& _URIParts) -> bool
	{
		if (oStrcmp(_URIParts.Scheme, DestURIParts.Scheme))
			return oErrorSetLast(oERROR_REFUSED, "Copying from scheme %s to scheme %s is not supported. Only same-scheme copies are currently supported.", _URIParts.Scheme.c_str(), DestURIParts.Scheme.c_str());
		return _pSchemeHandler->Copy(_URIParts, DestURIParts, _Recursive);
	});
}

bool oStreamContext::Move(const char* _SourceURIReference, const char* _DestinationURI, bool _OverwriteDestination) threadsafe
{
	oURIParts DestURIParts;
	if (!oURIDecompose(_DestinationURI, &DestURIParts))
		return false; // pass through error

	return VisitURIReference(_SourceURIReference, [&](threadsafe oSchemeHandler* _pSchemeHandler, const oURIParts& _URIParts) -> bool
	{
		if (oStrcmp(_URIParts.Scheme, DestURIParts.Scheme))
			return oErrorSetLast(oERROR_REFUSED, "Moving from scheme %s to scheme %s is not supported. Only same-scheme moves are currently supported.", _URIParts.Scheme.c_str(), DestURIParts.Scheme.c_str());
		return _pSchemeHandler->Move(_URIParts, DestURIParts, _OverwriteDestination);
	});
}

bool oStreamContext::Delete(const char* _URI) threadsafe
{
	auto pThis = oLockThis(SchemeHandlersMutex);
	return pThis->VisitURIReferenceInternal(_URI, [&](threadsafe oSchemeHandler* _pSchemeHandler, const oURIParts& _URIParts) -> bool
	{
		return _pSchemeHandler->Delete(_URIParts);
	});
}

bool oRegisterSchemeHandler(threadsafe oSchemeHandler* _pSchemeHandler)
{
	return oStreamContext::Singleton()->RegisterSchemeHandler(_pSchemeHandler);
}

void oUnregisterSchemeHandler(threadsafe oSchemeHandler* _pSchemeHandler)
{
	oStreamContext::Singleton()->UnregisterSchemeHandler(_pSchemeHandler);
}

bool oFindSchemeHandler(const char* _Scheme, threadsafe oSchemeHandler** _ppSchemeHandler)
{
	return oStreamContext::Singleton()->FindSchemeHandler(_Scheme, _ppSchemeHandler);
}

bool oStreamGetDesc(const char* _URIReference, oSTREAM_DESC* _pDesc, oURIParts* _pResolvedURIParts)
{
	return oStreamContext::Singleton()->GetDesc(_URIReference, _pDesc, _pResolvedURIParts);
}

bool oStreamReaderCreate(const char* _URIReference, threadsafe oStreamReader** _ppReader)
{
	return oStreamContext::Singleton()->CreateStreamReader(_URIReference, _ppReader);
}

bool oStreamWriterCreate(const char* _URI, threadsafe oStreamWriter** _ppWriter)
{
	return oStreamContext::Singleton()->CreateStreamWriter(_URI, true, _ppWriter);
}

bool oStreamLogWriterCreate(const char* _URI, threadsafe oStreamWriter** _ppLogWriter)
{
	return oStreamContext::Singleton()->CreateStreamWriter(_URI, false, _ppLogWriter);
}

bool oStreamMonitorCreate(const char* _URIReference, const oSTREAM_ON_EVENT& _OnEvent, threadsafe oStreamMonitor** _ppMonitor)
{
	return oStreamContext::Singleton()->CreateStreamMonitor(_URIReference, _OnEvent, _ppMonitor);
}

bool oStreamCopy(const char* _SourceURIReference, const char* _DestinationURI, bool _Recursive)
{
	return oStreamContext::Singleton()->Copy(_SourceURIReference, _DestinationURI, _Recursive);
}

bool oStreamMove(const char* _SourceURIReference, const char* _DestinationURI, bool _OverwriteDestination)
{
	return oStreamContext::Singleton()->Move(_SourceURIReference, _DestinationURI, _OverwriteDestination);
}

bool oStreamDelete(const char* _URI)
{
	return oStreamContext::Singleton()->Delete(_URI);
}

bool oStreamSetURIBaseSearchPath(const char* _URIBaseSearchPath)
{
	return oStreamContext::Singleton()->SetURIBaseSearchPath(_URIBaseSearchPath);
}

char* oStreamGetURIBaseSearchPath(char* _StrDestination, size_t _SizeofStrDestination)
{
	return oStreamContext::Singleton()->GetURIBaseSearchPath(_StrDestination, _SizeofStrDestination);
}

struct oStreamReaderWindowedImpl : oStreamReader
{
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_NOOP_QUERYINTERFACE();

	oStreamReaderWindowedImpl(threadsafe oStreamReader* _pReader, const oSTREAM_RANGE& _Window, bool* _pSuccess)
		: Reader(_pReader)
		, WindowStart(_Window.Offset)
		, WindowEnd(_Window.Offset + _Window.Size)
	{
		if(!Reader)
		{
			oErrorSetLast(oERROR_INVALID_PARAMETER, "Failed to specify reader");
			return;
		}

		oSTREAM_DESC FDesc;
		Reader->GetDesc(&FDesc);

		if (*WindowStart > FDesc.Size || *WindowEnd > FDesc.Size)
		{
			oErrorSetLast(oERROR_INVALID_PARAMETER, "Window is outside of the file's range");
			return;
		}

		FDesc.Size = *WindowEnd - *WindowStart; // patch the file size
		Desc.Initialize(FDesc);
		*_pSuccess = true;
	}

	bool WindowRange(const oSTREAM_RANGE& _Range, oSTREAM_RANGE* _pWindowedRange) threadsafe
	{
		oULLong Start = *WindowStart + _Range.Offset; 

		if(Start > *WindowEnd)
			return false;

		oULLong End = Start + _Range.Size;

		if(End > *WindowEnd)
			return false;

		_pWindowedRange->Offset = Start;
		_pWindowedRange->Size = End - Start;
		return true;
	}

	void GetDesc(oSTREAM_DESC* _pDesc) threadsafe override { *_pDesc = *Desc; }

	const oURIParts& GetURIParts() const threadsafe override { return Reader->GetURIParts(); }

	void DispatchRead(const oSTREAM_READ& _Read, continuation_t _Continuation) threadsafe override
	{
		oSTREAM_READ AdjustedRead(_Read);
		if (!WindowRange(_Read.Range, &AdjustedRead.Range))
		{
			oErrorSetLast(oERROR_IO, "Specified range will read past file size");
			_Continuation(false, this, _Read);
			return;
		}

		Reader->DispatchRead(AdjustedRead, _Continuation);
	}

	bool Read(const oSTREAM_READ& _Read) threadsafe override
	{
		oSTREAM_READ AdjustedRead(_Read);
		if (!WindowRange(_Read.Range, &AdjustedRead.Range))
			return oErrorSetLast(oERROR_IO, "Specified range will read past file size");
		return Reader->Read(AdjustedRead);
	}

	void Close() threadsafe override
	{
		Reader->Close();
	}

	oRefCount RefCount;
	oInitOnce<oSTREAM_DESC> Desc;
	oRef<threadsafe oStreamReader> Reader;
	oInitOnce<unsigned long long> WindowStart;
	oInitOnce<unsigned long long> WindowEnd;
};

bool oStreamReaderCreateWindowed(threadsafe oStreamReader* _pReader, const oSTREAM_RANGE& _Window, threadsafe oStreamReader** _ppWindowedReader)
{
	bool success = false;
	oCONSTRUCT(_ppWindowedReader, oStreamReaderWindowedImpl(_pReader, _Window, &success));
	return success;
}

bool oStreamIsNewer(const char* _URIReference, time_t _ReferenceUnixTimestamp)
{
	oSTREAM_DESC sd;
	if (!oStreamGetDesc(_URIReference, &sd))
		return false; // pass through error
	if (_ReferenceUnixTimestamp >= sd.Written)
		return oErrorSetLast(oERROR_REDUNDANT, "%s is not newer", _URIReference);
	return true;
}

oUTF_TYPE oStreamGetUTFType(const char* _URIReference)
{
	// http://code.activestate.com/recipes/173220-test-if-a-file-or-string-is-text-or-binary/
	// "The difference between text and binary is ill-defined, so this duplicates 
	// "the definition used by Perl's -T flag, which is: <br/> The first block 
	// "or so of the file is examined for odd characters such as strange control
	// "codes or characters with the high bit set. If too many strange characters 
	// (>30%) are found, it's a -B file, otherwise it's a -T file. Also, any file 
	// containing null in the first block is considered a binary file."

	static const size_t BLOCK_SIZE = 512;
	static const float PERCENTAGE_THRESHOLD_TO_BE_BINARY = 0.10f; // 0.30f; // @oooii-tony: 30% seems too high to me.

	oRef<threadsafe oStreamReader> Reader;
	if (!oStreamReaderCreate(_URIReference, &Reader))
		return oUNKNOWN_UTF_TYPE; // pass through error

	char buf[BLOCK_SIZE];
	oSTREAM_READ r;
	r.pData = buf;
	r.Range.Size = __min(BLOCK_SIZE, sizeof(buf));
	if (!Reader->Read(r))
		return oUNKNOWN_UTF_TYPE; // pass through error
	return oMemGetUTFType(buf, sizeof(buf));
}
