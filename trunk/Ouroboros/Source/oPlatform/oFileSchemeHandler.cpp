/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2011 Antony Arciuolo & Kevin Myers                       *
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
#include "oDispatchQueueGlobalIOCP.h"
#include "oFileInternal.h"
#include "oIOCP.h"

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
	{
		*_pSuccess = false;

		oStringPath Path;
		oSystemURIPartsToPath(Path, _URIParts);
		hFile = CreateFile(Path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, Unbuffered ? FILE_FLAG_NO_BUFFERING : 0, NULL);
		if (hFile == INVALID_HANDLE_VALUE)
		{
			oWinSetLastError();
			return;
		}
		else
		{
			oSTREAM_DESC FDesc;
			oFileGetDesc(Path, &FDesc);
			Desc.Initialize(FDesc);

			if (!oDispatchQueueCreateGlobalIOCP("File reader dispatch queue", 10, &ReadQueue))
			{
				oErrorSetLast(oERROR_INVALID_PARAMETER, "Could not create IOCP dispatch queue.");
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
			fileSize = oByteAlign(fileSize, UNBUFFERED_ALIGN_REQUIREMENT); // unbuffered io is always a multiple of the alignment
		
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
			oErrorSetLast(oERROR_IO, "Specified range will read past file size");
			_Continuation(false, this, _StreamRead);
			return;
		}
	}

	bool Read(const oSTREAM_READ& _StreamRead) threadsafe
	{
		oASSERT(!Unbuffered || (oIsByteAligned(_StreamRead.pData, UNBUFFERED_ALIGN_REQUIREMENT) && 
			oIsByteAligned(_StreamRead.Range.Offset, UNBUFFERED_ALIGN_REQUIREMENT) && oIsByteAligned(_StreamRead.Range.Size, UNBUFFERED_ALIGN_REQUIREMENT)), 
			"Unbuffered io has very restrict requirements. file offset, read size, and pointer address must be 4k aligned");

		DWORD numBytesRead;
		long offsetLow, offsetHigh;
		oSetHighLowOffset(offsetLow, offsetHigh, _StreamRead.Range.Offset);

		// Need to lock since SetFilePointer/ReadFile are separate calls. And to play nice with close
		oLockGuard<oMutex> lock(Mutex);

		if(hFile == INVALID_HANDLE_VALUE)
		{
			return oErrorSetLast(oERROR_CANCELED);
		}

		SetFilePointer(hFile, offsetLow, &offsetHigh, FILE_BEGIN);
		if (!ReadFile(hFile, _StreamRead.pData, oUInt(_StreamRead.Range.Size), &numBytesRead, nullptr))
			return oWinSetLastError();

		// Unbuffered io is inconsistent on what it returns for these numbers and 
		// it doesn't really apply for anyway because unbuffered always physically 
		// reads in an even number of sectors.
		if (!Unbuffered && numBytesRead != _StreamRead.Range.Size) // happens when trying to read past end of file, but ReadFile itself still succeeds
		{
			// failed, but not a bug if reading past end of file. 
			oASSERT(_StreamRead.Range.Offset + _StreamRead.Range.Size >= Desc->Size, "ReadFile didn't read enough bytes, but didn't read past end of file");
			return oErrorSetLast(oERROR_END_OF_FILE);
		}

		return true;
	}

	void GetDesc(oSTREAM_DESC* _pDesc) threadsafe override
	{
		*_pDesc = *Desc;
	}

	const oURIParts& GetURIParts() const threadsafe override { return *URIParts; }

	void Close() threadsafe override
	{
		oLockGuard<oMutex> lock(Mutex);

		if (hFile != INVALID_HANDLE_VALUE)
			CloseHandle(hFile);

		hFile = INVALID_HANDLE_VALUE;
	}

	oInitOnce<oSTREAM_DESC> Desc;
	oRef<threadsafe oDispatchQueueGlobal> ReadQueue;
	oMutex Mutex;
	oInitOnce<oURIParts> URIParts;
	HANDLE hFile;
	oRefCount RefCount;
	bool Unbuffered;
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
				oErrorSetLast(oERROR_INVALID_PARAMETER, "Could not create IOCP dispatch queue.");
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
		oLockGuard<oMutex> lock(Mutex);

		if(hFile == INVALID_HANDLE_VALUE)
		{
			return oErrorSetLast(oERROR_CANCELED);
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
			return oErrorSetLast(oERROR_IO, "Failed to write data to disk.");

		return true;
	}

	const oURIParts& GetURIParts() const threadsafe override { return *URIParts; }

	void Close() threadsafe override
	{
		oLockGuard<oMutex> lock(Mutex);

		if (hFile != INVALID_HANDLE_VALUE)
			CloseHandle(hFile);

		hFile = INVALID_HANDLE_VALUE;
	}

	oInitOnce<oURIParts> URIParts;
	oInitOnce<oStringPath> ResolvedPath;
	oHandle hFile;
	oRef<threadsafe oDispatchQueueGlobal> WriteQueue;
	oMutex Mutex;
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

struct oFileMonitorImpl : public oStreamMonitor
{
	oDEFINE_NOOP_QUERYINTERFACE();

	static VOID CALLBACK WaitOrTimerCallback(PVOID lpParameter, BOOLEAN TimerOrWaitFired)
	{
		static_cast<oFileMonitorImpl*>(lpParameter)->CheckAccessible();
	}

	oFileMonitorImpl(const oURIParts& _URIParts, const oSTREAM_ON_EVENT& _OnEvent, bool* _pSuccess)
		: URIParts(_URIParts)
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

		oStringPath Path;
		if (!oSystemURIPartsToPath(Path, _URIParts))
		{
			oStringURI URI;
			oURIRecompose(URI, _URIParts);
			oErrorSetLast(oERROR_INVALID_PARAMETER, "Invalid uri: %s", URI.c_str());
			return;
		}

		oSTREAM_DESC fd;
		if (!oFileGetDesc(Path, &fd))
		{
			oStringURI URI;
			oURIRecompose(URI, _URIParts);
			oErrorSetLast(oERROR_INVALID_PARAMETER, "Invalid uri: %s", URI.c_str());
			return;
		}

		// @oooii-tony; We SHOULD support this soon, but first bring over things 
		// AS-IS
		//oStringPath Parent(Path);
		//*oGetFilebase(Path) = 0;
		
		if (!fd.Directory)
		{
			oStringURI URI;
			oURIRecompose(URI, _URIParts);
			oErrorSetLast(oERROR_INVALID_PARAMETER, "Invalid uri (files not yet supported): %s", URI.c_str());
			return;
		}

		Desc.Initialize(fd);

		HANDLE hTimer = nullptr;
		if (!CreateTimerQueueTimer(
			&hTimer
			, hTimerQueue
			, oFileMonitorImpl::WaitOrTimerCallback
			, this
			, 1000 // @oooii-tony: Might want to expose this timeout value to client code
			, 1000
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
			oErrorSetLast(oERROR_INVALID_PARAMETER, "Could not create IOCP.");
			return;
		}

		oIOCPOp* pOp = pIOCP->AcquireSocketOp();
		if (!ReadDirectoryChangesW(hMonitor, FileNotifyInfoBuffers[FileNotifyInfoBufferIndex], kBufferSize, true, kWatchChanges, nullptr, pOp, nullptr))
		{
			oErrorSetLast(oERROR_GENERIC, "Could not initiate a windows request to monitor a folder.");
			return;
		}

		*_pSuccess = true;
	}

	~oFileMonitorImpl()
	{
		oLockGuard<oMutex> lock(CheckAccessibleMutex);

		Closing = true;
		if (hMonitor != INVALID_HANDLE_VALUE)
			CloseHandle((HANDLE)hMonitor);

		if (MonitorFinished.valid())
			MonitorFinished.wait();

		oVB(DeleteTimerQueue(hTimerQueue));
	}

	void OnDirectoryChanges(FILE_NOTIFY_INFORMATION* _pNotify)
	{
		FILE_NOTIFY_INFORMATION* n = _pNotify;
		double Now = oTimer();
		do
		{
			oSTREAM_EVENT e = oAsStreamEvent(n->Action);
			if (e != oSTREAM_UNSUPPORTED)
			{
				oASSERT(n->FileNameLength < (oStringPath::Capacity*2), "not expecting paths that are longer than they should be"); // *2 for that fact that FileNameLength is # bytes for a wide-char non-null-term string

				// note the path given to us by windows is only the portion after the 
				// path of the folder we are monitoring, so have to reform the full uri.

				// These strings aren't null-terminated, AND in wide-character. Make it
				// not this way...
				wchar_t ForNullTermination[oStringPath::Capacity];
				memcpy(ForNullTermination, n->FileName, n->FileNameLength);
				ForNullTermination[n->FileNameLength / 2] = 0;
				oStringPath path(ForNullTermination);

				oURIParts parts;
				parts = *URIParts;
				oStrAppendf(parts.Path, "/%s", path.c_str());

				oStringURI URI;
				oVERIFY(oURIRecompose(URI, parts));

				if (e == oSTREAM_ADDED)
				{
					oLockGuard<oMutex> lock(DeferredEventMutex);
					auto it = EventRecords.find(URI);
					if (it == EventRecords.end())
					{
						oEVENT_RECORD r;
						r.LastEventTimestamp = Now;
						r.LastEvent = e;
						EventRecords[URI] = r;
					}
					else
					{
						it->second.LastEventTimestamp = Now;
						it->second.LastEvent = e;
					}
				}

				oTRACE_MONITOR("%s: %s", oAsString(e), URI.c_str());
				OnEvent(e, URI);
			}

			n = oByteAdd(n, n->NextEntryOffset);

		} while (n->NextEntryOffset);
	}

	void CheckAccessible()
	{
		oLockGuard<oMutex> lock(CheckAccessibleMutex);

		if (Closing)
			return;

		if (!EventRecords.empty())
		{
			Accessibles.clear();
			{
				oLockGuard<oMutex> lock(DeferredEventMutex);
				for (auto it = EventRecords.cbegin(); it != EventRecords.cend(); /* no increment */)
				{
					oStringPath path;
					oURIParts p;
					oURIDecompose(it->first, &p);
					oSystemURIPartsToPath(path, p);
				
					bool CanAccessFile = false;
					HANDLE hFile = CreateFile(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
					if (hFile != INVALID_HANDLE_VALUE)
					{
						CanAccessFile = true;
						oVB(CloseHandle(hFile));
					}

					const double kEventTimeout = 1.0;
					if (CanAccessFile)
					{
						Accessibles.push_back(it->first);
						EventRecords.erase(it++);
					}
					else
						++it;
				}
			}
			for (auto it = Accessibles.cbegin(); it != Accessibles.cend(); ++it)
			{
				oTRACE_MONITOR("oSTREAM_ACCESSIBLE: %s", it->c_str());
				OnEvent(oSTREAM_ACCESSIBLE, *it);
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
		MonitorFinished = oStd::async([this, FileNotifyInfoBufferIndexCopy] { OnDirectoryChanges(reinterpret_cast<FILE_NOTIFY_INFORMATION*>(FileNotifyInfoBuffers[FileNotifyInfoBufferIndexCopy])); });

		FileNotifyInfoBufferIndex = (FileNotifyInfoBufferIndex + 1) % 2;
	
		oIOCPOp* pOp = pIOCP->AcquireSocketOp();
		if (!ReadDirectoryChangesW(hMonitor, FileNotifyInfoBuffers[FileNotifyInfoBufferIndex], sizeof(FileNotifyInfoBuffers[FileNotifyInfoBufferIndex]), true, kWatchChanges, nullptr, pOp, nullptr) && !Closing)
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
	oSTREAM_ON_EVENT OnEvent;
	oInitOnce<oStringPath> MonitorPath;
	oInitOnce<oURIParts> URIParts;
	oInitOnce<oSTREAM_DESC> Desc;

	DWORD FileNotifyInfoBuffers[2][kBufferSize];
	int FileNotifyInfoBufferIndex;
	oStd::future<void> MonitorFinished;

	oMutex DeferredEventMutex;

	struct oEVENT_RECORD
	{
		double LastEventTimestamp;
		oSTREAM_EVENT LastEvent;
	};

	oMutex CheckAccessibleMutex;
	std::map<oStringURI, oEVENT_RECORD, oStdLessI<oStringURI>> EventRecords;
	std::vector<oStringURI> Accessibles;
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
	bool CreateStreamMonitor(const oURIParts& _URIParts, const oSTREAM_ON_EVENT& _OnEvent, threadsafe oStreamMonitor** _ppMonitor) threadsafe override;

protected:
	oRefCount RefCount;
};

const oGUID& oGetGUID(threadsafe const oFileSchemeHandler* threadsafe const *)
{
	// {684A582C-240E-4C7D-839C-CF4897F2F5D7}
	static const oGUID guid = { 0x684a582c, 0x240e, 0x4c7d, { 0x83, 0x9c, 0xcf, 0x48, 0x97, 0xf2, 0xf5, 0xd7 } };
	return guid;
}

bool oFileSchemeHandlerCreate(threadsafe oFileSchemeHandler** _ppFileSchemeHandler)
{
	bool success = true;
	oCONSTRUCT(_ppFileSchemeHandler, oFileSchemeHandlerImpl());
	return success;
}

bool oFileSchemeHandlerImpl::GetDesc(const oURIParts& _URIParts, oSTREAM_DESC* _pDesc) threadsafe
{
	oStringPath Path;
	oSystemURIPartsToPath(Path, _URIParts);
	return oFileGetDesc(Path, _pDesc);
}

bool oFileSchemeHandlerImpl::Copy(const oURIParts& _Source, const oURIParts& _Destination, bool _Recursive) threadsafe
{
	oStringPath S, D;
	oSystemURIPartsToPath(S, _Source);
	oSystemURIPartsToPath(D, _Destination);
	return oFileCopy(S, D, _Recursive);
}

bool oFileSchemeHandlerImpl::Move(const oURIParts& _Source, const oURIParts& _Destination, bool _OverwriteDestination) threadsafe
{
	oStringPath S, D;
	oSystemURIPartsToPath(S, _Source);
	oSystemURIPartsToPath(D, _Destination);
	return oFileMove(S, D, _OverwriteDestination);
}

bool oFileSchemeHandlerImpl::Delete(const oURIParts& _URIParts) threadsafe
{
	oStringPath Path;
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

bool oFileSchemeHandlerImpl::CreateStreamMonitor(const oURIParts& _URIParts, const oSTREAM_ON_EVENT& _OnEvent, threadsafe oStreamMonitor** _ppMonitor) threadsafe
{
	bool success = false;
	oCONSTRUCT(_ppMonitor, oFileMonitorImpl(_URIParts, _OnEvent, &success));
	return success;
}