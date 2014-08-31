// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#pragma once
#ifndef oGPU_color_target_h
#define oGPU_color_target_h

#include <oGPU/resource.h>
#include <oSurface/image.h>
#include <vector>

namespace ouro { 
	
class window;

	namespace gpu {

static const uint max_num_mrts = 8;

class device;
class command_list;
class depth_target;

class basic_color_target : public resource
{
public:
	basic_color_target() : crw(nullptr) {}

	operator bool() const { return !!ro; }

	uint2 dimensions() const;
	uint array_size() const;

	void* get_target(uint index = 0) const { return rws[index]; }
	inline void* get_compute_target() const { return crw; }

	surface::image make_snapshot(uint index = 0, const allocator& a = default_allocator);
	inline surface::image make_snapshot(const allocator& a) { return make_snapshot(0, a); }
	
	void clear(command_list& cl, const color& c, uint index = 0);

	static void set_draw_target(command_list& cl, uint num_colors, basic_color_target* const* colors, depth_target* depth, const viewport& vp = viewport());
	static void set_draw_target(command_list& cl, uint num_colors, basic_color_target* const* colors, const uint* color_indices, depth_target* depth, uint depth_index, const viewport& vp = viewport());

	void set_draw_target(command_list& cl, uint index = 0, depth_target* depth = nullptr, uint depth_index = 0, const viewport& vp = viewport());
	inline void set_draw_target(command_list& cl, uint index, depth_target& depth, uint depth_index = 0, const viewport& vp = viewport()) { set_draw_target(cl, index, &depth, depth_index, vp); }
	inline void set_draw_target(command_list& cl, depth_target& depth, const viewport& vp = viewport()) { set_draw_target(cl, 0, &depth, 0, vp); }
	inline void set_draw_target(command_list& cl, depth_target* depth, const viewport& vp = viewport()) { set_draw_target(cl, 0, depth, 0, vp); }

protected:
	std::vector<void*> rws;
	void* crw;

	void basic_deinitialize(); // takes care of rws's, crw and resource fields
};

class color_target : public basic_color_target
{
public:
	~color_target() { deinitialize(); }

	void initialize(const char* name, device& dev, surface::format format, uint width, uint height, uint array_size, bool mips);
	void deinitialize();
	
	operator bool() const { return !!ro; }

	void resize(const uint2& dimensions);

	void generate_mips(command_list& cl);

private:
	void internal_initialize(const char* name, void* dev, surface::format format, uint width, uint height, uint array_size, bool mips);
};
	
}}

#endif
