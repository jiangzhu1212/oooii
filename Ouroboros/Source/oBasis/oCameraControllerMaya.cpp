// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#include <oBasis/oCameraControllerMaya.h>
#include <oCompute/arcball.h>
#include <oBasis/oRefCount.h>

struct oCameraControllerModelerImpl : oCameraControllerModeler
{
	oCameraControllerModelerImpl(const oCAMERA_CONTROLLER_MODELER_DESC& _Desc, bool* _pSuccess);
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_TRIVIAL_QUERYINTERFACE2(oCameraController, oCameraControllerModeler);

	int OnAction(const ouro::input::action& _Action) override;
	void Tick() override {}
	void OnLostCapture() override;
	void SetView(const float4x4& _View) override { Arcball.view(_View); }
	float4x4 GetView(float _DeltaTime = 0.0f) override { return Arcball.view(); }
	void SetLookAt(const float3& _LookAt) override { Arcball.lookat(_LookAt); }
	float3 GetLookAt() const override { return Arcball.lookat(); }
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
	ouro::arcball Arcball;
};

oCameraControllerModelerImpl::oCameraControllerModelerImpl(const oCAMERA_CONTROLLER_MODELER_DESC& _Desc, bool* _pSuccess)
	: Desc(_Desc)
	, PointerPosition(0.0f)
	, LastPointerPosition(0.0f)
	, Arcball(ouro::arcball::y_up)
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

int oCameraControllerModelerImpl::OnAction(const ouro::input::action& _Action)
{
	bool WasTumbling = KeyStates[oCAMERA_CONTROLLER_MODELER_DESC::TUMBLER];
	bool WasTracking = !WasTumbling && KeyStates[oCAMERA_CONTROLLER_MODELER_DESC::TRACK];
	bool WasDolly = !WasTracking && KeyStates[oCAMERA_CONTROLLER_MODELER_DESC::DOLLY];

	Arcball.constraint(Desc.Constraint);

	// Always show pointer
	int Response = oCAMERA_CONTROLLER_SHOW_POINTER;

	LastPointerPosition = PointerPosition.xy();
	record_state(_Action, Desc.Controls.data(), Desc.Controls.size(), KeyStates.data(), KeyStates.size(), &PointerPosition);
	float2 PointerPositionDelta = PointerPosition.xy() - LastPointerPosition;

	const bool kActive = Desc.Controls[oCAMERA_CONTROLLER_MODELER_DESC::ACTIVATION] == ouro::input::none || KeyStates[oCAMERA_CONTROLLER_MODELER_DESC::ACTIVATION];

	if (kActive && KeyStates[oCAMERA_CONTROLLER_MODELER_DESC::TUMBLER])
	{
		if (!WasTumbling)
			Arcball.begin_rotation();

		Arcball.rotate(PointerPositionDelta * Desc.RotationSpeed);
		Response |= oCAMERA_CONTROLLER_ROTATING_YAW|oCAMERA_CONTROLLER_ROTATING_PITCH|oCAMERA_CONTROLLER_ROTATING_AROUND_COI|oCAMERA_CONTROLLER_LOCK_POINTER;
	}

	else if (kActive && KeyStates[oCAMERA_CONTROLLER_MODELER_DESC::TRACK])
	{
		Arcball.pan(PointerPositionDelta * Desc.PanSpeed);
		Response |= oCAMERA_CONTROLLER_TRANSLATING_X|oCAMERA_CONTROLLER_TRANSLATING_Y|oCAMERA_CONTROLLER_LOCK_POINTER;
	}

	else if (kActive && KeyStates[oCAMERA_CONTROLLER_MODELER_DESC::DOLLY])
	{
		Arcball.dolly(PointerPositionDelta * Desc.DollySpeed);
		Response |= oCAMERA_CONTROLLER_TRANSLATING_Z|oCAMERA_CONTROLLER_LOCK_POINTER;
	}

	else
	{
		if (WasTumbling || WasTracking || WasDolly)
			Response |= oCAMERA_CONTROLLER_UNLOCK_POINTER;

		if (!ouro::equal(PointerPosition.z, 0.0f))
		{
			float2 delta(PointerPosition.z, 0.0f);
			Arcball.dolly(delta * 0.05f * Desc.DollySpeed);
			Response |= oCAMERA_CONTROLLER_TRANSLATING_Z;
		}
	}

	return Response;
}

void oCameraControllerModelerImpl::OnLostCapture()
{
	KeyStates.fill(false);
}