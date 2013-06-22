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
#include <oBasis/oCameraControllerMaya.h>
#include <oBasis/oArcball.h>
#include <oBasis/oGUI.h>
#include <oBasis/oRefCount.h>

struct oCameraControllerModelerImpl : oCameraControllerModeler
{
	oCameraControllerModelerImpl(const oCAMERA_CONTROLLER_MODELER_DESC& _Desc, bool* _pSuccess);
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_TRIVIAL_QUERYINTERFACE2(oCameraController, oCameraControllerModeler);

	int OnAction(const oGUI_ACTION_DESC& _Action) override;
	void Tick() override {}
	void OnLostCapture() override;
	void SetView(const float4x4& _View) override { Arcball.SetView(_View); }
	float4x4 GetView(float _DeltaTime = 0.0f) override { return Arcball.GetView(); }
	void SetLookAt(const float3& _LookAt) override { Arcball.SetLookAt(_LookAt); }
	float3 GetLookAt() const override { return Arcball.GetLookAt(); }
	void SetDesc(const oCAMERA_CONTROLLER_MODELER_DESC& _Desc) override { Desc = _Desc; }
	void GetDesc(oCAMERA_CONTROLLER_MODELER_DESC* _pDesc) const override { *_pDesc = Desc; }

private:
	oCAMERA_CONTROLLER_MODELER_DESC Desc;
	oRefCount RefCount;
	std::array<bool, oCAMERA_CONTROLLER_MODELER_DESC::NUM_CONTROLS> KeyStates;
	float3 PointerPosition;
	// keep this as a float2... it ensures we ignore the mouse wheel in 
	// transformation calculations because that's already relative. For mouse 
	// wheel support - use PointerPosition.z as its own delta.
	float2 LastPointerPosition;
	oArcball Arcball;
};

oCameraControllerModelerImpl::oCameraControllerModelerImpl(const oCAMERA_CONTROLLER_MODELER_DESC& _Desc, bool* _pSuccess)
	: Desc(_Desc)
	, PointerPosition(0.0f)
	, LastPointerPosition(0.0f)
	, Arcball(oARCBALL_CONSTRAINT_Y_UP)
{
	KeyStates.fill(false);
	*_pSuccess = true;
}

bool oCameraControllerModelerCreate(const oCAMERA_CONTROLLER_MODELER_DESC& _Desc, oCameraControllerModeler** _ppCameraController)
{
	bool success = false;
	oCONSTRUCT(_ppCameraController, oCameraControllerModelerImpl(_Desc, &success));
	return success;
}

int oCameraControllerModelerImpl::OnAction(const oGUI_ACTION_DESC& _Action)
{
	bool WasTumbling = KeyStates[oCAMERA_CONTROLLER_MODELER_DESC::TUMBLER];
	bool WasTracking = !WasTumbling && KeyStates[oCAMERA_CONTROLLER_MODELER_DESC::TRACK];
	bool WasDolly = !WasTracking && KeyStates[oCAMERA_CONTROLLER_MODELER_DESC::DOLLY];

	Arcball.SetConstraint(Desc.Constraint);

	// Always show pointer
	int Response = oCAMERA_CONTROLLER_SHOW_POINTER;

	LastPointerPosition = PointerPosition.xy();
	oGUIRecordInputState(_Action, Desc.Controls.data(), Desc.Controls.size(), KeyStates.data(), KeyStates.size(), &PointerPosition);
	float2 PointerPositionDelta = PointerPosition.xy() - LastPointerPosition;

	const bool kActive = Desc.Controls[oCAMERA_CONTROLLER_MODELER_DESC::ACTIVATION] == oGUI_KEY_NONE || KeyStates[oCAMERA_CONTROLLER_MODELER_DESC::ACTIVATION];

	if (kActive && KeyStates[oCAMERA_CONTROLLER_MODELER_DESC::TUMBLER])
	{
		if (!WasTumbling)
			Arcball.BeginRotation();

		Arcball.Rotate(PointerPositionDelta * Desc.RotationSpeed);
		Response |= oCAMERA_CONTROLLER_ROTATING_YAW|oCAMERA_CONTROLLER_ROTATING_PITCH|oCAMERA_CONTROLLER_ROTATING_AROUND_COI|oCAMERA_CONTROLLER_LOCK_POINTER;
	}

	else if (kActive && KeyStates[oCAMERA_CONTROLLER_MODELER_DESC::TRACK])
	{
		Arcball.Pan(PointerPositionDelta * Desc.PanSpeed);
		Response |= oCAMERA_CONTROLLER_TRANSLATING_X|oCAMERA_CONTROLLER_TRANSLATING_Y|oCAMERA_CONTROLLER_LOCK_POINTER;
	}

	else if (kActive && KeyStates[oCAMERA_CONTROLLER_MODELER_DESC::DOLLY])
	{
		Arcball.Dolly(PointerPositionDelta * Desc.DollySpeed);
		Response |= oCAMERA_CONTROLLER_TRANSLATING_Z|oCAMERA_CONTROLLER_LOCK_POINTER;
	}

	else
	{
		if (WasTumbling || WasTracking || WasDolly)
			Response |= oCAMERA_CONTROLLER_UNLOCK_POINTER;

		if (!oStd::equal(PointerPosition.z, 0.0f))
		{
			float2 delta(PointerPosition.z, 0.0f);
			Arcball.Dolly(delta * 0.05f * Desc.DollySpeed);
			Response |= oCAMERA_CONTROLLER_TRANSLATING_Z;
		}
	}

	return Response;
}

void oCameraControllerModelerImpl::OnLostCapture()
{
	KeyStates.fill(false);
}