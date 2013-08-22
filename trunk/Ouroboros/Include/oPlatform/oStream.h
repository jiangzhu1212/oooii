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
// Abstracts all I/O (file, socket, etc.) for IO schemes such as http, ftp, 
// file, etc.
#pragma once
#ifndef oStream_h
#define oStream_h

#include <oStd/function.h>
#include <oStd/memory.h>
#include <oBasis/oInterface.h>
#include <oBasis/oURI.h>
#include <oBasis/oRTTI.h>

enum oSTREAM_EVENT
{
	oSTREAM_UNSUPPORTED,
	oSTREAM_ADDED,
	oSTREAM_REMOVED,
	oSTREAM_MODIFIED,
	oSTREAM_ACCESSIBLE, // after an add the stream is polled until it can be accessed for reading. Once it can, this event occurs.
};
oRTTI_ENUM_DECLARATION(oRTTI_CAPS_ARRAY, oSTREAM_EVENT)
typedef oFUNCTION<void (oSTREAM_EVENT _Event, const oStd::uri_string& _ChangedURI)> oSTREAM_ON_EVENT;

struct oSTREAM_DESC
{
	time_t Created; // Timestamp of creation
	time_t Accessed; // Timestamp of last accessed
	time_t Written; // Timestamp of last modification
	unsigned long long Size; // Size of the stream's buffer
	bool Directory; // True if a directory (container of other streams)
	bool Hidden; // True if not meant to be visible to the general user
	bool ReadOnly; // True if not meant to be modified by the general user
};

struct oSTREAM_MONITOR_DESC
{
	oSTREAM_MONITOR_DESC()
		: AccessibilityCheckMS(1000)
		, EventTimeoutMS(1000)
		, TraceEvents(true)
		, WatchSubtree(true)
	{}

	oStd::uri_string Monitor;
	unsigned int AccessibilityCheckMS;
	unsigned int EventTimeoutMS;
	bool TraceEvents;
	bool WatchSubtree;
};

struct oSTREAM_RANGE
{
	oSTREAM_RANGE()
		: Offset(0)
		, Size(0)
	{}

	oSTREAM_RANGE(unsigned long long _Offset, unsigned long long _Size)
		: Offset(_Offset)
		, Size(_Size)
	{}

	oSTREAM_RANGE(const oSTREAM_DESC& _Desc)
		: Offset(0)
		, Size(_Desc.Size)
	{}

	unsigned long long Offset; // Stream operation starts at this value
	unsigned long long Size; // The length of the stream operation
};

struct oSTREAM_READ
{
	oSTREAM_READ()
		: pData(nullptr)
		, LocalModifiedTimestamp(0)
	{}

	void* pData;
	oSTREAM_RANGE Range;
	time_t LocalModifiedTimestamp;
};

struct oSTREAM_WRITE
{
	oSTREAM_WRITE()
		: pData(nullptr)
	{}

	const void* pData;
	oSTREAM_RANGE Range;
};

// Use this as the offset if "always at the end" is desired. This is 
// particularly important when several async writes are being done to the same
// stream, such as when worker threads log info to one log file.
static const unsigned long long oSTREAM_APPEND = ~0ull;

interface oStream : oInterface
{
	// Fills a description about the current stream
	virtual void GetDesc(oSTREAM_DESC* _pDesc) threadsafe = 0;

	// Returns the parsed URI parts used to create the stream. Remember the 
	// reference is into a buffer that has the same lifetime as this oStream, so
	// threadsafety doesn't mean the lifetime persistence is guaranteed.
	virtual const oURIParts& GetURIParts() const threadsafe = 0;
};

interface oStreamReader : oStream
{
	// Fills _pData with the range of data in this stream. If the last 
	// modification time for this stream is eariler than the specified timestamp
	// then this function will return false with an oErrorGetLast() of 
	// std::errc::operation_in_progress indicating that any local buffer with such a local 
	// timestamp is up to date and no new data is necessary.
	virtual bool Read(const oSTREAM_READ& _Read) threadsafe = 0;

	// If Read reads past the end of the file, it will truncate its read and 
	// set a flag that can be accessed here. This does not apply to DispatchRead.
	virtual bool EndOfFile() threadsafe const = 0;
	
	// Bind a function to be called immediately after the read finishes in the 
	// same thread which processes the read. _Success is the same as a return 
	// value from a Read() call and oErrorGetLast() is appropriate to call for 
	// extended error information. Use this with DispatchRead to schedule a read
	// to occur asynchronously. The actual read can be serviced by any thread in
	// the task manager.

