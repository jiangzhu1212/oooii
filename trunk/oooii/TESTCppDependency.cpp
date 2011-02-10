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
#include "pch.h"
#include <oooii/oRef.h>
#include <oooii/oBuffer.h>
#include <oooii/oStdio.h>
#include <oooii/oLockedPointer.h>
#include <oooii/oPath.h>
#include <oooii/oString.h>
#include <oooii/oTest.h>
#include <time.h>

const char* TESTxmmintrin_h =
	"#ifdef __ICL\n"
	"#ifdef _MM_FUNCTIONALITY\n"
	"#include \"xmm_func.h\"\n"
	"#else\n"
	"typedef long long __m128;\n"
	"	#endif\n"
	"#else\n"
	"\n"
	"... stuff ...\n"
	"\n"
	"#ifndef _INC_MALLOC\n"
	"#include <malloc.h>\n"
	"#endif\n"
	"#endif\n"
	"This is where the final pointer should be.\n";

const oMACRO TESTmacros0[] = 
{
	{0, 0},
};

const oMACRO TESTmacros1[] = 
{
	{ "__ICL", "1" },
	{ "_MM_FUNCTIONALITY", "1" },
	{0, 0},
};

const oMACRO TESTmacros2[] = 
{
	{ "__ICL", "1" },
	{0, 0},
};

const oMACRO* TESTmacros[] = 
{
	TESTmacros0,
	TESTmacros1,
	TESTmacros2,
};

const char* TESTcheckFor[] = 
{
	"... stuff ...",
	"#include \"xmm_func.h\"",
	"typedef long long __m128",
};

const char* TESTcheckNotThere[] = 
{
	"#include \"xmm_func.h\"",
	"typedef long long __m128",
	"#include \"xmm_func.h\"",
};

static time_t GetModifiedDate(const char* _Path)
{
	oFile::DESC desc;
	void* fc = 0;
	if (!oFile::FindFirst(&desc, _Path, &fc))
	{
		oSetLastError(ENOENT, "Found %s, but between then and this call the file is no longer available", oSAFESTRN(_Path));
		return 0;
	}

	oVERIFY(oFile::CloseFind(fc));

	return desc.Written;
}

static bool LoadBuffer(void* _pDestination, size_t _SizeofDestination, const char* _Path)
{
	return oLoadBuffer(_pDestination, _SizeofDestination, 0, _Path, true);
}

struct TESTCppDependency : public oTest
{
	struct ScopedFileStatus
	{
		ScopedFileStatus(const char* _Fullpath)
			: Fullpath(_Fullpath)
			, OriginalTimestamp()
		{
			oSetLastError(0);
			oFile::DESC desc;
			void* fc = 0;
			if (oFile::FindFirst(&desc, Fullpath, &fc))
			{
				oVERIFY(oFile::CloseFind(fc));
				OriginalTimestamp = desc.Written;
				Hidden = desc.Hidden;
				ReadOnly = desc.ReadOnly;
			}
			else
				oSetLastError(ENOENT, "Found %s, but between then and this call the file is no longer available", oSAFESTRN(Fullpath));
		}

		~ScopedFileStatus()
		{
			oFile::Touch(Fullpath, OriginalTimestamp);
			oFile::MarkHidden(Fullpath, Hidden);
			oFile::MarkReadOnly(Fullpath, ReadOnly);
		}

		const char* Fullpath;
		time_t OriginalTimestamp;
		bool Hidden;
		bool ReadOnly;
	};

	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override
	{
		const char* kTestSource = "TESTCppDependency.cpp";
		
		char compilerIncludePath[512];
		oTESTB(oGetSysPath(compilerIncludePath, oSYSPATH_COMPILER_INCLUDES), "Failed to find compiler include path: %s", oGetLastError());

		char headerSearchPath[2048];
		sprintf_s(headerSearchPath, ".;../inc/;%s", compilerIncludePath);

		char fullpath[_MAX_PATH];
		oTESTB(oTestManager::Singleton()->FindFullPath(fullpath, kTestSource), "Failed to find %s", kTestSource);

		threadsafe oRef<oBuffer> SourceCode;
		oTESTB(oBuffer::Create(fullpath, true, &SourceCode), "Failed to load %s", fullpath);

		oLockedPointer<oBuffer> pLockedSourceCode(SourceCode);

		oIFDEF_BLOCK blocks[16];
		oTESTB(oGetNextMatchingIfdefBlocks(blocks, 0, TESTxmmintrin_h, 0), "%s: %s", oGetErrnoString(oGetLastError()), oGetLastErrorDesc());

		// Find the end so we're sure where we left off
		{
			oIFDEF_BLOCK* pCurBlock = blocks;
			while (pCurBlock->Type != oIFDEF_BLOCK::ENDIF)
				pCurBlock++;
			oTESTB(!memcmp(pCurBlock->BlockEnd, "\nThis is", 8), "oGetNextMatchingIfdefBlocks() didn't leave off at the right place.");
		}

		char xmmintrin_h_copy[1024];

		for (size_t i = 0; i < oCOUNTOF(TESTmacros); i++)
		{
			strcpy_s(xmmintrin_h_copy, TESTxmmintrin_h);
			oZeroIfdefs(xmmintrin_h_copy, TESTmacros[i]);
			oTESTB(strstr(xmmintrin_h_copy, TESTcheckFor[i]), "Could not find expected string");
			oTESTB(!strstr(xmmintrin_h_copy, TESTcheckNotThere[i]), "Found string expected to be zeroed");
		}

		ScopedFileStatus restoreFileStatus(fullpath);
		oASSERT(oGetLastError() == 0, "");

		time_t tNow = restoreFileStatus.OriginalTimestamp;
		tm tmTime;
		localtime_s(&tmTime, &tNow);
		tmTime.tm_year += 2;
		time_t tFuture = mktime(&tmTime);

		tmTime.tm_year -= 4;
		time_t tPast = mktime(&tmTime);

		oFile::MarkReadOnly(fullpath, false);

		oTESTB(oFile::Touch(fullpath, tPast), "Touch %s with a past timestamp failed: %s: %s", fullpath, oGetErrnoString(oGetLastError()), oGetLastErrorDesc());
		oTESTB(!oHeadersAreUpToDate(pLockedSourceCode->GetData<const char>(), fullpath, 0, oFile::Exists, GetModifiedDate, LoadBuffer, headerSearchPath), "%s purposefully dated to in the past, but headers don't report as newer", fullpath);

		oTESTB(oFile::Touch(fullpath, tFuture), "Touch %s with a future timestamp failed: %s: %s", fullpath, oGetErrnoString(oGetLastError()), oGetLastErrorDesc());
		oTESTB(oHeadersAreUpToDate(pLockedSourceCode->GetData<const char>(), fullpath, 0, oFile::Exists, GetModifiedDate, LoadBuffer, headerSearchPath), "%s", oGetLastErrorDesc());

		return SUCCESS;
	}
};

TESTCppDependency TestCppDependency;
