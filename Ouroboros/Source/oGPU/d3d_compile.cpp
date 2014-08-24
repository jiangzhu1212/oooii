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
#include "d3d_compile.h"
#include "d3d_include.h"
#include <oMemory/byte.h>
#include <oCore/windows/win_util.h>
#include <d3d11.h>
#include <D3Dcompiler.h>

namespace ouro { namespace gpu { namespace d3d {

static void d3dcompile_convert_error_buffer(char* _OutErrorMessageString, size_t _SizeofOutErrorMessageString, ID3DBlob* _pErrorMessages, const char** _pIncludePaths, size_t _NumIncludePaths)
{
	const char* msg = (const char*)_pErrorMessages->GetBufferPointer();

	if (!_OutErrorMessageString)
		oTHROW_INVARG0();

	if (_pErrorMessages)
	{
		std::string tmp;
		tmp.reserve(oKB(10));
		tmp.assign(msg);
		replace_all(tmp, "%", "%%");

		// Now make sure header errors include their full paths
		if (_pIncludePaths && _NumIncludePaths)
		{
			size_t posShortHeaderEnd = tmp.find(".h(");
			while (posShortHeaderEnd != std::string::npos)
			{
				size_t posShortHeader = tmp.find_last_of("\n", posShortHeaderEnd);
				if (posShortHeader == std::string::npos)
					posShortHeader = 0;
				else
					posShortHeader++;

				posShortHeaderEnd += 2; // absorb ".h" from search

				size_t shortHeaderLen = posShortHeaderEnd - posShortHeader;

				std::string shortPath;
				shortPath.assign(tmp, posShortHeader, shortHeaderLen);
				std::string path;
				path.reserve(oKB(1));
				
				for (size_t i = 0; i < _NumIncludePaths; i++)
				{
					path.assign(_pIncludePaths[i]);
					path.append("/");
					path.append(shortPath);

					if (filesystem::exists(path.c_str()))
					{
						tmp.replace(posShortHeader, shortHeaderLen, path.c_str());
						posShortHeaderEnd = tmp.find("\n", posShortHeaderEnd); // move to end of line
						break;
					}
				}

				posShortHeaderEnd = tmp.find(".h(", posShortHeaderEnd);
			}
		}

		const char* start = tmp.c_str();
		const char* TruncatedPath = strstr(start, "?????");
		if (TruncatedPath)
			start = TruncatedPath + 5;

		strlcpy(_OutErrorMessageString, start, _SizeofOutErrorMessageString);
	}

	else
		*_OutErrorMessageString = 0;
}

scoped_allocation compile_shader(const char* _CommandLineOptions, const path& _ShaderSourceFilePath, const char* _ShaderSource, const allocator& _Allocator)
{
	int argc = 0;
	const char** argv = argtok(malloc, nullptr, _CommandLineOptions, &argc);
	finally OSCFreeArgv([&] { free(argv); });

	std::string UnsupportedOptions("Unsupported options: ");
	size_t UnsupportedOptionsEmptyLen = UnsupportedOptions.size();

	const char* TargetProfile = "";
	const char* EntryPoint = "main";
	std::vector<const char*> IncludePaths;
	std::vector<std::pair<std::string, std::string>> Defines;
	unsigned int Flags1 = 0, Flags2 = 0;

	for (int i = 0; i < argc; i++)
	{
		const char* inc = nullptr;
		const char* sw = argv[i];
		const int o = to_upper(*(sw+1));
		const int o2 = to_upper(*(sw+2));
		const int o3 = to_upper(*(sw+3));

		std::string StrSw(sw);

		#define TRIML(str) ((str) + strspn(str, oWHITESPACE))

		if (*sw == '/')
		{
			switch (o)
			{
				case 'T':
					TargetProfile = TRIML(sw+2);
					if (!*TargetProfile)
						TargetProfile = argv[i+1];
					break;
				case 'E':
					EntryPoint = TRIML(sw+2);
					if (!*EntryPoint)
						EntryPoint = argv[i+1];
					break;
				case 'I':
					inc = TRIML(sw+2);
					if (!*inc)
						inc = argv[i+1];
					IncludePaths.push_back(inc);
					break;
				case 'O':
				{
					switch (o2)
					{
						case 'D':	Flags1 |= D3DCOMPILE_SKIP_OPTIMIZATION; break;
						case 'P':	Flags1 |= D3DCOMPILE_NO_PRESHADER; break;
						case '0': Flags1 |= D3DCOMPILE_OPTIMIZATION_LEVEL0; break;
						case '1': Flags1 |= D3DCOMPILE_OPTIMIZATION_LEVEL1; break;
						case '2': Flags1 |= D3DCOMPILE_OPTIMIZATION_LEVEL2; break;
						case '3': Flags1 |= D3DCOMPILE_OPTIMIZATION_LEVEL3; break;
						default: UnsupportedOptions.append(" " + StrSw); break;
					}

					break;
				}
				case 'W':
				{
					if (o2 == 'X') Flags1 |= D3DCOMPILE_WARNINGS_ARE_ERRORS;
					else UnsupportedOptions.append(" " + StrSw);
					break;
				}
				case 'V':
				{
					if (o2 == 'D') Flags1 |= D3DCOMPILE_SKIP_VALIDATION;
					else UnsupportedOptions.append(" " + StrSw);
					break;
				}
				case 'Z':
				{
					switch (o2)
					{
						case 'I':
							Flags1 |= D3DCOMPILE_DEBUG;
							break;
						case 'P':
						{
							switch (o3)
							{
								case 'R': Flags1 |= D3DCOMPILE_PACK_MATRIX_ROW_MAJOR; break;
								case 'C': Flags1 |= D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR; break;
								default: UnsupportedOptions.append(" " + StrSw); break;
							}

							break;
						}
						default: UnsupportedOptions.append(" " + StrSw); break;
					}
						
					break;
				}
				case 'G':
				{
					if (o2 == 'P' && o3 == 'P') Flags1 |= D3DCOMPILE_PARTIAL_PRECISION;
					else if (o2 == 'F' && o3 == 'A') Flags1 |= D3DCOMPILE_AVOID_FLOW_CONTROL;
					else if (o2 == 'F' && o3 == 'P') Flags1 |= D3DCOMPILE_PREFER_FLOW_CONTROL;
					else if (o2 == 'D' && o3 == 'P') Flags2 |= D3D10_EFFECT_COMPILE_ALLOW_SLOW_OPS;
					else if (o2 == 'E' && o3 == 'S') Flags1 |= D3DCOMPILE_ENABLE_STRICTNESS;
					else if (o2 == 'E' && o3 == 'C') Flags1 |= D3DCOMPILE_ENABLE_BACKWARDS_COMPATIBILITY;
					else if (o2 == 'I' && o3 == 'S') Flags1 |= D3DCOMPILE_IEEE_STRICTNESS;
					else if (o2 == 'C' && o3 == 'H') Flags2 |= D3D10_EFFECT_COMPILE_CHILD_EFFECT;
					else UnsupportedOptions.append(" " + StrSw);
					break;
				}
				case 'D':
				{
					const char* k = TRIML(sw+2);
					if (!*k)
						k = argv[i+1];
					const char* sep = strchr(k, '=');
					const char* v = "1";
					if (sep)
						v = TRIML(v);
					else
						sep = k + strlen(k);

					Defines.resize(Defines.size() + 1);
					Defines.back().first.assign(k, sep-k);
					Defines.back().second.assign(v);
					break;
				}
				
				default: UnsupportedOptions.append(" " + StrSw); break;
			}
		}
	}

	if (UnsupportedOptionsEmptyLen != UnsupportedOptions.size())
		oTHROW_INVARG("%s", UnsupportedOptions.c_str());

	std::vector<D3D_SHADER_MACRO> Macros;
	Macros.resize(Defines.size() + 1);
	for (size_t i = 0; i < Defines.size(); i++)
	{
		Macros[i].Name = Defines[i].first.c_str();
		Macros[i].Definition = Defines[i].second.c_str();
	}

	Macros.back().Name = nullptr;
	Macros.back().Definition = nullptr;

	path_string SourceName;
	snprintf(SourceName, "%s", _ShaderSourceFilePath);

	include D3DInclude(_ShaderSourceFilePath);
	for (const path& p : IncludePaths)
		D3DInclude.add_search_path(p);

	intrusive_ptr<ID3DBlob> Code, Errors;
	HRESULT hr = D3DCompile(_ShaderSource
		, strlen(_ShaderSource)
		, SourceName
		, Macros.data()
		, &D3DInclude
		, EntryPoint
		, TargetProfile
		, Flags1
		, Flags2
		, &Code
		, &Errors);

	if (FAILED(hr))
	{
		size_t size = Errors->GetBufferSize() + 1 + oKB(10); // conversion can expand buffer, but not by very much, so pad a lot and hope expansion stays small

		std::unique_ptr<char[]> Errs(new char[size]);
		d3dcompile_convert_error_buffer(Errs.get(), size, Errors, IncludePaths.data(), IncludePaths.size());
		oTHROW(io_error, "shader compilation error:\n%s", Errs);
	}

	void* buffer = _Allocator.allocate(Code->GetBufferSize(), 0);
	memcpy(buffer, Code->GetBufferPointer(), Code->GetBufferSize());

	return scoped_allocation(buffer, Code->GetBufferSize(), _Allocator.deallocate);
}

D3D_FEATURE_LEVEL feature_level(const version& _ShaderModel)
{
	D3D_FEATURE_LEVEL level = D3D_FEATURE_LEVEL_9_1;
	if (_ShaderModel == version(3,0)) level = D3D_FEATURE_LEVEL_9_3;
	else if (_ShaderModel == version(4,0)) level = D3D_FEATURE_LEVEL_10_0;
	else if (_ShaderModel == version(4,1)) level = D3D_FEATURE_LEVEL_10_1;
	else if (_ShaderModel == version(5,0)) level = D3D_FEATURE_LEVEL_11_0;
	return level;
}

const char* shader_profile(D3D_FEATURE_LEVEL level, const stage::value& stage)
{
	static const char* sDX9Profiles[] = { "vs_3_0", nullptr, nullptr, nullptr, "ps_3_0", nullptr, };
	static const char* sDX10Profiles[] = { "vs_4_0", nullptr, nullptr, "gs_4_0", "ps_4_0", nullptr, };
	static const char* sDX10_1Profiles[] = { "vs_4_1", nullptr, nullptr, "gs_4_1", "ps_4_1", nullptr, };
	static const char* sDX11Profiles[] = { "vs_5_0", "hs_5_0", "ds_5_0", "gs_5_0", "ps_5_0", "cs_5_0", };

	const char** profiles = 0;
	switch (level)
	{
		case D3D_FEATURE_LEVEL_9_1: case D3D_FEATURE_LEVEL_9_2: case D3D_FEATURE_LEVEL_9_3: profiles = sDX9Profiles; break;
		case D3D_FEATURE_LEVEL_10_0: profiles = sDX10Profiles; break;
		case D3D_FEATURE_LEVEL_10_1: profiles = sDX10_1Profiles; break;
		case D3D_FEATURE_LEVEL_11_0: profiles = sDX11Profiles; break;
		oNODEFAULT;
	}

	const char* profile = profiles[stage];
	if (!profile)
	{
		version ver = version((level>>12) & 0xffff, (level>>8) & 0xffff);
		sstring StrVer;
		oTHROW(not_supported, "Shader profile does not exist for D3D%s's stage %s", to_string2(StrVer, ver), as_string(stage));
	}

	return profile;
}

}}}
