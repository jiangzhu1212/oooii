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
#include <oPlatform/oFileSchemeHandler.h>
#include <oPlatform/oFile.h>
#include <oPlatform/oSystem.h>
#include <oConcurrency/mutex.h>
#include "oDispatchQueueGlobalIOCP.h"
#include "oFileInternal.h"
#include "oIOCP.h"

using namespace oConcurrency;

#define oTRACE_MONITOR oTRACE

static void oSetHighLowOffset(long& _low, long& _high, oULLong _offset)
{
	unsigned long long offset = _offset.Ref();
	_low = offset & 0xffffffffull;
	_high = offset >> 32ull;
}

struct oFileReaderImpl : public oStreamReader
{
	static const int UNBUFFERED_ALIGN_REQUIREMENT = 4096;

	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_NOOP_QUERYINTERFACE();
	
	oFileReaderImpl(const oURIParts& _URIParts, bool _Unbuffered, bool* _pSuccess)
		: URIParts(_URIParts)
		, Unbuffered(_Unbuffered)
		, hFile(nullptr)
		, EoF(false)
	{
		*_pSuccess = false;

		oStd::path_string Path;
		oSystemURIPartsToPath(Path, _URIParts);
		hFile = CreateFile(Path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, Unbuffered ? FILE_FLAG_NO_BUFFERING : 0, NULL);
		if (hFile == INVALID_HANDLE_VALUE)
		{
			oWinSetLastError();
			switch (oErrorGetLast())
			{
				case std::errc::no_such_file_or_directory:
				{
					oStd::uri_string URIRef;
					oVERIFY(oURIRecompose(URIRef, _URIParts));
					oErrorSetLast(std::errc::no_such_file_or_directory, "not found: %s", URIRef.c_str());
					break;
				}

				case std::errc::invalid_argument:
				{
					oStd::uri_string URIRef;
					oVERIFY(oURIRecompose(URIRef, _URIParts));
					oErrorSetLast(std::errc::invalid_argument, "invalid URI: %s", URIRef.c_str());
					break;
				}

				default:
					break;
			}

			return;
		}
		else
		{
			oSTREAM_DESC FDesc;
			oFileGetDesc(Path, &FDesc);
			Desc.Initialize(FDesc);

			if (!oDispatchQueueCreateGlobalIOCP("File reader dispatch queue", 10, &ReadQueue))
			{
				oErrorSetLast(std::errc::invalid_argument, "Could not create IOCP dispatch queue.");
				CloseHandle(hFile);
				return;
			}
		}

		*_pSuccess = true;
	}

	~oFileReaderImpl()
	{
		if(ReadQueue)
		{
			ReadQueue->Join();
			oASSERT(!ReadQueue->Joinable(), "The ReadQueue should have joined! What is running that prevented it from joining?");
		}

		Close();
	}

	void DispatchRead(const oSTREAM_READ& _StreamRead, continuation_t _Continuation) threadsafe override
	{
		unsigned long long fileSize = Desc->Size;
		if(Unbuffered)
			fileSize = oStd::byte_align(fileSize, UNBUFFERED_ALIGN_REQUIREMENT); // unbuffered io is always a multiple of the alignment
		
		if ((_StreamRead.Range.Offset + _StreamRead.Range.Size) <= fileSize)
		{
			oRef<threadsafe oFileReaderImpl> self(this);
			// Doing it this way because standard windows async file io is not 
			// GUARANTEED to be asynchronous. Windows can decided to do the read 
			// synchronously on its own, so FORCE async this way.
			ReadQueue->Dispatch([self, _StreamRead, _Continuation]() mutable {
				bool success = self->Read(_StreamRead);
				_Continuation(success, self.c_ptr(), _StreamRead);
			});
		}
		else
		{
			oErrorSetLast(std::errc::io_error, "Specified range will read past file size");
			_Continuation(false, this, _StreamRead);
			return;
		}
	}

