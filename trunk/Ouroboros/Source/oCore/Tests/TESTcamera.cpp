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
#include <oCore/camera.h>
#include <oBase/assert.h>

#include "../../test_services.h"

namespace ouro {

	const char* as_string(const camera::format& _Format); // @tony: why is this necessary?

	namespace tests {

void TESTcamera(test_services& _Services)
{
	bool reported = false;

	camera::enumerate([&](std::shared_ptr<camera> _Camera)->bool
	{
		if (!reported)
		{
			_Services.report("camera \"%s\" detected. See trace for more info.", _Camera->name());
			reported = true;
		}

		oTRACE("oCore::camera \"%s\", supports:", _Camera->name());
		_Camera->enumerate_modes([&](const camera::mode& _Mode)->bool
		{
			oTRACE("- %dx%d %s %d bitrate", _Mode.dimensions.x, _Mode.dimensions.y, as_string(_Mode.format), _Mode.bit_rate);
			return true;
		});
		return true;
	});

	if (!reported)
		_Services.report("no camera detected.");

#if 0 // @tony: make a simple test app for cameras

	struct CONTEXT
	{
		CONTEXT(ouro::intrusive_ptr<threadsafe oCamera> _pCamera)
			: Camera(_pCamera)
			, LastFrame(oInvalid)
			, Running(true)
		{}

		ouro::intrusive_ptr<threadsafe oCamera> Camera;
		ouro::intrusive_ptr<oWindow> Window;
		unsigned int LastFrame;
		bool Running;

		void OnEvent(const window::basic_event& _Event)
		{
			switch (_Event.Type)
			{
				case ouro::event_type::closing:
					Running = false;
					break;
				default:
					break;
			}
		}
	};

	std::vector<CONTEXT> Contexts;
	unsigned int index = 0;

	while (1)
	{
		ouro::intrusive_ptr<threadsafe oCamera> Camera;
		if (oCameraEnum(index++, &Camera))
		{
			oCamera::MODE mode;
			mode.Dimensions = int2(640, 480);
			mode.Format = ouro::surface::r8g8b8_unorm;
			mode.BitRate = ~0u;

			oCamera::MODE closest;
			if (!Camera->FindClosestMatchingMode(mode, &closest))
				oASSERT(false, "");

			if (!Camera->SetMode(closest))
			{
				oMSGBOX_DESC d;
				d.Type = oMSGBOX_ERR;
				d.Title = "oCamera Test";
				oMsgBox(d, "Camera %s does not support mode %s %dx%d", Camera->GetName(), ouro::as_string(mode.Format), mode.Dimensions.x, mode.Dimensions.y);
				continue;
			}

			Contexts.push_back(Camera);
		}

		else if (oErrorGetLast() == std::errc::no_such_device)
			break;
	}

//	if (oErrorGetLast() == std::errc::no_such_device && !Contexts.empty())
//	{
//		oMSGBOX_DESC d;
//		d.Type = oMSGBOX_ERR;
//		d.Title = "oCamera Test";
//#if o64BIT
//		oMsgBox(d, "Enumerating system cameras is not supported on 64-bit systems.");
//#else
//		oMsgBox(d, "Failed to enumerate system cameras because an internal interface could not be found.");
//#endif
//		return -1;
//	}

	if (Contexts.empty())
	{
		oMSGBOX_DESC d;
		d.Type = oMSGBOX_INFO;
		d.Title = "oCamera Test";
		oMsgBox(d, "No cameras were found, so no windows will open");
	}

	for (size_t i = 0; i < Contexts.size(); i++)
	{
		oCamera::DESC cd;
		Contexts[i].Camera->GetDesc(&cd);

		ouro::lstring Title;
		snprintf(Title, "%s (%dx%d %s)", Contexts[i].Camera->GetName(), cd.Mode.Dimensions.x, cd.Mode.Dimensions.y, ouro::as_string(cd.Mode.Format));

		window::init init;
		init.shape.ClientSize = cd.Mode.Dimensions;
		init.shape.ClientPosition = int2(30, 30) * int2(oUInt(i + 1), oUInt(i + 1));
		init.title = Title;
		init.on_event = std::bind(&CONTEXT::OnEvent, &Contexts[i], oBIND1);
		Contexts[i].Window = window::make(init);
	}

	int OpenWindowCount = 0;
	for (size_t i = 0; i < Contexts.size(); i++)
		if (Contexts[i].Window)
			OpenWindowCount++;

	while (OpenWindowCount)
	{
		for (size_t i = 0; i < Contexts.size(); i++)
		{
			Contexts[i].Window->FlushMessages();

			oCamera::DESC cd;
			Contexts[i].Camera->GetDesc(&cd);

			oCamera::MAPPED mapped;
			if (Contexts[i].Camera->Map(&mapped))
			{
				if (mapped.Frame != Contexts[i].LastFrame)
				{
					HWND hWnd = (HWND)Contexts[i].Window->GetNativeHandle();

					oVERIFY(oGDIStretchBits(hWnd, cd.Mode.Dimensions, cd.Mode.Format, mapped.pData, mapped.RowPitch));
					Contexts[i].LastFrame = mapped.Frame;

					float fps = Contexts[i].Camera->GetFPS();
					ouro::sstring sFPS;
					snprintf(sFPS, "FPS: %.01f", fps + (rand() %100) / 100.0f);
					
					RECT rClient;
					GetClientRect(hWnd, &rClient);

					ouro::text_info td;
					td.Foreground = ouro::CadetBlue;
					td.Position = int2(0, 0);
					td.Size = oWinRectSize(rClient);
					td.Shadow = ouro::Black;
					td.ShadowOffset = int2(1,1);
					td.Alignment = ouro::alignment::middle_left;

					oGDIScopedGetDC hDC(hWnd);
					oGDIDrawText(hDC, td, sFPS);
				}

				Contexts[i].Camera->Unmap();
			}
		}

		OpenWindowCount = 0;
		for (size_t i = 0; i < Contexts.size(); i++)
			if (Contexts[i].Running)
				OpenWindowCount++;
	}

	return 0;
#endif
}

	} // namespace tests
} // namespace ouro

