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
// A system for easily defining unit tests that handles registration, running
// and reporting details.
#pragma once
#ifndef oTest_h
#define oTest_h

#include <oBasis/oFilterChain.h>
#include <oBasis/oStddef.h>
#include <oBasis/oString.h>
#include <oPlatform/oProcess.h>
#include <oPlatform/oSingleton.h> // @oooii-tony: Is it necessary to guarantee a test to be singular? If not this can take a step towards being cross-platform.

#define oTESTERROR(format, ...) do { oPrintf(_StrStatus, _SizeofStrStatus, format, ## __VA_ARGS__); oTRACE("FAILING: %s (oErrorGetLast() == %s (%s))", _StrStatus, oErrorAsString(oErrorGetLast()), oErrorGetLastString()); return oTest::FAILURE; } while(false)
#define oTESTB(expr, errMsg, ...) do { if (!(expr)) { oTESTERROR(errMsg, ## __VA_ARGS__); } } while(false)
#define oTESTB0(expr) do { if (!(expr)) { oPrintf(_StrStatus, _SizeofStrStatus, "%s: %s", oErrorAsString(oErrorGetLast()), oErrorGetLastString()); return oTest::FAILURE; } } while(false)
#define oTESTB_FWD( _FWDFn) \
	{ \
		RESULT Result = _FWDFn; \
		if( SUCCESS != Result ) \
			return Result; \
	}
#define oTESTI(oImagePointer) oTESTB0(TestImage(oImagePointer));

#define oTESTI2(oImagePointer, NthFrame) oTESTB0(TestImage(oImagePointer, NthFrame));
#define oTESTI_CUSTOM_TOLERANCE(oImagePointer, NthFrame, ColorChannelTolerance, MaxRMSError, DiffImageMultiplier) oTESTB0(TestImage(oImagePointer, NthFrame, ColorChannelTolerance, MaxRMSError, DiffImageMultiplier))

#define oTEST_FUTURE(_Future) do \
{	try { _Future.wait(); } \
	catch (std::exception& e) \
	{	oErrorSetLast(e); \
		oPrintf(_StrStatus, _SizeofStrStatus, "%s: %s", oErrorAsString(oErrorGetLast()), oErrorGetLastString()); \
		return oTest::FAILURE; \
	} \
} while (false)

#define oTESTFI(_FutureImage) do \
{	oTEST_FUTURE(_FutureImage); \
	oTESTI(_FutureImage.get()); \
} while (false)

#define oTESTFI2(_FutureImage, _NthFrame) do \
{	oTEST_FUTURE(_FutureImage); \
	oTESTI2(_FutureImage.get(), _NthFrame); \
} while (false)

#define oTESTFI_CUSTOM_TOLERANCE(_FutureImage, _NthFrame, _ColorChannelTolerance, _MaxRMSError, _DiffImageMultiplier) do \
{	oTEST_FUTURE(_FutureImage); \
	oTESTI_CUSTOM_TOLERANCE(_FutureImage.get(), _NthFrame, _ColorChannelTolerance, _MaxRMSError, _DiffImageMultiplier); \
} while (false)

// _____________________________________________________________________________
// User registration macros

// Use this to register tests that are expected to work
#define oTEST_REGISTER(_TestClassName) oTestManager::RegisterTest<_TestClassName> oCONCAT(_TestClassName, _Instance)

// Use this to register test that spawn processes other than special-mode tests
// For any given process name (executable name) oMODULE_DEBUG_SUFFIX_A will be
// appended as a second entry, so both debug and release versions are handled
// with a single specification.
#define oTEST_REGISTER_MULTIPROCESS(_TestClassName, _StrSemiColorDelimitedStringOfSpawnedProcesses) oTestManager::RegisterTest<_TestClassName> oCONCAT(_TestClassName, _Instance)(0, oTest::SUCCESS, _StrSemiColorDelimitedStringOfSpawnedProcesses)

// Use this to convert oTEST_REGISTER() to a skipped bug based on a known issue.
// Specify an ID that can be searched on the project's bug database for more
// info on the known issue.
#define oTEST_REGISTER_BUGGED(_TestClassName, _BugNumber) oTestManager::RegisterTest<_TestClassName> oCONCAT(_TestClassName, _Instance)(_BugNumber, oTest::BUGGED)

#ifdef o64BIT
	#define oTEST_REGISTER_BUGGED32(_TestClassName, _BugNumber) oTEST_REGISTER(_TestClassName)
