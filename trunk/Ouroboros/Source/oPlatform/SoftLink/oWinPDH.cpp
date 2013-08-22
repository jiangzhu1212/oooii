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
#include "oWinPDH.h"
#include <oStd/assert.h>
#include <oPlatform/oSystem.h>
#include <PdhMsg.h>

// @oooii-tony: I cannot get this to work. If I put the query stuff in a loop in
// its own thread that sleeps for 1000ms and wakes up to query counters... it 
// never reports even close to what the Windows Task Manager reports, and is 
// nowhere close to oProcessCalculateCPUUsage's methodology. What gives? Also 
// there are bugs in this pdh stuff...
// http://social.msdn.microsoft.com/Forums/hr/perfctr/thread/bba5cd74-a4fd-4c8f-bded-8cb4fc8e3c00
// So I'm not sure what to think. If you have time, take a look and see if I'm
// doing something dumb.

const char* oAsStringPDH(PDH_STATUS _Status)
{
	switch (_Status)
	{
		case PDH_CSTATUS_VALID_DATA: return "PDH_CSTATUS_VALID_DATA";
		case PDH_CSTATUS_NEW_DATA: return "PDH_CSTATUS_NEW_DATA";
		case PDH_CSTATUS_NO_MACHINE: return "PDH_CSTATUS_NO_MACHINE";
		case PDH_CSTATUS_NO_INSTANCE: return "PDH_CSTATUS_NO_INSTANCE";
		case PDH_MORE_DATA: return "PDH_MORE_DATA";
		case PDH_CSTATUS_ITEM_NOT_VALIDATED: return "PDH_CSTATUS_ITEM_NOT_VALIDATED";
		case PDH_RETRY: return "PDH_RETRY";
		case PDH_NO_DATA: return "PDH_NO_DATA";
		case PDH_CALC_NEGATIVE_DENOMINATOR: return "PDH_CALC_NEGATIVE_DENOMINATOR";
		case PDH_CALC_NEGATIVE_TIMEBASE: return "PDH_CALC_NEGATIVE_TIMEBASE";
		case PDH_CALC_NEGATIVE_VALUE: return "PDH_CALC_NEGATIVE_VALUE";
		case PDH_DIALOG_CANCELLED: return "PDH_DIALOG_CANCELLED";
		case PDH_END_OF_LOG_FILE: return "PDH_END_OF_LOG_FILE";
		case PDH_ASYNC_QUERY_TIMEOUT: return "PDH_ASYNC_QUERY_TIMEOUT";
		case PDH_CANNOT_SET_DEFAULT_REALTIME_DATASOURCE: return "PDH_CANNOT_SET_DEFAULT_REALTIME_DATASOURCE";
		case PDH_UNABLE_MAP_NAME_FILES: return "PDH_UNABLE_MAP_NAME_FILES";
		case PDH_PLA_VALIDATION_WARNING: return "PDH_PLA_VALIDATION_WARNING";
		case PDH_CSTATUS_NO_OBJECT: return "PDH_CSTATUS_NO_OBJECT";
		case PDH_CSTATUS_NO_COUNTER: return "PDH_CSTATUS_NO_COUNTER";
		case PDH_CSTATUS_INVALID_DATA: return "PDH_CSTATUS_INVALID_DATA";
		case PDH_MEMORY_ALLOCATION_FAILURE: return "PDH_MEMORY_ALLOCATION_FAILURE";
		case PDH_INVALID_HANDLE: return "PDH_INVALID_HANDLE";
		case PDH_INVALID_ARGUMENT: return "PDH_INVALID_ARGUMENT";
		case PDH_FUNCTION_NOT_FOUND: return "PDH_FUNCTION_NOT_FOUND";
		case PDH_CSTATUS_NO_COUNTERNAME: return "PDH_CSTATUS_NO_COUNTERNAME";
		case PDH_CSTATUS_BAD_COUNTERNAME: return "PDH_CSTATUS_BAD_COUNTERNAME";
		case PDH_INVALID_BUFFER: return "PDH_INVALID_BUFFER";
		case PDH_INSUFFICIENT_BUFFER: return "PDH_INSUFFICIENT_BUFFER";
		case PDH_CANNOT_CONNECT_MACHINE: return "PDH_CANNOT_CONNECT_MACHINE";
		case PDH_INVALID_PATH: return "PDH_INVALID_PATH";
		case PDH_INVALID_INSTANCE: return "PDH_INVALID_INSTANCE";
		case PDH_INVALID_DATA: return "PDH_INVALID_DATA";
		case PDH_NO_DIALOG_DATA: return "PDH_NO_DIALOG_DATA";
		case PDH_CANNOT_READ_NAME_STRINGS: return "PDH_CANNOT_READ_NAME_STRINGS";
		case PDH_LOG_FILE_CREATE_ERROR: return "PDH_LOG_FILE_CREATE_ERROR";
		case PDH_LOG_FILE_OPEN_ERROR: return "PDH_LOG_FILE_OPEN_ERROR";
		case PDH_LOG_TYPE_NOT_FOUND: return "PDH_LOG_TYPE_NOT_FOUND";
		case PDH_NO_MORE_DATA: return "PDH_NO_MORE_DATA";
		case PDH_ENTRY_NOT_IN_LOG_FILE: return "PDH_ENTRY_NOT_IN_LOG_FILE";
		case PDH_DATA_SOURCE_IS_LOG_FILE: return "PDH_DATA_SOURCE_IS_LOG_FILE";
		case PDH_DATA_SOURCE_IS_REAL_TIME: return "PDH_DATA_SOURCE_IS_REAL_TIME";
		case PDH_UNABLE_READ_LOG_HEADER: return "PDH_UNABLE_READ_LOG_HEADER";
		case PDH_FILE_NOT_FOUND: return "PDH_FILE_NOT_FOUND";
		case PDH_FILE_ALREADY_EXISTS: return "PDH_FILE_ALREADY_EXISTS";
		case PDH_NOT_IMPLEMENTED: return "PDH_NOT_IMPLEMENTED";
		case PDH_STRING_NOT_FOUND: return "PDH_STRING_NOT_FOUND";
		case PDH_UNKNOWN_LOG_FORMAT: return "PDH_UNKNOWN_LOG_FORMAT";
		case PDH_UNKNOWN_LOGSVC_COMMAND: return "PDH_UNKNOWN_LOGSVC_COMMAND";
		case PDH_LOGSVC_QUERY_NOT_FOUND: return "PDH_LOGSVC_QUERY_NOT_FOUND";
		case PDH_LOGSVC_NOT_OPENED: return "PDH_LOGSVC_NOT_OPENED";
		case PDH_WBEM_ERROR: return "PDH_WBEM_ERROR";
		case PDH_ACCESS_DENIED: return "PDH_ACCESS_DENIED";
		case PDH_LOG_FILE_TOO_SMALL: return "PDH_LOG_FILE_TOO_SMALL";
		case PDH_INVALID_DATASOURCE: return "PDH_INVALID_DATASOURCE";
		case PDH_INVALID_SQLDB: return "PDH_INVALID_SQLDB";
		case PDH_NO_COUNTERS: return "PDH_NO_COUNTERS";
		case PDH_SQL_ALLOC_FAILED: return "PDH_SQL_ALLOC_FAILED";
		case PDH_SQL_ALLOCCON_FAILED: return "PDH_SQL_ALLOCCON_FAILED";
		case PDH_SQL_EXEC_DIRECT_FAILED: return "PDH_SQL_EXEC_DIRECT_FAILED";
		case PDH_SQL_FETCH_FAILED: return "PDH_SQL_FETCH_FAILED";
		case PDH_SQL_ROWCOUNT_FAILED: return "PDH_SQL_ROWCOUNT_FAILED";
		case PDH_SQL_MORE_RESULTS_FAILED: return "PDH_SQL_MORE_RESULTS_FAILED";
		case PDH_SQL_CONNECT_FAILED: return "PDH_SQL_CONNECT_FAILED";
		case PDH_SQL_BIND_FAILED: return "PDH_SQL_BIND_FAILED";
		case PDH_CANNOT_CONNECT_WMI_SERVER: return "PDH_CANNOT_CONNECT_WMI_SERVER";
		case PDH_PLA_COLLECTION_ALREADY_RUNNING: return "PDH_PLA_COLLECTION_ALREADY_RUNNING";
		case PDH_PLA_ERROR_SCHEDULE_OVERLAP: return "PDH_PLA_ERROR_SCHEDULE_OVERLAP";
		case PDH_PLA_COLLECTION_NOT_FOUND: return "PDH_PLA_COLLECTION_NOT_FOUND";
		case PDH_PLA_ERROR_SCHEDULE_ELAPSED: return "PDH_PLA_ERROR_SCHEDULE_ELAPSED";
		case PDH_PLA_ERROR_NOSTART: return "PDH_PLA_ERROR_NOSTART";
		case PDH_PLA_ERROR_ALREADY_EXISTS: return "PDH_PLA_ERROR_ALREADY_EXISTS";
		case PDH_PLA_ERROR_TYPE_MISMATCH: return "PDH_PLA_ERROR_TYPE_MISMATCH";
		case PDH_PLA_ERROR_FILEPATH: return "PDH_PLA_ERROR_FILEPATH";
		case PDH_PLA_SERVICE_ERROR: return "PDH_PLA_SERVICE_ERROR";
		case PDH_PLA_VALIDATION_ERROR: return "PDH_PLA_VALIDATION_ERROR";
		case PDH_PLA_ERROR_NAME_TOO_LONG: return "PDH_PLA_ERROR_NAME_TOO_LONG";
		case PDH_INVALID_SQL_LOG_FORMAT: return "PDH_INVALID_SQL_LOG_FORMAT";
		case PDH_COUNTER_ALREADY_IN_QUERY: return "PDH_COUNTER_ALREADY_IN_QUERY";
		case PDH_BINARY_LOG_CORRUPT: return "PDH_BINARY_LOG_CORRUPT";
		case PDH_LOG_SAMPLE_TOO_SMALL: return "PDH_LOG_SAMPLE_TOO_SMALL";
		case PDH_OS_LATER_VERSION: return "PDH_OS_LATER_VERSION";
		case PDH_OS_EARLIER_VERSION: return "PDH_OS_EARLIER_VERSION";
		case PDH_INCORRECT_APPEND_TIME: return "PDH_INCORRECT_APPEND_TIME";
		case PDH_UNMATCHED_APPEND_COUNTER: return "PDH_UNMATCHED_APPEND_COUNTER";
		case PDH_SQL_ALTER_DETAIL_FAILED: return "PDH_SQL_ALTER_DETAIL_FAILED";
		case PDH_QUERY_PERF_DATA_TIMEOUT: return "PDH_QUERY_PERF_DATA_TIMEOUT";
		case MSG_Publisher_Name: return "MSG_Publisher_Name";
		oNODEFAULT;
	}
}

