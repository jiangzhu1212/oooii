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

#define oTESTERROR(format, ...) do { sprintf_s(_StrStatus, _SizeofStrStatus, format, ## __VA_ARGS__); oTRACE("FAILING: %s (GetLastError() == %s (%s))", _StrStatus, oGetErrnoString(oGetLastError()), oGetLastErrorDesc()); return oTest::FAILURE; } while(false)
#define oTESTB(expr, errMsg, ...) do { if (!(expr)) { oTESTERROR(errMsg, ## __VA_ARGS__); } } while(false)
#define oTESTI(oImagePointer) oTESTB(TestImage(oImagePointer), "Image compare failed: %s: %s", oGetErrnoString(oGetLastError()), oGetLastErrorDesc());
#define oTESTI2(oImagePointer, NthFrame) oTESTB(TestImage(oImagePointer, NthFrame), "Image compare (%u%s frame) failed: %s: %s", NthFrame, oOrdinal(NthFrame), oGetErrnoString(oGetLastError()), oGetLastErrorDesc());

// Use this to register tests that are expected to work
#define oTEST_REGISTER(_TestClassName) oTestManager::RegisterTest<_TestClassName> _TestClassName##_Instance;

// Use this to register test that spawn processes other than special-mode tests
#define oTEST_REGISTER_MULTIPROCESS(_TestClassName, _StrSemiColorDelimitedStringOfSpawnedProcesses) oTestManager::RegisterTest<_TestClassName> _TestClassName##_Instance(0, oTest::SUCCESS, _StrSemiColorDelimitedStringOfSpawnedProcesses);

// Use this to convert oTEST_REGISTER() to a skipped bug based on a known issue.
// Specify an ID that can be searched on the project's bug database for more
// info on the known issue.
#define oTEST_REGISTER_BUGGED(_TestClassName, _BugNumber) oTestManager::RegisterTest<_TestClassName> _TestClassName##_Instance(_BugNumber, oTest::BUGGED);

// Use this when it is appropriate to commit a test, but it's not quite ready 
// yet. Specify an ID in the project's bug database for more info on the 
// emerging test.
#define oTEST_REGISTER_NEW(_TestClassName, _BugNumber) oTestManager::RegisterTest<_TestClassName> _TestClassName##_Instance(_BugNumber, oTest::NOTREADY);

struct oTest : oNoncopyable
{
	enum RESULT
	{
		SUCCESS,
		FAILURE,
		NOTFOUND,
		FILTERED, // skipped by command line
		SKIPPED, // skipped by test itself
		BUGGED, // The test was registered with a bug so was not run
		NOTREADY, // The test is newer and not yet quite ready
		LEAKS,
		NUM_TEST_RESULTS,
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
	// Create a process (suspended) for running the unit test in a special mode.
	// This should bee called from a main-process unit test that wants to set up
	// a client-server or multi-process test. The process is created and then run
	// with a separate call to Run() so there is an opportunity for the developer
	// to place a breakpoint and attach to this new process during development.
	static bool CreateProcess(const char* _SpecialTestName, threadsafe interface oProcess** _ppProcess);

	// Run the specified process as was created from oSpecialTest::CreateProcess 
	// in a special mode that runs the unit test with a specific test. This way 
	// multi-process or client-server type tests can have a consistent way to set 
	// up the runtime environment with full oTest reporting. NOTE: The special 
	// test must call NotifyReady() when ready because this function will block 
	// until that is called. This way if something in the special test needs to be 
	// set up, such as a server set to listening mode, it can be done without 
	// timing errors. Create the process that will run the special test in a 
	// suspended mode. By calling this and StartAndWaitToBeReady separately, a 
	// developer can put breakpoints and attach to the process before it starts 
	// running easily.
	static bool Start(threadsafe interface oProcess* _pProcess, char* _StrStatus, size_t _SizeofStrStatus, int* _pExitCode, unsigned int _TimeoutMS = 10000);

	// Run blocks until it receives an event from the special test (this)
	void NotifyReady();
};

interface oTestManager : oNoncopyable
{
	struct RegisterTestBase : oNoncopyable
	{
		RegisterTestBase(unsigned int _BugNumber = 0, oTest::RESULT _BugResult = oTest::SUCCESS, const char* _PotentialZombieProcesses = "");
		~RegisterTestBase();
		virtual oTest* New() = 0;
		virtual const char* GetTypename() const = 0;
		virtual bool IsSpecialTest() const = 0;
		inline unsigned int GetBugNumber() const { return BugNumber; }
		inline oTest::RESULT GetBugResult() const { return BugResult; }
		inline const char* GetPotentialZombieProcesses() { return PotentialZombieProcesses; }
	protected:
		unsigned int BugNumber; // if non-zero, this test is disabled because of the specified bug
		oTest::RESULT BugResult; // if BugNumber is non-zero, this result is returned instead of running the test
		char PotentialZombieProcesses[256];
	};

	template<typename TestT> struct RegisterTest : RegisterTestBase
	{
		RegisterTest(unsigned int _BugNumber = 0, oTest::RESULT _BugResult = oTest::SUCCESS, const char* _PotentialZombieProcesses = "") : RegisterTestBase(_BugNumber, _BugResult, _PotentialZombieProcesses) {}
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
			, EnableSpecialTestTimeouts(true)
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
		bool EnableSpecialTestTimeouts;
		// @oooii-tony: todo: Add redirect status, redirect printf
	};

	static oTestManager* Singleton();

	virtual void GetDesc(DESC* _pDesc) = 0;
	virtual void SetDesc(DESC* _pDesc) = 0;

	// RunTests can fail due to a bad compilation of filters. If this returns -1, check oGetLastError() for more
	// details.
	virtual oTest::RESULT RunTests(oFilterChain::FILTER* _pTestFilters, size_t _SizeofTestFilters) = 0;
	template<size_t size> inline oTest::RESULT RunTests(oFilterChain::FILTER (&_pTestFilters)[size]) { return RunTests(_pDesc, _pTestFilters, size); }

	// Special mode re-runs the test exe for tests that need a client-server 
	// multi-process setup.
	virtual oTest::RESULT RunSpecialMode(const char* _Name) = 0;

	virtual bool FindFullPath(char* _StrFullPath, size_t _SizeofStrFullPath, const char* _StrRelativePath) const = 0;
	template<size_t size> inline bool FindFullPath(char (&_StrFullPath)[size], const char* _StrRelativePath) { return FindFullPath(_StrFullPath, size, _StrRelativePath); }
};

#endif
