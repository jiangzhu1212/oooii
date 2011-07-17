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
#include <oooii/oImage.h>
#include <oooii/oFilterChain.h>
#include <oooii/oNoncopyable.h>
#include <oooii/oString.h>

#define oTESTERROR(format, ...) do { sprintf_s(_StrStatus, _SizeofStrStatus, format, ## __VA_ARGS__); oTRACE("FAILING: %s (GetLastError() == %s (%s))", _StrStatus, oGetErrnoString(oGetLastError()), oGetLastErrorDesc()); return oTest::FAILURE; } while(0)
#define oTESTB(expr, errMsg, ...) do { if (!(expr)) { oTESTERROR(errMsg, ## __VA_ARGS__); } } while(0)
#define oTESTI(oImagePointer) oTESTB(TestImage(oImagePointer), "Image compare failed: %s: %s", oGetErrnoString(oGetLastError()), oGetLastErrorDesc());
#define oTESTI2(oImagePointer, NthFrame) oTESTB(TestImage(oImagePointer, NthFrame), "Image compare (%u%s frame) failed: %s: %s", NthFrame, oOrdinal(NthFrame), oGetErrnoString(oGetLastError()), oGetLastErrorDesc());

#define oTEST_REGISTER(_TestClassName) oTestManager::RegisterTest<_TestClassName> _TestClassName##_Instance;

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
	virtual const char* GetName() const;

	// Visual tests should prepare an oImage and then use this API to submit the
	// test image to be compared against a "golden" image, one that has been 
	// verified as being correct. If valid, then this returns true. If the images
	// differ, then the test image that failed will be written to the OutputPath.
	bool TestImage(oImage* _pImage, unsigned int _NthImage = 0);

	virtual RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) = 0;
	static void BuildPath(char* _StrDestination, size_t _SizeofStrDestination, const char* _TestName, const char* _DataPath, const char* _DataSubpath, const char* _Path, unsigned int _NthImage, const char* _Ext);

	template<size_t size> inline void BuildPath(char (&_StrDestination)[size], const char* _TestName, const char* _DataPath, const char* _DataSubpath, const char* _Path, unsigned int _NthImage, const char* _Ext) { BuildPath(_StrDestination, size, _TestName, _DataPath, _DataSubpath, _Path, _NthImage, _Ext); }
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
	struct RegisterTestBase
	{
		RegisterTestBase();
		~RegisterTestBase();
		virtual oTest* New() = 0;
		virtual const char* GetTypename() const = 0;
		virtual bool IsSpecialTest() const = 0;
	};

	template<typename TestT> struct RegisterTest : RegisterTestBase
	{
		oTest* New() override { return new TestT(); }
		const char* GetTypename() const override { return typeid(TestT).name(); }
		bool IsSpecialTest() const override { return std::tr1::is_base_of<oSpecialTest, TestT>::value; }
	};

	struct DESC
	{
		DESC()
			: TestSuiteName(0)
			, DataPath(0)
			, GoldenPath(0)
			, OutputPath(0)
			, NameColumnWidth(20)
			, TimeColumnWidth(10)
			, StatusColumnWidth(10)
			, RandomSeed(0)
			, NumRunIterations(1)
			, ImageFuzziness(2)
			, PixelPercentageMinSuccess(100)
			, DiffImageMultiplier(4)
			, TestTooSlowTimeInSeconds(10.0f)
			, TestReallyTooSlowTimeInSeconds(20.0f)
		{}

		const char* TestSuiteName;
		const char* DataPath;
		const char* GoldenPath;
		const char* OutputPath;
		unsigned int NameColumnWidth;
		unsigned int TimeColumnWidth;
		unsigned int StatusColumnWidth;
		unsigned int RandomSeed;
		unsigned int NumRunIterations;
		unsigned int ImageFuzziness;
		unsigned int PixelPercentageMinSuccess;
		unsigned int DiffImageMultiplier;
		float TestTooSlowTimeInSeconds;
		float TestReallyTooSlowTimeInSeconds;
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

	virtual bool FindFullPath(char* _StrFullPath, size_t _SizeofStrFullPath, const char* _StrRelativePath) const = 0;
	template<size_t size> inline bool FindFullPath(char (&_StrFullPath)[size], const char* _StrRelativePath) { return FindFullPath(_StrFullPath, size, _StrRelativePath); }
};

bool oTestRunSpecialTest(const char* _SpecialTestName, char* _StrStatus, size_t _SizeofStrStatus, int* _pExitCode, threadsafe interface oProcess** _ppProcess = 0);

#endif
