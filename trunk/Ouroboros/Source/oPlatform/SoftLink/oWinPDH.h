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
#pragma once
#ifndef oWinPDH_h
#define oWinPDH_h

#include <oPlatform/oModuleUtil.h>
#include <pdh.h>

struct oWinPDH : oProcessSingleton<oWinPDH>
{
	static const oGUID GUID;
	oWinPDH();
	~oWinPDH();
	PDH_STATUS (__stdcall *PdhGetFormattedCounterValue)(PDH_HCOUNTER hCounter, DWORD dwFormat, LPDWORD lpdwType, PPDH_FMT_COUNTERVALUE pValue);
	PDH_STATUS (__stdcall *PdhAddCounterA)(PDH_HQUERY hQuery, LPCTSTR szFullCounterPath, DWORD_PTR dwUserData, PDH_HCOUNTER *phCounter);
	PDH_STATUS (__stdcall *PdhOpenQueryA)(LPCTSTR szDataSource, DWORD_PTR dwUserData, PDH_HQUERY *phQuery);
	PDH_STATUS (__stdcall *PdhCloseQuery)(PDH_HQUERY hQuery);
	PDH_STATUS (__stdcall *PdhCollectQueryData)(PDH_HQUERY hQuery);

	// returns the percent of total system CPU usage used between two successive
	// calls to this API.
	inline double GetSystemCPUUsage() { GetCounterAsDouble(hQuery, hSystemCPUUsage, &SystemCPUCounter); return SystemCPUCounter.doubleValue; }

	// returns the percent of the total system CPU used by this process
	inline double GetProcessCPUUsage() { GetCounterAsDouble(hQuery, hProcessCPUUsage, &ProcessCPUCounter); return ProcessCPUCounter.doubleValue; }

	void GetCounterAsDouble(PDH_HQUERY _hQuery, PDH_HCOUNTER _hCounter, PDH_FMT_COUNTERVALUE* _pValue);

protected: 
	oHMODULE hModule;
	PDH_HQUERY hQuery;
	double QueryDataTimestamp;
	PDH_HCOUNTER hSystemCPUUsage;
	PDH_HCOUNTER hProcessCPUUsage;
	PDH_FMT_COUNTERVALUE SystemCPUCounter;
	PDH_FMT_COUNTERVALUE ProcessCPUCounter;
};

#endif
