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
#ifndef oBase_scc_svn_h
#define oBase_scc_svn_h

#include <oBase/scc.h>

namespace ouro {
	namespace detail {

class scc_svn : public scc
{
public:
	scc_svn(const scc_spawn& _Spawn, unsigned int _SpawnTimeoutMS = 10000) : Spawn(_Spawn), SpawnTimeoutMS(_SpawnTimeoutMS) {}

	scc_protocol::value protocol() const override { return scc_protocol::svn; }
	bool available() const override;
	char* root(const char* _Path, char* _StrDestination, size_t _SizeofStrDestination) const override;
	unsigned int revision(const char* _Path) const override;
	void status(const char* _Path, unsigned int _UpToRevision, scc_visit_option::value _Option, const scc_file_enumerator& _Enumerator) const override;
	scc_revision change(const char* _Path, unsigned int _Revision) const override;
	void sync(const char* _Path, unsigned int _Revision, bool _Force = false) override;
	void sync(const char* _Path, const oStd::ntp_date& _Date, bool _Force = false) override;
	void add(const char* _Path) override;
	void remove(const char* _Path, bool _Force = false) override;
	void edit(const char* _Path) override;
	void revert(const char* _Path) override;
private:
	scc_spawn Spawn;
	unsigned int SpawnTimeoutMS;
private:
	void spawn(const char* _Command, const scc_get_line& _GetLine) const;
	void spawn(const char* _Command, ouro::xlstring& _Stdout) const;
};

std::shared_ptr<scc> make_scc_svn(const scc_spawn& _Spawn);

	} // namespace detail
} // namespace ouro

#endif