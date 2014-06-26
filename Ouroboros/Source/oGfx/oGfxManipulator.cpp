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
#include <oGfx/oGfxManipulator.h>

#if 0

#include <oGPU/oGPUUtil.h>
#include <oGPU/vertex_layouts.h>
#include <oGfx/oGfxPipelines.h>

using namespace ouro;
using namespace ouro::gpu;
using namespace ouro::mesh;

struct LineListContext
{
	std::shared_ptr<buffer> LineList;
	std::shared_ptr<util_mesh> CapMesh;
	std::shared_ptr<util_mesh> PickMesh;
	oURI URI;
	color LineColor;
};

struct oGfxManipulatorImpl : public oGfxManipulator
{
	enum EVENT
	{
		BEGIN_MANIPULATE,
		ROTATING,
		END_MANIPULATE,
		TRANSLATING,
	};


	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_NOOP_QUERYINTERFACE();
	void GetDesc(DESC* _pDesc) const override {Manipulator->GetDesc(_pDesc);}

	bool Pick(uint _ObjectID) override;
	void StopPick() override;

	void SetMousePosition(float2 _position) override;
	void SetViewProjection(float4x4 _ViewMatrixLH, float4x4 _ProjectionMatrixLH) override;
	void GetTransform(float4x4 *_pTransform) const override;
	void SetTransform(const float4x4& _Transform) override;
	
	void GetManipulatorPickLineMeshes(command_list* _pCommandList, std::function<void(util_mesh* _pMesh, uint _ObjectID)> _Callback) override;
	void GetManipulatorVisualLines(command_list* _pCommandList, std::function<void(buffer* _pLineList, uint _NumLines)> _Callback) override;
	void GetManipulatorMeshes(command_list* _pCommandList, std::function<void(util_mesh* _pMesh, float4x4 _Transform, color _MeshColor, uint _ObjectID)> _Callback) override;

	oGfxManipulatorImpl(const char* _Name, const oGfxManipulator::DESC& _Desc, device* _pDevice, bool* _pSuccess);

	template<typename GEOMETRY_T>
	std::shared_ptr<util_mesh> CreateGeometryMesh(device* _pDevice, const char* _Name, const GEOMETRY_T& _GeometryDesc);

	template<typename PICK_GEOMETRY_T>
	bool CreateAxisGeometry(device* _pDevice, const char* _BaseName, const PICK_GEOMETRY_T& _PickGeometryDesc, unsigned int _NumLines, oManipulator::AXIS _Axis);
	
	template<typename PICK_GEOMETRY_T, typename CONE_GEOMETRY_T>
	bool CreateAxisGeometry(device* _pDevice, const char* _BaseName, const PICK_GEOMETRY_T& _PickGeometryDesc, const CONE_GEOMETRY_T& _ConeGeometryDesc, unsigned int _NumLines, oManipulator::AXIS _Axis);

	void Update();

	oGfxManipulator::DESC Desc;
	oRefCount RefCount;
	intrusive_ptr<oManipulator> Manipulator;
	intrusive_ptr<oGeometryFactory> GeometryFactory;

	float4x4 Transform;
	float2 CurrentMousePosition;
	float4x4 CurrentViewLH;
	float4x4 CurrentProjectionLH;

	std::vector<float3> VerticesWorking; //just a temp list to use for whatever. member variable to cut down on allocs
	LineListContext Lines[oManipulator::NUM_AXES];
	oManipulator::AXIS PickedAxis;
};


template<typename GEOMETRY_T>
std::shared_ptr<util_mesh> oGfxManipulatorImpl::CreateGeometryMesh(device* _pDevice, const char* _Name, const GEOMETRY_T& _GeometryDesc)
{
	ouro::mesh::layout::value GeoLayout = ouro::mesh::layout::pos;

	intrusive_ptr<oGeometry> Geometry; 
	if(!GeometryFactory->Create(_GeometryDesc, GeoLayout, &Geometry))
		return false;

	layout_array layouts;
	layouts[0] = GeoLayout;
	return util_mesh::make(_pDevice, _Name, layouts, Geometry);
}

