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
#include <oBasis/oCameraControllerMMO.h>
#include <oBasis/oGUI.h>
#include <oBasis/oRefCount.h>
#include <oCompute/eye.h>

struct oCameraControllerMMOImpl : oCameraControllerMMO
{
	oCameraControllerMMOImpl(const oCAMERA_CONTROLLER_MMO_DESC& _Desc, bool* _pSuccess);
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_TRIVIAL_QUERYINTERFACE2(oCameraController, oCameraControllerMMO);

	int OnAction(const ouro::input::action& _Action) override;
	void Tick() override { }
	void OnLostCapture() override;
	void SetView(const float4x4& _View) override { Eye.view(_View); }
	float4x4 GetView(float _DeltaTime = 0.0f) override;
	void SetLookAt(const float3& _LookAt) override { Eye.lookat(_LookAt); }
	float3 GetLookAt() const override { return Eye.lookat(); }
	void SetDesc(const oCAMERA_CONTROLLER_MMO_DESC& _Desc) override { Desc = _Desc; }
	void GetDesc(oCAMERA_CONTROLLER_MMO_DESC* _pDesc) const override { *_pDesc = Desc; }

private:
	oCAMERA_CONTROLLER_MMO_DESC Desc;
	oRefCount RefCount;

	std::array<bool, oCAMERA_CONTROLLER_MMO_DESC::NUM_CONTROLS> KeyStates;
	float3 PointerPosition;
	float3 LastPointerPosition;
	ouro::eye Eye;
	bool WasRotating;

private:
	bool GetControl(ouro::input::key _Key, oCAMERA_CONTROLLER_MMO_DESC::CONTROL* _pControl);
	void UpdateRotation(float _DeltaTime);
	void UpdateTranslation(float _DeltaTime);
};

oCameraControllerMMOImpl::oCameraControllerMMOImpl(const oCAMERA_CONTROLLER_MMO_DESC& _Desc, bool* _pSuccess)
	: Desc(_Desc)
	, PointerPosition(0.0f)
	, LastPointerPosition(0.0f)
	, WasRotating(false)
{
	*_pSuccess = false;
	KeyStates.fill(false);
	*_pSuccess = true;
}

bool oCameraControllerMMOCreate(const oCAMERA_CONTROLLER_MMO_DESC& _Desc, oCameraControllerMMO** _ppCameraController)
{
	bool success = false;
	oCONSTRUCT(_ppCameraController, oCameraControllerMMOImpl(_Desc, &success));
	return success;
}

int oCameraControllerMMOImpl::OnAction(const ouro::input::action& _Action)
{
	oGUIRecordInputState(_Action, Desc.Controls.data(), Desc.Controls.size(), KeyStates.data(), KeyStates.size(), &PointerPosition);
	int Response = 0;

	if (Desc.AllowMouseWheelAcceleration && !ouro::equal(PointerPosition.z, 0.0f))
	{
		if (PointerPosition.z < 0.0f)
		{
			Desc.WalkSpeed = max(Desc.WalkSpeedMin, Desc.WalkSpeed / Desc.Acceleration);
			Desc.RunSpeed = max(Desc.RunSpeedMin, Desc.RunSpeed / Desc.Acceleration);
		}

		else
		{
			Desc.WalkSpeed = min(Desc.WalkSpeedMax, Desc.WalkSpeed * Desc.Acceleration);
			Desc.RunSpeed = min(Desc.RunSpeedMax, Desc.RunSpeed * Desc.Acceleration);
		}
	}

	// Determine rotation response
	if ((KeyStates[oCAMERA_CONTROLLER_MMO_DESC::ROTATE_VIEW] && (!ouro::equal(PointerPosition, LastPointerPosition) || WasRotating))
		|| (KeyStates[oCAMERA_CONTROLLER_MMO_DESC::ROTATE_VIEW] && KeyStates[oCAMERA_CONTROLLER_MMO_DESC::ROTATE_VIEW_AND_FORWARD]))
		Response |= oCAMERA_CONTROLLER_ROTATING_YAW|oCAMERA_CONTROLLER_ROTATING_PITCH;
	if (KeyStates[oCAMERA_CONTROLLER_MMO_DESC::ROLL_LEFT] || KeyStates[oCAMERA_CONTROLLER_MMO_DESC::ROLL_LEFT])
		Response |= oCAMERA_CONTROLLER_ROTATING_ROLL;
	// MMO/FPS controls are not around a center of interest/lookat, so no flag there

	// Determine translation response
	if (KeyStates[oCAMERA_CONTROLLER_MMO_DESC::STRAFE_RIGHT] || KeyStates[oCAMERA_CONTROLLER_MMO_DESC::STRAFE_LEFT])
		Response |= oCAMERA_CONTROLLER_TRANSLATING_X;
	if (KeyStates[oCAMERA_CONTROLLER_MMO_DESC::SWIM_UP] || KeyStates[oCAMERA_CONTROLLER_MMO_DESC::SWIM_UP])
		Response |= oCAMERA_CONTROLLER_TRANSLATING_Y;
	if (KeyStates[oCAMERA_CONTROLLER_MMO_DESC::FORWARD] || KeyStates[oCAMERA_CONTROLLER_MMO_DESC::BACKWARD] || (KeyStates[oCAMERA_CONTROLLER_MMO_DESC::ROTATE_VIEW] && KeyStates[oCAMERA_CONTROLLER_MMO_DESC::ROTATE_VIEW_AND_FORWARD]))
		Response |= oCAMERA_CONTROLLER_TRANSLATING_Z;

	bool IsRotating = !!(Response & oCAMERA_CONTROLLER_ROTATING);

	if (!WasRotating && IsRotating)
		Response |= oCAMERA_CONTROLLER_HIDE_POINTER|oCAMERA_CONTROLLER_LOCK_POINTER|oCAMERA_CONTROLLER_SAVE_POINTER_POSITION;
	else if (WasRotating && !IsRotating)
		Response |= oCAMERA_CONTROLLER_SHOW_POINTER|oCAMERA_CONTROLLER_UNLOCK_POINTER|oCAMERA_CONTROLLER_LOAD_POINTER_POSITION;

	WasRotating = IsRotating;
	return Response;
}

