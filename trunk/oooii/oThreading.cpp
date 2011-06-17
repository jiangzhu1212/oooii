// $(header)
#include <oooii/oThreading.h>
#include <oooii/oAssert.h>
#include <oooii/oErrno.h>
#include <oooii/oEvent.h>
#include <oooii/oFile.h>
#include <oooii/oRef.h>
#include <oooii/oRefCount.h>
#include <oooii/oSingleton.h>
#include <oooii/oThread.h>
#include <oooii/oWindows.h>

unsigned int oGetCurrentThreadID()
{
	return ::GetCurrentThreadId();
}

unsigned int oGetMainThreadID()
{
	return oWinGetMainThreadID();
}

void* oGetCurrentThreadNativeHandle()
{
	return ::GetCurrentThread();
}

unsigned int oGetCurrentProcessID()
{
	return ::GetCurrentProcessId();
}

void* oGetProcessNativeHandle(const char* _Name)
{
	return oWinGetProcessHandle(_Name);
}

void oYield()
{
	#if defined(_WIN32) || defined(_WIN64)
		SwitchToThread();
	#else
		#error Unsupported platform
	#endif
}

void oSleep(unsigned int _Milliseconds)
{
	Sleep((DWORD)_Milliseconds);
}

void oSerialFor(oFUNCTION<void(size_t _Index)> _Function, size_t _Begin, size_t _End)
{
	for (size_t i = _Begin; i < _End; i++)
		_Function(i);
}

void oRunSerialTasks(oFUNCTION<void()>* _pFunctions, size_t _NumFunctions)
{
	for (size_t i = 0; i < _NumFunctions; i++)
		_pFunctions[i]();
}
