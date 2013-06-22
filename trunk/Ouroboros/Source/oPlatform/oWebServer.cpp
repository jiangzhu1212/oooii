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
#include <oPlatform/oWebServer.h>
#include <oPlatform/oFileCacheMonitoring.h>
#include <oPlatform/oFile.h>
#include <oPlatform/oSystem.h>
#include <oBasis/oLockThis.h>
#include <oBasis/oOSC.h>

namespace
{
	class oHandlerEntry
	{
	public:
		typedef oStd::fixed_string<char, oOSC_MAX_FIXED_STRING_LENGTH> oStringCapture;

		oHandlerEntry() {}
		oHandlerEntry(oHandlerEntry&& _other);
		oHandlerEntry& operator=(oHandlerEntry&& _other);
		void AddKey(const char* _key, oHTTPHandler* _handler);
		const oHTTPHandler* MatchURIPath(const char* _uriPath, oFUNCTION<void (const char* _capture)> _CaptureVar, oFUNCTION<void (const char* _OSC)> _OSCCapture, oFUNCTION<const oHTTPURICapture* (const char* _key)> _GetCaptureHandler) const;

	private:
		oHandlerEntry(const oHandlerEntry& _other);
		oHandlerEntry& operator=(const oHandlerEntry& _other);

		void AddKey(std::vector<std::pair<bool, oStd::sstring>>& _key, int _keyIndex, oHTTPHandler* _handler);
		const oHTTPHandler* MatchURIPath(std::vector<oStringCapture>& _parsedURI, int _keyIndex, oFUNCTION<void (const char* _capture)> _CaptureVar, oFUNCTION<void (const char* _OSC)> _OSCCapture, oFUNCTION<const oHTTPURICapture* (const char* _key)> _GetCaptureHandler) const;

		oStd::sstring Value;
		oRef<oHTTPHandler> Handler;

		std::vector<oHandlerEntry> LiteralChildren;
		std::unique_ptr<oHandlerEntry> CaptureChild; //there can be only one capture
	};

	oHandlerEntry::oHandlerEntry(oHandlerEntry&& _other)
	{
		Value = _other.Value;
		Handler = std::move(_other.Handler);
		LiteralChildren = std::move(_other.LiteralChildren);
		CaptureChild = std::move(_other.CaptureChild);
	}

	oHandlerEntry& oHandlerEntry::operator=(oHandlerEntry&& _other)
	{
		Value = _other.Value;
		Handler = std::move(_other.Handler);
		LiteralChildren = std::move(_other.LiteralChildren);
		CaptureChild = std::move(_other.CaptureChild);
		return *this;
	}

	void oHandlerEntry::AddKey(const char* _key, oHTTPHandler* _handler)
	{
		std::vector<std::pair<bool, oStd::sstring>> key; //true if a literal, false for a capture

		oStrParse(_key, "/", [&](const char* _part){
			int len = oInt(oStrlen(_part));
			key.resize(key.size()+1);
			auto& entry = key.back();
			if(_part[0] == '?')
			{
				entry.first = false;
				entry.second.assign(_part + 1, _part + len); //just want the osc string itself
			}
			else
			{
				entry.first = true;
				entry.second.assign(_part, _part + len);
			}
		});

		AddKey(key, 0, _handler);
	}

	void oHandlerEntry::AddKey(std::vector<std::pair<bool, oStd::sstring>>& _key, int _keyIndex, oHTTPHandler* _handler)
	{
		if(_keyIndex == oInt(_key.size()))
		{
			oASSERT(!Handler, "Already a handler registered for this key");

			Handler = _handler;
			return;
		}

		auto& entry = _key[_keyIndex];

		if(entry.first) //literal
		{
			auto result = std::find_if(begin(LiteralChildren), end(LiteralChildren), [&](const oHandlerEntry& _entry) -> bool { return oStrcmp(_entry.Value.c_str(), entry.second.c_str()) == 0; });
			++_keyIndex;
			if(result == end(LiteralChildren))
			{
				LiteralChildren.resize(LiteralChildren.size()+1);
				LiteralChildren.back().Value = entry.second;
				LiteralChildren.back().AddKey(_key, _keyIndex, _handler);
			}
			else
			{
				result->AddKey(_key, _keyIndex, _handler);
			}
		}
		else
		{		
			++_keyIndex;
			if(!CaptureChild)
			{
				CaptureChild.reset(new oHandlerEntry);
				CaptureChild->Value = entry.second;
			}
			oASSERT(oStrcmp(CaptureChild->Value, entry.second) == 0, "all osc strings for a capture must be the same");

			CaptureChild->AddKey(_key, _keyIndex, _handler);
		}
	}

