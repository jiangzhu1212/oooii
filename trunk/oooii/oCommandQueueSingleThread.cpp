#include <oooii/oCommandQueueSingleThread.h>
#include <oooii/oRefCount.h>

class oCommandQueueProc : public oThread::Proc
{
public:
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_TRIVIAL_QUERYINTERFACE(oGetGUID<oCommandQueueProc>());

	void RunIteration() override
	{
		if(!Finished || !EmptyFunction())
			TickFunction(); 
		if(!Finished)
			CommandEvent.Wait();
	}

	bool OnBegin() override { return true; }
	void OnEnd() override {}

	void ProcessCommand() {CommandEvent.Set();}
	//Note that the thread will spin, waisting time once Finish is called. The thread should be shut down as soon as all commands are flushed after calling this.
	void Finish() { oSWAP(&Finished, true); CommandEvent.Set(); }

	oCommandQueueProc(oFUNCTION<void ()> _tickFunction, oFUNCTION<bool ()> _emptyFunction);
	~oCommandQueueProc();

private:
	oRefCount RefCount;
	oEvent CommandEvent;
	int Finished;
	oFUNCTION<void ()> TickFunction;
	oFUNCTION<bool ()> EmptyFunction;
};

const oGUID& oGetGUID( threadsafe const oCommandQueueProc* threadsafe const * )
{
	// {2a44e059-8aa4-42d2-a74c-4c5d37b652fa}
	static const oGUID oIIDoCommandQueueProc = { 0x2a44e059, 0x8aa4, 0x42d2, { 0xa7, 0x4c, 0x4c, 0x5d, 0x37, 0xb6, 0x52, 0xfa } };
	return oIIDoCommandQueueProc; 
}

oCommandQueueProc::oCommandQueueProc(oFUNCTION<void ()> _tickFunction, oFUNCTION<bool ()> _emptyFunction)
	: TickFunction(_tickFunction), EmptyFunction(_emptyFunction), CommandEvent(true), Finished(false)
{
};

oCommandQueueProc::~oCommandQueueProc()
{
};

oCommandQueueSingleThread::oCommandQueueSingleThread()
{
	ThreadProc /= new oCommandQueueProc(
		oBIND( &oCommandQueueSingleThread::ExecuteNext, this ),
		oBIND( &oCommandQueueSingleThread::Empty, this ) );

	if (!oThread::Create("oCommandQueueSingleThread Thread", oKB(64), false, ThreadProc.c_ptr(), &Thread))
	{
		oSetLastError(EINVAL, "Failed to create a thread for a single threaded command queue.");
	}
};

oCommandQueueSingleThread::~oCommandQueueSingleThread()
{
	Disable();
	ThreadProc->Finish();
	Flush();
	Thread = nullptr;
	ThreadProc = nullptr;
};

void oCommandQueueSingleThread::IssueCommand() threadsafe
{
	ThreadProc->ProcessCommand();
};
