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
#include <oPlatform/Windows/oWinWindowing.h>
#include <oPlatform/Windows/oGDI.h>
#include <oPlatform/Windows/oWinAsString.h>
#include <Commdlg.h>
#include <CdErr.h>

using namespace ouro;

static bool oWinDialogGetOpenOrSavePath(path& _Path, const char* _DialogTitle, const char* _FilterPairs, HWND _hParent, bool _IsOpenNotSave)
{
	path_string StrPath;
	if (!_Path.empty())
	{
		if (!clean_path(StrPath, _Path, '\\'))
			return oErrorSetLast(std::errc::invalid_argument, "bad path: %s", _Path.c_str());
	}

	std::string filters;
	char defext[4] = {0};
	if (_FilterPairs)
	{
		filters.assign(_FilterPairs, _FilterPairs+strlen(_FilterPairs)+1);
		filters.append("x"); // set up double nul ptr
		filters[filters.size()-1] = 0;
		for (size_t i = 0; i < filters.size(); i++)
			if (filters[i] == '|')
				filters[i] = '\0';

		// use first ext as default ext...
		const char* first = filters.c_str() + strlen(filters.c_str())+1;
		first = strchr(first, '.');
		if (first)
		{
			defext[0] = *(first+1);
			defext[1] = *(first+2);
			defext[2] = *(first+3);
		}
	}

	OPENFILENAMEA o = {0};
	o.lStructSize = sizeof(OPENFILENAMEA);
	o.hwndOwner = _hParent;
	o.hInstance = nullptr;
	o.lpstrFilter = _FilterPairs ? filters.c_str() : nullptr;
	o.lpstrCustomFilter = nullptr;
	o.nMaxCustFilter = 0;
	o.nFilterIndex = 1;
	o.lpstrFile = StrPath;
	o.nMaxFile = oInt(StrPath.capacity());
	o.lpstrFileTitle = nullptr;
	o.nMaxFileTitle = 0;
	o.lpstrInitialDir = nullptr;
	o.lpstrTitle = _DialogTitle;

	if (_IsOpenNotSave)
		o.Flags = OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST;
	else
		OFN_OVERWRITEPROMPT;
	
	o.nFileOffset = 0;
	o.nFileExtension = 0;
	o.lpstrDefExt = _FilterPairs ? defext : nullptr;
	o.lCustData = (LPARAM)nullptr;
	o.lpfnHook = nullptr;
	o.lpTemplateName = nullptr;
	o.pvReserved = nullptr;
	o.dwReserved = 0;
	o.FlagsEx = 0;

	bool success = _IsOpenNotSave ? !!GetOpenFileName(&o) : !!GetSaveFileName(&o);
	if (!success)
	{
		DWORD err = CommDlgExtendedError();
		if (err == CDERR_GENERALCODES)
			return oErrorSetLast(std::errc::operation_canceled, "dialog was canceled");
		return oErrorSetLast(std::errc::protocol_error, "%s", oWinAsStringCDERR(err));
	}

	_Path = StrPath;
	return true;
}

bool oWinDialogGetOpenPath(path& _Path, const char* _DialogTitle, const char* _FilterPairs, HWND _hParent)
{
	return oWinDialogGetOpenOrSavePath(_Path, _DialogTitle, _FilterPairs, _hParent, true);
}

bool oWinDialogGetSavePath(path& _Path, const char* _DialogTitle, const char* _FilterPairs, HWND _hParent)
{
	return oWinDialogGetOpenOrSavePath(_Path, _DialogTitle, _FilterPairs, _hParent, false);
}

bool oWinDialogGetColor(color* _pColor, HWND _hParent)
{
	int r,g,b,a;
	_pColor->decompose(&r, &g, &b, &a);

	std::array<COLORREF, 16> custom;
	custom.fill(RGB(255,255,255));

	CHOOSECOLOR cc = {0};
	cc.lStructSize = sizeof(CHOOSECOLOR);
	cc.hwndOwner = _hParent;
	cc.rgbResult = RGB(r,g,b);
	cc.lpCustColors = custom.data();
	cc.Flags = CC_ANYCOLOR|CC_FULLOPEN|CC_RGBINIT|CC_SOLIDCOLOR;

	if (!ChooseColor(&cc))
	{
		DWORD err = CommDlgExtendedError();
		if (err == CDERR_GENERALCODES)
			return oErrorSetLast(std::errc::operation_canceled, "dialog was canceled");
		return oErrorSetLast(std::errc::protocol_error, "%s", oWinAsStringCDERR(err));
	}

	*_pColor = color(GetRValue(cc.rgbResult), GetGValue(cc.rgbResult), GetBValue(cc.rgbResult), a);
	return true;
}

bool oWinDialogGetFont(LOGFONT* _pLogicalFont, color* _pColor, HWND _hParent)
{
	oGDIScopedGetDC hDC(_hParent);
	CHOOSEFONT cf = {0};
	cf.lStructSize = sizeof(CHOOSEFONT);
	cf.hwndOwner = _hParent;
	cf.hDC = hDC;
	cf.lpLogFont = _pLogicalFont;
	cf.iPointSize = 0;
	cf.Flags = CF_EFFECTS|CF_INITTOLOGFONTSTRUCT;

	if (_pColor)
	{
		int r,g,b,a;
		_pColor->decompose(&r, &g, &b, &a);
		cf.rgbColors = RGB(r,g,b);
	}
	else
		cf.rgbColors = RGB(0,0,0);

	cf.nFontType = SCREEN_FONTTYPE;

	if (!ChooseFont(&cf))
	{
		DWORD err = CommDlgExtendedError();
		if (err == CDERR_GENERALCODES)
			return oErrorSetLast(std::errc::operation_canceled, "dialog was canceled");
		return oErrorSetLast(std::errc::protocol_error, "%s", oWinAsStringCDERR(err));
	}

	if (_pColor)
		*_pColor = color(GetRValue(cf.rgbColors), GetGValue(cf.rgbColors), GetBValue(cf.rgbColors), 255);

	return true;
}