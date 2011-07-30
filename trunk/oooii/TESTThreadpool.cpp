// $(header)
#include <oooii/oCountdownLatch.h>
#include <oooii/oEvent.h>
#include <oooii/oRef.h>
#include <oooii/oStdio.h>
#include <oooii/oTest.h>
#include <oooii/oThreading.h>
#include <oooii/oThreadpool.h>

// John Ratcliff's JobSwarm benchmark source, with a bit of wrapping
// @oooii-tony: This way I can compare various implementations, including
// JobSwarm, in an apples to apples way.

namespace RatcliffJobSwarm {

#if 1
	// Settings used in John Ratcliffe's code
	#define FRACTAL_SIZE 2048
	#define SWARM_SIZE 8
	#define MAX_ITERATIONS 65536
#else
	// Super-soak test... thread_pool tests take about ~30 sec each
	#define FRACTAL_SIZE 16384
	#define SWARM_SIZE 64
	#define MAX_ITERATIONS 65536
#endif

//********************************************************************************
// solves a single point in the mandelbrot set.
//********************************************************************************
static inline unsigned int mandelbrotPoint(unsigned int iterations,double real,double imaginary)
{
	double fx,fy,xs,ys;
	unsigned int count;

  double two(2.0);

	fx = real;
	fy = imaginary;
  count = 0;

  do
  {
    xs = fx*fx;
    ys = fy*fy;
		fy = (two*fx*fy)+imaginary;
		fx = xs-ys+real;
    count++;
	} while ( xs+ys < 4.0 && count < iterations);

	return count;
}

static inline unsigned int solvePoint(unsigned int x,unsigned int y,double x1,double y1,double xscale,double yscale)
{
  return mandelbrotPoint(MAX_ITERATIONS,(double)x*xscale+x1,(double)y*yscale+y1);
}

void MandelbrotTask(size_t _Index, void* _pData, unsigned int _X1, unsigned int _Y1, double _FX, double _FY, double _XScale, double _YScale)
{
	unsigned char *fractal_image = (unsigned char *)_pData;
	for (unsigned int y=0; y<SWARM_SIZE; y++)
	{
		unsigned int index = _X1 == oINVALID ? static_cast<unsigned int>(_Index) : ((y+_Y1)*FRACTAL_SIZE+_X1);
		unsigned char *dest = &fractal_image[index];
		for (unsigned int x=0; x<SWARM_SIZE; x++)
		{
			unsigned int v = solvePoint(x+_X1,y+_Y1,_FX,_FY,_XScale,_YScale);
			if ( v == MAX_ITERATIONS )
				v = 0;
			else
				v = v&0xFF;
			*dest++ = (char)v;
		}
	}
}

class MandelbrotJob
{
public:
	void Schedule(threadsafe oThreadpool* _pThreadpool, threadsafe oCountdownLatch* _pLatch, unsigned int x1, unsigned int y1, double fx, double fy, double xscale, double yscale, unsigned char* dest)
	{
    mFX = fx;
    mFY = fy;
    mXscale = xscale;
    mYscale = yscale;
    mData = dest;
    mX1 = x1;
    mY1 = y1;
		pLatch = _pLatch;
		_pThreadpool->ScheduleTask(TaskProc, this);
	}

	static void TaskProc(void* data)
	{
		static_cast<MandelbrotJob*>(data)->Run();
	}

	void Run()
	{
		MandelbrotTask(0, mData, mX1, mY1, mFX, mFY, mXscale, mYscale);
		pLatch->Release();
	}

private:
  double mFX;
  double mFY;
  double mXscale;
  double mYscale;
	void* mData;
	threadsafe oCountdownLatch* pLatch;
  unsigned int mX1;
  unsigned int mY1;
};

#pragma fenv_access (on)

} // namespace RatcliffJobSwarm

