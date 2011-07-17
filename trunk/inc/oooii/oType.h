// $(header)
// Encapsulates a C++ type as a 32-bit id suitable for use in a hash.
#pragma once
#ifndef oType_h
#define oType_h

#include <stddef.h>

class oType
{
	int id;
public:

	struct Inspection
	{
		// Interface for a user to define how to handle types unknown 
		// to the Type class such as enums, classes, structs, and 
		// unions. The specified type has CLASS and MODIFIER masked out
		// already, so what's left is the unmodified, unclassified raw 
		// type id.

		// Returns false if the system does not recognize the type
		virtual bool IsValid(const oType& _Type) const = 0;

		// Returns the name of the type as a string. If _TypeinfoName is
		// true, this will return the same string typeid(type).name would
		// return.
		virtual const char* AsString(const oType& _Type, bool _TypeinfoName) const = 0;

		// Fills the memory pointed to by _pDestination with the value of
		// _ValueAsString after the type has been used to convert it from
		// string to its native form.
		virtual errno_t FromString(const oType& _Type, void* _pDestination, const char* _ValueAsString) const = 0;

		// Converts the memory pointed to by _pValue to a string formatted
		// by its type. This returns ERANGE if a truncation occured, EINVAL 
		// if a parameter is invalid, or 0 if successful.
		virtual errno_t ToString(const oType& _Type, char* _StrDestination, size_t _SizeofStrDestination, const void* _pValue) const = 0;

		// if the query cannot determine size, return ~0.
		virtual size_t GetSize(const oType& t) const = 0;
	};

	// This will be used by oType to implement APIs such as size for 
	// user-defined types. The user is responsible for the lifetime of 
	// the object.
	static void RegisterInspection(Inspection* _pTypeInspection);
	static void UnregisterInspection(Inspection* _pTypeInspection);

	enum CLASS
	{
		CLASS_POD,
		CLASS_ENUM,
		CLASS_CLASS,
		CLASS_UNION,
	};

	enum TYPE
	{
		// preface enums with TYPE_ to avoid any other source that defines
		// types.
		TYPE_UNKNOWN,
		TYPE_VOID,
		TYPE_BOOL,
		TYPE_CHAR,
		TYPE_UCHAR,
		TYPE_SHORT,
		TYPE_USHORT,
		TYPE_INT,
		TYPE_UINT,
		TYPE_LONG,
		TYPE_ULONG,
		TYPE_LONGLONG,
		TYPE_ULONGLONG,
		TYPE_HALF, // considered a POD by oooii lib
		TYPE_FLOAT,
		TYPE_DOUBLE,
		NUM_TYPES,
	};

	enum MODIFIER
	{
		MODIFIER_NONE,
		MODIFIER_REFERENCE,
		MODIFIER_POINTER,
		MODIFIER_POINTER_TO_POINTER,
	};

	inline static int MakeTypeID(CLASS _Class, MODIFIER _Modifier, int _RawType)
	{	// id will be a hash, so lose the most insignificant bits to make room for _Class and _Modifier
		if (_Class != CLASS_POD) _RawType >>= 4;									
		return ((_Modifier&0x3)<<30) | ((_Class&0x3)<<28) | (_RawType&0x0fffffff);
	}

	// Constructors
	oType() : id(TYPE_UNKNOWN) {}
	oType(TYPE _Type, MODIFIER _Modifier = MODIFIER_NONE) : id(MakeTypeID(CLASS_POD, _Modifier, _Type)) {} // POD constructor
	oType(int _RawType, CLASS _Class = CLASS_ENUM, MODIFIER _Modifier = MODIFIER_NONE) : id(MakeTypeID(_Class, _Modifier, _RawType)) {}  // Non-POD constructor
	oType(const char* _TypeinfoName, const char* _EnsureTypeSpecifier = 0); // Parses RTTI name, optionally ensuring type-specifier {class|struct|enum|etc.} is in the result
	oType(const oType& _Type) { id = _Type.id; } // Default copy
	oType(const oType& _Type, MODIFIER _ForcedModifier) { id = (_Type.id&0x3fffffff)|((_ForcedModifier&0x3)<<30); } // Copy that ignores the modifier

	// Comparisons
	inline oType Unmodified() const { return oType(*this, MODIFIER_NONE); }
	inline bool operator==(const oType& _Type) const { return id == _Type.id; }
	inline bool operator!=(const oType& _Type) const { return !(*this == _Type); }
	inline bool operator==(const TYPE& _Type) const { return id == _Type; }
	bool IsValid() const;

	// Type casting
	operator int() const { return id; }
	operator unsigned int() const { return static_cast<unsigned int>(id); }
	operator TYPE() const { return (TYPE)Unmodified().id; }

	// String conversions
	const char* AsString(bool _TypeinfoName = false) const; // returns unmodified type string (*'s or &'s must be added separately)
	errno_t FromString(void* _pDestination, const char* _ValueAsString) const; // writes a string value of this type to memory
	errno_t ToString(char* _StrDestination, size_t _SizeofStrDestination, const void* _pSource) const; // writes a string value from memory of this type
	template<size_t size> errno_t ToString(char (&_StrDestination)[size], const void* _pSource) const { return ToString(_StrDestination, size, _pSource); }
	size_t GetSize() const; // Returns oINVALID_SIZE_T if size could not be determined.

	// Type flag accessors
	inline static int GetUnmodified(int _Type) { return _Type&0x3fffffff; }
	inline static int GetRawType(int _Type) { return _Type&0x0fffffff; }
	inline static CLASS GetClass(int _Type) { return (CLASS)((_Type>>28)&0x3); }
	inline static MODIFIER GetModifier(int _Type) { return (MODIFIER)((_Type>>30)&0x3); }
	inline static TYPE GetPodType(int _Type) { return IsPod(_Type) ? (TYPE)GetUnmodified(_Type) : TYPE_UNKNOWN; }
	inline int GetRawType() const { return GetRawType(id); }
	inline CLASS GetClass() const { return GetClass(id); }
	inline MODIFIER GetModifier() const { return GetModifier(id); }
	inline TYPE GetPodType() const { return GetPodType(id); }

	// Convenience wrappers for type flag accessors
	inline static bool IsPod(int _Type) { return GetClass(_Type) == CLASS_POD; }
	inline static bool IsPointer(int _Type) { return GetModifier(_Type) == MODIFIER_POINTER || GetModifier(_Type) == MODIFIER_POINTER_TO_POINTER; }
	inline static bool IsPointerToPointer(int _Type) { return GetModifier(_Type) == MODIFIER_POINTER_TO_POINTER; }
	inline static bool IsReference(int _Type) { return GetModifier(_Type) == MODIFIER_REFERENCE; }
	inline static bool IsClass(int _Type) { return GetClass(_Type) == CLASS_CLASS; }
	inline static bool IsStruct(int _Type) { return IsClass(_Type); }
	inline static bool IsUnion(int _Type) { return GetClass(_Type) == CLASS_UNION; }
	inline static bool IsEnum(int _Type) { return GetClass(_Type) == CLASS_ENUM; }
	inline int GetUnmodified() { return GetUnmodified(id); }
	inline bool IsPod() const { return IsPod(id); }
	inline bool IsPointer() const { return IsPointer(id); }
	inline bool IsPointerToPointer() const { return IsPointerToPointer(id); }
	inline bool IsReference() const { return IsReference(id); }
	inline bool IsClass() const { return IsClass(id); }
	inline bool IsStruct() const { return IsClass(); }
	inline bool IsUnion() const { return IsUnion(id); }
	inline bool IsEnum() const { return IsEnum(id); }
};

#endif
