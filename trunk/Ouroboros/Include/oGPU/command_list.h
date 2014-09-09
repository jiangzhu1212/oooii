// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#pragma once
#ifndef oGPU_command_list_h
#define oGPU_command_list_h

#include <oBase/types.h>

namespace ouro { namespace gpu {

class device;

class command_list
{
public:
	static const uint immediate = uint(-1);

	command_list() : context(nullptr), id(0) {}
	~command_list() { deinitialize(); }

	void initialize(const char* name, device& dev, uint id);
	void deinitialize();

	inline bool is_immediate() const { return id == immediate; }

	inline uint get_id() const { return id; }

	// sends the command list to the device
	void flush();

	// resets default command list state
	void reset();

private:
	void* context;
	uint id;

	command_list(const command_list&);
	const command_list& operator=(const command_list&);
};

}}

#endif