#else
	#define oTEST_REGISTER_BUGGED32(_TestClassName, _BugNumber) oTEST_REGISTER_BUGGED(_TestClassName, _BugNumber)
#endif

// Use this when it is appropriate to commit a test, but it's not quite ready 
// yet. Specify an ID in the project's bug database for more info on the 
// emerging test.
#define oTEST_REGISTER_NEW(_TestClassName, _BugNumber) oTestManager::RegisterTest<_TestClassName> oCONCAT(_TestClassName, _Instance)(_BugNumber, oTest::NOTREADY)

#define oTEST_REGISTER_PERFTEST(_TestClassName) oTestManager::RegisterTest<_TestClassName> oCONCAT(_TestClassName, _Instance)(1, oTest::PERFTEST)

interface oImage;

struct oTest : oProcessSingleton<oTest>
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
		PERFTEST, // The test is not run normally because it can take a long time and is meant to test performance, not correctness.
		NUM_TEST_RESULTS,
	};

	enum PATH_TYPE
	{
		EXECUTABLES,
		DATA,
		GOLDEN_BINARIES,
		GOLDEN_IMAGES,
		TEMP,
		INPUT,
		OUTPUT,
	};

	struct FileMustExistFlag {};
	static const FileMustExistFlag FileMustExist;

	struct TEST_IMAGE_DESC
	{
		// These values are 

		int ColorChannelTolerance;
		float MaxRMSError;
		unsigned int DiffImageMultiplier;
	};

	oTest();
	virtual ~oTest();
	virtual const char* GetName() const;

	// Does a memcmp against a golden binary files that is named consistently with
	// the test (like golden images) but uses the specified file extension so that
	// the golden binary can still be opened from the operating system.
	bool TestBinary(const void* _pBuffer, size_t _SizeofBuffer, const char* _FileExtension, unsigned int _NthImage = 0);

	// Visual tests should prepare an oImage and then use this API to submit the
	// test image to be compared against a "golden" image, one that has been 
	// verified as being correct. If valid, then this returns true. If the images
	// differ, then the test image that failed will be written to the OutputPath.
	// It may make sense in rare cases to explicitly override default test system 
	// policies for image comparisons. If so, _ColorChannelTolerance, 
	// _MaxRMSError, and _DiffImageMultiplier can be overridden, else the default
	// values specified here will be replaced with values from oTestManager::DESC.
	bool TestImage(oImage* _pImage, unsigned int _NthImage = 0, int _ColorChannelTolerance = oDEFAULT, float _MaxRMSError = -1.0f, unsigned int _DiffImageMultiplier = oDEFAULT);

	virtual RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) = 0;

	// Always specify resources for read/write as relative paths from the Data 
	// root directories specified in the DESC and use this method in tests to 
	// resolve those relative paths to absolute paths. This returns false if the 
	// path could not be built. If _PathMustExist, failure includes if the file 
	// could not be found.
	inline bool BuildPath(char* _StrFullPath, size_t _SizeofStrFullPath, const char* _StrRelativePath, PATH_TYPE _PathType) const { return BuildPath(_StrFullPath, _SizeofStrFullPath, _StrRelativePath, _PathType, false); }
	inline bool BuildPath(char* _StrFullPath, size_t _SizeofStrFullPath, const char* _StrRelativePath, PATH_TYPE _PathType, const FileMustExistFlag&) const { return BuildPath(_StrFullPath, _SizeofStrFullPath, _StrRelativePath, _PathType, true); }

	template<size_t size> bool BuildPath(char (&_StrFullPath)[size], const char* _StrRelativePath, PATH_TYPE _PathType) { return BuildPath(_StrFullPath, size, _StrRelativePath, _PathType, false); }
	template<size_t size> bool BuildPath(char (&_StrFullPath)[size], const char* _StrRelativePath, PATH_TYPE _PathType, const FileMustExistFlag&) { return BuildPath(_StrFullPath, size, _StrRelativePath, _PathType, true); }

	template<size_t size> bool BuildPath(oStd::fixed_string<char, size>& _StrDestination, const char* _StrRelativePath, PATH_TYPE _PathType) { return BuildPath(_StrDestination, _StrDestination.capacity(), _StrRelativePath, _PathType, false); }
	template<size_t size> bool BuildPath(oStd::fixed_string<char, size>& _StrDestination, const char* _StrRelativePath, PATH_TYPE _PathType, const FileMustExistFlag&) { return BuildPath(_StrDestination, _StrDestination.capacity(), _StrRelativePath, _PathType, true); }

	bool FindInputFile(oStd::path_string& _FullPath, const char* _StrRelativePath) { return BuildPath(_FullPath, _FullPath.capacity(), _StrRelativePath, INPUT, FileMustExist); }

