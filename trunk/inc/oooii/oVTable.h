/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2011 Antony Arciuolo & Kevin Myers                       *
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