// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.

// Declarations of oBase unit tests. These throw on failure.

#pragma once
#include <functional>

namespace ouro { class test_services; namespace tests {

void TESTaaboxf();
void TESTcompression(test_services& services);
void TESTconcurrent_growable_object_pool();
void TESTdate(test_services& services);
void TESTequal();
void TESTfilter_chain();
void TESTfourcc();
void TESThash_map(test_services& services);
void TESTosc();

}}
