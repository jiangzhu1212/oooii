/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2013 Antony Arciuolo.                                    *
 * arciuolo@gmail.com                                                     *
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
// A simple progress bar for very simple applications. NOTE: This is for the 90%
// case. Dialogs are fairly easy to construct and should be tailored to the 
// task,so if there's a complex case such as wanting to see individual task v. 
// overall progress, or seeing progress of each thread, a different dialog box 
// should be created. In the meantime, ALL components of a major task should add 
// up to the single completion of the progress bar. I.e. don't have Step1 go to 
// 100% and then Step2 resets and goes to 100%, just update the task and have 
// each add up to 50% of the overall progress. See GUI suggestions on the net. 
// There's a good starting point at:
// http://msdn.microsoft.com/en-us/library/aa511486.aspx
#pragma once
#ifndef oProgressBar_h
#define oProgressBar_h

#include <oPlatform/oWindow.h>

// {1357E33D-DCED-4DE1-9E02-9A0A492AE1FE}
oDEFINE_GUID_I(oProgressBar, 0x1357e33d, 0xdced, 0x4de1, 0x9e, 0x2, 0x9a, 0xa, 0x49, 0x2a, 0xe1, 0xfe);
interface oProgressBar : oInterface
{
public:
	// Allowable interface from oWindow
	inline oGUI_WINDOW GetNativeHandle() const threadsafe { return GetWindow()->GetNativeHandle(); }
	inline int GetDisplayId() const { return GetWindow()->GetDisplayId(); }
	inline bool IsWindowThread() const threadsafe { return GetWindow()->IsWindowThread(); }
	inline oGUI_WINDOW_STATE GetState() const { oGUI_WINDOW_SHAPE_DESC s = GetWindow()->GetShape(); return s.State; }
	inline void Hide() threadsafe { GetWindow()->SetState(oGUI_WINDOW_HIDDEN); }
	inline void Minimize() threadsafe { GetWindow()->SetState(oGUI_WINDOW_MINIMIZED); }
	inline void Restore() threadsafe { GetWindow()->SetState(oGUI_WINDOW_RESTORED); }
	inline void SetClientPosition(const int2& _ClientPosition) threadsafe { oGUI_WINDOW_SHAPE_DESC s; s.ClientPosition = _ClientPosition; GetWindow()->SetShape(s); }
	inline int2 GetClientPosition() const { oGUI_WINDOW_SHAPE_DESC s = GetWindow()->GetShape(); return s.ClientPosition; }
	inline int2 GetClientSize() const { oGUI_WINDOW_SHAPE_DESC s = GetWindow()->GetShape(); return s.ClientSize; }
	inline void SetIcon(oGUI_ICON _hIcon) threadsafe { GetWindow()->SetIcon(_hIcon); }
	inline oGUI_ICON GetIcon() const { return GetWindow()->GetIcon(); }
	inline void SetParent(oWindow* _pParent) threadsafe { GetWindow()->SetParent(_pParent); }
	inline oWindow* GetParent() const { GetWindow()->GetParent(); }
	void SetOwner(oWindow* _pOwner) threadsafe { GetWindow()->SetOwner(_pOwner); }
	oWindow* GetOwner() const { return GetWindow()->GetOwner(); }
	void SetSortOrder(oGUI_WINDOW_SORT_ORDER _SortOrder) threadsafe { GetWindow()->SetSortOrder(_SortOrder); }
	oGUI_WINDOW_SORT_ORDER GetSortOrder() const { return GetWindow()->GetSortOrder(); }
	void SetFocus(bool _Focus = true) threadsafe { GetWindow()->SetFocus(_Focus); }
	bool HasFocus() const { return GetWindow()->HasFocus(); }
	void SetTitleV(const char* _Format, va_list _Args) threadsafe { return GetWindow()->SetTitleV(_Format, _Args); }
	char* GetTitle(char* _StrDestination, size_t _SizeofStrDestination) const { return GetWindow()->GetTitle(_StrDestination, _SizeofStrDestination); }
	inline void SetTitle(const char* _Format, ...) threadsafe { va_list args; va_start(args, _Format); SetTitleV(_Format, args); va_end(args); }
	template<size_t size> char* GetTitle(char (&_StrDestination)[size]) const { return GetTitle(_StrDestination, size); }
	template<size_t capacity> char* GetTitle(ouro::fixed_string<char, capacity>& _StrDestination) const { return GetTitle(_StrDestination, _StrDestination.capacity()); }
	inline void FlushMessages(bool _WaitForNext = false) { GetWindow()->FlushMessages(_WaitForNext); }
	inline void Quit() threadsafe { GetWindow()->Quit(); }
	// Unique to oProgressBar