template<typename PICK_GEOMETRY_T>
bool oGfxManipulatorImpl::CreateAxisGeometry(device* _pDevice, const char* _BaseName, const PICK_GEOMETRY_T& _GeometryDesc, unsigned int _NumLines, oManipulator::AXIS _Axis)
{
	auto& Line = Lines[_Axis];
	
	uri_string Name;
	snprintf(Name, "%s_%s", _BaseName, ouro::as_string(_Axis));
	Line.LineList = _pDevice->make_vertex_buffer<vertex_pos_color>(Name, _NumLines * 2);
	Line.PickMesh = CreateGeometryMesh(_pDevice, Name, _GeometryDesc);

	Line.URI = Name;
	return true;
}


template<typename PICK_GEOMETRY_T, typename CONE_GEOMETRY_T>
bool oGfxManipulatorImpl::CreateAxisGeometry( device* _pDevice, const char* _BaseName, const PICK_GEOMETRY_T& _PickGeometryDesc, const CONE_GEOMETRY_T& _ConeGeometryDesc, unsigned int _NumLines, oManipulator::AXIS _Axis )
{
	if(!CreateAxisGeometry(_pDevice, _BaseName, _PickGeometryDesc, _NumLines, _Axis))
		return false;

	auto& Line = Lines[_Axis];
	Line.CapMesh = CreateGeometryMesh(_pDevice, _BaseName, _ConeGeometryDesc);

	return true;
}


oGfxManipulatorImpl::oGfxManipulatorImpl(const char* _Name, const oGfxManipulator::DESC& _Desc, device* _pDevice, bool* _pSuccess) 
	: Desc(_Desc)
	, Transform(oIDENTITY4x4)
	, PickedAxis((oManipulator::AXIS)(size_t)ouro::invalid)
{
	if(!oManipulatorCreate(_Desc, &Manipulator))
	{
		oErrorPrefixLast("oGfxManipulator failed to create a manipulator: ");
		return;
	}

	if(!oGeometryFactoryCreate(&GeometryFactory))
	{
		oErrorPrefixLast("oGfxManipulator failed to create a geometry factory: ");
		return;
	}

	switch(Desc.Type)
	{
	case oManipulator::DESC::TRANSLATION:
		{
			oGeometryFactory::RECT_DESC pickd;
			pickd.FaceType = face_type::front_cw;
			pickd.Divide = 0;
			pickd.Height = 1;
			pickd.Width = 1;

			oGeometryFactory::CONE_DESC capd;
			capd.FaceType = face_type::front_cw;
			capd.Divide = 4;
			capd.Facet = 31;
			capd.Radius = 0.4f;
			capd.Height = 1.2f;
			capd.IncludeBase = false;

			if(!CreateAxisGeometry(_pDevice, _Name, pickd, capd, 1, oManipulator::X))
				return;
			if(!CreateAxisGeometry(_pDevice, _Name, pickd, capd, 1, oManipulator::Y))
				return;
			if(!CreateAxisGeometry(_pDevice, _Name, pickd, capd, 1, oManipulator::Z))
				return;
			if(!CreateAxisGeometry(_pDevice, _Name, pickd, 4, oManipulator::SCREEN))
				return;
		}
		break;
	case oManipulator::DESC::SCALE:
		{
			oGeometryFactory::RECT_DESC pickd;
			pickd.FaceType = face_type::front_cw;
			pickd.Divide = 0;
			pickd.Height = 1;
			pickd.Width = 1;

			const float3 BoxSize = float3(0.4f, 0.4f, 0.4f);
			oGeometryFactory::BOX_DESC boxd;
			boxd.FaceType = face_type::front_cw;
			boxd.Divide = 4;
			boxd.Bounds = aaboxf(aaboxf::min_max, -BoxSize, BoxSize);

			if(!CreateAxisGeometry(_pDevice, _Name, pickd, boxd, 1, oManipulator::X))
				return;
			if(!CreateAxisGeometry(_pDevice, _Name, pickd, boxd, 1, oManipulator::Y))
				return;
			if(!CreateAxisGeometry(_pDevice, _Name, pickd, boxd, 1, oManipulator::Z))
				return;

			// Special case for screen
			{
				boxd.Bounds = aaboxf(aaboxf::min_max, -5.0f * BoxSize, 5.0f * BoxSize);
				Lines[oManipulator::SCREEN].CapMesh = CreateGeometryMesh(_pDevice, _Name, boxd);

				uri_string AxisName;
				snprintf(AxisName, "%s_%s", _Name, ouro::as_string(oManipulator::SCREEN));
				Lines[oManipulator::SCREEN].URI = AxisName;
			}

		}
		break;
	case oManipulator::DESC::ROTATION:
		{
			oGeometryFactory::TORUS_DESC pickd;
			pickd.FaceType = face_type::front_cw;
			pickd.InnerRadius = 5;
			pickd.OuterRadius = 7;
			pickd.Divide = oManipulator::ROTATION_PICK_TORUS_DIVIDE;
			pickd.Facet = oManipulator::ROTATION_PICK_TORUS_FACET;
			pickd.Color = white;

			if(!CreateAxisGeometry(_pDevice, _Name, pickd, oManipulator::ROTATION_CIRCLE_VCOUNT*2, oManipulator::X))
				return;
			if(!CreateAxisGeometry(_pDevice, _Name, pickd, oManipulator::ROTATION_CIRCLE_VCOUNT*2, oManipulator::Y))
				return;
			if(!CreateAxisGeometry(_pDevice, _Name, pickd, oManipulator::ROTATION_CIRCLE_VCOUNT*2, oManipulator::Z))
				return;
			if(!CreateAxisGeometry(_pDevice, _Name, pickd, oManipulator::ROTATION_CIRCLE_VCOUNT*2, oManipulator::SCREEN))
				return;

		}
		break;
		oNODEFAULT;
	}

	// This ensures nothing is picked
	Pick(0); 
	*_pSuccess = true;
}

