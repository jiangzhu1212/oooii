// $(header)
#include <oooii/oAsyncFileIO.h>
#include <oooii/oCommandQueue.h>
#include <oooii/oFile.h>
#include <oooii/oSingleton.h>

class oAsyncFileIOCtx : public oProcessSingleton<oAsyncFileIOCtx>
{
public:
	void ScheduleIO( oAsyncFileIO::COMMAND* _pCommand )
	{
		if( _pCommand->CommandType == oAsyncFileIO::READ )
			CommandQueue.Enqueue(oBIND(&oAsyncFileIOCtx::ExecuteRead, this, _pCommand));
		else
			CommandQueue.Enqueue(oBIND(&oAsyncFileIOCtx::ExecuteWrite, this, _pCommand));
	}
private:

	void ExecuteRead(oAsyncFileIO::COMMAND* _pCommand)
	{
		_pCommand->Success = oFile::LoadHeader( _pCommand->pDataOut, _pCommand->SizeOfDataBlob, _pCommand->PathToFile, _pCommand->DataIsText );
		_pCommand->Continuation(_pCommand);
	}
	void ExecuteWrite(oAsyncFileIO::COMMAND* _pCommand)
	{
		_pCommand->Success = oFile::SaveBuffer(_pCommand->PathToFile, _pCommand->pDataOut, _pCommand->SizeOfDataBlob, _pCommand->DataIsText, oAsyncFileIO::WRITE_APPEND == _pCommand->CommandType ? true : false );
		_pCommand->Continuation(_pCommand);
	}
	oCommandQueue CommandQueue;
};

struct oAsyncFileIOCtxInstantiator
{
	oAsyncFileIOCtxInstantiator()
	{
		oAsyncFileIOCtx::Singleton();
	}
};

oAsyncFileIOCtxInstantiator oAsyncFileIOCtxInstantiation;

void oAsyncFileIO::ScheduleIO( COMMAND* _pCommand )
{
	oAsyncFileIOCtx::Singleton()->ScheduleIO(_pCommand);
}

