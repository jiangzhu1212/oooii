/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2011 Antony Arciuolo & Kevin Myers                       *
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

