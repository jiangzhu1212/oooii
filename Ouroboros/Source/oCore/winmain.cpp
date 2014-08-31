// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#include <oString/string.h>
#include <oCore/filesystem.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

// Have a standard WinMain just call out to the normal 
// int main(int argc, const char* argv[]).
static void* argv_alloc(size_t _Size) { return HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, _Size); }
int main(int argc, const char* argv[]);
int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	const char** argv = nullptr;
	int argc = 0;
	{
		ouro::path AppPath = ouro::filesystem::app_path(true);
		argv = ouro::argtok(argv_alloc, AppPath.c_str(), lpCmdLine, &argc);
	}

	int result = main(argc, argv);
	HeapFree(GetProcessHeap(), 0, argv);

	return result;
}
