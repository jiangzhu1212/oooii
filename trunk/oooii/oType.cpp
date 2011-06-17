// $(header)
#include <oooii/oType.h>
#include <oooii/oAssert.h>
#include <oooii/oErrno.h>
#include <oooii/oHash.h>
#include <oooii/oSingleton.h>
#include <oooii/oStddef.h>
#include <oooii/oStl.h>
#include <oooii/oString.h>
#include <half.h>

struct oTypeContext : oProcessSingleton<oTypeContext>
{
	typedef oArray<oType::Inspection*, 8> container_t;

	container_t TypeInspections;
};

namespace detail {

// These should match the results of typeid(<mytype>), so there 
// may have to be some ifdef'ing per compiler here. Not using 
// typeid().name() directly so the user has the option of not 
// using RTTI but still use this class for PODs.
static const char* sCppNames[] =
{
	"void",
	"void",
	"bool",
	"char", 
	"unsigned char",
	"short", 
	"unsigned short",
	"int", 
	"unsigned int",
	"long", 
	"unsigned long",
	"long long",
	"unsigned long long",
	"half",
	"float", 
	"double",
};

static size_t sCppSizes[] = 
{
	~0u,
	~0u,
	sizeof(bool),
	sizeof(char),
	sizeof(unsigned char),
	sizeof(short),
	sizeof(unsigned short),
	sizeof(int),
	sizeof(unsigned int),
	sizeof(long),
	sizeof(unsigned long),
	sizeof(long long),
	sizeof(unsigned long long),
	sizeof(half),
	sizeof(float),
	sizeof(double),
};
oSTATICASSERT(oCOUNTOF(sCppNames) == oCOUNTOF(sCppSizes));

static inline oType::TYPE PodAsType(const char* _TypeinfoName)
{
	for (int i = 0; i < oCOUNTOF(sCppNames); i++)
		if (!strcmp(sCppNames[i], _TypeinfoName))
			return (oType::TYPE)i;
	return oType::TYPE_UNKNOWN;
}

static inline const char* AsString(oType::TYPE _Type)
{
	oASSERT(_Type >= 0 && _Type < oCOUNTOF(sCppNames), "");
	return sCppNames[_Type];
}

static inline size_t GetPodSize(oType::TYPE _Type)
{
	oASSERT(_Type >= 0 && _Type < oCOUNTOF(sCppSizes), "");
	return sCppSizes[_Type];
}

static inline int HashName(const char* _Name)
{
	const char* end = _Name + strlen(_Name) - 1;
	while (strchr("&* ", *end)) // ignore pointer modifiers (handled separately)
		end--;
	return static_cast<int>(oHash_superfast(_Name, static_cast<unsigned int>(end - _Name + 1)));
}

static void CalculateOffsetAndType(const char* _TypeinfoName, int* _pOffset, oType::CLASS* _pClassType)
{
	*_pOffset = 0;
	*_pClassType = oType::CLASS_POD;
	if (!memcmp(_TypeinfoName, "enum ", 5)) *_pOffset = 5, *_pClassType = oType::CLASS_ENUM;
	else if (!memcmp(_TypeinfoName, "struct ", 7)) *_pOffset = 7, *_pClassType = oType::CLASS_CLASS;
	else if (!memcmp(_TypeinfoName, "class ", 6)) *_pOffset = 6, *_pClassType = oType::CLASS_CLASS; 
	else if (!memcmp(_TypeinfoName, "union ", 6)) *_pOffset = 6, *_pClassType = oType::CLASS_UNION;
}

static bool KillPtr64(std::string& _TypeinfoString)
{
	std::string::size_type pos64 = _TypeinfoString.rfind(" __ptr64");
	std::string::size_type templateEnd = _TypeinfoString.rfind(">");
	if (pos64 != std::string::npos && (templateEnd == std::string::npos || pos64 > templateEnd))
	{
		_TypeinfoString.erase(pos64);
		return true;
	}

	return false;
}

static void InterpretAndRemoveModifiers(std::string& _TypeinfoString, oType::MODIFIER* _pModifier)
{
	KillPtr64(_TypeinfoString);

	oType::MODIFIER modifier = oType::MODIFIER_NONE;
	std::string::iterator it = _TypeinfoString.end() - 1;

	while (*it == '*' || *it == '&')
	{
		if (*it == '*')
		{
			oASSERT(oType::MODIFIER_REFERENCE, "Pointers to references or references to pointers not supported");
			_TypeinfoString.erase(it);
			it = _TypeinfoString.end() - 1;
			modifier = (modifier == oType::MODIFIER_POINTER) ? oType::MODIFIER_POINTER_TO_POINTER : oType::MODIFIER_POINTER;
		}

		if (*it == '&')
		{
			oASSERT(modifier == oType::MODIFIER_NONE, "Pointers to references or references to pointers not supported");
			_TypeinfoString.erase(it);
			it = _TypeinfoString.end() - 1;
		}

		while (isspace(*it) && it > _TypeinfoString.begin())
		{
			_TypeinfoString.erase(it);
			it = _TypeinfoString.end() - 1;
		}

		if (KillPtr64(_TypeinfoString))
			it = _TypeinfoString.end() - 1;
	}

	*_pModifier = modifier;
}

static int MakeTypeID(const char* _TypeinfoName)
{
	int offset = 0;
	oType::CLASS classType = oType::CLASS_POD;
	CalculateOffsetAndType(_TypeinfoName, &offset, &classType);

	std::string typeinfoString(_TypeinfoName);
	oType::MODIFIER modifierType = oType::MODIFIER_NONE;
	InterpretAndRemoveModifiers(typeinfoString, &modifierType);

	int rawType = 0;
	if (offset) // then there was a keyword indicating an aggregate type
		rawType = static_cast<int>(oHash_superfast(typeinfoString.c_str(), static_cast<unsigned int>(typeinfoString.size())));
	else
		rawType = static_cast<int>(PodAsType(typeinfoString.c_str()));

	return oType::MakeTypeID(classType, modifierType, rawType);
}

const char* SkipCV(const char* _Name)
{
	if (!memcmp(_Name, "mutable ", 8)) _Name += 8;
	if (!memcmp(_Name, "const ", 6)) _Name += 6;
	if (!memcmp(_Name, "volatile ", 9)) _Name += 9;
	return _Name;
}

errno_t PodToString(char* _StrDestination, size_t _SizeofStrDestination, oType::TYPE _Type, const void* _pMemory)
{
	errno_t err = 0;
	#define TS(type) err = oToString(_StrDestination, _SizeofStrDestination, *(const oType*)_pMemory)
	switch (_Type)
	{
		case oType::TYPE_BOOL: TS(bool); break;
		case oType::TYPE_CHAR: TS(char); break;
		case oType::TYPE_UCHAR: TS(unsigned char); break;
		case oType::TYPE_SHORT: TS(short); break;
		case oType::TYPE_USHORT: TS(unsigned short); break;
		case oType::TYPE_INT: TS(int); break;
		case oType::TYPE_UINT: TS(unsigned int); break;
		case oType::TYPE_LONG: TS(long); break;
		case oType::TYPE_ULONG: TS(unsigned long); break;
		case oType::TYPE_LONGLONG: TS(long long); break;
		case oType::TYPE_ULONGLONG: TS(unsigned long long); break;
		case oType::TYPE_HALF: TS(half); break;
		case oType::TYPE_FLOAT: TS(float); break;
		case oType::TYPE_DOUBLE: TS(double); break;
		default: err = EINVAL; break;
	}
	#undef TS
	return err;
}

errno_t PodFromString(void* _pDestination, oType::TYPE _Type, const char* _String)
{
	errno_t err = 0;
	#define FS(type) err = oFromString(static_cast<type*>(_pDestination), _String)
	switch (_Type)
	{
	case oType::TYPE_BOOL: FS(bool); break;
	case oType::TYPE_CHAR: FS(char); break;
	case oType::TYPE_UCHAR: FS(unsigned char); break;
	case oType::TYPE_SHORT: FS(short); break;
	case oType::TYPE_USHORT: FS(unsigned short); break;
	case oType::TYPE_INT: FS(int); break;
	case oType::TYPE_UINT: FS(unsigned int); break;
	case oType::TYPE_LONG: FS(long); break;
	case oType::TYPE_ULONG: FS(unsigned long); break;
	case oType::TYPE_LONGLONG: FS(long long); break;
	case oType::TYPE_ULONGLONG: FS(unsigned long long); break;
	case oType::TYPE_HALF: FS(half); break;
	case oType::TYPE_FLOAT: FS(float); break;
	case oType::TYPE_DOUBLE: FS(double); break;
	default: err = EINVAL; break;
	}
	#undef FS
	return err;
}

} // namespace details