	bool Read(const oSTREAM_READ& _StreamRead) threadsafe
	{
		oASSERT(!Unbuffered || (oStd::byte_aligned(_StreamRead.pData, UNBUFFERED_ALIGN_REQUIREMENT) && 
			oStd::byte_aligned(_StreamRead.Range.Offset, UNBUFFERED_ALIGN_REQUIREMENT) && oStd::byte_aligned(_StreamRead.Range.Size, UNBUFFERED_ALIGN_REQUIREMENT)), 
			"Unbuffered io has very restrict requirements. file offset, read size, and pointer address must be 4k aligned");

		DWORD numBytesRead;
		long offsetLow, offsetHigh;
		oSetHighLowOffset(offsetLow, offsetHigh, _StreamRead.Range.Offset);

		// Need to lock since SetFilePointer/ReadFile are separate calls. And to play nice with close
		lock_guard<mutex> lock(Mutex);

		if(hFile == INVALID_HANDLE_VALUE)
		{
			return oErrorSetLast(std::errc::operation_canceled);
		}

		SetFilePointer(hFile, offsetLow, &offsetHigh, FILE_BEGIN);
		EoF = false; // because of file pointer move
		if (!ReadFile(hFile, _StreamRead.pData, oUInt(_StreamRead.Range.Size), &numBytesRead, nullptr))
			return oWinSetLastError();
		EoF = (numBytesRead == 0);

		// Unbuffered io is inconsistent on what it returns for these numbers and 
		// it doesn't really apply for anyway because unbuffered always physically 
		// reads in an even number of sectors.
		if (!Unbuffered && numBytesRead != _StreamRead.Range.Size) // happens when trying to read past end of file, but ReadFile itself still succeeds
		{
			// failed, but not a bug if reading past end of file. 
			oASSERT(_StreamRead.Range.Offset + _StreamRead.Range.Size >= Desc->Size, "ReadFile didn't read enough bytes, but didn't read past end of file");
			return false;
		}

		return true;
	}

	bool EndOfFile() threadsafe const override { return EoF; }

	void GetDesc(oSTREAM_DESC* _pDesc) threadsafe override
	{
		*_pDesc = *Desc;
	}

	const oURIParts& GetURIParts() const threadsafe override { return *URIParts; }

	void Close() threadsafe override
	{
		lock_guard<mutex> lock(Mutex);

		if (hFile != INVALID_HANDLE_VALUE)
			CloseHandle(hFile);

		hFile = INVALID_HANDLE_VALUE;
	}

	oInitOnce<oSTREAM_DESC> Desc;
	oRef<threadsafe oDispatchQueueGlobal> ReadQueue;
	mutex Mutex;
	oInitOnce<oURIParts> URIParts;
	HANDLE hFile;
	oRefCount RefCount;
	bool Unbuffered;
	bool EoF;
};

struct oFileWriterImpl : public oStreamWriter
{
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_NOOP_QUERYINTERFACE();

	struct Operation
	{
		oSTREAM_WRITE Write;
		continuation_t Continuation;
	};

	oFileWriterImpl(const oURIParts& _URIParts, bool _SupportAsyncWrites, bool* _pSuccess)
		: URIParts(_URIParts)
		, hFile(nullptr)
	{
		*_pSuccess = false;

		oSystemURIPartsToPath(ResolvedPath.Initialize(), _URIParts);
		if (!oFileEnsureParentFolderExists(*ResolvedPath))
			return; // pass through error

		// @oooii-tony:
		// FILE_SHARE_READ added here for a sole purpose: so a programmer can open
		// a log file and see stuff written so far while the log is still being 
		// appended to. If this causes problems, take this out immediately as no 
		// code per-sae relies on this behavior.
		hFile = CreateFile(*ResolvedPath, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, 0/* FILE_FLAG_OVERLAPPED*/, NULL);
		if (hFile == INVALID_HANDLE_VALUE)
		{
			oWinSetLastError();
			return;
		}
		else if (_SupportAsyncWrites)
		{
			if (!oDispatchQueueCreateGlobalIOCP("File writer dispatch queue", 16, &WriteQueue))
			{
				CloseHandle(hFile);
				oErrorSetLast(std::errc::invalid_argument, "Could not create IOCP dispatch queue.");
				return;
			}
		}

		*_pSuccess = true;
	}

