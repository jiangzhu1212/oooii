// $(header)
#pragma once
#ifndef oCSV_h
#define oCSV_h

#include <oBasis/oInterface.h>

interface oCSV : oInterface
{
	virtual size_t GetDocumentSize() const threadsafe = 0;
	virtual const char* GetDocumentName() const threadsafe = 0;
	virtual size_t GetNumRows() const threadsafe = 0;
	virtual size_t GetNumColumns() const threadsafe = 0;
	virtual const char* GetElement(size_t _ColumnIndex, size_t _RowIndex) const threadsafe = 0;
};

bool oCSVCreate(const char* _DocumentName, const char* _CSVString, threadsafe oCSV** _ppCSV);

#endif