private:
	virtual bool BuildPath(char* _StrFullPath, size_t _SizeofStrFullPath, const char* _StrRelativePath, PATH_TYPE _PathType, bool _PathMustExist) const;
	virtual bool TestImage(oImage* _pTestImage, const char* _GoldenImagePath, const char* _FailedImagePath, unsigned int _NthImage, int _ColorChannelTolerance = oDEFAULT, float _MaxRMSError = -1.0f, unsigned int _DiffImageMultiplier = oDEFAULT, bool _OutputGoldenImage = false);
};

// Special tests are ones that are new processes spawned from a regular test 
// used to test more complex inter-process functionality, such as a client-
// server model. By deriving from this class, it means the user will handle when 
// this spawns and the test system will otherwise skip over this.
struct oSpecialTest : public oTest
{
	// Create a process (suspended) for running the unit test in a special mode.
	// This should bee called from a main-process unit test that wants to set up
	// a client-server or multi-process test. The process is created and then run
	// with a separate call to Run() so there is an opportunity for the developer
	// to place a breakpoint and attach to this new process during development.
	static bool CreateProcess(const char* _SpecialTestName, threadsafe oProcess** _ppProcess);

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

interface oTestManager
{
	struct RegisterTestBase
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
	private:
		RegisterTestBase(const RegisterTestBase&)/* = delete*/;
		const RegisterTestBase& operator=(const RegisterTestBase&)/* = delete*/;
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
			, ExecutablesPath(nullptr)
			, GoldenBinariesPath(nullptr)
			, GoldenImagesPath(nullptr)
			, TempPath(nullptr)
			, InputPath(nullptr)
			, OutputPath(nullptr)
			, NameColumnWidth(20)
			, TimeColumnWidth(8)
			, StatusColumnWidth(10)
			, RandomSeed(0)
			, NumRunIterations(1)
			, MaxRMSError(1.0f)
      , ColorChannelTolerance(0)
			, DiffImageMultiplier(8)
			, TestTooSlowTimeInSeconds(10.0f)
			, TestReallyTooSlowTimeInSeconds(20.0f)
			, EnableSpecialTestTimeouts(true)
			, CaptureCallstackForTestLeaks(false)
			, EnableLeakTracking(true)
			, Exhaustive(false)
			, AutomatedMode(false)
			, EnableOutputGoldenImages(false)
		{}

		const char* TestSuiteName;
		const char* DataPath;
		const char* ExecutablesPath;
		const char* GoldenBinariesPath;
		const char* GoldenImagesPath;
		const char* TempPath;
		const char* InputPath;
		const char* OutputPath;
		unsigned int NameColumnWidth;
		unsigned int TimeColumnWidth;
		unsigned int StatusColumnWidth;
		unsigned int RandomSeed;
		unsigned int NumRunIterations;
    unsigned int ColorChannelTolerance;
		float MaxRMSError;
		unsigned int DiffImageMultiplier;
		float TestTooSlowTimeInSeconds;
		float TestReallyTooSlowTimeInSeconds;
		bool EnableSpecialTestTimeouts;
		bool CaptureCallstackForTestLeaks; // slow! but useful, filter tests carefully
		bool EnableLeakTracking;
		bool Exhaustive; // allow time consuming tests to run. Tests should have a fast mode, and check this flag to see if its ok to run in slow mode.
		bool AutomatedMode; // Don't prompt the user for action, do something sensible for running this on an automated test server
		bool EnableOutputGoldenImages;
		// @oooii-tony: todo: Add redirect status, redirect printf
	};

	static oTestManager* Singleton();

	virtual void GetDesc(DESC* _pDesc) = 0;
	virtual void SetDesc(DESC* _pDesc) = 0;

	// RunTests can fail due to a bad compilation of filters. If this returns -1, check oErrorGetLast() for more
	// details.
	virtual oTest::RESULT RunTests(oFilterChain::FILTER* _pTestFilters, size_t _SizeofTestFilters) = 0;
	template<size_t size> oTest::RESULT RunTests(oFilterChain::FILTER (&_pTestFilters)[size]) { return RunTests(_pDesc, _pTestFilters, size); }

	// Special mode re-runs the test exe for tests that need a client-server 
	// multi-process setup.
	virtual oTest::RESULT RunSpecialMode(const char* _Name) = 0;
};

#endif
