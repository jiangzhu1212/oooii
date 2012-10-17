// $(noheader)
#include "RatcliffJobSwarmTest.h"
#include <oBasis/oDispatchQueue.h>
#include <oBasis/oFixedString.h>
#include <oBasis/oInvalid.h>
#include <oBasis/oTask.h>
#include <oBasis/oTimer.h>

// John Ratcliff's JobSwarm benchmark source, with a bit of wrapping
// @oooii-tony: This way I can compare various implementations, including
// JobSwarm, in an apples to apples way. Maybe one day I should have my own 
// test, but that would reduce comparability with Ratcliff's effort. Until then,
// unapologetically use his stuff.

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
		unsigned int index = _X1 == oInvalid ? static_cast<unsigned int>(_Index) : ((y+_Y1)*FRACTAL_SIZE+_X1);
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
	void Schedule(threadsafe oDispatchQueue* _pDispatchQueue, unsigned int x1, unsigned int y1, double fx, double fy, double xscale, double yscale, unsigned char* dest)
	{
    mFX = fx;
    mFY = fy;
    mXscale = xscale;
    mYscale = yscale;
    mData = dest;
    mX1 = x1;
    mY1 = y1;
		_pDispatchQueue->Dispatch(&MandelbrotJob::Run, this);
	}

	void Run()
	{
		MandelbrotTask(0, mData, mX1, mY1, mFX, mFY, mXscale, mYscale);
	}

private:
  double mFX;
  double mFY;
  double mXscale;
  double mYscale;
	void* mData;
  unsigned int mX1;
  unsigned int mY1;
};

#pragma fenv_access (on)

#ifdef _DEBUG
	#define DEBUG_DISCLAIMER "(DEBUG: Non-authoritive) "
#else
	#define DEBUG_DISCLAIMER
#endif

bool RunDispatchQueueTest(const char* _Name, threadsafe oDispatchQueue* _pDispatchQueue)
{
	unsigned int taskRow = FRACTAL_SIZE/SWARM_SIZE;
	unsigned int taskCount = taskRow*taskRow;
	oStringS timeStr, scheduleLoopStr;

	RatcliffJobSwarm::MandelbrotJob* jobs = new RatcliffJobSwarm::MandelbrotJob[taskCount];

	double x1 = -0.56017680903960034334758968;
	double x2 = -0.5540396934395273995800156;
	double y1 = -0.63815211573948702427222672;
	double y2 = y1+(x2-x1);
	double xscale = (x2-x1)/(double)FRACTAL_SIZE;
	double yscale = (y2-y1)/(double)FRACTAL_SIZE;

	unsigned char* fractal = new unsigned char[FRACTAL_SIZE*FRACTAL_SIZE];
	RatcliffJobSwarm::MandelbrotJob* next_job = jobs;

	double start = oTimer();
	for (unsigned int y=0; y<FRACTAL_SIZE; y+=SWARM_SIZE)
		for (unsigned int x=0; x<FRACTAL_SIZE; x+=SWARM_SIZE, next_job++)
			next_job->Schedule(_pDispatchQueue, x, y, x1, y1, xscale, yscale, fractal);
	
	double endScheduleLoop = oTimer();

	_pDispatchQueue->Join();
	oFormatTimeSize(timeStr, oTimer() - start, true);
	oFormatTimeSize(scheduleLoopStr, endScheduleLoop - start, true);
	delete [] fractal;
	delete [] jobs;

	oErrorSetLast(oERROR_NONE, DEBUG_DISCLAIMER "Ratcliff/Mandelbrot: %s (%s sched)", timeStr.c_str(), scheduleLoopStr.c_str());
	return true;
}

bool RunParallelForTest(const char* _Name)
{
	unsigned int taskRow = FRACTAL_SIZE/SWARM_SIZE;
	unsigned int taskCount = taskRow*taskRow;
	oStringS timeStr;
	double start = oTimer();

	double x1 = -0.56017680903960034334758968;
	double x2 = -0.5540396934395273995800156;
	double y1 = -0.63815211573948702427222672;
	double y2 = y1+(x2-x1);
	double xscale = (x2-x1)/(double)FRACTAL_SIZE;
	double yscale = (y2-y1)/(double)FRACTAL_SIZE;

	unsigned char* fractal = new unsigned char[FRACTAL_SIZE*FRACTAL_SIZE];

	oTaskParallelFor(0, taskCount, oBIND(&RatcliffJobSwarm::MandelbrotTask, oBIND1, fractal, oInvalid, 0, x1, y1, xscale, yscale));
	oFormatTimeSize(timeStr, oTimer() - start, true);

	delete [] fractal;
	oErrorSetLast(oERROR_NONE, DEBUG_DISCLAIMER "Ratcliff/Mandelbrot: %s", timeStr.c_str());
	return true;
}

} // namespace RatcliffJobSwarm