	~oFileWriterImpl()
	{
		if(WriteQueue)
			WriteQueue->Join();

		Close();
	}
	
	void GetDesc(oSTREAM_DESC* _pDesc) threadsafe override
	{
		oVERIFY(oFileGetDesc(*ResolvedPath, _pDesc));
	}

	void DispatchWrite(const oSTREAM_WRITE& _Write, continuation_t _Continuation) threadsafe override
	{
		oRef<threadsafe oFileWriterImpl> self(this);
		// Doing it this way because standard windows async file io is not 
		// GUARANTEED to be asynchronous. Windows can decided to do the read 
		// synchronously on its own, so FORCE async this way.
		WriteQueue->Dispatch([self, _Write, _Continuation]() mutable {
			bool success = self->Write(_Write);
			_Continuation(success, self.c_ptr(), _Write);
		});
	}

	bool Write(const oSTREAM_WRITE& _Write) threadsafe
	{
		// Need to lock since SetFilePointer/ReadFile are separate calls.
		lock_guard<mutex> lock(Mutex);

		if(hFile == INVALID_HANDLE_VALUE)
		{
			return oErrorSetLast(std::errc::operation_canceled);
		}

		DWORD numBytesWritten;
		if (_Write.Range.Offset == oSTREAM_APPEND)
		{
			long offsetHigh = 0; // if you don't pass high, you can't seek to the end once file is larger than 4 gig
			SetFilePointer(hFile, 0, &offsetHigh, FILE_END);
		}
		else
		{
			long offsetLow, offsetHigh;
			oSetHighLowOffset(offsetLow, offsetHigh, _Write.Range.Offset);
			SetFilePointer(hFile, offsetLow, &offsetHigh, FILE_BEGIN);
		}

		if (!WriteFile(hFile, _Write.pData, oUInt(_Write.Range.Size), &numBytesWritten, nullptr))
			return oWinSetLastError();

		if (numBytesWritten != _Write.Range.Size) // happens when trying to read past end of file, but ReadFile itself still succeeds
			return oErrorSetLast(std::errc::io_error, "Failed to write data to disk.");

		return true;
	}

	const oURIParts& GetURIParts() const threadsafe override { return *URIParts; }

	void Close() threadsafe override
	{
		lock_guard<mutex> lock(Mutex);

		if (hFile != INVALID_HANDLE_VALUE)
			CloseHandle(hFile);

		hFile = INVALID_HANDLE_VALUE;
	}

	oInitOnce<oURIParts> URIParts;
	oInitOnce<oStd::path_string> ResolvedPath;
	oHandle hFile;
	oRef<threadsafe oDispatchQueueGlobal> WriteQueue;
	mutex Mutex;
	oRefCount RefCount;
};

static oSTREAM_EVENT oAsStreamEvent(DWORD _NotifyAction)
{
	switch (_NotifyAction)
	{
		case FILE_ACTION_ADDED: return oSTREAM_ADDED;
		case FILE_ACTION_REMOVED: return oSTREAM_REMOVED;
		case FILE_ACTION_MODIFIED: return oSTREAM_MODIFIED;
		case FILE_ACTION_RENAMED_OLD_NAME: return oSTREAM_REMOVED;
		case FILE_ACTION_RENAMED_NEW_NAME: return oSTREAM_ADDED;
		default: return oSTREAM_UNSUPPORTED;
	}
}

// Returns true if the file exists and can be opened for shared reading, false 
// otherwise.
static bool oWinIsAccessible(const char* _Path)
{
	bool IsAccessible = false;
	HANDLE hFile = CreateFile(_Path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		IsAccessible = true;
		oVB(CloseHandle(hFile));
	}

	return IsAccessible;
}