bool oGfxManipulatorImpl::Pick(uint _ObjectID)
{
	static const color unpickedColors[oManipulator::NUM_AXES] = {red, green, blue, gray};
	for(size_t i = 0; i < oManipulator::NUM_AXES; ++i)
		Lines[i].LineColor = unpickedColors[i];

	if(0 != _ObjectID)
	{
		for(size_t i = 0; i < oManipulator::NUM_AXES; ++i)
		{
			oManipulator::AXIS Axis = (oManipulator::AXIS)i;
			auto& Line = Lines[Axis];
			if(_ObjectID == (uint)Line.URI.Hash())
			{
				Line.LineColor = yellow;
				if(PickedAxis != Axis)
				{
					PickedAxis = Axis;
					Manipulator->BeginManipulation(Axis, CurrentMousePosition);
				}
				return true;
			}
		}
	}
	return false;
}

void oGfxManipulatorImpl::StopPick()
{
	Manipulator->GetTransform(&Transform);
	Manipulator->EndManipulation();
	PickedAxis = (oManipulator::AXIS)(size_t)ouro::invalid;
	Update();
}

void oGfxManipulatorImpl::SetMousePosition(float2 _position)
{
	CurrentMousePosition = _position;
	Update();
}

void oGfxManipulatorImpl::SetTransform(const float4x4& _Transform)
{
	Transform = _Transform;
	Update();
}

void oGfxManipulatorImpl::GetTransform(float4x4 *_pTransform) const
{ 
	Manipulator->GetTransform(_pTransform);
}

void oGfxManipulatorImpl::SetViewProjection(float4x4 _ViewMatrixLH, float4x4 _ProjectionMatrixLH)
{
	CurrentViewLH = _ViewMatrixLH;
	CurrentProjectionLH = _ProjectionMatrixLH;
	Update();
}

void oGfxManipulatorImpl::Update()
{
	Manipulator->Update(CurrentMousePosition, Transform, CurrentViewLH, CurrentProjectionLH);
}