bool RunThreadpoolTest(const char* _Name, threadsafe oThreadpool* _pThreadpool, char* _StrStatus, size_t _SizeofStrStatus)
{
	unsigned int taskRow = FRACTAL_SIZE/SWARM_SIZE;
	unsigned int taskCount = taskRow*taskRow;
	double start = oTimer();

	RatcliffJobSwarm::MandelbrotJob* jobs = new RatcliffJobSwarm::MandelbrotJob[taskCount];

	double x1 = -0.56017680903960034334758968;
	double x2 = -0.5540396934395273995800156;
	double y1 = -0.63815211573948702427222672;
	double y2 = y1+(x2-x1);
	double xscale = (x2-x1)/(double)FRACTAL_SIZE;
	double yscale = (y2-y1)/(double)FRACTAL_SIZE;

	unsigned char* fractal = new unsigned char[FRACTAL_SIZE*FRACTAL_SIZE];
	RatcliffJobSwarm::MandelbrotJob* next_job = jobs;

	oCountdownLatch latch("WorkDone", taskCount);

	for (unsigned int y=0; y<FRACTAL_SIZE; y+=SWARM_SIZE)
		for (unsigned int x=0; x<FRACTAL_SIZE; x+=SWARM_SIZE, next_job++)
			next_job->Schedule(_pThreadpool, &latch, x, y, x1, y1, xscale, yscale, fractal);

	latch.Wait();
	double end = oTimer();
	delete fractal;
	delete jobs;
	sprintf_s(_StrStatus, _SizeofStrStatus, "%s: %u ms", _Name, static_cast<unsigned int>((end - start)*1000.0));
	return true;
}

template<size_t size> bool RunThreadpoolTest(const char* _Name, threadsafe oThreadpool* _pThreadpool, char (&_StrStatus)[size]) { return RunThreadpoolTest(_Name, _pThreadpool, _StrStatus, size); }

bool RunParallelForTest(const char* _Name, char* _StrStatus, size_t _SizeofStrStatus)
{
	unsigned int taskRow = FRACTAL_SIZE/SWARM_SIZE;
	unsigned int taskCount = taskRow*taskRow;
	double start = oTimer();

	double x1 = -0.56017680903960034334758968;
	double x2 = -0.5540396934395273995800156;
	double y1 = -0.63815211573948702427222672;
	double y2 = y1+(x2-x1);
	double xscale = (x2-x1)/(double)FRACTAL_SIZE;
	double yscale = (y2-y1)/(double)FRACTAL_SIZE;

	unsigned char* fractal = new unsigned char[FRACTAL_SIZE*FRACTAL_SIZE];

	oParallelFor(oBIND(&RatcliffJobSwarm::MandelbrotTask, oBIND1, fractal, oINVALID, 0, x1, y1, xscale, yscale), 0, taskCount);

	double end = oTimer();
	delete [] fractal;
	sprintf_s(_StrStatus, _SizeofStrStatus, "%s: %u ms", _Name, static_cast<unsigned int>((end - start)*1000.0));
	return true;
}

struct TESTThreadpoolBase : public oTest
{
	static const int mArraySize = 128;
	int mTestArrayA[mArraySize];
	int mTestArrayB[mArraySize];
};

struct TESTThreadpoolWinTP : public oTest
{
	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override
	{
		oThreadpool::DESC desc;
		desc.Implementation = oThreadpool::WINDOWS_THREAD_POOL;
		desc.NumThreads = oINVALID;
		oRef<threadsafe oThreadpool> tp;
		oTESTB(oThreadpool::Create(&desc, &tp), "Failed to create windows thread pool");
		oTESTB(RunThreadpoolTest("WinTP", tp, _StrStatus, _SizeofStrStatus), "Windows thread pool failed");
		return SUCCESS;
	}
};

struct TESTThreadpoolOOOii : public oTest
{
	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override
	{
		oThreadpool::DESC desc;
		desc.Implementation = oThreadpool::OOOII;
		desc.NumThreads = oINVALID;
		oRef<threadsafe oThreadpool> tp;
		oTESTB(oThreadpool::Create(&desc, &tp), "Failed to create OOOii thread pool");
		oTESTB(RunThreadpoolTest("OOOiiTP", tp, _StrStatus, _SizeofStrStatus), "OOOii thread pool failed");
		return SUCCESS;
	}
	static const int mArraySize = 128;
	int mTestArrayA[mArraySize];
	int mTestArrayB[mArraySize];
};

struct TESTThreadpoolTBB : public oTest
{
	RESULT Run(char* _StrStatus, size_t _SizeofStrStatus) override
	{
		oTESTB(RunParallelForTest("TBB parallel_for", _StrStatus, _SizeofStrStatus), "oParallelFor failed");
		return SUCCESS;
	}
	static const int mArraySize = 128;
	int mTestArrayA[mArraySize];
	int mTestArrayB[mArraySize];
};

oTEST_REGISTER(TESTThreadpoolWinTP);
oTEST_REGISTER(TESTThreadpoolOOOii);
oTEST_REGISTER(TESTThreadpoolTBB);
