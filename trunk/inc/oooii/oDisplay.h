// $(header)
#pragma once
#ifndef oDisplay_h
#define oDisplay_h

struct oDisplay
{
	// Use this for any of the DESC values below to use current settings
	static const int DEFAULT = 0x80000000;

	struct MODE_DESC
	{
		MODE_DESC()
			: Width(DEFAULT)
			, Height(DEFAULT)
			, Bitdepth(DEFAULT)
			, RefreshRate(DEFAULT)
		{}

		int Width;
		int Height;
		
		// usually 16- or 32-bit
		// Use of DEFAULT means "use the current settings"
		int Bitdepth;

		// Usually 60, 75, 85, 120, 240
		// Use of DEFAULT means "use the current settings"
		int RefreshRate;
	};

	struct DESC
	{
		DESC()
			: NativeHandle(0)
			, Index(oINVALID)
			, X(DEFAULT)
			, Y(DEFAULT)
			, WorkareaX(DEFAULT)
			, WorkareaY(DEFAULT)
			, WorkareaWidth(DEFAULT)
			, WorkareaHeight(DEFAULT)
			, IsPrimary(false)
		{}

		void* NativeHandle; // HMONITOR on Windows

		MODE_DESC ModeDesc;

		// display ordinal
		unsigned int Index;
		
		// Offset of display in the virtual rect
		int X;
		int Y;

		// Workarea minus any system constructs like the Windows 
		// task bar
		int WorkareaX;
		int WorkareaY;
		int WorkareaWidth;
		int WorkareaHeight;

		bool IsPrimary;
	};

	static bool GetDesc(unsigned int _Index, DESC* _pDesc);
	static bool SetDesc(unsigned int _Index, const DESC* _pDesc);

	// Turns all monitors on or sets them to a low-power state
	static void SetPowerOn(bool _On);

	// Returns true if the specified nth monitor is powered on (not off or in a 
	// low-power state)
	static bool IsPowerOn(unsigned int _Index);

	static unsigned int GetPrimary(); // returns oINVALID if none found
	static unsigned int GetNumDisplays();
	static void GetVirtualRect(int* _pX, int* _pY, int* _pWidth, int* _pHeight);
};

#endif
