/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2014 Antony Arciuolo.                                    *
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
// This provides an API similar to that for touch-based devices to abstract 
// skeleton-tracking input devices such as Kinect.
#pragma once
#ifndef oCore_win_skeleton_h
#define oCore_win_skeleton_h

#include <oBase/macros.h>
#include <array>
#include <functional>

namespace ouro {
	namespace windows {
		namespace skeleton {

// Handle used in window messages to identify a particular skeleton
typedef void* handle;

struct tracking_clipping
{
	tracking_clipping()
		: left(false)
		, right(false)
		, top(false)
		, bottom(false)
		, front(false)
		, back(false)
	{}

	bool left : 1;
	bool right : 1;
	bool top : 1;
	bool bottom : 1;
	bool front : 1;
	bool back : 1;
};

struct bone_info
{
	static const int max_num_bones = 32;

	bone_info(unsigned int _SourceID = 0)
		: source_id(_SourceID)
	{ positions.fill(float4(0.0f, 0.0f, 0.0f, -1.0f)); }

	unsigned int source_id;
	tracking_clipping clipping;
	std::array<float4, max_num_bones> positions;
};

// Use these to register a skeleton source, i.e. Kinect.
// Basically all that's needed is an indirect to call into the integration code
// to get a snapshot of the skeleton.
void register_source(handle _hSkeleton, const std::function<void(bone_info* _pSkeleton)>& _GetSkeleton);
void unregister_source(handle _hSkeleton);

// Call this from an oWM_SKELETON message. Returns false if the results in 
// _pSkeleton are not valid.
bool get_info(handle _hSkeleton, bone_info* _pSkeleton);

		} // namespace skeleton
	} // namespace windows
} // namespace ouro

#endif
