// $(header)
#include "oD2DWindowUI.h"
#include <oBasis/oByte.h>
#include <oPlatform/oD2D.h>
#include <oBasis/oMemory.h>
#include <oBasis/oSize.h>
#include <oPlatform/oWinRect.h>
#include <oPlatform/oWinWindowing.h>

// Current only creates a pre-multiplied alpha blended BGRA
oD2DWindowUILine::oD2DWindowUILine(const DESC& _Desc, threadsafe oWindow* _pWindow, bool* _pSuccess)
	: oWindowUIElementBaseMixin(_Desc, _pWindow)
{
	oHOOK_ONEVENT(oD2DWindowUILine);
	*_pSuccess = true;
}

oD2DWindowUILine::~oD2DWindowUILine()
{
	oUNHOOK_ONEVENT();
}

bool oD2DWindowUILine::OnEvent(oWindow::EVENT _Event, unsigned int _SuperSampleScale, const oWindow::DESC& _Desc)
{
	oRef<ID2D1RenderTarget> D2DRenderTarget;
	oVERIFY(Window->QueryInterface((const oGUID&)__uuidof(ID2D1RenderTarget), &D2DRenderTarget));
	switch (_Event)
	{
		case oWindow::RESIZING:
			Brush = nullptr;
			break;
		case oWindow::RESIZED:
			oV(D2DRenderTarget->CreateSolidColorBrush(oD2DAsColorF(MIXINGetDesc().Color), &Brush));
			break;

		case oWindow::DRAW_UIAA:
		{
			const DESC& d = MIXINGetDesc();
			if (!oIsTransparentColor(d.Color))
			{
				Brush->SetColor(oD2DAsColorF(d.Color));
				D2DRenderTarget->DrawLine(oD2DAsPOINT(d.P1), oD2DAsPOINT(d.P2), Brush, static_cast<float>(d.Thickness));
			}
			
			break;
		}

		default:
			break;
	}

	return true;
}

oD2DWindowUIBox::oD2DWindowUIBox(const DESC& _Desc, threadsafe oWindow* _pWindow, bool* _pSuccess)
	: oWindowUIElementBaseMixin(_Desc, _pWindow)
{
	oHOOK_ONEVENT(oD2DWindowUIBox);
	*_pSuccess = true;
}

oD2DWindowUIBox::~oD2DWindowUIBox()
{
	oUNHOOK_ONEVENT();
}

bool oD2DWindowUIBox::OnEvent(oWindow::EVENT _Event, unsigned int _SuperSampleScale, const oWindow::DESC& _Desc)
{
	oRef<ID2D1RenderTarget> D2DRenderTarget;
	oVERIFY(Window->QueryInterface((const oGUID&)__uuidof(ID2D1RenderTarget), &D2DRenderTarget));
	switch (_Event)
	{
		case oWindow::RESIZING:
			Brush = nullptr;
			BorderBrush = nullptr;
			break;

		case oWindow::RESIZED:
		{
			const DESC& d = MIXINGetDesc();
			oV(D2DRenderTarget->CreateSolidColorBrush(oD2DAsColorF(d.Color), &Brush));
			oV(D2DRenderTarget->CreateSolidColorBrush(oD2DAsColorF(d.BorderColor), &BorderBrush));
			break;
		}

		case oWindow::DRAW_UIAA:
		{
			const DESC& d = MIXINGetDesc();
			if (!oIsTransparentColor(d.Color) || !oIsTransparentColor(d.BorderColor))
			{
				Brush->SetColor(oD2DAsColorF(d.Color));
				BorderBrush->SetColor(oD2DAsColorF(d.BorderColor));

				D2D1_ROUNDED_RECT r;
				r.rect = oD2DAsRect(oWinRectResolve(Window, d));
				r.radiusX = r.radiusY = d.Roundness;
				oVERIFY(oD2DDrawRoundedRect(D2DRenderTarget, r, Brush, BorderBrush));
			}
			break;
		}

		default:
			break;
	}

	return true;
}

