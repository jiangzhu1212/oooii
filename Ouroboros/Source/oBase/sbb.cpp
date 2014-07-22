// $(header)
#include <oBase/sbb.h>
#include <oBase/throw.h>
#include <oBase/assert.h>

#define SBB_FATAL(msg) oTHROW_INVARG(msg)
#define SBB_MAX(x,y) ((x) > (y) ? (x) : (y))

typedef size_t sbb_page_t;
typedef uint32_t sbb_node_t;

enum sbb_constants
{
#ifdef _M_X64
  kPageBitsLog2 = 6,
#else
  kPageBitsLog2 = 5,
#endif

  kPageBytes = sizeof(sbb_page_t),
  kPageBits = kPageBytes * 8,
  kPageBitMask = kPageBits - 1,
  kPageMaxBits = kPageBits - 1,
};

static const sbb_node_t kRoot = 1;
static const sbb_page_t kRootMask = sbb_page_t(1) << (kPageBits - 2);

struct sbb_bookkeeping_t
{
  void* arena;
  size_t arena_bytes; // must be pow2
  uint32_t min_block_size; // must be pow2
	uint32_t min_block_size_log2; // log2i(min_block_size)
  uint32_t num_nodes;
  uint32_t unused; // keeps size consistently aligned between 64- and 32-bit compiles
};

// returns a pointer to the page that contains the bit representation of the node
static inline sbb_page_t* sbb_page(sbb_bookkeeping_t* bookkeeping, sbb_node_t node)
{
  const size_t pagei = node >> kPageBitsLog2;
  return ((sbb_page_t*)(bookkeeping + 1)) + pagei;
}

// returns the bit index into a page where the node is
static inline int sbb_pagebit(sbb_node_t node)
{
	return kPageMaxBits - (node & kPageBitMask);
}

static inline sbb_page_t sbb_mask(int bit_index)
{
	return sbb_page_t(1) << bit_index;
}

// returns the node's bit page as well as the bitmask to where the node is
static inline sbb_page_t* sbb_page(sbb_bookkeeping_t* bookkeeping, sbb_node_t node, sbb_page_t* out_bitmask)
{
  int bit = sbb_pagebit(node);
  *out_bitmask = sbb_mask(bit);
  return sbb_page(bookkeeping, node);
}

// returns the node's bit page as well as the bitmask to where the node is and another bitmask to its sibling node
static inline sbb_page_t* sbb_page(sbb_bookkeeping_t* bookkeeping, sbb_node_t node, sbb_page_t* out_bitmask, sbb_page_t* out_sibling_mask)
{
  int bit = sbb_pagebit(node);
  int sibling_bit = bit ^ 1;
  *out_bitmask = sbb_mask(bit);
  *out_sibling_mask = sbb_mask(sibling_bit);
  return sbb_page(bookkeeping, node);
}

// returns the node's children's bit page (always the same page for both) as well as a bitmask for both children
static inline sbb_page_t* sbb_children_page(sbb_bookkeeping_t* bookkeeping, sbb_node_t node, sbb_page_t* out_childrenmask)
{
	uint32_t left = cbtree_left_child(node);
	if (left >= bookkeeping->num_nodes)
		return nullptr;
	int bit = sbb_pagebit(left);
	*out_childrenmask = sbb_mask(bit) | sbb_mask(bit-1);
	return sbb_page(bookkeeping, left);
}

// returns true if the node is marked as used. This isn't enough information to tell if this 
// actually represents an allocation because used bits are promoted up the tree to optimize 
// free block searches.
static inline bool sbb_used(sbb_page_t* page, sbb_page_t mask)
{
  return (*page & mask) == 0;
}

// returns true if the node is marked as free. This implies that at least one subnode is also
// marked as free.
static inline bool sbb_free(sbb_page_t* page, sbb_page_t mask)
{
  return (*page & mask) != 0;
}

// sets the node marked as used
static inline void sbb_mark_used(sbb_page_t* page, sbb_page_t mask)
{
  *page &=~ mask;
}

// sets the node marked as free
static inline void sbb_mark_free(sbb_page_t* page, sbb_page_t mask)
{
  *page |= mask;
}

// convert node to a memory pointer
static inline void* sbb_to_ptr(sbb_bookkeeping_t* bookkeeping, sbb_node_t node, size_t level_block_size)
{
  size_t offset = (node - cbtree_level_first_uncle(node)) * level_block_size;
  return (char*)bookkeeping->arena + offset;
}

size_t sbb_bookkeeping_size(size_t arena_bytes, size_t min_block_size)
{
  if (!ispow2(arena_bytes))
    SBB_FATAL("arena must be a power of two");
  if (!ispow2(min_block_size))
    SBB_FATAL("min_block_size must be a power of two");

	sbb_node_t nnodes, nlevels;
  cbtree_metrics(sbb_node_t(arena_bytes / min_block_size), &nnodes, &nlevels);
  sbb_node_t npages = SBB_MAX(1, nnodes >> kPageBitsLog2);
  
  return npages * sizeof(sbb_page_t) + sizeof(sbb_bookkeeping_t);
}