#define oPDH_V(_fn) do { PDH_STATUS STATUS__ = _fn; oASSERT(STATUS__ == ERROR_SUCCESS,  #_fn "(PDH API) failed: %s", oAsStringPDH(STATUS__)); } while(false)

static const char* sExportedAPIs[] = 
{
	"PdhGetFormattedCounterValue",
	"PdhAddCounterA",
	"PdhOpenQueryA",
	"PdhCloseQuery",
	"PdhCollectQueryData",
};

oWinPDH::oWinPDH()
{
	// available counters: http://technet.microsoft.com/en-us/library/cc780836(v=ws.10).aspx

	hModule = oModuleLinkSafe("pdh.dll", sExportedAPIs, (void**)&PdhGetFormattedCounterValue, oCOUNTOF(sExportedAPIs));
	oPDH_V(PdhOpenQuery(nullptr, 0, &hQuery));
	oPDH_V(PdhAddCounter(hQuery, "\\Processor(_Total)\\% Processor Time", 0, &hSystemCPUUsage));
	
	oStd::path_string ProcessPath;
	oVERIFY(oSystemGetPath(ProcessPath, oSYSPATH_APP_FULL));

	oStd::path_string ProcessName = oGetFilebase(ProcessPath);
	*oGetFileExtension(ProcessName) = 0;

	oStd::path_string CounterPath;
	oPrintf(CounterPath, "\\Process(%s)\\%% Processor Time", ProcessName.c_str());
	oPDH_V(PdhAddCounter(hQuery, CounterPath, 0, &hProcessCPUUsage));
	
	// the idea here is to seed values so that first-calls to get stats functions
	// work more accurately.
	oPDH_V(PdhCollectQueryData(hQuery));

	QueryDataTimestamp = oTimer();
}

oWinPDH::~oWinPDH()
{
	if (hQuery)
		oPDH_V(PdhCloseQuery(hQuery));

	oModuleUnlink(hModule);
}

void oWinPDH::GetCounterAsDouble(PDH_HQUERY _hQuery, PDH_HCOUNTER _hCounter, PDH_FMT_COUNTERVALUE* _pValue)
{
	oASSERT(false, "not ready for usage. See comment at top of oWinPDH.cpp");

	if ((QueryDataTimestamp + 1.0) < oTimer())
	{
		oPDH_V(PdhCollectQueryData(_hQuery));
		PdhGetFormattedCounterValue(_hCounter, PDH_FMT_DOUBLE, 0, _pValue);
	}
}

// {B2CED12A-A78F-45CD-911E-FF7B82B66D11}
const oGUID oWinPDH::GUID = { 0xb2ced12a, 0xa78f, 0x45cd, { 0x91, 0x1e, 0xff, 0x7b, 0x82, 0xb6, 0x6d, 0x11 } };