struct oFileMonitorImpl : public oStreamMonitor
{
	oDEFINE_NOOP_QUERYINTERFACE();

	static VOID CALLBACK WaitOrTimerCallback(PVOID lpParameter, BOOLEAN TimerOrWaitFired)
	{
		static_cast<oFileMonitorImpl*>(lpParameter)->CheckAccessible();
	}

	oFileMonitorImpl(const oURIParts& _URIParts, const oSTREAM_MONITOR_DESC& _Desc, const oSTREAM_ON_EVENT& _OnEvent, bool* _pSuccess)
		: MDesc(_Desc)
		, OnEvent(_OnEvent)
		, hMonitor(nullptr)
		, pIOCP(nullptr)
		, FileNotifyInfoBufferIndex(0)
		, hTimerQueue(CreateTimerQueue())
		, Closing(false)
	{
		*_pSuccess = false;

		Accessibles.reserve(16);

		if (!hTimerQueue)
		{
			oWinSetLastError();
			return;
		}

		oStd::path_string Path;
		if (!oSystemURIPartsToPath(Path, _URIParts))
		{
			oStd::uri_string URI;
			oURIRecompose(URI, _URIParts);
			oErrorSetLast(std::errc::invalid_argument, "Invalid uri: %s", URI.c_str());
			return;
		}

		char* filebase = oGetFilebase(Path.c_str());
		MonitorFilename.Initialize(filebase);
		*filebase = 0;

		oURIParts FolderURIParts = _URIParts;
		*oGetFilebase(FolderURIParts.Path.c_str()) = 0;
		URIParts.Initialize(FolderURIParts);

		oSTREAM_DESC fd;
		if (!oFileGetDesc(Path, &fd))
		{
			oStd::uri_string URI;
			oURIRecompose(URI, _URIParts);
			oErrorSetLast(std::errc::invalid_argument, "Invalid uri: %s", URI.c_str());
			return;
		}
		
		if (!fd.Directory)
		{
			oStd::uri_string URI;
			oURIRecompose(URI, _URIParts);
			oErrorSetLast(std::errc::invalid_argument, "Invalid uri (filename should have been stripped): %s", URI.c_str());
			return;
		}

		Desc.Initialize(fd);

		HANDLE hTimer = nullptr;
		if (!CreateTimerQueueTimer(
			&hTimer
			, hTimerQueue
			, oFileMonitorImpl::WaitOrTimerCallback
			, this
			, MDesc.AccessibilityCheckMS
			, MDesc.AccessibilityCheckMS
			, WT_EXECUTEDEFAULT
			))
		{
			oWinSetLastError();
			return;
		}

		hMonitor = (oHandle)CreateFile(Path, FILE_LIST_DIRECTORY, FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE, nullptr, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS|FILE_FLAG_OVERLAPPED, nullptr);

		if (hMonitor == INVALID_HANDLE_VALUE)
		{
			oWinSetLastError();
			return;
		}

		MonitorPath.Initialize(Path);

		oIOCP::DESC IOCPDesc;
		IOCPDesc.Handle = hMonitor;
		IOCPDesc.IOCompletionRoutine = oBIND(&oFileMonitorImpl::IOCPCallback, this, oBIND1);
		IOCPDesc.MaxOperations = 4;
		IOCPDesc.PrivateDataSize = 0;

		if (!oIOCPCreate(IOCPDesc, [&](){ delete this; }, &pIOCP))
		{
			oErrorSetLast(std::errc::invalid_argument, "Could not create IOCP.");
			return;
		}

		oIOCPOp* pOp = pIOCP->AcquireSocketOp();
		if (!ReadDirectoryChangesW(hMonitor, FileNotifyInfoBuffers[FileNotifyInfoBufferIndex], kBufferSize, MDesc.WatchSubtree, kWatchChanges, nullptr, pOp, nullptr))
		{
			oErrorSetLast(std::errc::protocol_error, "Could not initiate a windows request to monitor a folder.");
			return;
		}

		*_pSuccess = true;
	}

