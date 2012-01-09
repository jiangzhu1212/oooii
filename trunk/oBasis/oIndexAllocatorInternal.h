// $(header)
#pragma once
#ifndef oIndexAllocatorInternal_h
#define oIndexAllocatorInternal_h

#include <oBasis/oIndexAllocator.h>
#include <climits>

static const size_t TAG_BITS = 8;
static const size_t TAG_MASK = (1 << TAG_BITS) - 1;
static const unsigned int TAGGED_INVALIDINDEX = UINT_MAX >> TAG_BITS;
static const size_t TAGGED_MAXINDEX = TAGGED_INVALIDINDEX - 1;

#endif
