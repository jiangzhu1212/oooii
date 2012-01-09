// $(header)
#include <oBasis/oArcball.h>
#include <oBasis/oMath.h>
#include <oBasis/oRefCount.h>

struct oArcballImpl : public oArcball
{
	oDEFINE_REFCOUNT_INTERFACE(Refcount);
	oDEFINE_CONST_GETDESC_INTERFACE(Desc,);
	oDEFINE_NOOP_QUERYINTERFACE();
	oArcballImpl(const DESC& _Desc, bool* _pSuccess);
	void SetDesc(const DESC& _Desc) override;
	void Reset(const float3& _Eye, const float3& _LookAt, const float3& _Up) override;
	void Reset() override;
	void ToView(float4x4* _pView) const override;
	void Trigger(EVENT _Event, const float3& _ScreenPosition) override;

private:
	DESC Desc;
	quatf Rotation;
	quatf BeginRotation;
	float3 Translation;
	float3 BeginTranslation;
	float3 LookAt;
	float2 LastScreenPosition;
	oRefCount Refcount;
};

bool oArcballCreate(const oArcball::DESC& _Desc, oArcball** _ppArcball)
{
	bool success = false;
	oCONSTRUCT(_ppArcball, oArcballImpl(_Desc, &success));
	return success;
}

oArcballImpl::oArcballImpl(const DESC& _Desc, bool* _pSuccess)
	: Desc(_Desc)
{
	*_pSuccess = true;
	Rotation = BeginRotation = quatf::Identity;
}

void oArcballImpl::SetDesc(const DESC& _Desc)
{
	Desc = _Desc;
}

void oArcballImpl::Reset(const float3& _Eye, const float3& _LookAt, const float3& _Up)
{
	LookAt = _LookAt;
	float4x4 v = oCreateLookAtLH(_Eye, _LookAt, _Up) * oCreateTranslation(LookAt);
	Translation = -v.Column3.xyz();
	Rotation = BeginRotation = oCreateRotationQ(v);
}

void oArcballImpl::Reset()
{
	Translation = 0.0f;
	LookAt = 0.0f;
	Rotation = BeginRotation = quatf::Identity;
}

void oArcballImpl::ToView(float4x4* _pView) const
{
	*_pView = float4x4(Rotation, -Translation) * oCreateTranslation(-LookAt);
}

void oArcballImpl::Trigger(EVENT _Event, const float3& _ScreenPosition)
{
	const float DAMPEN_SCALE = 0.001f; // otherwise the user must be careful to treat Z scaling very differently from X and Y scaling

	Translation.z += _ScreenPosition.z * Desc.TranslationScale.z * DAMPEN_SCALE;

	switch (_Event)
	{
		case BEGIN_ROTATING:
			if (greater_than(_ScreenPosition.xy(), float2(0.0f, 0.0f)) && less_than(_ScreenPosition.xy(), Desc.WindowDimensions))
			{
				BeginRotation = Rotation;
				BeginTranslation = oScreenToVector(_ScreenPosition.xy(), Desc.WindowDimensions, Desc.Radius);
			}
			break;

		case ROTATING:
			Rotation = oCreateRotationQ(BeginTranslation, oScreenToVector(_ScreenPosition.xy(), Desc.WindowDimensions, Desc.Radius)) * BeginRotation;
			break;

		case TRANSLATING:
			 Translation += float3((LastScreenPosition - _ScreenPosition.xy()) * float2(Desc.TranslationScale.x, -Desc.TranslationScale.y) / Desc.WindowDimensions, 0.0f);
			// pass thru

		case BEGIN_TRANSLATING:
			LastScreenPosition = _ScreenPosition.xy();
			break;

		default:
			break;
	}
}
