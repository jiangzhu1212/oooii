// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
// Include interface to pass to D3DCompile
#pragma once
#ifndef oGPU_d3d_include_h
#define oGPU_d3d_include_h

#include <oBase/hash_map.h>
#include <oString/path.h>
#include <d3d11.h>
#include <atomic>

namespace ouro {
	namespace gpu {
		namespace d3d {

class include : public ID3DInclude
{
public:
	enum flags
	{
		// if specified Release() will delete itself when the refcount is zero. If this object 
		// is allocated on the stack omitting this flag will not allow delete to be called on this.
		respect_refcount = 1<<0,
	};

	include(const path& _ShaderSourcePath, int _Flags = 0)
		: RefCount(1)
		, Flags(_Flags)
		, ShaderSourcePath(_ShaderSourcePath)
		, Cache(50)
	{}

	ULONG AddRef() { return ++RefCount; }
	ULONG Release() { ULONG r = --RefCount; if (!r && (Flags & respect_refcount)) delete this; return r; }
	IFACEMETHOD(QueryInterface)(THIS_ REFIID riid, void** ppvObject) { return ppvObject ? E_NOINTERFACE : E_POINTER; }
	IFACEMETHOD(Open)(THIS_ D3D_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID *ppData, unsigned int *pBytes);
	IFACEMETHOD(Close)(THIS_ LPCVOID pData);

	void add_search_path(const path& _SearchPath) { SearchPaths.push_back(_SearchPath); }
	void clear_search_paths() { SearchPaths.clear(); }
	void clear_cached_files() { Cache.clear(); }

protected:
	path ShaderSourcePath;
	std::vector<path> SearchPaths;
	hash_map<unsigned long long, scoped_allocation> Cache;
	int Flags;
	std::atomic<int> RefCount;
};

		} // namespace d3d
	} // namespace gpu
} // namespace ouro

#endif
