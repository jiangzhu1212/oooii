/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2013 OOOii.                                              *
 * antony.arciuolo@oooii.com                                              *
 * kevin.myers@oooii.com                                                  *
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
#include <oPlatform/oWindowAppController.h>
#include <oPlatform/Windows/oWinKey.h>
#include <oPlatform/Windows/oWinWindowing.h>
#include <oPlatform/Windows/oGDI.h>
#include <oPlatform/oProcess.h>

const oGUID& oGetGUID( threadsafe const oWindowAppController* threadsafe const * )
{
	// {852BF2A0-E58C-42E4-BDBC-6AC5614DE6B4}
	static const oGUID guid = 
	{ 0x852bf2a0, 0xe58c, 0x42e4, { 0xbd, 0xbc, 0x6a, 0xc5, 0x61, 0x4d, 0xe6, 0xb4 } };
	return guid;
}

class oWindowAppControllerImpl : public oWindowAppController
{
public:
	oDEFINE_REFCOUNT_INTERFACE(Refcount);
	oDEFINE_TRIVIAL_QUERYINTERFACE(oWindowAppController);
	oWindowAppControllerImpl(const char* _pProcessName, const char* _pWindowName, bool* _pSuccess);

	virtual bool SendX11Keys(oGUI_KEY* _pKeys, int _NumberKeys) override;
	virtual bool SendASCII(const char* _pASCII) override;
	virtual bool CreateSnapshot( oImage** _ppSnapshot ) override;
	virtual bool SendMouseButtonAtPosition(const float2 &_Position, oGUI_KEY _MouseButton, bool _MouseUp) override;
	virtual bool SendKeyDown(oGUI_KEY _Key, bool _KeyUp) override;
private:
	oRefCount Refcount;

	unsigned int ProcessID;
	unsigned int ProcessThreadID;
	HWND hWnd;
};

bool oWindowAppControllerCreate(const char* _pProcessName, const char* _pWindowName, oWindowAppController** _ppWindowAppController)
{
	bool success = false;
	oCONSTRUCT(_ppWindowAppController, oWindowAppControllerImpl(_pProcessName, _pWindowName, &success));
	return success;
}

oWindowAppControllerImpl::oWindowAppControllerImpl(const char* _pProcessName, const char* _pWindowName, bool* _pSuccess)
{
	ProcessID = oProcessGetID(_pProcessName);
	if(0 == ProcessID)
	{
		oErrorSetLast(std::errc::no_such_process);
		return; // Pass error
	}

	if(!oWinGetProcessTopWindowAndThread(ProcessID, &hWnd, &ProcessThreadID, _pWindowName))
		return; // Pass error

	*_pSuccess = true;
}

bool oWindowAppControllerImpl::SendMouseButtonAtPosition(const float2 &_Position, oGUI_KEY _MouseButton, bool _MouseUp)
{
	oWinKeySend(hWnd, _MouseButton, !_MouseUp, _Position);
	return true;
}

bool oWindowAppControllerImpl::SendKeyDown(oGUI_KEY _Key, bool _KeyUp)
{
	oWinKeySend(hWnd, _Key, !_KeyUp);
	return true;
}

bool oWindowAppControllerImpl::SendX11Keys( oGUI_KEY* _pKeys, int _NumberKeys )
{
	static const int SANE_NUMBER_OF_KEYS = 2048;
	if(_NumberKeys > SANE_NUMBER_OF_KEYS)
		return oErrorSetLast(std::errc::no_buffer_space, "More than %d keys is not supported.", SANE_NUMBER_OF_KEYS);

	short int* VirtualKeys = (short int*)oStackAlloc(sizeof(short int) * _NumberKeys);

	for(int i = 0; i < _NumberKeys; ++i)
	{
		VirtualKeys[i] = (short int)oWinKeyFromKey(_pKeys[i]);
	}
	return oWinSendKeys(hWnd, ProcessThreadID, VirtualKeys, _NumberKeys);
}

bool oWindowAppControllerImpl::SendASCII( const char* _pASCII )
{
	return oWinSendASCIIMessage(hWnd, ProcessThreadID, _pASCII);
}

bool oWindowAppControllerImpl::CreateSnapshot( oImage** _ppSnapshot )
{
	void* pImageData = nullptr;
	size_t szImageData = 0;
	oWinSetFocus(hWnd);
	if( !oGDIScreenCaptureWindow(hWnd, false, [&](size_t _Size)->void*{ return new char[_Size]; }, &pImageData, &szImageData, false) )
	{
		delete[] pImageData;
		return false;
	}

	bool Created = oImageCreate("WinowAppController Screenshot", pImageData, szImageData, _ppSnapshot);
	delete pImageData;
	return Created;

}