	virtual void ShowStop(bool _Show = true) threadsafe = 0;
	virtual bool StopShown() const = 0;

	virtual void SetStopped(bool _Stopped = true) threadsafe = 0;
	virtual bool GetStopped() const = 0;

	virtual void SetTextV(const char* _Format, va_list _Args) threadsafe = 0;
	virtual char* GetText(char* _StrDestination, size_t _SizeofStrDestination) const = 0;

	inline void SetText(const char* _Format, ...) threadsafe { va_list args; va_start(args, _Format); SetTextV(_Format, args); va_end(args); }
	template<size_t size> char* GetText(char (&_StrDestination)[size]) const { return GetText(_StrDestination, size); }
	template<size_t capacity> char* GetText(ouro::fixed_string<char, capacity>& _StrDestination) const { return GetText(_StrDestination, _StrDestination.capacity()); }

	virtual void SetSubtextV(const char* _Format, va_list _Args) threadsafe = 0;
	virtual char* GetSubtext(char* _StrDestination, size_t _SizeofStrDestination) const = 0;

	inline void SetSubtext(const char* _Format, ...) threadsafe { va_list args; va_start(args, _Format); SetSubtextV(_Format, args); va_end(args); }
	template<size_t size> char* GetSubtext(char (&_StrDestination)[size]) const { return GetSubtext(_StrDestination, size); }
	template<size_t capacity> char* GetSubtext(ouro::fixed_string<char, capacity>& _StrDestination) const { return GetSubtext(_StrDestination, _StrDestination.capacity()); }

	// Sets the percentage complete. This value is internally clamped to [0,100], 
	// unless set to a negative value in which case an unknown progress display is
	// shown (marquee).
	virtual void SetPercentage(int _Percentage) threadsafe = 0;

	// Adds the amount (clamps total [0,100]). This way several threads can pre-
	// calculate their individual contribution and asynchronously add it to the
	// progress. If this number goes negative then an unknown progress display is
	// shown (marquee).
	virtual void AddPercentage(int _Percentage) threadsafe = 0;

	virtual int GetPercentage() const = 0;

private:
	virtual const threadsafe oWindow* GetWindow() const threadsafe = 0;
	virtual threadsafe oWindow* GetWindow() threadsafe = 0;
	virtual const oWindow* GetWindow() const = 0;
	virtual oWindow* GetWindow() = 0;
};

bool oProgressBarCreate(const oTASK& _OnStopPressed, const char* _Title, oGUI_ICON _hIcon, oProgressBar** _ppProgressBar);
inline bool oProgressBarCreate(const oTASK& _OnStopPressed, const char* _Title, oProgressBar** _ppProgressBar) { return oProgressBarCreate(_OnStopPressed, _Title, nullptr, _ppProgressBar); }
inline bool oProgressBarCreate(const oTASK& _OnStopPressed, oGUI_ICON _hIcon, oProgressBar** _ppProgressBar) { return oProgressBarCreate(_OnStopPressed, nullptr, _hIcon, _ppProgressBar); }
inline bool oProgressBarCreate(const oTASK& _OnStopPressed, oProgressBar** _ppProgressBar) { return oProgressBarCreate(_OnStopPressed, nullptr, nullptr, _ppProgressBar); }

#endif