	typedef oFUNCTION<void(bool _Success, threadsafe oStreamReader* _pReader, const oSTREAM_READ& _Read)> continuation_t;
	virtual void DispatchRead(const oSTREAM_READ& _Read, continuation_t _Continuation) threadsafe = 0;

	//This stream will no longer be usable after this call. Any pending reads/writes will not complete. 
	//	This is safe to call from within a continuation. 
	virtual void Close() threadsafe = 0; //? Does it make sense to close an oStreamMonitor too? If so could promote this to oStream.
};

interface oStreamWriter : oStream
{
	// Fills the stream at the specified range with data from _pData.
	virtual bool Write(const oSTREAM_WRITE& _Write) threadsafe = 0;

	// Bind a function to be called immediately after the write finishes in the 
	// same thread which processes the write.
	typedef oFUNCTION<void(bool _Success, threadsafe oStreamWriter* _pWriter, const oSTREAM_WRITE& _Write)> continuation_t;
	virtual void DispatchWrite(const oSTREAM_WRITE& _Write, continuation_t _Continuation) threadsafe = 0;

	//This stream will no longer be usable after this call. Any pending reads/writes will not complete. 
	//	This is safe to call from within a continuation. 
	virtual void Close() threadsafe = 0;
};

interface oStreamMonitor : oStream
{
	virtual void GetMonitorDesc(oSTREAM_MONITOR_DESC* _pMonitorDesc) const threadsafe = 0;
};

// {DCA57E7E-A75F-4E77-85FC-E41C959FEFC7}
oDEFINE_GUID_I(oSchemeHandler, 0xdca57e7e, 0xa75f, 0x4e77, 0x85, 0xfc, 0xe4, 0x1c, 0x95, 0x9f, 0xef, 0xc7);
interface oSchemeHandler : oInterface
{
	// Returns the priority of this scheme handler. When scheme handlers are 
	// registered to the oFileSystem, they are sorted by this value from low to 
	// high. This value should be specified at initialization time and remain 
	// constant throughout the lifetime of the handler. It is expected this value
	// is only used during registration and thus registration will not change if 
	// this value changes.
	virtual int GetOrder() const threadsafe = 0;

	// Returns the string representing the URI scheme this handler is designed to 
	// handle.
	virtual const char* GetScheme() const threadsafe = 0;
	
	// Return a desc of the resource specified by URI. This API is included 
	// because in some usage patterns and in some platform implementations it is 
	// more efficient to query the meta-data associated with a stream (file) than 
	// it is to open the stream to query its info.
	virtual bool GetDesc(const oURIParts& _URIParts, oSTREAM_DESC* _pDesc) threadsafe = 0;

	// Copies one resource to another of the SAME SCHEME, this scheme. Handling
	// copies across differing schemes is handled outside an oSchemeHandler 
	// implementation.
	virtual bool Copy(const oURIParts& _Source, const oURIParts& _Destination, bool _Recursive = true) threadsafe = 0;

	// Moves/Renames one resource to another of the SAME SCHEME, this scheme. 
	// Handling copies across differing schemes is handled outside an 
	// oSchemeHandler implementation.
	virtual bool Move(const oURIParts& _Source, const oURIParts& _Destination, bool _OverwriteDestination = true) threadsafe = 0;

	// Remove the specified resource from the underlying resource managed by the
	// scheme handler.
	virtual bool Delete(const oURIParts& _URIParts) threadsafe = 0;
	
	// Create a stream for reading based on a fully-qualified URI.
	virtual bool CreateStreamReader(const oURIParts& _URIParts, threadsafe oStreamReader** _ppReader) threadsafe = 0;

	// Create a stream for writing based on a fully-qualified URI.
	virtual bool CreateStreamWriter(const oURIParts& _URIParts, bool _SupportAsyncWrites, threadsafe oStreamWriter** _ppWriter) threadsafe = 0;

	// Create a stream that monitors a URI (folder, file or other; local or 
	// remote) and fires the specified event handler when something happens.
	virtual bool CreateStreamMonitor(const oURIParts& _URIParts, const oSTREAM_MONITOR_DESC& _Desc, const oSTREAM_ON_EVENT& _OnEvent, threadsafe oStreamMonitor** _ppMonitor) threadsafe = 0;
};

// Register the specified scheme handler with this device. The device retains
// a reference to the handler while registered.
oAPI bool oRegisterSchemeHandler(threadsafe oSchemeHandler* _pSchemeHandler);

