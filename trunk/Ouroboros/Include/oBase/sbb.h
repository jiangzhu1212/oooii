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
// Segregated Binary-Buddy memory allocator.
// This is an implementation of a binary-buddy memory allocator that keeps 
// its bookkeeping separate from the heap itself so that write-combined 
// memory can be managed. To keep the bookkeeping small this uses a compact 
// binary tree comprised of 1 bit per allocation block at any level rather 
// than a linked-list freelist node. O(log n) allocation/free time.
// Per-sbb overhead: 32-bit: 24-bytes + pages; 64-bit: 32-bytes + pages.

#pragma once
#ifndef sbb_h
#define sbb_h

#include <oBase/cbtree.h>

typedef void* sbb_t;

// returns the bytes required to maintain bookkeeping for the allocator
size_t sbb_bookkeeping_size(size_t arena_bytes, size_t min_block_size);

// initializes an sbb for the specified arena. Its size must be a power or two
// and the minimum block size must also be a power of two. bookkeeping should 
// be allocated to the sized returned by sbb_bookkeeping_size.
sbb_t sbb_create(void* arena, size_t arena_bytes, size_t min_block_size, void* bookkeeping);

// deinitializes an sbb after this the sbb is no longer valid
void sbb_destroy(sbb_t sbb);

// returns the pointer passed as arena in sbb_create
void* sbb_arena(sbb_t sbb);

// returns the size specified for the arena in sbb_create
size_t sbb_arena_bytes(sbb_t sbb);

// returns the pointer passed as bookkeeping in sbb_create
void* sbb_bookkeeping(sbb_t sbb);

// returns the internal block size (not user size) allocated for ptr
// O(log n). If the size is already known, the blocks size is the next
// larget power of 2.
size_t sbb_block_size(sbb_t sbb, void* ptr);

// returns the minimum block size specified in sbb_create
size_t sbb_min_block_size(sbb_t sbb);

// returns the maximum block size currently available for allocation
// use this as a fragmentation hint
size_t sbb_max_free_block_size(sbb_t sbb);

// returns the number of blocks marked as free
// use this as a fragmentation hint
size_t sbb_num_free_blocks(sbb_t sbb);

// returns the fixed overhead for the allocator
size_t sbb_overhead(sbb_t sbb);

// walks used and largest available blocks
typedef void (*sbb_walker)(void* ptr, size_t size, int used, void* user);
void sbb_walk_heap(sbb_t sbb, sbb_walker walker, void* user);

// returns true if the heap is valid
bool sbb_check_heap(sbb_t sbb);

// common malloc api: bytes will be rounded to the next power of two consistent
// with the binary-buddy algorithm.
void* sbb_malloc(sbb_t sbb, size_t bytes);
void* sbb_realloc(sbb_t sbb, void* ptr, size_t bytes);
void sbb_free(sbb_t sbb, void* ptr);
void* sbb_memalign(sbb_t sbb, size_t align, size_t bytes);

#endif
