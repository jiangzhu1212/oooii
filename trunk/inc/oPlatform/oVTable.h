// $(header)
#pragma once
#ifndef oVTable_h
#define oVTable_h

// !!!WARNING!!! All of this code is highly compiler dependent, it has only been verified to work with MSVC
// Here is some utility code for manipulating VTables and moving them around.  This is sometimes necessary
// when it is desirable to align the VTables of two separate processes

// Returns the size of the VTable in bytes
size_t oVTableSize(void *_pInterfaceImplementation);

// oVTableRemap remaps the supplied implementation's VTable by copying it to a new location while
// overwriting the compiler generated VTable with information to redirect to this new location
// Once oVTableRemap has been called on a particular implementation oVTablePatch 
// must be called every time an object of the particular implementation is instantiated
// Returns the size of the VTable if successful or 0 if there was not enough room to remap
size_t oVTableRemap(void *_pInterfaceImplementation, void* _pNewVTableLocation, size_t _SizeOfNewVTableLocation);

// Patches this instantiation of the interface so that it uses a previously remapped VTable
void oVTablePatch(void* _pInterfaceImplementation);

#endif //oVTable_h