// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.

// Declarations of oMemory unit tests. These throw on failure.

#pragma once

namespace ouro { class test_services; namespace tests {

void TESTconcurrent_linear_allocator(test_services& services);
void TESTconcurrent_pool(test_services& services);
void TESTpool(test_services& services);
void TESTsbb(test_services& services);
void TESTtlsf_allocator(test_services& services);

}}