oType::oType(const char* _TypeinfoName, const char* _EnsureTypeSpecifier)
	: id(TYPE_UNKNOWN)
{
	const char* s = detail::SkipCV(_TypeinfoName);
	char buf[2048];
	if (_EnsureTypeSpecifier && memcmp(s, _EnsureTypeSpecifier, strlen(_EnsureTypeSpecifier)))
		sprintf_s(buf, oCOUNTOF(buf), "%s %s", _EnsureTypeSpecifier, s), s = buf;
	id = detail::MakeTypeID(s);
}

bool oType::IsValid() const
{
	if (IsPod())
		return true;

	oTypeContext::container_t& ti = oTypeContext::Singleton()->TypeInspections;
	for (oTypeContext::container_t::iterator it = ti.begin(); it != ti.end(); ++it)
		if ((*it)->IsValid(*this))
			return true;

	return false;
}

const char* oType::AsString(bool _TypeinfoName) const
{
	if (IsPod())
		return detail::AsString((oType::TYPE)Unmodified());
	else
	{
		oTypeContext::container_t& ti = oTypeContext::Singleton()->TypeInspections;
		for (oTypeContext::container_t::iterator it = ti.begin(); it != ti.end(); ++it)
		{
			const char* s = (*it)->AsString(*this, _TypeinfoName);
			if (s) return s;
		}
	}

	return "oType error";
}