	~oFileMonitorImpl()
	{
		lock_guard<mutex> lock(CheckAccessibleMutex);

		Closing = true;
		if (hMonitor != INVALID_HANDLE_VALUE)
			CloseHandle((HANDLE)hMonitor);

		if (MonitorFinished.valid())
			MonitorFinished.wait();

		oVB(DeleteTimerQueue(hTimerQueue));
	}

	void GetMonitorDesc(oSTREAM_MONITOR_DESC* _pMonitorDesc) const threadsafe override { *_pMonitorDesc = oThreadsafe(this)->MDesc; }

	void OnDirectoryChanges(FILE_NOTIFY_INFORMATION* _pNotify)
	{
		FILE_NOTIFY_INFORMATION* n = _pNotify;
		double Now = oTimer();
		do
		{
			oSTREAM_EVENT e = oAsStreamEvent(n->Action);
			if (e != oSTREAM_UNSUPPORTED)
			{
				oASSERT(n->FileNameLength < (oStd::path_string::Capacity*2), "not expecting paths that are longer than they should be"); // *2 for that fact that FileNameLength is # bytes for a wide-char non-null-term string

				// note the path given to us by windows is only the portion after the 
				// path of the folder we are monitoring, so have to reform the full uri.

				// These strings aren't null-terminated, AND in wide-character. Make it
				// not this way...
				wchar_t ForNullTermination[oStd::path_string::Capacity];
				memcpy(ForNullTermination, n->FileName, n->FileNameLength);
				ForNullTermination[n->FileNameLength / 2] = 0;
				oStd::path_string path(ForNullTermination);

				oURIParts parts;
				parts = *URIParts;
				oStrAppendf(parts.Path, "/%s", path.c_str());

				oStd::uri_string URI;
				oVERIFY(oURIRecompose(URI, parts));
				
				if (MonitorFilename->empty() || oMatchesWildcard(MonitorFilename->c_str(), path.c_str()))
				{
					if (e == oSTREAM_ADDED || e == oSTREAM_MODIFIED)
					{
						lock_guard<mutex> lock(DeferredEventMutex);
						oEVENT_RECORD r;
						r.LastEventTimestamp = Now;
						r.LastEvent = e;
						r.Handled = false;
						EventRecords[URI] = r;
					}

					if (MDesc.TraceEvents)
						oTRACE_MONITOR("%s: %s", oStd::as_string(e), URI.c_str());

					OnEvent(e, URI);
				}
				else
				{
					if (MDesc.TraceEvents)
						oTRACE_MONITOR("%s: %s not matched by %s", oStd::as_string(e), URI.c_str(), MonitorFilename->c_str());
				}
			}

			n = oStd::byte_add(n, n->NextEntryOffset);

		} while (n->NextEntryOffset);
	}

	void CheckAccessible()
	{
		lock_guard<mutex> lock(CheckAccessibleMutex);

		if (Closing)
			return;

		if (!EventRecords.empty())
		{
			Accessibles.clear();
			{
				lock_guard<mutex> lock(DeferredEventMutex);
				for (auto it = std::begin(EventRecords); it != std::end(EventRecords); /* no increment */)
				{
					bool IncrementIterator = true;

					const oStd::uri_string& URI = it->first;
					oEVENT_RECORD& r = it->second;
					if (r.Handled)
					{
						if (r.LastEventTimestamp > MDesc.EventTimeoutMS)
						{
							it = EventRecords.erase(it);
							IncrementIterator = false;
						}
					}

					else
					{
						oStd::path_string path;
						oURIParts p;
						oURIDecompose(URI, &p);
						oSystemURIPartsToPath(path, p);
				
						if (oWinIsAccessible(path))
						{
							Accessibles.push_back(URI);
							r.Handled = true;
						}
					}

					if (IncrementIterator)
						++it;
				}
			}

			oFOR(const auto& a, Accessibles)
			{
				if (MDesc.TraceEvents)
				{
					oTRACE_MONITOR("oSTREAM_ACCESSIBLE: %s", a.c_str());
				}
				OnEvent(oSTREAM_ACCESSIBLE, a);
			}
		}
	}

