/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2013 OOOii.                                              *
 * antony.arciuolo@oooii.com                                              *
 * kevin.myers@oooii.com                                                  *
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
#include <oPlatform/oVTable.h>
#include <oPlatform/Windows/oWindows.h>
#include <oPlatform/oReporting.h>

// Utility code for remapping VTables

#ifdef _DEBUG
static void WARNVTableNotPatched()
{
	// If you've reached this code the object you're trying to use has been run through oVTableRemap
	// so that the VTable can be moved to somewhere else.  This means everytime one of these objects
	// are instantiated (typically via placement new) they must be run through oVTablePatch so that
	// they operate with the correct VTable
	oASSERT( false, "Class has been flagged for VTable patching but class has not been patched! To do this:\n1. Ensure a ctor YourClass( HVTCOOKIE _Cookie ) exists for your class (might be in an infrastructure macro)\n2. Define that ctor to be empty (just used to get at class vtable)\n3. Add \"oVTablePatch(this);\" as the first line of any \"real\" ctors for the class.\n4. In oRendererVtablePatch add a REMAP_CLASS() macro entry for your class where the others are." );
}
#endif

// We assume the vtable is referenced by the first pointer in an interface based on MSVC
struct tableLocation
{
	void* table;
};

struct VTableRemap
{
	void* pEntries[1];
};

size_t oVTableRemap(void *_pInterfaceImplementation, void* _pNewVTableLocation, size_t _SizeOfNewVTableLocation)
{
	size_t SizeOfVTable = oVTableSize( _pInterfaceImplementation );
	if( SizeOfVTable > _SizeOfNewVTableLocation )
		return 0;

	tableLocation* location = (tableLocation*)_pInterfaceImplementation;

	// Copy off the VTable to our remapped location
	memcpy( _pNewVTableLocation, location->table, SizeOfVTable );

	// Store the start of the remapped VTable in the first entry
	// of the compiler generated VTable, to do this we need to temporarily 
	// allow write access to the compiler's VTable
	VTableRemap *Remap = (VTableRemap*)location->table;
	DWORD oldPermissions = 0;
	VirtualProtect(&Remap->pEntries[0], sizeof(void*), PAGE_EXECUTE_READWRITE, &oldPermissions);

#ifdef _DEBUG
	// Overwrite every entry but the first with a pointer to the warning message
	void** ppIndividualEntries = &Remap->pEntries[0];
	size_t numVTablesEntries = SizeOfVTable / sizeof(void*);
	for( size_t i = 1; i < numVTablesEntries; ++i )
	{
		DWORD DEBUGOldPermissions = 0;
		VirtualProtect(&ppIndividualEntries[i], sizeof(void*), PAGE_EXECUTE_READWRITE, &DEBUGOldPermissions);
		ppIndividualEntries[i] = &WARNVTableNotPatched;
		VirtualProtect(&ppIndividualEntries[i], sizeof(void*), DEBUGOldPermissions, &DEBUGOldPermissions);
	}
#endif
	Remap->pEntries[0] = _pNewVTableLocation;
	VirtualProtect(&Remap->pEntries[0], sizeof(void*), oldPermissions, &oldPermissions);

	return SizeOfVTable;
}

void oVTablePatch(void* _pInterfaceImplementation)
{
	// The assumption here is that the Vtable for this object is now stored in the first entry of the current VTable
	((tableLocation*)_pInterfaceImplementation)->table = ((VTableRemap*)((tableLocation*)_pInterfaceImplementation)->table)->pEntries[0];
}

size_t oVTableSize( void *_pInterfaceImplementation )
{
	// Based on experimentation with MSVC we have determined that the VTable is NULL terminated in _DEBUG
	// We have yet to develop a reliable method for determining the vtable size in release so we asssume
	// a max that is checked in debug
	static const size_t MAX_VTABLE_ENTRIES = 32;
#ifdef _DEBUG
	tableLocation* location = (tableLocation*)_pInterfaceImplementation;
	void** table = (void**)location->table;

	for( int i = 0; i < MAX_VTABLE_ENTRIES; ++i )
	{
		if( !table[i] )
			return i * sizeof(void*);
	}

	oASSERT( false, "VTable is larger than max entries %i", MAX_VTABLE_ENTRIES );
	return 0;
#else
	return MAX_VTABLE_ENTRIES * sizeof(void*);
#endif
}

void* oVTableGet(void* _pInterfaceImplementation)
{
	return *(void**)((tableLocation*)_pInterfaceImplementation)->table;
}