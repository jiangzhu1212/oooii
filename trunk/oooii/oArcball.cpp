/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2011 Antony Arciuolo & Kevin Myers                       *
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
#include <oooii/oArcball.h>
#include <oooii/oRefCount.h>

struct oArcballImpl : public oArcball
{
	oDEFINE_REFCOUNT_INTERFACE(Refcount);
	oDEFINE_CONST_GETDESC_INTERFACE(Desc,);
	oDEFINE_NOOP_QUERYINTERFACE();

	oArcballImpl(const DESC& _Desc, bool* _pSuccess);

	virtual void SetDesc(const DESC* _pDesc) override;

	void Reset(float3 _Eye, float3 _LookAt, float3 _Up) override;
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

bool oArcball::Create(const DESC& _Desc, oArcball** _ppArcball)
{
	bool success = false;
	oCONSTRUCT( _ppArcball, oArcballImpl( _Desc, &success) );
	return success;
}

oArcballImpl::oArcballImpl(const DESC& _Desc, bool* _pSuccess)
	: Desc(_Desc)
{
	*_pSuccess = true;
	Rotation = BeginRotation = quatf::Identity;
}

void oArcballImpl::SetDesc( const DESC* _pDesc )
{
	Desc = *_pDesc;
}

void oArcballImpl::Reset(float3 _Eye, float3 _LookAt, float3 _Up)
{
	LookAt = _LookAt;

	float4x4 v = oCreateLookAtLH(_Eye, _LookAt, _Up) * oCreateTranslation(LookAt);
	Translation = -v.Column3.XYZ();
	Rotation = BeginRotation = oCreateRotationQ( v );
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
			if (_ScreenPosition.x > 0.0f && _ScreenPosition.y > 0.0f && _ScreenPosition.x < Desc.WindowDimensions.x && _ScreenPosition.y < Desc.WindowDimensions.y )
			{
				BeginRotation = Rotation;
				BeginTranslation = oScreenToVector(_ScreenPosition.XY(), Desc.WindowDimensions, Desc.Radius);
			}
			break;

		case ROTATING:
			Rotation = oCreateRotationQ(BeginTranslation, oScreenToVector(_ScreenPosition.XY(), Desc.WindowDimensions, Desc.Radius) ) * BeginRotation;
			break;

		case TRANSLATING:
			 Translation += float3((LastScreenPosition - _ScreenPosition.XY()) * float2(Desc.TranslationScale.x, -Desc.TranslationScale.y) / Desc.WindowDimensions, 0.0f);
			// pass thru

		case BEGIN_TRANSLATING:
			LastScreenPosition = _ScreenPosition.XY();
			break;

		default:
			break;
	}
}


