// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#pragma once
#ifndef oBase_csv_h
#define oBase_csv_h

// Parses a string as an CSV document by replacing certain delimiters inline 
// with null terminators and caching indices into the Buffers where values
// begin for very fast access to contents.

#include <oBase/text_document.h>
#include <oBase/macros.h>
#include <cstring>
#include <vector>

namespace ouro {

class csv
{
public:
	typedef unsigned int index_type;
	
	csv(const char* _URI, char* _pData, const text_document_deleter_t& _Delete, size_t _EstNumRows = 100, size_t _EstNumCols = 20)
		: Buffer(_URI, _pData, _Delete)
		, NumCols(0)
	{
		Size = sizeof(*this) + strlen(Buffer.pData) + 1;
		Cells.reserve(_EstNumRows);
		index_buffer(_EstNumCols);
		for (auto it = Cells.begin(); it != Cells.end(); ++it)
			Size += it->capacity() * sizeof(index_type);
	}

	inline const char* name() const { return Buffer.URI; }
	inline size_t size() const { return Size; }
	inline size_t rows() const { return Cells.size(); }
	inline size_t cols() const { return NumCols; }
	inline const char* cell(size_t _ColIndex, size_t _RowIndex) const { return (_RowIndex < Cells.size() && _ColIndex < Cells[_RowIndex].size()) ? Buffer.pData + Cells[_RowIndex][_ColIndex] : ""; }

private:
	detail::text_buffer Buffer;
	std::vector<std::vector<index_type>> Cells;
	size_t Size;
	size_t NumCols;
	inline void index_buffer(size_t _EstNumCols);
};

void csv::index_buffer(size_t _EstNumCols)
{
	Cells.resize(1);
	bool inquotes = false;
	char* c = Buffer.pData;
	char* start = c;
	while (1)
	{
		if (*c == '\"')
		{
			if (!inquotes) inquotes = true;
			else if (*(c+1) != '\"') inquotes = false;
			c++;
		}

		if (strchr(oNEWLINE, *c) || *c == '\0')
		{
			const bool done = *c == '\0';
			if (inquotes) throw new text_document_error(text_document_errc::generic_parse_error);
			*c++ = '\0';
			Cells.back().push_back(static_cast<index_type>(std::distance(Buffer.pData, start)));
			start = c;
			if (done) break;
			else
			{
				c += strspn(c, oNEWLINE);
				Cells.resize(Cells.size() + 1);
				Cells.back().reserve(Cells[Cells.size()-2].size());
			}
		}

		else
		{
			if (!inquotes)
			{
				if (*c == ',')
				{
					*c++ = '\0';
					Cells.back().push_back(static_cast<index_type>(std::distance(Buffer.pData, start)));
					start = c;
				}
				else
					c++;
			}
			else
				c++;
		}
	}

	for (auto it = Cells.begin(); it != Cells.end(); ++it)
		NumCols = __max(NumCols, it->size());
}

}

#endif