oD2DWindowUIFont::oD2DWindowUIFont(const DESC& _Desc, threadsafe oWindow* _pWindow, bool* _pSuccess)
	: oWindowUIElementBaseMixin(_Desc, _pWindow)
{
	oWStringS FName = _Desc.FontName;
	oV(oD2DGetSharedDWriteFactory()->CreateTextFormat(
		FName
		, 0
		, (_Desc.Style == BOLD || _Desc.Style == BOLDITALIC) ? DWRITE_FONT_WEIGHT_BOLD : DWRITE_FONT_WEIGHT_NORMAL
		, (_Desc.Style == ITALIC || _Desc.Style == BOLDITALIC) ? DWRITE_FONT_STYLE_ITALIC : DWRITE_FONT_STYLE_NORMAL
		, DWRITE_FONT_STRETCH_NORMAL
		, oPointToDIP(_Desc.PointSize)
		, L"en-us"
		, &Format
		));

	#ifdef _DEBUG
		oV(Format->GetFontFamilyName(FName.c_str(), oSize32(FName.capacity())));
		oStringS fontFamilyName = FName;
		oV(Format->GetLocaleName(FName.c_str(), oSize32(FName.capacity())));
		oStringS locale = FName;
		oTRACE("DWriteFont Created: %s %s%s %s", fontFamilyName.c_str(), Format->GetFontWeight() == DWRITE_FONT_WEIGHT_REGULAR ? "" : "bold", Format->GetFontStyle() == DWRITE_FONT_STYLE_ITALIC ? "italic" : "", locale.c_str());
	#endif
	*_pSuccess = true;
}

oD2DWindowUIFont::~oD2DWindowUIFont()
{
}

oD2DWindowUIText::oD2DWindowUIText(const DESC& _Desc, threadsafe oWindow* _pWindow, bool* _pSuccess)
	: oWindowUIElementBaseMixin(_Desc, _pWindow)
{
	oHOOK_ONEVENT(oD2DWindowUIText);
	*_pSuccess = true;
}

oD2DWindowUIText::~oD2DWindowUIText()
{
	oUNHOOK_ONEVENT();
}

void oD2DWindowUIText::GetFont(threadsafe oWindowUIFont** _ppFont) threadsafe
{
	oSharedLock<oSharedMutex> lock(FontTextMutex);
	Font->Reference();
	*_ppFont = Font;
}

void oD2DWindowUIText::SetFont(threadsafe oWindowUIFont* _pFont) threadsafe
{
	oLockGuard<oSharedMutex> lock(FontTextMutex);
	Font = _pFont;
	Window->Refresh(false);
}

void oD2DWindowUIText::SetText(const char* _Text) threadsafe
{
	oLockGuard<oSharedMutex> lock(FontTextMutex);
	thread_cast<oD2DWindowUIText*>(this)->Text = oSAFESTR(_Text);
	Window->Refresh(false);
}

bool oD2DWindowUIText::OnEvent(oWindow::EVENT _Event, unsigned int _SuperSampleScale, const oWindow::DESC& _Desc)
{
	oRef<ID2D1RenderTarget> D2DRenderTarget;
	oVERIFY(Window->QueryInterface((const oGUID&)__uuidof(ID2D1RenderTarget), &D2DRenderTarget));
	switch (_Event)
	{
		case oWindow::RESIZING:
			Brush = nullptr;
			ShadowBrush = nullptr;
			break;
		
		case oWindow::RESIZED:
		{
			const DESC& d = MIXINGetDesc();
			oV(D2DRenderTarget->CreateSolidColorBrush(oD2DAsColorF(d.Color), &Brush));
			oV(D2DRenderTarget->CreateSolidColorBrush(oD2DAsColorF(d.ShadowColor), &ShadowBrush));
			break;
		}

		case oWindow::DRAW_UI:
		{
			const DESC& d = MIXINGetDesc();
			D2D1_RECT_F rAdjusted = oD2DAsRect(oWinRectResolve(Window, d));
			oLockGuard<oSharedMutex> lock(FontTextMutex);
			if (Font && *Text && !oIsTransparentColor(d.Color))
			{
				IDWriteTextFormat* pFormat = static_cast<threadsafe oD2DWindowUIFont*>(Font.c_ptr())->GetFormat();
				oVERIFY(oD2DSetAlignment(pFormat, d.Alignment));
				pFormat->SetWordWrapping(d.MultiLine ? DWRITE_WORD_WRAPPING_WRAP : DWRITE_WORD_WRAPPING_NO_WRAP);

				if (!oIsTransparentColor(d.ShadowColor))
				{
					oWindowUIFont::DESC fdesc;
					Font->GetDesc(&fdesc);
					D2D1_RECT_F rShadow = rAdjusted; rShadow.left += fdesc.ShadowOffset; rShadow.top += fdesc.ShadowOffset; rShadow.right += fdesc.ShadowOffset; rShadow.bottom += fdesc.ShadowOffset;
					ShadowBrush->SetColor(oD2DColor(d.ShadowColor));
					D2DRenderTarget->DrawText(Text.c_str(), oSize32(Text.size()), pFormat, rShadow, ShadowBrush);
				}

				Brush->SetColor(oD2DColor(d.Color));
				D2DRenderTarget->DrawText(Text.c_str(), oSize32(Text.size()), pFormat, rAdjusted, Brush);
			}
			break;
		}

		default:
			break;
	}

	return true;
}

