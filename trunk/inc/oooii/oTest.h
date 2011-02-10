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
#pragma once
#ifndef oTest_h
#define oTest_h

#include <oooii/oInterface.h>
#include <oooii/oFilterChain.h>
#include <oooii/oNoncopyable.h>

#define oTESTERROR(format, ...) do { sprintf_s(_StrStatus, _SizeofStrStatus, format, ## __VA_ARGS__); oTRACE("FAILING: %s", _StrStatus); return oTest::FAILURE; } while(0)
#define oTESTB(expr, errMsg, ...) do { if (!(expr)) { oTESTERROR(errMsg, ## __VA_ARGS__); } } while(0)

interface oImage;

template<typename T>
struct oTestScopedArray
{
	// When using oTESTERROR or oTESTB, the application can return
	// early, so here provide an RAII method of allocating memory
	// so the macros can keep the code simple.

	oTestScopedArray(size_t _Count)
		: Pointer(new T[_Count])
		, Count(_Count)
	{}

	~oTestScopedArray() { delete [] Pointer; }
	T* GetPointer() { return Pointer; }
	const T* GetPointer() const { return Pointer; }
	size_t GetCount() const { return Count; }

	const T& operator[](size_t i) const { return Pointer[i]; }
	T& operator[](size_t i) { return Pointer[i]; }

protected:
	T* Pointer;
	size_t Count;
};

struct oTest : oNoncopyable
{
	enum RESULT
	{
		SUCCESS,
		FAILURE,
		FILTERED, // skipped by command line
		SKIPPED, // skipped by test itself
		LEAKS,
	};

	oTest();
	virtual ~oTest();
	const char* GetName() const;

	// Visual tests should prepare an oImage and then use this API to submit the
	// test image to be compared against a "golden" image, one that has been 
	// verified as being correct. If valid, then this returns true. If the images
	// differ, then the test image that failed will be written to the OutputPath.
	bool TestImage(oImage* _pImage, unsigned int _NthImage = 0);

	virtual RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) = 0;
};

// Special tests are ones that are new processes spawned 
// from a regular test used to test more complex inter-
// process functionality, such as a client-server model. 
// By deriving from this class, it means the user will 
// handle when this spawns and the test system will 
// otherwise skip over this.
struct oSpecialTest : public oTest
{
};

interface oTestManager : oNoncopyable
{
	struct DESC
	{
		DESC()
			: TestSuiteName(0)
			, DataPath(0)
			, GoldenPath(0)
			, OutputPath(0)
			, NameColumnWidth(20)
			, StatusColumnWidth(10)
			, RandomSeed(0)
			, NumRunIterations(1)
		{}

		const char* TestSuiteName;
		const char* DataPath;
		const char* GoldenPath;
		const char* OutputPath;
		unsigned int NameColumnWidth;
		unsigned int StatusColumnWidth;
		unsigned int RandomSeed;
		unsigned int NumRunIterations;
		// @oooii-tony: todo: Add redirect status, redirect printf
	};

	static oTestManager* Singleton();

	virtual void GetDesc(DESC* _pDesc) = 0;
	virtual void SetDesc(DESC* _pDesc) = 0;

	// RunTests can fail due to a bad compilation of filters. If this returns -1, check oGetLastError() for more
	// details.
	virtual int RunTests(oFilterChain::FILTER* _pTestFilters, size_t _SizeofTestFilters) = 0;
	template<size_t size> inline int RunTests(oFilterChain::FILTER (&_pTestFilters)[size]) { return RunTests(_pDesc, _pTestFilters, size); }

	// Special mode re-runs the test exe for tests that need a client-server 
	// multi-process setup.
	virtual int RunSpecialMode(const char* _Name) = 0;

	virtual bool RegisterSpecialMode(oSpecialTest* _pSpecialModeTest) = 0;

	virtual bool FindFullPath(char* _StrFullPath, size_t _SizeofStrFullPath, const char* _StrRelativePath) const = 0;
	template<size_t size> inline bool FindFullPath(char (&_StrFullPath)[size], const char* _StrRelativePath) { return FindFullPath(_StrFullPath, size, _StrRelativePath); }
};

bool oTestRunSpecialTest(const char* _SpecialTestName, char* _StrStatus, size_t _SizeofStrStatus, int* _pExitCode, threadsafe interface oProcess** _ppProcess);

#endif
