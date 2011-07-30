// $(header)
#include <oooii/oAsyncFileIO.h>
#include <oooii/oCommandQueue.h>
#include <oooii/oFile.h>
#include <oooii/oSingleton.h>

class oAsyncFileIOCtx : public oProcessSingleton<oAsyncFileIOCtx>
{
public:
	void ScheduleIO( const oAsyncFileIO::COMMAND& _Command )
	{
		if( _Command.CommandType == oAsyncFileIO::READ )
			CommandQueue.Enqueue(oBIND(&oAsyncFileIOCtx::ExecuteRead, this, _Command));
		else
			CommandQueue.Enqueue(oBIND(&oAsyncFileIOCtx::ExecuteWrite, this, _Command));
	}
private:

	void ExecuteRead(oAsyncFileIO::COMMAND _Command)
	{
		_Command.Success = oFile::LoadHeader( _Command.pDataOut, _Command.SizeOfDataBlob, _Command.PathToFile, _Command.DataIsText );
		if( _Command.OptionalContinuation)
			_Command.OptionalContinuation(_Command);
	}
	void ExecuteWrite(oAsyncFileIO::COMMAND _Command)
	{
		_Command.Success = oFile::SaveBuffer(_Command.PathToFile, _Command.pDataOut, _Command.SizeOfDataBlob, _Command.DataIsText, oAsyncFileIO::WRITE_APPEND == _Command.CommandType ? true : false );
		if( _Command.OptionalContinuation)
			_Command.OptionalContinuation(_Command);
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

void oAsyncFileIO::ScheduleIO( const COMMAND& _Command )
{
	oAsyncFileIOCtx::Singleton()->ScheduleIO(_Command);
}