	const oHTTPHandler* oHandlerEntry::MatchURIPath(const char* _uriPath, oFUNCTION<void (const char* _capture)> _CaptureVar, oFUNCTION<void (const char* _OSC)> _OSCCapture, oFUNCTION<const oHTTPURICapture* (const char* _key)> _GetCaptureHandler) const
	{
		std::vector<oStringCapture> parsedURI;

		oStrParse(_uriPath, "/", [&](const char* _part){
			parsedURI.resize(parsedURI.size()+1);
			auto& entry = parsedURI.back();
			entry = _part;
		});

		_OSCCapture(","); //osc strings start with a ,
		const oHTTPHandler* result = MatchURIPath(parsedURI, 0, _CaptureVar, _OSCCapture, _GetCaptureHandler);
		if(!result)
			oTRACE("Could not match %s to a registered oWebServer handler, could be static content", _uriPath);

		return result;
	}

	const oHTTPHandler* oHandlerEntry::MatchURIPath(std::vector<oStringCapture>& _parsedURI, int _keyIndex, oFUNCTION<void (const char* _capture)> _CaptureVar, oFUNCTION<void (const char* _OSC)> _OSCCapture, oFUNCTION<const oHTTPURICapture* (const char* _key)> _GetCaptureHandler) const
	{
		if(_keyIndex == oInt(_parsedURI.size()))
		{
			if(Handler)
				return Handler;
			else
			{
				if(CaptureChild)
				{
					if(CaptureChild->Value.size() > 1)
						_OSCCapture("8");
					else
						_OSCCapture(CaptureChild->Value);
					return CaptureChild->MatchURIPath(_parsedURI, _keyIndex, _CaptureVar, _OSCCapture, _GetCaptureHandler); //trailing set of captures are optional, so keep trying to match
				}
				else
					return nullptr; //could not match.
			}
		}

		oFOR(auto& _entry, LiteralChildren)
		{
			if(oStrcmp(_entry.Value, _parsedURI[_keyIndex]) == 0)
			{
				return _entry.MatchURIPath(_parsedURI, _keyIndex+1, _CaptureVar, _OSCCapture, _GetCaptureHandler);
			}
		}

		if(CaptureChild)
		{
			if(CaptureChild->Value.size() > 1)
			{
				const oHTTPURICapture* capHandler = _GetCaptureHandler(CaptureChild->Value);
				if(!capHandler)
					return nullptr;

				oStd::uri_string captureURI;
				for (int i = _keyIndex; i < oInt(_parsedURI.size()); ++i)
				{
					oStrcat(captureURI, _parsedURI[i].c_str());
					oStrcat(captureURI, "/");
				}

				const char* remaining = captureURI;
				if(!capHandler->AttemptCapture(captureURI, &remaining))
				{
					return nullptr;
				}
				oStringCapture captured;
				captured.assign(captureURI, remaining);
				if(captured.empty())
					return nullptr;
				_keyIndex += oStrCCount(captured, '/');

				_CaptureVar(captured);
				_OSCCapture("8");
			}
			else
			{
				_CaptureVar(_parsedURI[_keyIndex]);
				_OSCCapture(CaptureChild->Value);
			}
			return CaptureChild->MatchURIPath(_parsedURI, _keyIndex + 1, _CaptureVar, _OSCCapture, _GetCaptureHandler);
		}
		
		return nullptr; //couldn't find a match
	}
}

class oWebServerImpl : public oWebServer
{
public:
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_TRIVIAL_QUERYINTERFACE(oWebServer);

	oWebServerImpl(const DESC& _Desc, bool* _pSuccess);

	void AddHTTPHandler(oHTTPHandler *_Handler) threadsafe override;
	void AddURICaptureHandler(oHTTPURICapture *_CaptureHandler) threadsafe override;

	bool Retrieve(const oHTTP_REQUEST& _Request, oHTTP_RESPONSE* _pResponse) threadsafe override;

private:

	oRefCount RefCount;
	oInitOnce<DESC> Desc;
	oStd::path_string RootPath;
	
	oInitOnce<oHandlerEntry> HTTPHandlers;
	oHandlerEntry BuildHandlers; //temporary, used while building pages.

	typedef std::unordered_map<oStd::sstring, oRef<oHTTPURICapture>, oStdHash<oStd::sstring>, oStd::equal_to<oStd::sstring>> URICaptureHandlers_t;
	oInitOnce<URICaptureHandlers_t> URICaptureHandlers;
	URICaptureHandlers_t BuildURICaptureHandlers; //temporary, used while building pages.

	oRef<threadsafe oFileCacheMonitoring> FileCache;

	oConcurrency::mutex AddHandlerMutex;
	bool Started;
};