oD2DWindowUIPicture::oD2DWindowUIPicture(const DESC& _Desc, threadsafe oWindow* _pWindow, bool* _pSuccess)
	: oWindowUIElementBaseMixin(_Desc, _pWindow)
	, CPUBitmap(nullptr)
	, CPUBitmapRowPitch(oSurfaceGetSize(oSURFACE_B8G8R8A8_UNORM) * _Desc.SurfaceDesc.Dimensions.x) // // We'll be converting to RGBA from RGB in Copy()
{
	CPUBitmap = new char[CPUBitmapRowPitch * _Desc.SurfaceDesc.Dimensions.y];
	memset(CPUBitmap, 0, CPUBitmapRowPitch * _Desc.SurfaceDesc.Dimensions.y);
	oHOOK_ONEVENT(oD2DWindowUIPicture);
	*_pSuccess = HookID != oInvalid;
}

oD2DWindowUIPicture::~oD2DWindowUIPicture()
{
	oUNHOOK_ONEVENT();
	if (CPUBitmap)
		delete [] CPUBitmap;
}

static void oMemcpy2dPreMultiplyAlpha(oColor* oRESTRICT _pDestination, size_t _DestinationPitch, const oColor* oRESTRICT _pSource, size_t _SourcePitch, size_t _SourceRowSize, size_t _NumRows)
{
	float r,g,b,a;
	const oColor* end = oByteAdd(_pSource, _SourcePitch * _NumRows);
	size_t nRowColors = _SourceRowSize / sizeof(oColor);
	for(; _pSource < end; _pDestination = oByteAdd(_pDestination, _DestinationPitch), _pSource = oByteAdd(_pSource, _SourcePitch))
	{
		for (size_t i = 0; i < nRowColors; i++)
		{
			oColorDecompose(_pSource[i], &r, &g, &b, &a);
			r *= a; g *= a; b *= a;
			_pDestination[i] = oColorCompose(r, g, b, a);
		}
	}
}

static void oMemcpy2d24To32Bit(oColor* oRESTRICT _pDestination, size_t _DestinationPitch, const void* oRESTRICT _pSource, size_t _SourcePitch, size_t _SourceRowSize, size_t _NumRows)
{
	const void* end = oByteAdd(_pSource, _SourcePitch * _NumRows);
	for(; _pSource < end; _pDestination = oByteAdd(_pDestination, _DestinationPitch), _pSource = oByteAdd(_pSource, _SourcePitch))
	{
		unsigned char* rgb = (unsigned char*)_pSource;
		const unsigned char* lineEnd = oByteAdd(rgb, _SourceRowSize);
		oColor* pDest = _pDestination;
		while (rgb < lineEnd)
		{
			unsigned int r,g,b;
			b = *rgb++;
			g = *rgb++;
			r = *rgb++;
			*pDest++ = oColorCompose(r, g, b, 0xff);
		}
	}
}

