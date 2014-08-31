// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#pragma once
#ifndef oGPU_vertex_layout_h
#define oGPU_vertex_layout_h

#include <oMesh/mesh.h>

namespace ouro {
	namespace gpu {

static const uint max_vertex_elements = 16;

class device;
class command_list;

class vertex_layout
{
public:
	vertex_layout() : layout(nullptr) {}
	vertex_layout(const char* name, device& dev, const mesh::element_array& elements, const void* vs_bytecode) : layout(nullptr) { initialize(name, dev, elements, vs_bytecode); }
	~vertex_layout() { deinitialize(); }
	void initialize(const char* name, device& dev, const mesh::element_array& elements, const void* vs_bytecode);
	void deinitialize();
	void set(command_list& cl, const mesh::primitive_type& prim_type) const;
	void* get() const { return layout; }
private:
	void* layout;
};

}}

#endif
