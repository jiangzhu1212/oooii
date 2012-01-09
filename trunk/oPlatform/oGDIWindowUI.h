// $(header)
#pragma once
#ifndef oGDIWindowUI_h
#define oGDIWindowUI_h

#include <oBasis/oFixedString.h>
#include "oWindowUIElementBaseMixin.h"

struct oGDIWindowUILine : oWindowUILine, oWindowUIElementBaseMixin<oWindowUILine, oGDIWindowUILine>
{
	oDEFINE_WINDOWUIELEMENT_INTERFACE();
	oDECLARE_CTORDTOR(oGDIWindowUILine);
	oDECLARE_ONEVENT();
};

struct oGDIWindowUIBox : oWindowUIBox, oWindowUIElementBaseMixin<oWindowUIBox, oGDIWindowUIBox>
{
	oDEFINE_WINDOWUIELEMENT_INTERFACE();
	oDECLARE_CTORDTOR(oGDIWindowUIBox);
	oDECLARE_ONEVENT();
};

struct oGDIWindowUIFont : oWindowUIFont, oWindowUIElementBaseMixin<oWindowUIFont, oGDIWindowUIFont>
{
	oDEFINE_WINDOWUIELEMENT_NOMAP_INTERFACE();
	oDECLARE_CTORDTOR(oGDIWindowUIFont);
	oDECLARE_ONEVENT();
	inline HFONT GetFont() const threadsafe { return hFont; }
protected:
	oGDIScopedObject<HFONT> hFont;
};

struct oGDIWindowUIText : oWindowUIText, oWindowUIElementBaseMixin<oWindowUIText, oGDIWindowUIText>
{
	oDEFINE_WINDOWUIELEMENT_INTERFACE();
	oDECLARE_CTORDTOR(oGDIWindowUIText);
	oDECLARE_ONEVENT();
	void GetFont(threadsafe oWindowUIFont** _ppFont) threadsafe override;
	void SetFont(threadsafe oWindowUIFont* _pFont) threadsafe override;
	void SetText(const char* _Text) threadsafe override;

protected:
	oSharedMutex FontStringMutex;
	oRef<threadsafe oWindowUIFont> Font;
	oStringL Text;
};

struct oGDIWindowUIPicture : oWindowUIPicture, oWindowUIElementBaseMixin<oWindowUIPicture, oGDIWindowUIPicture>
{
	oDEFINE_WINDOWUIELEMENT_NOMAP_INTERFACE();
	oDECLARE_CTORDTOR(oGDIWindowUIPicture);
	oDECLARE_ONEVENT();
	void Copy(const void* _pSourceData, size_t _SourcePitch, bool _FlipHorizontally = false, bool _FlipVertically = false) threadsafe override;

protected:
	oSharedMutex BitmapMutex;
	oGDIScopedObject<HBITMAP> hBitmap;
	oGDIScopedObject<HBITMAP> hFlipBitmap;
};

#endif
