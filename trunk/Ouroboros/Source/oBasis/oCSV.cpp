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
#include <oBasis/oCSV.h>
#include <oBasis/oAssert.h>
#include <oBasis/oFixedString.h>
#include <oBasis/oInitOnce.h>
#include <oBasis/oMacros.h>
#include <oBasis/oRefCount.h>
#include <vector>

typedef std::vector<std::vector<const char*> > RECORDS;

bool CSVParse(RECORDS& _OutRecords, char* _CSVDestination, const char* _CSVSource)
{
	_OutRecords.reserve(100);
	if (!oSTRVALID(_CSVSource) || !_CSVDestination)
		return oErrorSetLast(oERROR_INVALID_PARAMETER);
	_OutRecords.resize(1);
	size_t nRowCommas = 0;
	bool inquotes = false;
	const char* r = _CSVSource;
	char* w = _CSVDestination;
	char* entryStart = w;
	while (1)
	{
		if (*r == '\"')
		{
			if (!inquotes)
				inquotes = true;
			else if (*(r+1) != '\"')
				inquotes = false;
			r++;
		}

		if (strchr(oNEWLINE, *r) || *r == 0)
		{
			if (inquotes)
				return oErrorSetLast(oERROR_INVALID_PARAMETER, "Parsing failure");
			*w++ = '\0';
			r += strspn(r, oNEWLINE);
			_OutRecords.back().push_back(entryStart);
			_OutRecords.resize(_OutRecords.size() + 1);
			_OutRecords.back().reserve(_OutRecords[_OutRecords.size()-2].size());
			entryStart = w;
			if (*r == 0)
				break;
		}

		else
		{
			if (!inquotes)
			{
				if (*r == ',')
				{
					*w++ = '\0';
					r++;
					_OutRecords.back().push_back(entryStart);
					entryStart = w;
				}
				else
					*w++ = *r++;
			}
			else
				*w++ = *r++;
		}
	}

	return true;
}

const oGUID& oGetGUID(threadsafe const oCSV* threadsafe const *)
{
	// {581FCA9F-DB40-4BA1-9514-1E548FB4E5EF}
	static const oGUID oIIDCSV = { 0x581fca9f, 0xdb40, 0x4ba1, { 0x95, 0x14, 0x1e, 0x54, 0x8f, 0xb4, 0xe5, 0xef } };
	return oIIDCSV;
}

struct oCSVImpl : oCSV
{
	oDEFINE_REFCOUNT_INTERFACE(RefCount);
	oDEFINE_TRIVIAL_QUERYINTERFACE(oCSV);

	oCSVImpl(const char* _DocumentName, const char* _CSVString, bool* _pSuccess);
	~oCSVImpl();

	size_t GetDocumentSize() const threadsafe override;
	const char* GetDocumentName() const threadsafe override { return DocumentName->c_str(); }
	size_t GetNumRows() const threadsafe override { return Records->size(); }
	size_t GetNumColumns() const threadsafe override { return NumColumns; }
	const char* GetElement(size_t _ColumnIndex, size_t _RowIndex) const threadsafe override;

	char* Data;
	size_t NumColumns;
	oInitOnce<oStringURI> DocumentName;
	oInitOnce<RECORDS> Records;
	oRefCount RefCount;
};

oCSVImpl::oCSVImpl(const char* _DocumentName, const char* _CSVString, bool* _pSuccess)
	: NumColumns(0)
	, DocumentName(_DocumentName)
{
	size_t numberOfElements = strlen(oSAFESTR(_CSVString))+1;
	Data = new char[numberOfElements];
	*_pSuccess = CSVParse(Records.Initialize(), Data, oSAFESTR(_CSVString));
	if (*_pSuccess)
		for (RECORDS::const_iterator it = Records->begin(); it != Records->end(); ++it)
			NumColumns = __max(NumColumns, it->size());
}

oCSVImpl::~oCSVImpl()
{
	delete [] Data;
}

bool oCSVCreate(const char* _DocumentName, const char* _CSVString, threadsafe oCSV** _ppCSV)
{
	if (!_CSVString || !_ppCSV)
	{
		oErrorSetLast(oERROR_INVALID_PARAMETER);
		return false;
	}
	bool success = false;
	oCONSTRUCT(_ppCSV, oCSVImpl(_DocumentName, _CSVString, &success));
	return success;
}

size_t oCSVImpl::GetDocumentSize() const threadsafe
{
	size_t size = sizeof(*this) + strlen(Data) + 1 + Records->capacity() * sizeof(RECORDS::value_type);
	for (RECORDS::const_iterator it = Records->begin(); it != Records->end(); ++it)
		size += it->capacity() * sizeof(RECORDS::value_type::value_type);
	return size;
}

const char* oCSVImpl::GetElement(size_t _ColumnIndex, size_t _RowIndex) const threadsafe
{
	if (_RowIndex < Records->size() && _ColumnIndex < (*Records)[_RowIndex].size())
		return (*Records)[_RowIndex][_ColumnIndex];
	return "";
}