void oGfxManipulatorImpl::GetManipulatorPickLineMeshes(command_list* _pCommandList, std::function<void(util_mesh* _pMesh, uint _ObjectID)> _Callback)
{
	size_t MaxNumLines, MaxNumPickVerts;
	Manipulator->GetMaxSizes(MaxNumLines, MaxNumPickVerts);
	VerticesWorking.resize(MaxNumPickVerts);

	for(size_t i = 0; i < oManipulator::NUM_AXES; ++i)
	{
		oManipulator::AXIS Axis = (oManipulator::AXIS)i;
		auto& Line = Lines[Axis];
		auto& Mesh = Line.PickMesh;
		if(Mesh && !Manipulator->IsClipped(Axis))
		{
			size_t PickVertexCount;
			Manipulator->GetLinesPickGeometry(Axis, &VerticesWorking[0], PickVertexCount);
			if(PickVertexCount > 0)
			{
				mesh::info MeshDesc = Mesh->get_info();

				ouro::surface::const_mapped_subresource msr;
				msr.data = &VerticesWorking[0];
				msr.row_pitch = sizeof(VerticesWorking[0]);
				msr.depth_pitch = MeshDesc.num_vertices * msr.row_pitch;
				_pCommandList->commit(Mesh->vertex_buffer(0), 0, msr);
				_Callback(Mesh.get(), (uint)Line.URI.Hash());
			}
		}
	}

}

void oGfxManipulatorImpl::GetManipulatorVisualLines(command_list* _pCommandList, std::function<void(buffer* _pLineList, uint _NumLines)> _Callback)
{
	size_t MaxNumLines, MaxNumPickVerts;
	Manipulator->GetMaxSizes(MaxNumLines, MaxNumPickVerts);
	VerticesWorking.resize(MaxNumLines);

	for(size_t i = 0; i < oManipulator::NUM_AXES; ++i)
	{
		oManipulator::AXIS Axis = (oManipulator::AXIS)i;
		auto& Line = Lines[Axis];
		if(Line.LineList && !Manipulator->IsClipped(Axis))
		{
			size_t LineVertexCount;
			Manipulator->GetLines(Axis, &VerticesWorking[0], LineVertexCount);
			if(LineVertexCount > 0)
			{
				ouro::surface::mapped_subresource msr = _pCommandList->reserve(Line.LineList, 0);

				auto LineCount = LineVertexCount == 2 ? 1 : LineVertexCount;

				for(size_t i = 0; i < LineCount; ++i)
				{
					((vertex_pos_color*)msr.data)[2 * i].position = VerticesWorking[i];
					((vertex_pos_color*)msr.data)[2 * i].color = Line.LineColor;

					((vertex_pos_color*)msr.data)[2 * i + 1].position = VerticesWorking[(i + 1)%LineVertexCount];
					((vertex_pos_color*)msr.data)[2 * i + 1].color = Line.LineColor;
				}

				_pCommandList->commit(Line.LineList, 0, msr, ouro::surface::box(as_int(LineCount * 2)));
				_Callback(Line.LineList.get(), as_uint(LineCount));
			}
		}
	}
}

void oGfxManipulatorImpl::GetManipulatorMeshes(command_list* _pCommandList, std::function<void(util_mesh* _pMesh, float4x4 _Transform, color _MeshColor, uint _ObjectID)> _Callback)
{
	for(size_t i = 0; i < oManipulator::NUM_AXES; ++i)
	{
		oManipulator::AXIS Axis = (oManipulator::AXIS)i;
		auto& Line = Lines[Axis];
		auto& Cap = Line.CapMesh;
		if(Cap)
		{
			float4x4 CapTransform;
			Manipulator->GetCapTransform(Axis, CapTransform);
			_Callback(Cap.get(), CapTransform, Line.LineColor, (uint)Line.URI.Hash());
		}
	}
}

bool oGfxManipulatorCreate(const char* _Name, const oGfxManipulator::DESC& _Desc, device* _pDevice, oGfxManipulator** _ppManipulator)
{
	bool success = false;
	oCONSTRUCT(_ppManipulator, oGfxManipulatorImpl(_Name, _Desc, _pDevice, &success));
	return success;
}
#endif