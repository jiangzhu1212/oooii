// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.

// Parses a string as a CSV document by replacing certain delimiters inline 
// with null terminators and caching indices into the Buffers where values
// begin for very fast access to contents.

#pragma once
#include <oString/text_document.h>
#include <cstdint>
#include <cstring>
#include <vector>

namespace ouro {

class csv
{
public:
	typedef uint32_t index_type;
	
	csv(const char* uri, char* data, deallocate_fn deallocate, size_t est_num_rows = 100, size_t est_num_cols = 20)
		: buffer(uri, data, deallocate)
		, num_cols(0)
	{
		size_ = sizeof(*this) + strlen(buffer.data) + 1;
		cells.reserve(est_num_rows);
		index_buffer(est_num_cols);
		for (const auto& cell : cells)
			size_ += cell.capacity() * sizeof(index_type);
	}

	csv(const char* uri, const char* data, const allocator& alloc = default_allocator, size_t est_num_rows = 100, size_t est_num_cols = 20)
		: buffer(uri, data, alloc, "csv doc")
		, num_cols(0)
	{
		size_ = sizeof(*this) + strlen(buffer.data) + 1;
		cells.reserve(est_num_rows);
		index_buffer(est_num_cols);
		for (const auto& cell : cells)
			size_ += cell.capacity() * sizeof(index_type);
	}

	inline const char* name() const { return buffer.uri; }
	inline size_t size() const { return size_; }
	inline size_t rows() const { return cells.size(); }
	inline size_t cols() const { return num_cols; }
	inline const char* cell(size_t col, size_t row) const { return (row < cells.size() && col < cells[row].size()) ? buffer.data + cells[row][col] : ""; }

private:
	detail::text_buffer buffer;
	std::vector<std::vector<index_type>> cells;
	size_t size_;
	size_t num_cols;
	inline void index_buffer(size_t est_num_cols);
};

void csv::index_buffer(size_t est_num_cols)
{
	cells.resize(1);
	bool inquotes = false;
	char* c = buffer.data;
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
			cells.back().push_back(static_cast<index_type>(std::distance(buffer.data, start)));
			start = c;
			if (done) break;
			else
			{
				c += strspn(c, oNEWLINE);
				cells.resize(cells.size() + 1);
				cells.back().reserve(cells[cells.size()-2].size());
			}
		}

		else
		{
			if (!inquotes)
			{
				if (*c == ',')
				{
					*c++ = '\0';
					cells.back().push_back(static_cast<index_type>(std::distance(buffer.data, start)));
					start = c;
				}
				else
					c++;
			}
			else
				c++;
		}
	}

	for (const auto& cell : cells)
		num_cols = std::max(num_cols, cell.size());
}

}