// Remove the device's reference to a scheme handler. If not called explicitly
// this will happen automatically during static destruction.
oAPI void oUnregisterSchemeHandler(threadsafe oSchemeHandler* _pSchemeHandler);

// Searches the registered scheme handlers for the one handling the specified 
// scheme.
oAPI bool oFindSchemeHandler(const char* _Scheme, threadsafe oSchemeHandler** _ppSchemeHandler);

// Walks through all URIBases and registered scheme handlers to fill the 
// specified oSTREAM_DESC. While implementation-specific, the goal of this API 
// is that there are often faster ways of getting just the description 
// information from a URI than opening the resource for reading. If a buffer is 
// specified and the function returns true, _pResolvedURIParts will be the parts 
// of the URI after search path evaluation.
oAPI bool oStreamGetDesc(const char* _URIReference, oSTREAM_DESC* _pDesc, oURIParts* _pResolvedURIParts = nullptr);

// Convenience when client code just wants to test if the resource is present
inline bool oStreamExists(const char* _URIReference)
{
	oSTREAM_DESC sd;
	return oStreamGetDesc(_URIReference, &sd);
}

// Walks through all URIBases and registered scheme handlers to create a 
// stream for reading.
oAPI bool oStreamReaderCreate(const char* _URIReference, threadsafe oStreamReader** _ppReader);

// Opens a stream for writing. The full URI must be specified for this - no 
// use of the search path is done.
oAPI bool oStreamWriterCreate(const char* _URI, threadsafe oStreamWriter** _ppWriter);

// Opens a stream for writing that does not support DispatchWrite and does not 
// interact with the underlying async IO management system and thus does not 
// block any async I/O system's flush operation because as a log file, this 
// stream is intended to be low-level and open as long as the process is open.
oAPI bool oStreamLogWriterCreate(const char* _URI, threadsafe oStreamWriter** _ppLogWriter);

// Creates a monitor that will call the specified _OnEvent whenever something in 
// the specified URI reference changes.
oAPI bool oStreamMonitorCreate(const oSTREAM_MONITOR_DESC& _Desc, const oSTREAM_ON_EVENT& _OnEvent, threadsafe oStreamMonitor** _ppMonitor);

// Copies the source URI to the destination URI. Currently the source URI must 
// resolve to the same scheme as the destination URI.
oAPI bool oStreamCopy(const char* _SourceURIReference, const char* _DestinationURI, bool _Recursive = true);

// Move works on either files or folders. This is the same thing as renaming the 
// file. Currently the source URI must resolve to the same scheme as the 
// destination URI.
oAPI bool oStreamMove(const char* _SourceURIReference, const char* _DestinationURI, bool _OverwriteDestination = false);

// Delete a resource. The full URI must be specified for this - no use of the 
// search path is done.
oAPI bool oStreamDelete(const char* _URI);

// Returns true if the specified URIReference has a late-written timestamp more 
// recent than the specified reference timestamp.
oAPI bool oStreamIsNewer(const char* _URIReference, time_t _ReferenceUnixTimestamp);

// A single string with a semi-colon-delimited list of search paths to look 
// into when evaluating a URI reference. The specified string is copied 
// internally.
oAPI bool oStreamSetURIBaseSearchPath(const char* _URIBaseSearchPath);

// Fills the destination with a semi-colon-delmited list of URI bases as is 
// currently set in this object. If none are set, the empty string is copied.
oAPI char* oStreamGetURIBaseSearchPath(char* _StrDestination, size_t _SizeofStrDestination);
template<size_t size> inline char* oStreamGetURIBaseSearchPath(char (&_StrDestination)[size]) { return oStreamGetURIBaseSearchPath(_StrDestination, size); }
template<size_t capacity> inline char* oStreamGetURIBaseSearchPath(oStd::fixed_string<char, capacity>& _StrDestination) { return oStreamGetURIBaseSearchPath(_StrDestination, _StrDestination.capacity()); }

// Creates a new oStreamReader interface for a subset of the specified 
// oStreamReader.
oAPI bool oStreamReaderCreateWindowed(threadsafe oStreamReader* _pReader, const oSTREAM_RANGE& _Window, threadsafe oStreamReader** _ppWindowedReader);

// Returns the type of the contents of the specified stream. This opens the 
// specified stream and reads a small number of bytes at the start of the file
// to make its determination. The rules for determining if the stream is an 
// ascii stream are the same as Perl -T.
// http://code.activestate.com/recipes/173220-test-if-a-file-or-string-is-text-or-binary/
oAPI oStd::utf_type::value oStreamGetUTFType(const char* _URIReference);

#endif
