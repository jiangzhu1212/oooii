// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
// This header describes the configurable handler for output generated by assert 
// macros. 
#pragma once
#ifndef oCore_reporting_h
#define oCore_reporting_h

#include <oBase/assert.h>
#include <oBase/path.h>

namespace ouro {
	namespace reporting {

struct info
{
	info()
		: prompt(true)
		, use_system_dialogs(false)
		, callstack(true)
		, prefix_file_line(true)
		, prefix_timestamp(false)
		, prefix_type(true)
		, prefix_thread_id(true)
		, prefix_id(false)
	{}

	bool prompt; // present user with a dialog on assert/errors
	bool use_system_dialogs; // default CRT dialogs are enabled/disabled with this
	bool callstack; // include the callstack with the message
	bool prefix_file_line; // MSVC-clickable file(line) format
	bool prefix_timestamp; // time of message
	bool prefix_thread_id; // [host.pid.tid]
	bool prefix_type; // trace/error
	bool prefix_id; // an id that can be used to filter out messages
};

void ensure_initialized();

void set_info(const info& _Info);
info get_info();

// Formats the messasge according to current settings.
void vformatf(
	const assert_context& _Assertion
	, const char* _Format
	, va_list _Args);

typedef std::function<void(const assert_context& _Assertion, const char* _Message)> emitter;
typedef std::function<assert_action::value(const assert_context& _Assertion, const char* _Message)> prompter;

// When a message event occurs, each of these is called with a formatted message
int add_emitter(const emitter& _Emitter);
void remove_emitter(int _Emitter);

// Set to the empty string to disable logging
void set_log(const path& _Path);
path get_log();

// This user will be prompted with this function if prompt is specified.
void set_prompter(const prompter& _Prompter);

// Filtered messages will not be emitted
void add_filter(unsigned int _ID);
void remove_filter(unsigned int _ID);

	} // namespace reporting
} // namespace ouro

#endif