sbb_t sbb_create(void* arena, size_t arena_bytes, size_t min_block_size, void* bookkeeping)
{
  sbb_bookkeeping_t* sbb = (sbb_bookkeeping_t*)bookkeeping;
  
	// it's ok to manage a null pointer if the desire is to return offsets
	// rather than true pointers.

  if (!bookkeeping)
    SBB_FATAL("a valid bookkeeping memory must be specified");
  if (!ispow2(arena_bytes))
    SBB_FATAL("arena must be a power of two");
  if (!ispow2(min_block_size))
    SBB_FATAL("min_block_size must be a power of two");

	const uint32_t min_block_size_log2 = log2i(min_block_size);

  sbb->arena = arena;
  sbb->arena_bytes = arena_bytes;
  sbb->min_block_size = (uint32_t)min_block_size;
	sbb->min_block_size_log2 = min_block_size_log2;
	uint32_t nlevels;
  cbtree_metrics(sbb_node_t(arena_bytes >> sbb->min_block_size_log2), &sbb->num_nodes, &nlevels);
	sbb->unused = 0;
	
	if ((size_t)sbb->min_block_size != min_block_size)
		SBB_FATAL("alignment must be uint32_t");

	const size_t num_pages = SBB_MAX(1, sbb->num_nodes >> kPageBitsLog2);
  
  // the top bit of the first page is a null since the binary tree's 
  // root starts a 1, so mark everything else as available
  sbb_page_t* page = sbb_page(sbb, 0);

  const sbb_page_t* page_end = page + num_pages;
  
  *page++ = ~(sbb_mask(kPageBits-1));
  for (; page < page_end; page++)
    *page = sbb_page_t(-1);

  return sbb;
}

void sbb_destroy(sbb_t sbb)
{
}

static void* sbb_memalign_internal(sbb_bookkeeping_t* bookkeeping, sbb_page_t* node_page, sbb_page_t node_mask, sbb_node_t node, size_t block_size, size_t level_block_size)
{
	// recurse to the best-fit level
	if (block_size < level_block_size)
	{
		auto left = cbtree_left_child(node);
		auto children_page = sbb_page(bookkeeping, left);
		auto left_mask = sbb_mask(sbb_pagebit(left));
		auto right_mask = left_mask >> 1;

		// if this is marked used and both children are free it means this is an allocated node
		// so there's no more memory here.
		if (sbb_used(node_page, node_mask))
		{
			auto both_mask = left_mask | right_mask;
			if ((*children_page & both_mask) == both_mask)
				return nullptr;
		}

		size_t child_level_block_size = level_block_size >> 1;
		void* ptr = sbb_memalign_internal(bookkeeping, children_page, left_mask, left, block_size, child_level_block_size);
		if (!ptr)
			ptr = sbb_memalign_internal(bookkeeping, children_page, right_mask, left + 1, block_size, child_level_block_size);
		
		// as unwinding from a found allocation, unmark parent nodes as wholly free
		if (ptr && sbb_free(node_page, node_mask))
			sbb_mark_used(node_page, node_mask);
		
		return ptr;
	}
	
	if (sbb_free(node_page, node_mask))
	{
		sbb_mark_used(node_page, node_mask);
		return sbb_to_ptr(bookkeeping, node, block_size);
	}

	return nullptr;
}

void* sbb_memalign(sbb_t sbb, size_t align, size_t size)
{
  sbb_bookkeeping_t* bookkeeping = (sbb_bookkeeping_t*)sbb;
  size_t block_size = SBB_MAX(bookkeeping->min_block_size, nextpow2(size));
  size_t level_block_size = bookkeeping->arena_bytes;
  if (block_size > level_block_size)
    return nullptr;

  return sbb_memalign_internal(bookkeeping, (sbb_page_t*)(bookkeeping + 1), kRootMask, kRoot, block_size, level_block_size);
}

