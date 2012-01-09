// $(header)
#include <oPlatform/oWindowUI.h>
#include "oD2DWindowUI.h"
#include "oGDIWindowUI.h"

const char* oAsString(const oANCHOR& _Anchor)
{
	switch (_Anchor)
	{
		case oTOPLEFT: return "oTOPLEFT";
		case oTOPCENTER: return "oTOPCENTER";
		case oTOPRIGHT: return "oTOPRIGHT";
		case oMIDDLELEFT: return "oMIDDLELEFT";
		case oMIDDLECENTER: return "oMIDDLECENTER";
		case oMIDDLERIGHT: return "oMIDDLERIGHT";
		case oBOTTOMLEFT: return "oBOTTOMLEFT";
		case oBOTTOMCENTER: return "oBOTTOMCENTER";
		case oBOTTOMRIGHT: return "oBOTTOMRIGHT";
		case oFITPARENT: return "oFITPARENT";
		default: break;
	}
	return "Unrecognized oANCHOR";
}

template<typename InterfaceT, typename GDIImplT, typename D2DImplT> bool oWindowUIElementCreate(const typename InterfaceT::DESC& _Desc, threadsafe oWindow* _pWindow, threadsafe InterfaceT** _ppElement)
{
	if (!_pWindow || !_ppElement)
	{
		oErrorSetLast(oERROR_INVALID_PARAMETER);
		return false;
	}

	bool success = false;
	switch (_pWindow->GetDrawMode())
	{
		case oWindow::USE_GDI: oCONSTRUCT(_ppElement, GDIImplT(_Desc, _pWindow, &success)); break;
		case oWindow::USE_D2D: oCONSTRUCT(_ppElement, D2DImplT(_Desc, _pWindow, &success)); break;
		default:
			oErrorSetLast(oERROR_NOT_FOUND, "oWindow with an incompatible oWindow::DRAW_MODE specified.");
			return false;
	}

	return success;
}

#define oDEFINE_WINDOWUI_CREATE(_ElementType) bool oWindowUI##_ElementType##Create(const oWindowUI##_ElementType::DESC& _Desc, threadsafe oWindow* _pWindow, threadsafe oWindowUI##_ElementType** _ppElement) { return oWindowUIElementCreate<oWindowUI##_ElementType, oGDIWindowUI##_ElementType, oD2DWindowUI##_ElementType>(_Desc, _pWindow, _ppElement); }
oDEFINE_WINDOWUI_CREATE(Line);
oDEFINE_WINDOWUI_CREATE(Box);
oDEFINE_WINDOWUI_CREATE(Font);
oDEFINE_WINDOWUI_CREATE(Text);
oDEFINE_WINDOWUI_CREATE(Picture);