oWebServerImpl::oWebServerImpl(const DESC& _Desc, bool* _pSuccess) : Started(false)
{
	*_pSuccess = false;
	
	oASSERT(_Desc.AllocBufferCallback, "You must provide an allocator to web file cache");

	DESC buildDesc = _Desc;
	
	oStd::uri_string resolvedRootURI;
	
	if (!oSystemURIToPath(RootPath, buildDesc.URIBase))
	{
		oErrorSetLast(std::errc::invalid_argument, "Invalid root uri given");
		return;
	}

	Desc.Initialize(buildDesc);

	if(!oStreamExists(buildDesc.URIBase))
	{
		oErrorSetLast(std::errc::invalid_argument, "Invalid root uri given");
		return;
	}

	oFileCacheMonitoring::DESC cacheDesc;
	cacheDesc.Disable = buildDesc.DisableCache;
	cacheDesc.RootPath = RootPath;

	if(!oFileCacheMonitoringCreate(cacheDesc, &FileCache))
		return;
	
	*_pSuccess = true;
}

void oWebServerImpl::AddHTTPHandler(oHTTPHandler *_Handler) threadsafe
{
	auto lockedThis = oLockThis(AddHandlerMutex);
	oASSERT(!Started, "All handler should be added before retrieve is called");
	if(!Started)
	{
		lockedThis->BuildHandlers.AddKey(_Handler->HandlesPath(), _Handler);
	}
}

void oWebServerImpl::AddURICaptureHandler(oHTTPURICapture *_CaptureHandler) threadsafe
{
	auto lockedThis = oLockThis(AddHandlerMutex);
	oASSERT(!Started, "All capture handlers should be added before retrieve is called");
	if(!Started)
	{
		oStd::sstring key = _CaptureHandler->GetCaptureName();
		oASSERT(lockedThis->BuildURICaptureHandlers.find(key) == end(lockedThis->BuildURICaptureHandlers), "already a capture handler registered to handle the tag %s", key.c_str());

		lockedThis->BuildURICaptureHandlers[key] = _CaptureHandler;
	}
}

namespace
{
	template<typename FieldType>
	bool ConvertStringToOSCStruct(void* _pField, const char* _CaptureString)
	{
		FieldType* field = reinterpret_cast<FieldType*>(_pField);
		return oStd::from_string(field, _CaptureString);
	}
}