void oCameraControllerMMOImpl::OnLostCapture()
{
	KeyStates.fill(false);
}

void oCameraControllerMMOImpl::UpdateRotation(float _DeltaTime)
{
	const float3 ScaledRotationSpeed = Desc.RotationSpeed * _DeltaTime;
	float3 RotationDelta(0.0f);

	// Update the rotation state of the look-at
	if (KeyStates[oCAMERA_CONTROLLER_MMO_DESC::ROTATE_VIEW])
	{
		float2 delta = PointerPosition.xy() - LastPointerPosition.xy();
		RotationDelta = float3(delta.y, delta.x, 0.0f) * ScaledRotationSpeed;
	}
	LastPointerPosition = PointerPosition;

	if (KeyStates[oCAMERA_CONTROLLER_MMO_DESC::ROLL_LEFT])
		RotationDelta.z += ScaledRotationSpeed.z;

	else if (KeyStates[oCAMERA_CONTROLLER_MMO_DESC::ROLL_RIGHT])
		RotationDelta.z -= ScaledRotationSpeed.z;

	Eye.rotate(RotationDelta);
}

void oCameraControllerMMOImpl::UpdateTranslation(float _DeltaTime)
{
	const float ScaledTranslationSpeed = (KeyStates[oCAMERA_CONTROLLER_MMO_DESC::RUN] ? Desc.RunSpeed : Desc.WalkSpeed) * _DeltaTime;

	// Update the direction and magnitude of movement based on translation input
	float3 Translation(0.0f);
	for (int i = oCAMERA_CONTROLLER_MMO_DESC::TRANSLATION_FIRST; i <= oCAMERA_CONTROLLER_MMO_DESC::TRANSLATION_LAST; i++)
		if (KeyStates[i])
			Translation[i / 2] += (i % 2) ? -1.0f : 1.0f; // += makes opposite directions cancel each other out.

	if (KeyStates[oCAMERA_CONTROLLER_MMO_DESC::ROTATE_VIEW] && KeyStates[oCAMERA_CONTROLLER_MMO_DESC::ROTATE_VIEW_AND_FORWARD] && Translation.z <= 0.0f)
		Translation.z += 1.0f;

	if (!ouro::equal(Translation, float3(0.0f)))
	{
		Translation = normalize(Translation) * ScaledTranslationSpeed;
		Eye.translate(Translation);
	}
}

float4x4 oCameraControllerMMOImpl::GetView(float _DeltaTime)
{
	if (!ouro::equal(0.0f, _DeltaTime))
	{
		UpdateRotation(_DeltaTime);
		UpdateTranslation(_DeltaTime);
	}
	return Eye.view();
}
