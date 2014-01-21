/**************************************************************************
 * The MIT License                                                        *
 * Copyright (c) 2013 Antony Arciuolo.                                    *
 * arciuolo@gmail.com                                                     *
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

using namespace ouro;

int main(int argc, const char* argv[])
{
	if (argc <= 1)
	{
		path exe(argv[0]);
		printf("%s <infile> [<outfile>]\n  If no outfile is specified, infile_ext.h is used.\n", exe.filename().c_str());
		return true;
	}

	path infile(argv[1]);
	path outfile;

	if (argc > 2)
		outfile = argv[2];
	else
	{
		outfile = infile;
		auto ext = infile.extension();
		char suffix[64];
		if (ext.empty())
			snprintf(suffix, ".h");
		else
			snprintf(suffix, "_%s.h", infile.extension().c_str() + 1);

		outfile.replace_extension_with_suffix(suffix);
	}

	auto Name = outfile.basename();

	try
	{
		size_t size = 0;
		auto b = filesystem::load(infile, &size);

		std::string s;
		s.reserve(size * 2 / 3);

		s = "const unsigned char ";
		s += Name.c_str();
		s += "[] = " oNEWLINE "{ // *** AUTO-GENERATED BUFFER, DO NOT EDIT *** (100b/row)";

		char buf[160];
		const unsigned char* p = (const unsigned char*)b.get();
		for (size_t i = 0; i < size; i++)
		{
			int offset = 0;
			if ((i%100)==0)
				offset = snprintf(buf, oNEWLINE "\t");
			snprintf(buf+offset, sizeof(buf)-offset, "%u,", p[i]);
			s += buf;
		}

		snprintf(buf, oNEWLINE "}; // %s" oNEWLINE, Name.c_str());
		s += buf;

		filesystem::save(outfile, s.c_str(), s.length(), filesystem::save_option::binary_write);
	}

	catch (std::exception& e)
	{
		printf("unhandled exception: %s", e.what());
		return -1;
	}

	return 0;
}