void sbb_free(sbb_t sbb, void* ptr)
{
	// check validity of ptr
	if (!ptr)
		return;

  sbb_bookkeeping_t* bookkeeping = (sbb_bookkeeping_t*)sbb;
  void* arena = bookkeeping->arena;
  const size_t arena_size = bookkeeping->arena_bytes;
	
  if (ptr < arena || ptr >= ((char*)arena + arena_size))
    SBB_FATAL("ptr is not from this sbb allocator");

	// find the leaf node representation of the ptr. Without the block size we can't know
	// what node represents the allocation, so start at the highest granularity and walk
	// up the tree until a block is marked as used. When an internal node is marked as 
	// used and all children are free, it means that internal node is the full allocation
	// for this pointer.
	const sbb_node_t num_nodes = bookkeeping->num_nodes;
	const uint32_t min_block_size_log2 = bookkeeping->min_block_size_log2;
	const size_t arena_block_count = arena_size >> min_block_size_log2;
	// convert to an offset in nodes (i.e. # min_block_size's)
	const size_t byte_offset = size_t((char*)ptr - (char*)arena);
	const uint32_t offset = uint32_t(byte_offset >> min_block_size_log2);

	// we have the node of the allocation if it were the min_block_size
	// the ruleset is that a node is 1 if all subchildren are free and 
	// 0 if not, so from the bottom up the first 0 is the allocation.
	auto node = cbtree_node_from_leaf_offset(offset, num_nodes);
	
	// A node marked used while both children are marked free is the allocation node so 
	// Walk up checking this and sibling nodes. If both are free and the parent is used
	// the parent is the allocation node.
	sbb_page_t* node_page = sbb_page(bookkeeping, node);
	int node_bit = sbb_pagebit(node);
  sbb_page_t node_mask = sbb_mask(node_bit);
	while (sbb_free(node_page, node_mask))
	{
		node = cbtree_parent(node);
		node_page = sbb_page(bookkeeping, node);
		node_bit = sbb_pagebit(node);
		node_mask = sbb_mask(node_bit);
	}
	
	// node is the allocation, so flag it as freed. If its buddy is free too
	// then promote it up the ancestry
	sbb_page_t sibling_mask = sbb_mask(node_bit ^ 1);
	
	sbb_mark_free(node_page, node_mask);
	while (sbb_free(node_page, sibling_mask))
	{
		node = cbtree_parent(node);
		node_page = sbb_page(bookkeeping, node);
		node_bit = sbb_pagebit(node);
		node_mask = sbb_mask(node_bit);
		sibling_mask = sbb_mask(node_bit ^ 1);
		sbb_mark_free(node_page, node_mask);
	}
}

// convert a node index to a page + bit index
void* sbb_malloc(sbb_t sbb, size_t size)
{
  return sbb_memalign(sbb, sbb_min_block_size(sbb), size);
}

void* sbb_realloc(sbb_t sbb, void* ptr, size_t bytes)
{
  // what is the size of the current alloc? I guess use free()'s algo
  // to find what block it is.
  
  // If bytes > blocks and sibling available && (recurse this to size)
  // then find, mark all that as used everywhere in the nodes. (PITA!)
  
  // If bytes < blocks then find the level at which the new size fits,
  // mark sibling and all its children available.

	SBB_FATAL("not yet implemented");
}

void* sbb_arena(sbb_t sbb)
{
	return ((sbb_bookkeeping_t*)sbb)->arena;
}

size_t sbb_arena_bytes(sbb_t sbb)
{
	return ((sbb_bookkeeping_t*)sbb)->arena_bytes;
}

void* sbb_bookkeeping(sbb_t sbb)
{
	return sbb;
}

size_t sbb_block_size(sbb_t sbb, void* ptr)
{
	// check validity of ptr
	if (!ptr)
		return 0;

  sbb_bookkeeping_t* bookkeeping = (sbb_bookkeeping_t*)sbb;
  void* arena = bookkeeping->arena;
  const size_t arena_size = bookkeeping->arena_bytes;
	
  if (ptr < arena || ptr >= ((char*)arena + arena_size))
    SBB_FATAL("ptr is not from this sbb allocator");

	// find the leaf node representation of the ptr. Without the block size we can't know
	// what node represents the allocation, so start at the highest granularity and walk
	// up the tree until a block is marked as used. When an internal node is marked as 
	// used and all children are free, it means that internal node is the full allocation
	// for this pointer.
	const sbb_node_t num_nodes = bookkeeping->num_nodes;
	const uint32_t min_block_size_log2 = bookkeeping->min_block_size_log2;
	const size_t arena_block_count = arena_size >> min_block_size_log2;
	// convert to an offset in nodes (i.e. # min_block_size's)
	const size_t byte_offset = size_t((char*)ptr - (char*)arena);
	const uint32_t offset = uint32_t(byte_offset >> min_block_size_log2);

	// we have the node of the allocation if it were the min_block_size
	// the ruleset is that a node is 1 if all subchildren are free and 
	// 0 if not, so from the bottom up the first 0 is the allocation.
	auto node = cbtree_node_from_leaf_offset(offset, num_nodes);

	// starting at the leaves, starting with min block size and for all free blocks and
	// parents, double the block size until the allocation node is found.
	size_t block_size = bookkeeping->min_block_size;
	
	// A node marked used while both children are marked free is the allocation node so 
	// Walk up checking this and sibling nodes. If both are free and the parent is used
	// the parent is the allocation node.
	sbb_page_t* node_page = sbb_page(bookkeeping, node);
	int node_bit = sbb_pagebit(node);
  sbb_page_t node_mask = sbb_mask(node_bit);
	while (sbb_free(node_page, node_mask))
	{
		node = cbtree_parent(node);
		node_page = sbb_page(bookkeeping, node);
		node_bit = sbb_pagebit(node);
		node_mask = sbb_mask(node_bit);
		block_size <<= 1;
	}

	return block_size > arena_size ? 0 : block_size;
}

