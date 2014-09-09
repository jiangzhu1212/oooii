// Copyright (c) 2014 Antony Arciuolo. See License.txt regarding use.
#pragma once
#ifndef oMemoryTests_h
#define oMemoryTests_h

// Declarations of oMemory unit tests. These throw on failure.

namespace ouro { class test_services; namespace tests {

void TESTconcurrent_linear_allocator(test_services& services);
void TESTconcurrent_pool(test_services& services);
void TESTpool(test_services& services);
void TESTsbb(test_services& services);
void TESTtlsf_allocator(test_services& services);

}}

#endif
