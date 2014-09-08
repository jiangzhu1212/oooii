// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#pragma once
#ifndef oStringTests_h
#define oStringTests_h

// Declarations of oString unit tests. These throw on failure.

namespace ouro { class test_services; namespace tests {

void TESTatof(test_services& services);
void TESTcsv(test_services& services);
void TESTini(test_services& services);
void TESTjson(test_services& services);
void TESTxml(test_services& services);

}}

#endif