size_t sbb_min_block_size(sbb_t sbb)
{
	return ((sbb_bookkeeping_t*)sbb)->min_block_size;
}

static inline size_t sbb_max_free_block_size_internal(sbb_bookkeeping_t* bookkeeping, sbb_page_t* node_page, sbb_page_t node_mask, sbb_node_t node, size_t level_block_size)
{
	// if this is marked used and both children are free it means this is an allocated node
	// so there's no more memory here.
	if (sbb_free(node_page, node_mask))
		return level_block_size;

	auto left = cbtree_left_child(node);
	auto children_page = sbb_page(bookkeeping, left);
	auto left_mask = sbb_mask(sbb_pagebit(left));
	auto right_mask = left_mask >> 1;

	size_t child_level_block_size = level_block_size >> 1;
	size_t lsize = sbb_max_free_block_size_internal(bookkeeping, children_page, left_mask, left, child_level_block_size);
	size_t rsize = sbb_max_free_block_size_internal(bookkeeping, children_page, right_mask, left + 1, child_level_block_size);

	return SBB_MAX(lsize, rsize);
}

size_t sbb_max_free_block_size(sbb_t sbb)
{
  sbb_bookkeeping_t* bookkeeping = (sbb_bookkeeping_t*)sbb;
	return sbb_max_free_block_size_internal(bookkeeping, (sbb_page_t*)(bookkeeping + 1), kRootMask, kRoot, bookkeeping->arena_bytes);
}

static inline size_t sbb_num_free_blocks_internal(sbb_bookkeeping_t* bookkeeping, sbb_page_t* node_page, sbb_page_t node_mask, sbb_node_t node)
{
	// this means all subnodes are free too making this undivided free block
	if (sbb_free(node_page, node_mask))
		return 1;

	auto left = cbtree_left_child(node);

	// used leaf node
	if (left >= bookkeeping->num_nodes)
		return 0;

	auto children_page = sbb_page(bookkeeping, left);
	auto left_mask = sbb_mask(sbb_pagebit(left));

	return sbb_num_free_blocks_internal(bookkeeping, children_page, left_mask, left) 
		+ sbb_num_free_blocks_internal(bookkeeping, children_page, left_mask >> 1, left + 1);
}

size_t sbb_num_free_blocks(sbb_t sbb)
{
  sbb_bookkeeping_t* bookkeeping = (sbb_bookkeeping_t*)sbb;
	return sbb_num_free_blocks_internal(bookkeeping, (sbb_page_t*)(bookkeeping + 1), kRootMask, kRoot);
}

size_t sbb_overhead(sbb_t sbb)
{
	sbb_bookkeeping_t* bookkeeping = (sbb_bookkeeping_t*)sbb;
	return sbb_bookkeeping_size(bookkeeping->arena_bytes, bookkeeping->min_block_size);
}

void sbb_walk_heap_internal(sbb_bookkeeping_t* bookkeeping, sbb_page_t* node_page, sbb_page_t node_mask, sbb_node_t node, size_t level_block_size, sbb_walker walker, void* user)
{
	int used = 0;

	if (sbb_used(node_page, node_mask))
	{
		// recurse

		auto children_level_block_size = level_block_size >> 1;
		auto left = cbtree_left_child(node);
		auto children_page = sbb_page(bookkeeping, left);
		auto left_mask = sbb_mask(sbb_pagebit(left));
		auto right_mask = left_mask >> 1;
		auto both_mask = left_mask | right_mask;

		if (left >= bookkeeping->num_nodes || (*children_page & both_mask) == both_mask)
			used = 1;
		else
		{
			sbb_walk_heap_internal(bookkeeping, children_page, left_mask, left, children_level_block_size, walker, user);
			sbb_walk_heap_internal(bookkeeping, children_page, right_mask, left + 1, children_level_block_size, walker, user);
			return;
		}
	}

	walker(sbb_to_ptr(bookkeeping, node, level_block_size), level_block_size, used, user);
}

void sbb_walk_heap(sbb_t sbb, sbb_walker walker, void* user)
{
  sbb_bookkeeping_t* bookkeeping = (sbb_bookkeeping_t*)sbb;
	sbb_walk_heap_internal(bookkeeping, (sbb_page_t*)(bookkeeping + 1), kRootMask, kRoot, bookkeeping->arena_bytes, walker, user);
}

