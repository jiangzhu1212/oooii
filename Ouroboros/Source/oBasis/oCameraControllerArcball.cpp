// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#include <oBasis/oCameraControllerArcball.h>
#include <oBasis/oRefCount.h>
#include <oCompute/arcball.h>

struct oCameraControllerArcballImpl : oCameraControllerArcball
{
	oCameraControllerArcballImpl(const oCAMERA_CONTROLLER_ARCBALL_DESC& _Desc, bool* _pSuccess);
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_TRIVIAL_QUERYINTERFACE2(oCameraController, oCameraControllerArcball);

	int OnAction(const ouro::input::action& _Action) override;
	void Tick() override {} 
	void OnLostCapture() override;
	void SetView(const float4x4& _View) override { Arcball.view(_View); }
	float4x4 GetView(float _DeltaTime = 0.0f) override { return Arcball.view(); }
	void SetLookAt(const float3& _LookAt) override { Arcball.lookat(_LookAt); }
	float3 GetLookAt() const override { return Arcball.lookat(); }
	void SetDesc(const oCAMERA_CONTROLLER_ARCBALL_DESC& _Desc) override { Desc = _Desc; }
	void GetDesc(oCAMERA_CONTROLLER_ARCBALL_DESC* _pDesc) const override { *_pDesc = Desc; }

private:
	oCAMERA_CONTROLLER_ARCBALL_DESC Desc;
	oRefCount RefCount;
	std::array<bool, oCAMERA_CONTROLLER_ARCBALL_DESC::NUM_CONTROLS> KeyStates;
	float3 PointerPosition;
	// keep this as a float2... it ensures we ignore the mouse wheel in 
	// transformation calculations because that's already relative. For mouse 
	// wheel support - use PointerPosition.z as its own delta.
	float2 LastPointerPosition;
	ouro::arcball Arcball;
};

oCameraControllerArcballImpl::oCameraControllerArcballImpl(const oCAMERA_CONTROLLER_ARCBALL_DESC& _Desc, bool* _pSuccess)
	: Desc(_Desc)
	, PointerPosition(0.0f)
	, LastPointerPosition(0.0f)
	, Arcball(ouro::arcball::none)
{
	*_pSuccess = false;
	KeyStates.fill(false);
	*_pSuccess = true;
}

bool oCameraControllerArcballCreate(const oCAMERA_CONTROLLER_ARCBALL_DESC& _Desc, oCameraControllerArcball** _ppCameraController)
{
	bool success = false;
	oCONSTRUCT(_ppCameraController, oCameraControllerArcballImpl(_Desc, &success));
	return success;
}

int oCameraControllerArcballImpl::OnAction(const ouro::input::action& _Action)
{
	bool WasOrbiting = KeyStates[oCAMERA_CONTROLLER_ARCBALL_DESC::ORBIT];
	bool WasPanning = !WasOrbiting && KeyStates[oCAMERA_CONTROLLER_ARCBALL_DESC::PAN];
	bool WasDolly = !WasPanning && KeyStates[oCAMERA_CONTROLLER_ARCBALL_DESC::DOLLY];

	int Response = oCAMERA_CONTROLLER_SHOW_POINTER;

	LastPointerPosition = PointerPosition.xy();
	record_state(_Action, Desc.Controls.data(), Desc.Controls.size(), KeyStates.data(), KeyStates.size(), &PointerPosition);
	float2 PointerPositionDelta = PointerPosition.xy() - LastPointerPosition;

	if (KeyStates[oCAMERA_CONTROLLER_ARCBALL_DESC::ORBIT])
	{
		if (!WasOrbiting)
			Arcball.begin_rotation();

		Arcball.rotate(PointerPositionDelta * Desc.RotationSpeed);
		Response |= oCAMERA_CONTROLLER_ROTATING_YAW|oCAMERA_CONTROLLER_ROTATING_PITCH|oCAMERA_CONTROLLER_ROTATING_AROUND_COI|oCAMERA_CONTROLLER_LOCK_POINTER;
	}

	else if (KeyStates[oCAMERA_CONTROLLER_ARCBALL_DESC::PAN])
	{
		Arcball.pan(PointerPositionDelta * Desc.PanSpeed);
		Response |= oCAMERA_CONTROLLER_TRANSLATING_X|oCAMERA_CONTROLLER_TRANSLATING_Y|oCAMERA_CONTROLLER_LOCK_POINTER;
	}

	else if (KeyStates[oCAMERA_CONTROLLER_ARCBALL_DESC::DOLLY])
	{
		Arcball.dolly(PointerPositionDelta * Desc.DollySpeed);
		Response |= oCAMERA_CONTROLLER_TRANSLATING_Z|oCAMERA_CONTROLLER_LOCK_POINTER;
	}

	else
	{
		if (WasOrbiting || WasPanning || WasDolly)
			Response |= oCAMERA_CONTROLLER_UNLOCK_POINTER|oCAMERA_CONTROLLER_SHOW_POINTER;

		if (!ouro::equal(PointerPosition.z, 0.0f))
		{
			float2 delta(PointerPosition.z, 0.0f);
			Arcball.dolly(delta * 0.05f * Desc.DollySpeed);
			Response |= oCAMERA_CONTROLLER_TRANSLATING_Z;
		}
	}

	return Response;
}

void oCameraControllerArcballImpl::OnLostCapture()
{
	KeyStates.fill(false);
}

