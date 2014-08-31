// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
// Encapsulate compiled shader code into a Caccessible form.

+ACM-ifndef oGfxShaders+AF8-h
+ACM-define oGfxShaders+AF8-h

+ACM-include +ADw-oMesh/mesh.h+AD4-

namespace ouro +AHs-
	namespace gfx +AHs-

namespace gfx+AF8-vl
+AHs-	enum value : uchar +AHs-
	
	null,
	count,

+AH0AOwB9-

namespace gfx+AF8-vs
+AHs-	enum value : uchar +AHs-

	null,
	count,

+AH0AOwB9-

namespace gfx+AF8-hs
+AHs-	enum value : uchar +AHs-

	null,
	count,

+AH0AOwB9-

namespace gfx+AF8-ds
+AHs-	enum value : uchar +AHs-

	null,
	count,

+AH0AOwB9-

namespace gfx+AF8-gs
+AHs-	enum value : uchar +AHs-

	null,
	count,

+AH0AOwB9-

namespace gfx+AF8-ps
+AHs-	enum value : uchar +AHs-

	null,
	count,

+AH0AOwB9-

namespace gfx+AF8-cs
+AHs-	enum value : uchar +AHs-

	null,
	count,

+AH0AOwB9-

// returns the elements of input
mesh::element+AF8-array elements(const gfx+AF8-vl::value+ACY- input)+ADs-

// returns the vertex shader byte code with the same input
// signature as input.
const void+ACo- vs+AF8-byte+AF8-code(const gfx+AF8-vl::value+ACY- input)+ADs-

// returns the buffer of bytecode compiled during executable compilation time
// (not runtime compilation)
const void+ACo- byte+AF8-code(const gfx+AF8-vs::value+ACY- shader)+ADs-
const void+ACo- byte+AF8-code(const gfx+AF8-hs::value+ACY- shader)+ADs-
const void+ACo- byte+AF8-code(const gfx+AF8-ds::value+ACY- shader)+ADs-
const void+ACo- byte+AF8-code(const gfx+AF8-gs::value+ACY- shader)+ADs-
const void+ACo- byte+AF8-code(const gfx+AF8-ps::value+ACY- shader)+ADs-
const void+ACo- byte+AF8-code(const gfx+AF8-cs::value+ACY- shader)+ADs-

	+AH0- // namespace gfx
+AH0- // namespace ouro

+ACM-endif