	void IOCPCallback(oIOCPOp* _pSocketOp)
	{
		if (Closing)
			return;

		pIOCP->ReturnOp(_pSocketOp);

		if (MonitorFinished.valid())
			MonitorFinished.wait();

		// Kick off user callback on a normal (non-IOCP) thread

		int FileNotifyInfoBufferIndexCopy = FileNotifyInfoBufferIndex; // otherwise lambda will bind this->FileNotifyInfoBufferIndex, which can be racy.
		MonitorFinished = oStd::async((oFUNCTION<void()>)[this, FileNotifyInfoBufferIndexCopy] { OnDirectoryChanges(reinterpret_cast<FILE_NOTIFY_INFORMATION*>(FileNotifyInfoBuffers[FileNotifyInfoBufferIndexCopy])); });

		FileNotifyInfoBufferIndex = (FileNotifyInfoBufferIndex + 1) % 2;
	
		oIOCPOp* pOp = pIOCP->AcquireSocketOp();
		if (!ReadDirectoryChangesW(hMonitor, FileNotifyInfoBuffers[FileNotifyInfoBufferIndex], sizeof(FileNotifyInfoBuffers[FileNotifyInfoBufferIndex]), MDesc.WatchSubtree, kWatchChanges, nullptr, pOp, nullptr) && !Closing)
		{
			DWORD error = GetLastError();
			oTRACE("oFileMonitorImpl platform error %d", error);
			oASSERT(false, "if this fails we won't be able to continue receiving folder change notifications");
		}
	}

	const oURIParts& GetURIParts() threadsafe const { return *URIParts; }
	void GetDesc(oSTREAM_DESC* _pDesc) threadsafe { *_pDesc = *Desc; }

	int Reference() threadsafe override { return pIOCP->Reference(); }
	void Release() threadsafe override { pIOCP->Release(); }

private:
	static const int kBufferSize = oKB(64); // If buffer is over 64K, you notifications for network drives fail.
	static const DWORD kWatchChanges = FILE_NOTIFY_CHANGE_LAST_WRITE|FILE_NOTIFY_CHANGE_FILE_NAME;

	oHandle hMonitor;
	oIOCP* pIOCP; // Because of IOCP requirements, lifetime management of the IOCP is special
	oSTREAM_MONITOR_DESC MDesc;
	oSTREAM_ON_EVENT OnEvent;
	oInitOnce<oStd::path_string> MonitorPath;
	oInitOnce<oStd::path_string> MonitorFilename;
	oInitOnce<oURIParts> URIParts;
	oInitOnce<oSTREAM_DESC> Desc;

	DWORD FileNotifyInfoBuffers[2][kBufferSize];
	int FileNotifyInfoBufferIndex;
	oStd::future<void> MonitorFinished;

	mutex DeferredEventMutex;

	struct oEVENT_RECORD
	{
		double LastEventTimestamp;
		oSTREAM_EVENT LastEvent;
		bool Handled;
	};

	mutex CheckAccessibleMutex;
	std::map<oStd::uri_string, oEVENT_RECORD, oStd::less_case_insensitive<oStd::uri_string>> EventRecords;
	std::vector<oStd::uri_string> Accessibles;
	HANDLE hTimerQueue;

	bool Closing;
};

struct oFileSchemeHandlerImpl : oFileSchemeHandler
{
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_TRIVIAL_QUERYINTERFACE2(oSchemeHandler, oFileSchemeHandler);

