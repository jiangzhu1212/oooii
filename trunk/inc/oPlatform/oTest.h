// $(header)
// A system for easily defining unit tests that handles registration, running
// and reporting details.
#pragma once
#ifndef oTest_h
#define oTest_h

#include <oBasis/oFilterChain.h>
#include <oBasis/oString.h>
#include <oBasis/oInterface.h>
#include <oBasis/oNoncopyable.h>
#include <oPlatform/oSingleton.h> // @oooii-tony: Is it necessary to guarantee a test to be singular? If not this can take a step towards being cross-platform.

#define oTESTERROR(format, ...) do { sprintf_s(_StrStatus, _SizeofStrStatus, format, ## __VA_ARGS__); oTRACE("FAILING: %s (oErrorGetLast() == %s (%s))", _StrStatus, oAsString(oErrorGetLast()), oErrorGetLastString()); return oTest::FAILURE; } while(false)
#define oTESTB(expr, errMsg, ...) do { if (!(expr)) { oTESTERROR(errMsg, ## __VA_ARGS__); } } while(false)
#define oTESTI(oImagePointer) oTESTB(TestImage(oImagePointer), "Image compare failed: %s: %s", oAsString(oErrorGetLast()), oErrorGetLastString());
#define oTESTI2(oImagePointer, NthFrame) oTESTB(TestImage(oImagePointer, NthFrame), "Image compare (%u%s frame) failed: %s: %s", NthFrame, oOrdinal(NthFrame), oAsString(oErrorGetLast()), oErrorGetLastString());

// _____________________________________________________________________________
// User registration macros

// Use this to register tests that are expected to work
#define oTEST_REGISTER(_TestClassName) oTestManager::RegisterTest<_TestClassName> oCONCAT(_TestClassName, _Instance)

// Use this to register test that spawn processes other than special-mode tests
#define oTEST_REGISTER_MULTIPROCESS(_TestClassName, _StrSemiColorDelimitedStringOfSpawnedProcesses) oTestManager::RegisterTest<_TestClassName> oCONCAT(_TestClassName, _Instance)(0, oTest::SUCCESS, _StrSemiColorDelimitedStringOfSpawnedProcesses)

// Use this to convert oTEST_REGISTER() to a skipped bug based on a known issue.
// Specify an ID that can be searched on the project's bug database for more
// info on the known issue.
#define oTEST_REGISTER_BUGGED(_TestClassName, _BugNumber) oTestManager::RegisterTest<_TestClassName> oCONCAT(_TestClassName, _Instance)(_BugNumber, oTest::BUGGED)

// Use this when it is appropriate to commit a test, but it's not quite ready 
// yet. Specify an ID in the project's bug database for more info on the 
// emerging test.
#define oTEST_REGISTER_NEW(_TestClassName, _BugNumber) oTestManager::RegisterTest<_TestClassName> oCONCAT(_TestClassName, _Instance)(_BugNumber, oTest::NOTREADY)

interface oImage;

struct oTest : oModuleSingleton<oTest>
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
	bool TestImageAgainstGoldenImage(oImage* _pImage, const oImage * goldenImage);

	virtual RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) = 0;
	static void BuildPath(char* _StrDestination, size_t _SizeofStrDestination, const char* _TestName, const char* _DataPath, const char* _DataSubpath, const char* _Path, unsigned int _NthImage, const char* _Ext);

	template<size_t size> inline void BuildPath(char (&_StrDestination)[size], const char* _TestName, const char* _DataPath, const char* _DataSubpath, const char* _Path, unsigned int _NthImage, const char* _Ext) { BuildPath(_StrDestination, size, _TestName, _DataPath, _DataSubpath, _Path, _NthImage, _Ext); }

	struct DESC //These values will default to test manager desc defaults
	{
    int colorChannelTolerance;
    float maxRMSError; //@oooii-doug rms error is a better gauge of image error than this

    unsigned int DiffImageMultiplier;

	};
	virtual void OverrideTestDesc(DESC &_desc) {}
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
			, maxRMSError(1.0f)
      , colorChannelTolerance(0)
			, DefaultDiffImageMultiplier(8)
			, TestTooSlowTimeInSeconds(10.0f)
			, TestReallyTooSlowTimeInSeconds(20.0f)
			, EnableSpecialTestTimeouts(true)
			, CaptureCallstackForTestLeaks(false)
			, EnableLeakTracking(true)
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
    unsigned int colorChannelTolerance;
		float maxRMSError;
		unsigned int DefaultDiffImageMultiplier;
		float TestTooSlowTimeInSeconds;
		float TestReallyTooSlowTimeInSeconds;
		bool EnableSpecialTestTimeouts;
		bool CaptureCallstackForTestLeaks; // slow! but useful, filter tests carefully
		bool EnableLeakTracking;
		// @oooii-tony: todo: Add redirect status, redirect printf
	};

	static oTestManager* Singleton();

	virtual void GetDesc(DESC* _pDesc) = 0;
	virtual void SetDesc(DESC* _pDesc) = 0;

	// RunTests can fail due to a bad compilation of filters. If this returns -1, check oErrorGetLast() for more
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
