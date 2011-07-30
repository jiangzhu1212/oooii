// $(header)
#include <oooii/oSingleton.h>
#include <oooii/oSerialization.h>

using namespace oSerialization;

class DisableCRTMemoryInitImpl : public oProcessSingleton<DisableCRTMemoryInitImpl>
{
public:
	DisableCRTMemoryInitImpl() : RecursiveCount(0) {}

	void RequestDisable()
	{
		int newValue = oINC(&RecursiveCount);
		if(newValue == 1)
			OldDebugFill = _CrtSetDebugFillThreshold(0);
	}
	void RequestEnable()
	{
		int newValue = oDEC(&RecursiveCount);
		if(newValue == 0)
			_CrtSetDebugFillThreshold(OldDebugFill);
	}
private:
	size_t OldDebugFill;
	int RecursiveCount;
};

DisableCRTMemoryInit::DisableCRTMemoryInit()
{
	DisableCRTMemoryInitImpl::Singleton()->RequestDisable();
}

DisableCRTMemoryInit::~DisableCRTMemoryInit()
{
	DisableCRTMemoryInitImpl::Singleton()->RequestEnable();
}