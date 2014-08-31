// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
// Convenience "all headers" header for precompiled header files. Do NOT use 
// this to be lazy when including headers in .cpp files. Be explicit.
#pragma once
#ifndef oGPU_all_h
#define oGPU_all_h
#include <oGPU/blend_state.h>
#include <oGPU/color_target.h>
#include <oGPU/command_list.h>
#include <oGPU/compute_target.h>
#include <oGPU/constant_buffer.h>
#include <oGPU/depth_stencil_state.h>
#include <oGPU/depth_target.h>
#include <oGPU/device.h>
#include <oGPU/index_buffer.h>
#include <oGPU/primary_target.h>
#include <oGPU/rasterizer_state.h>
#include <oGPU/raw_buffer.h>
#include <oGPU/readback_buffer.h>
#include <oGPU/resource.h>
#include <oGPU/rwstructured_buffer.h>
#include <oGPU/sampler_state.h>
#include <oGPU/shader.h>
#include <oGPU/texture1d.h>
#include <oGPU/texture2d.h>
#include <oGPU/texture3d.h>
#include <oGPU/texturecube.h>
#include <oGPU/timer_query.h>
#include <oGPU/vertex_buffer.h>
#include <oGPU/vertex_layout.h>
#endif
