// $(header)
// Interface for working with monitors/displays
#pragma once
#ifndef oDisplay_h
#define oDisplay_h

#include <oBasis/oMathTypes.h>
#include <oPlatform/oStddef.h>

struct oDISPLAY_MODE
{
	oDISPLAY_MODE()
		: Size(oDEFAULT, oDEFAULT)
		, Bitdepth(oDEFAULT)
		, RefreshRate(oDEFAULT)
	{}
	int2 Size;
		
	// usually 16- or 32-bit
	// oDEFAULT implies current settings
	int Bitdepth;

	// Usually 60, 75, 85, 120, 240
	// oDEFAULT implies current settings
	int RefreshRate;
};

struct oDISPLAY_DESC
{
	oDISPLAY_DESC()
		: NativeHandle(nullptr)
		, Position(oDEFAULT, oDEFAULT)
		, WorkareaPosition(oDEFAULT, oDEFAULT)
		, WorkareaSize(oDEFAULT, oDEFAULT)
		, Index(oInvalid)
		, IsPrimary(false)
		, IsPowerOn(false)
	{}
	
	oDISPLAY_MODE Mode;
	void* NativeHandle; // HMONITOR on Windows
	int2 Position;
	int2 WorkareaPosition;
	int2 WorkareaSize;
	unsigned int Index;
	bool IsPrimary;
	bool IsPowerOn;
};

// If Index doesn't exist, this function will return false with a 
// oErrorGetLast() of ENOENT. If successful, _pDesc is filled with a description 
// of the specified display.
bool oDisplayEnum(unsigned int _Index, oDISPLAY_DESC* _pDesc);
bool oDisplaySetMode(unsigned int _Index, const oDISPLAY_MODE& _Mode);

// Restores state after a call to oDisplaySetMode()
bool oDisplayResetMode(unsigned int _Index);

class oScopedDisplayMode
{	unsigned int Index;
public:
	oScopedDisplayMode(unsigned int _Index, const oDISPLAY_MODE& _Mode) : Index(_Index) { oDisplaySetMode(_Index, _Mode); }
	~oScopedDisplayMode() { oDisplayResetMode(Index); }
};

// Turns all monitors on or sets them to a low-power state
bool oDisplaySetPowerOn(bool _On = true);

// returns oInvalid if none found
unsigned int oDisplayGetPrimaryIndex();
unsigned int oDisplayGetNum();
void oDisplayGetVirtualRect(int2* _pPosition, int2* _pSize);

#endif
