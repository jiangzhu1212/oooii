// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
// Interface for working with display adapters (video cards)
#pragma once
#ifndef oCore_adapter_h
#define oCore_adapter_h

#include <oBase/macros.h>
#include <oBase/vendor.h>
#include <oBase/version.h>
#include <oHLSL/oHLSLTypes.h>
#include <functional>

namespace ouro {

	namespace display { class id; }
	
	namespace adapter {

class id
{
public:
	id() : Handle(-1) {}

	bool operator==(const id& _That) const { return Handle == _That.Handle; }
	bool operator!=(const id& _That) const { return !(*this == _That); }
	operator bool() const { return Handle != -1; }

private:
	int Handle;
};

struct info
{
	class id id;
	mstring description;
	mstring plugnplay_id;
	struct version version;
	vendor::value vendor;
	struct version feature_level;
};

void enumerate(const std::function<bool(const info& _Info)>& _Enumerator);

// Ouroboros requires a minimum adapter driver version. This returns that 
// version.
version minimum_version(vendor::value _Vendor);

// Checks that all adapters meet or exceed the minimum version
inline bool all_up_to_date()
{
	bool IsUpToDate = true;
	enumerate([&](const info& _Info)->bool
	{
		if (_Info.version < minimum_version(_Info.vendor))
			IsUpToDate = false;
		return IsUpToDate;
	});
	return IsUpToDate;
}

// Given the specified virtual desktop position, find the adapter responsible
// for drawing to it. In this way a device can be matched with a specific 
// screen's adapter. It will also be verified against the specified minimum or 
// exact version.
info find(const int2& _VirtualDesktopPosition, 
	const version& _MinVersion = version(), 
	bool _ExactVersion = true);
inline info find(const version& _MinVersion, bool _ExactVersion = true) { return find(int2(oDEFAULT, oDEFAULT), _MinVersion, _ExactVersion); }

info find(display::id* _DisplayID);

	} // namespace adapter
} // namespace ouro

#endif