bool oWebServerImpl::Retrieve(const oHTTP_REQUEST& _Request, oHTTP_RESPONSE* _pResponse) threadsafe
{
	if(!Started) //first call to retrieve (or no pages)
	{
		auto lockedThis = oLockThis(AddHandlerMutex);
		lockedThis->HTTPHandlers.Initialize(std::move(lockedThis->BuildHandlers));
		lockedThis->URICaptureHandlers.Initialize(std::move(lockedThis->BuildURICaptureHandlers));
		Started = true;
	}

	//have to be careful. file cache is threadsafe on its own, the code in this function doesn't modify any member variables, so no need
	// to lock. be sure it stays that way. minus initial call handled above.

	//fail by default
	_pResponse->StatusLine.StatusCode = oHTTP_NOT_FOUND;

	oURIParts uriParts;
	if(!oURIDecompose(_Request.RequestLine.RequestURI, &uriParts))
	{
		return false; //invalid uri
	}
	
	if(strstr(uriParts.Path, "..") != nullptr)
	{
		_pResponse->StatusLine.StatusCode = oHTTP_UNAUTHORIZED; //security violation
		return false; 
	}
	
	oHTTPAddHeader(_pResponse->HeaderFields, oHTTP_HEADER_CACHE_CONTROL, "no-cache");

	//favicon gets first try
	if(oStrncmp(oGetFilebase(uriParts.Path), "favicon.ico", 11) == 0)
	{
		if(_Request.RequestLine.Method != oHTTP_GET) //users can't be modifying or deleting our icon
		{
			_pResponse->StatusLine.StatusCode = oHTTP_UNAUTHORIZED;
			return false; 
		}
		extern void GetDescoooii_ico(const char** ppBufferName, const void** ppBuffer, size_t* pSize);

		const char* name = nullptr;
		const void* pBuffer = nullptr;
		size_t size = 0;
		GetDescoooii_ico(&name, &pBuffer, &size);
		_pResponse->StatusLine.StatusCode = oHTTP_OK;
		_pResponse->Content.Type = oMIME_IMAGE_ICO;
		_pResponse->Content.Length = oUInt(size);
		_pResponse->Content.pData = Desc->AllocBufferCallback(_pResponse->Content.Length);
		if(!_pResponse->Content.pData)
			return false;
		memcpy(_pResponse->Content.pData, pBuffer, _pResponse->Content.Length);
		return true;
	}

	//redirect
	if(oStrcmp(uriParts.Path, "/") == 0) 
	{
		oHTTPAddHeader(_pResponse->HeaderFields, oHTTP_HEADER_LOCATION, Desc->DefaultURIReference);
		_pResponse->StatusLine.StatusCode = oHTTP_MOVED_PERMANENTLY;

		return true;
	}
		
	//rest handlers next
	{
		std::vector<oHandlerEntry::oStringCapture> captures;
		oStd::sstring oscString;
		auto handler = HTTPHandlers->MatchURIPath(uriParts.Path, [&](const char* _Capture){
			captures.push_back(_Capture);
		}, [&](const char* _OSC){
			oStrcat(oscString, _OSC);
		}, [&](const char* _key) -> const oHTTPURICapture*
		{
			auto cap = URICaptureHandlers->find(_key);
			if(cap == std::end(*URICaptureHandlers))
				return nullptr;
			else
				return cap->second.c_ptr();
		});

		oHTTPHandler::CommonParams params;
		params.Query = uriParts.Query.c_str();
		params.pResponse = _pResponse;
		_pResponse->Content.pData = nullptr;
		params.AllocateResponse = [&](size_t _BufferSize){
			oASSERT(!_pResponse->Content.pData, "tried to allocate a response buffer more than once");
			_pResponse->Content.Length = oUInt(_BufferSize);
			_pResponse->Content.pData = Desc->AllocBufferCallback(_BufferSize);
		};
		int capIndex = 0;
		params.GetCapturedImpl = [&](void* _Struct, int _SizeOfStruct) -> bool {

			bool succeeded = true;
			capIndex = 0;

			auto handleOSC = [&](int _Type, void* _pField, size_t _SizeofField){
				if(capIndex < oInt(captures.size()))
				{
					bool parsed = false;
					switch (_Type)
					{
					case 'i':
						parsed = ConvertStringToOSCStruct<int>(_pField, captures[capIndex]);
						break;
					case 'h':
						parsed = ConvertStringToOSCStruct<llong>(_pField, captures[capIndex]);
						break;
					case 'f':
						parsed = ConvertStringToOSCStruct<float>(_pField, captures[capIndex]);
						break;
					case 'd':
						parsed = ConvertStringToOSCStruct<double>(_pField, captures[capIndex]);
						break;
					case 'c':
						reinterpret_cast<char*>(_pField)[0] = captures[capIndex][0];
						break;
					case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
						{
							int maxChars = (_Type - '0') * 64;
							parsed = oStrcpy(reinterpret_cast<char*>(_pField), maxChars, captures[capIndex].c_str()) != nullptr;
						}
						break;
					default:
						oASSERT(false, "Unsupported osc type tag. oWebServer uses a subset of osc. see docs.");
						parsed = false;
						break;
					};

					if(!parsed)
					{
						succeeded = false; //pass to outer code
						oASSERT(false, "could not parse uri text %s to an type as specified by the osc type %c", captures[capIndex], _Type);
					}
				}
				++capIndex;
			};

			if(!oOSCVisitStructFields(oscString, _Struct, _SizeOfStruct, handleOSC))
				succeeded = false;

			return succeeded;
		};

		if(handler)
		{
			if(_Request.RequestLine.Method == oHTTP_GET)
				handler->OnGet(params);
			if(_Request.RequestLine.Method == oHTTP_POST)
				handler->OnPost(params, _Request.Content);
			if(_Request.RequestLine.Method == oHTTP_PUT)
				handler->OnPut(params, _Request.Content);
			if(_Request.RequestLine.Method == oHTTP_DELETE)
				handler->OnDelete(params);
			return true;
		}
	}

	//static content gets last dibs. only get requests are valid for these
	if(_Request.RequestLine.Method == oHTTP_GET)
	{
		oMIME_TYPE mimeType;
		oMIMEFromExtension(&mimeType, oGetFileExtension(uriParts.Path));
		if(mimeType == oMIME_UNKNOWN)
		{
			_pResponse->StatusLine.StatusCode = oHTTP_UNAUTHORIZED; //invalid extension
			return false; 
		}
		
		const oRef<oBuffer> cacheBuffer;
		if(!FileCache->Retrieve(uriParts.Path, &cacheBuffer) || !cacheBuffer)
		{
			return false; //file probably didn't exist
		}

		_pResponse->Content.Length = oInt(cacheBuffer->GetSize());
		_pResponse->StatusLine.StatusCode = oHTTP_OK;
		_pResponse->Content.Type = mimeType;
		_pResponse->Content.pData = Desc->AllocBufferCallback(cacheBuffer->GetSize());
		if(!_pResponse->Content.pData)
			return false;
		memcpy(_pResponse->Content.pData, cacheBuffer->GetData(), cacheBuffer->GetSize());

		return true;
	}

	return false;
};

bool oWebServerCreate(const oWebServer::DESC& _Desc, threadsafe oWebServer** _ppObject)
{
	bool success = false;
	oCONSTRUCT(_ppObject, oWebServerImpl(_Desc, &success));
	return success;
}