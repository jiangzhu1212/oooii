// $(header)
#include "oTaskPool.h"
#include <oBasis/oBackoff.h>
#include <oBasis/oBlockAllocatorGrowable.h>
#include <oPlatform/oSingleton.h>

struct oTaskPoolContext : oProcessSingleton<oTaskPoolContext>
{
	static const oGUID GUID;
	oBlockAllocatorGrowableT<oTask> Pool;
};

// {20241B5E-39D6-4388-BF40-296923402B55}
const oGUID oTaskPoolContext::GUID = { 0x20241b5e, 0x39d6, 0x4388, { 0xbf, 0x40, 0x29, 0x69, 0x23, 0x40, 0x2b, 0x55 } };

void* oTask::operator new(size_t _Size) { return oTaskPoolContext::Singleton()->Pool.Allocate(); }
void oTask::operator delete(void* _Pointer) { oTaskPoolContext::Singleton()->Pool.Deallocate((oTask*)_Pointer); }
void* oTask::operator new[](size_t _Size) { return oTaskPoolContext::Singleton()->Pool.Allocate(); }
void oTask::operator delete[](void* _Pointer) { oTaskPoolContext::Singleton()->Pool.Deallocate((oTask*)_Pointer); }

oTask* oTaskPoolAllocate(oTASK _Task)
{
	oBackoff bo;
	oTask* t = nullptr;
	while (true)
	{
		t = new oTask(_Task);
		if (t) break;
		bo.Pause();
	}

	return t;
}

void oTaskPoolDeallocate(oTask* _pTask)
{
	delete _pTask;
}