void oD2DWindowUIPicture::Copy(const void* _pSourceData, size_t _SourcePitch, bool _FlipHorizontally, bool _FlipVertically) threadsafe
{
	HWND hWnd = (HWND)Window->GetNativeHandle();
	oLockGuard<oSharedMutex> lock(BitmapMutex);

	const DESC& d = MIXINGetDesc();

	if (oSurfaceIsAlphaFormat(d.SurfaceDesc.Format))
		oMemcpy2dPreMultiplyAlpha(static_cast<oColor*>(CPUBitmap), CPUBitmapRowPitch, static_cast<const oColor*>(_pSourceData), _SourcePitch, d.SurfaceDesc.Dimensions.x * sizeof(oColor), d.SurfaceDesc.Dimensions.y);
	else if (3 == oSurfaceGetSize(d.SurfaceDesc.Format))
		oMemcpy2d24To32Bit(static_cast<oColor*>(CPUBitmap), CPUBitmapRowPitch, _pSourceData, _SourcePitch, d.SurfaceDesc.Dimensions.x * 3, d.SurfaceDesc.Dimensions.y);
	else
		oMemcpy2d(CPUBitmap, CPUBitmapRowPitch, _pSourceData, _SourcePitch, d.SurfaceDesc.Dimensions.x * sizeof(oColor), d.SurfaceDesc.Dimensions.y);
	
	BitmapDirty = true;
	FlippedHorizontally = _FlipHorizontally;
	FlippedVertically = _FlipVertically;

	oRECT r = oRect(oWinRectResolve(Window, d));
	Window->Refresh(false, &r);
}

void oD2DWindowUIPicture::ApplyCPUBitmap() threadsafe
{
	oLockGuard<oSharedMutex> lock(BitmapMutex);
	oV(Bitmap->CopyFromMemory(nullptr, CPUBitmap, CPUBitmapRowPitch));
	BitmapDirty = false;
}

bool oD2DWindowUIPicture::OnEvent(oWindow::EVENT _Event, unsigned int _SuperSampleScale, const oWindow::DESC& _Desc)
{
	oRef<ID2D1RenderTarget> D2DRenderTarget;
	oVERIFY(Window->QueryInterface((const oGUID&)__uuidof(ID2D1RenderTarget), &D2DRenderTarget));
	switch (_Event)
	{
		case oWindow::RESIZING:
			Bitmap = nullptr;
			break;
		case oWindow::RESIZED:
		{
			const DESC& d = MIXINGetDesc();
			if (!oD2DCreateBitmap(D2DRenderTarget, d.SurfaceDesc.Dimensions.xy(), oSURFACE_B8G8R8A8_UNORM, &Bitmap))
				return false;
			ApplyCPUBitmap();
			break;
		}
		case oWindow::DRAW_UIAA:
		{
			if (BitmapDirty)
				ApplyCPUBitmap();

			HWND hWnd = (HWND)Window->GetNativeHandle();
			RECT cr;
			oVB(GetClientRect(hWnd, &cr));

			D2D1_RECT_F r = oD2DAsRect(oWinRectResolve(Window, MIXINGetDesc()));
			D2D1::Matrix3x2F flipMatrix;
			CalculateTransform(r, &flipMatrix);

			D2DRenderTarget->SetTransform(flipMatrix);
			D2DRenderTarget->DrawBitmap(Bitmap, r);
			D2DRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
			break;
		}
		default:
			break;
	}

	return true;
}

void oD2DWindowUIPicture::CalculateTransform(const D2D1_RECT_F& _Rect, D2D1::Matrix3x2F* _pMatrix)
{
	float w = _Rect.right - _Rect.left;
	float h = _Rect.bottom - _Rect.top;

	float centerX = _Rect.left + w / 2.0f;
	float centerY = _Rect.top + h / 2.0f;

	*_pMatrix = D2D1::Matrix3x2F::Identity();

	if (FlippedHorizontally)
	{
		_pMatrix->_11 = -1.0f;
		_pMatrix->_31 = centerX * 2.0f;
	}

	if (FlippedVertically)
	{
		_pMatrix->_22 = -_pMatrix->_22;
		_pMatrix->_32 = centerY * 2.0f;
	}
}
