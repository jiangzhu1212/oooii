// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#pragma once
#ifndef oMeshTests_obj_test_h
#define oMeshTests_obj_test_h

#include <oMesh/obj.h>

namespace ouro {
	namespace tests {

struct obj_test
{
	enum which
	{
		cube,

		count, 
	};

	static std::shared_ptr<obj_test> make(which _Which);

	// Provides a hand-constructed OBJ and its binary equivalents. Use this to 
	// test OBJ handling code.

	// returns info for the mesh
	virtual mesh::obj::info get_info() const = 0;

	// returns the text of an OBJ file as if it had been fread into memory. ('\n'
	// is the only newline here)
	virtual const char* file_contents() const = 0;
};

	} // tests
}

#endif