	int GetOrder() const threadsafe { return 1000; }
	const char* GetScheme() const threadsafe { return "file"; }
	bool GetDesc(const oURIParts& _URI, oSTREAM_DESC* _pDesc) threadsafe override;
	bool Copy(const oURIParts& _Source, const oURIParts& _Destination, bool _Recursive = true) threadsafe override;
	bool Move(const oURIParts& _Source, const oURIParts& _Destination, bool _OverwriteDestination = false) threadsafe override;
	bool Delete(const oURIParts& _URIParts) threadsafe override;
	bool CreateStreamReader(const oURIParts& _URIParts, threadsafe oStreamReader** _ppReader) threadsafe override;
	bool CreateStreamReaderNonBuffered4K(const oURIParts& _URIParts, threadsafe oStreamReader** _ppReader) threadsafe override;
	bool CreateStreamWriter(const oURIParts& _URIParts, bool _SupportAsyncWrites, threadsafe oStreamWriter** _ppWriter) threadsafe override;
	bool CreateStreamMonitor(const oURIParts& _URIParts, const oSTREAM_MONITOR_DESC& _Desc, const oSTREAM_ON_EVENT& _OnEvent, threadsafe oStreamMonitor** _ppMonitor) threadsafe override;

protected:
	oRefCount RefCount;
};

bool oFileSchemeHandlerCreate(threadsafe oFileSchemeHandler** _ppFileSchemeHandler)
{
	bool success = true;
	oCONSTRUCT(_ppFileSchemeHandler, oFileSchemeHandlerImpl());
	return success;
}

bool oFileSchemeHandlerImpl::GetDesc(const oURIParts& _URIParts, oSTREAM_DESC* _pDesc) threadsafe
{
	oStd::path_string Path;
	oSystemURIPartsToPath(Path, _URIParts);
	return oFileGetDesc(Path, _pDesc);
}

bool oFileSchemeHandlerImpl::Copy(const oURIParts& _Source, const oURIParts& _Destination, bool _Recursive) threadsafe
{
	oStd::path_string S, D;
	oSystemURIPartsToPath(S, _Source);
	oSystemURIPartsToPath(D, _Destination);
	return oFileCopy(S, D, _Recursive);
}

bool oFileSchemeHandlerImpl::Move(const oURIParts& _Source, const oURIParts& _Destination, bool _OverwriteDestination) threadsafe
{
	oStd::path_string S, D;
	oSystemURIPartsToPath(S, _Source);
	oSystemURIPartsToPath(D, _Destination);
	return oFileMove(S, D, _OverwriteDestination);
}

bool oFileSchemeHandlerImpl::Delete(const oURIParts& _URIParts) threadsafe
{
	oStd::path_string Path;
	oSystemURIPartsToPath(Path, _URIParts);
	return oFileDelete(Path);
}

bool oFileSchemeHandlerImpl::CreateStreamReader(const oURIParts& _URIParts, threadsafe oStreamReader** _ppReader) threadsafe
{
	bool success = false;
	oCONSTRUCT(_ppReader, oFileReaderImpl(_URIParts, false, &success));
	return success;
}

bool oFileSchemeHandlerImpl::CreateStreamReaderNonBuffered4K(const oURIParts& _URIParts, threadsafe oStreamReader** _ppReader) threadsafe
{
	bool success = false;
	oCONSTRUCT(_ppReader, oFileReaderImpl(_URIParts, true, &success));
	return success;
}

bool oFileSchemeHandlerImpl::CreateStreamWriter(const oURIParts& _URIParts, bool _SupportAsyncWrites, threadsafe oStreamWriter** _ppWriter) threadsafe
{
	bool success = false;
	oCONSTRUCT(_ppWriter, oFileWriterImpl(_URIParts, _SupportAsyncWrites, &success));
	return success;
}

bool oFileSchemeHandlerImpl::CreateStreamMonitor(const oURIParts& _URIParts, const oSTREAM_MONITOR_DESC& _Desc, const oSTREAM_ON_EVENT& _OnEvent, threadsafe oStreamMonitor** _ppMonitor) threadsafe
{
	bool success = false;
	oCONSTRUCT(_ppMonitor, oFileMonitorImpl(_URIParts, _Desc, _OnEvent, &success));
	return success;
}