errno_t oType::FromString(void* _pDestination, const char* _ValueAsString) const
{
	errno_t err = EINVAL;
	if (_pDestination && _ValueAsString)
	{
		oASSERT(!(IsPointer() || IsReference()), "");
		if (IsPod())
			err = detail::PodFromString(_pDestination, GetPodType(), _ValueAsString);
		else
		{
			oTypeContext::container_t& ti = oTypeContext::Singleton()->TypeInspections;
			for (oTypeContext::container_t::iterator it = ti.begin(); it != ti.end(); ++it)
			{
				err = (*it)->FromString(*this, _pDestination, _ValueAsString);
				if (!err) break;
			}
		}
	}	
	return err;
}

errno_t oType::ToString(char* _StrDestination, size_t _SizeofStrDestination, const void* _pSource) const
{
	errno_t err = 0;

	if (IsPointer() || IsReference())
		sprintf_s(_StrDestination, _SizeofStrDestination, "%p", *(const char**)_pSource);

	else if (IsPod())
		err = detail::PodToString(_StrDestination, _SizeofStrDestination, GetPodType(), _pSource);

	else
	{
		oTypeContext::container_t& ti = oTypeContext::Singleton()->TypeInspections;
		for (oTypeContext::container_t::iterator it = ti.begin(); it != ti.end(); ++it)
		{
			err = (*it)->ToString(*this, _StrDestination, _SizeofStrDestination, _pSource);
			if (!err) break;
		}
	}

	return err;
}

size_t oType::GetSize() const
{
	if (GetModifier() != oType::MODIFIER_NONE)
		return sizeof(char*);

	if (IsPod())
		return detail::GetPodSize(GetPodType());
	
	if (IsEnum())
		return sizeof(oType::TYPE);

	oTypeContext::container_t& ti = oTypeContext::Singleton()->TypeInspections;
	for (oTypeContext::container_t::iterator it = ti.begin(); it != ti.end(); ++it)
	{
		size_t s = (*it)->GetSize(*this);
		if (s != ~0u) return s;
	}

	return ~0u;
}

void oType::RegisterInspection(Inspection* _pTypeInspection)
{
	oPushBackUnique(oTypeContext::Singleton()->TypeInspections, _pTypeInspection);
}

void oType::UnregisterInspection(Inspection* _pInspection)
{
	oFindAndErase(oTypeContext::Singleton()->TypeInspections, _pInspection);
}
