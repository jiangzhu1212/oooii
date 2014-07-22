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

static const sbb_page_t kRootMask = 1ull << (kPageBits - 2);

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
static inline int sbb_pagebit(sbb_bookkeeping_t* bookkeeping, sbb_node_t node)
{
	return kPageMaxBits - (node & kPageBitMask);
}

// returns the node's bit page as well as the bitmask to where the node is
static inline sbb_page_t* sbb_page(sbb_bookkeeping_t* bookkeeping, sbb_node_t node, sbb_page_t* out_bitmask)
{
  int bit = sbb_pagebit(bookkeeping, node);
  *out_bitmask = sbb_page_t(1) << bit;
  return sbb_page(bookkeeping, node);
}

// returns the node's bit page as well as the bitmask to where the node is and another bitmask to its sibling node
static inline sbb_page_t* sbb_page(sbb_bookkeeping_t* bookkeeping, sbb_node_t node, sbb_page_t* out_bitmask, sbb_page_t* out_sibling_mask)
{
  int bit = sbb_pagebit(bookkeeping, node);
  int sibling_bit = bit ^ 1;
  *out_bitmask = sbb_page_t(1) << bit;
  *out_sibling_mask = sbb_page_t(1) << sibling_bit;
  return sbb_page(bookkeeping, node);
}

// returns the node's children's bit page (always the same page for both) as well as a bitmask for both children
static inline sbb_page_t* sbb_children_page(sbb_bookkeeping_t* bookkeeping, sbb_node_t node, sbb_page_t* out_childrenmask)
{
	uint32_t left = cbtree_left_child(node);
	if (left >= bookkeeping->num_nodes)
		return nullptr;
	int bit = sbb_pagebit(bookkeeping, left);
	*out_childrenmask = (sbb_page_t(1)<<bit) | (sbb_page_t(1)<<(bit-1));
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
  
  *page++ = ~(sbb_page_t(1)<<(kPageBits-1));
  for (; page < page_end; page++)
    *page = sbb_page_t(-1);

  return sbb;
}

void sbb_destroy(sbb_t sbb)
{
}

static void* sbb_memalign_internal(sbb_bookkeeping_t* bookkeeping, sbb_node_t node, size_t block_size, size_t level_block_size)
{
	// recurse to the best-fit level
	if (block_size < level_block_size)
	{
		// if this is marked used and both children are free it means this is an allocated node
		// so there's no more memory here.
		sbb_page_t mask;
		auto page = sbb_page(bookkeeping, node, &mask);
		if (sbb_used(page, mask))
		{
			sbb_page_t children_mask;
			auto children_page = sbb_children_page(bookkeeping, node, &children_mask);
			if ((*children_page & children_mask) == children_mask)
				return nullptr;
		}

		size_t child_level_block_size = level_block_size >> 1;
		sbb_node_t left = cbtree_left_child(node);
		void* ptr = sbb_memalign_internal(bookkeeping, left, block_size, child_level_block_size);
		if (!ptr)
			ptr = sbb_memalign_internal(bookkeeping, left + 1, block_size, child_level_block_size);
		
		// as unwinding from a found allocation, unmark parent nodes as wholly free
		if (ptr)
			sbb_mark_used(page, mask);
		
		return ptr;
	}
	
	sbb_page_t mask;
	auto page = sbb_page(bookkeeping, node, &mask);
	if (sbb_free(page, mask))
	{
		sbb_mark_used(page, mask);
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
  return sbb_memalign_internal(bookkeeping, 1, block_size, level_block_size);
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
	uint32_t offset = uint32_t(byte_offset >> min_block_size_log2);

	// we have the node of the allocation if it were the min_block_size
	// the ruleset is that a node is 1 if all subchildren are free and 
	// 0 if not, so from the bottom up the first 0 is the allocation.
	uint32_t node = cbtree_node_from_leaf_offset(offset, num_nodes);

	// walk up to the first used node (NOTE: we don't need to calc the buddy_mask every iter here,
	// just once at the end, so consider optimizing this by hoisting the bit-shift out of sbb_page.
	sbb_page_t mask, buddy_mask;
	auto page = sbb_page(bookkeeping, node, &mask, &buddy_mask);
	while (sbb_free(page, mask))
	{
		node = cbtree_parent(node);
		page = sbb_page(bookkeeping, node, &mask, &buddy_mask);
	}
	
	// node is the allocation, so flag it as freed. If its buddy is free too
	// then promote it up the ancestry
	
	sbb_mark_free(page, mask);
	while (sbb_free(page, buddy_mask))
	{
		node = cbtree_parent(node);
		page = sbb_page(bookkeeping, node, &mask, &buddy_mask);
		sbb_mark_free(page, mask);
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
	// this algorithm is the same algorithm as free: it just doesn't modify anything
	// and keeps an extra recording of the block size

  if (!ptr)
    return 0;
   
  sbb_bookkeeping_t* bookkeeping = (sbb_bookkeeping_t*)sbb;

  void* arena = bookkeeping->arena;
  const size_t arena_size = bookkeeping->arena_bytes;
	const sbb_node_t num_nodes = bookkeeping->num_nodes;
	const uint32_t min_block_size_log2 = bookkeeping->min_block_size_log2;
	const size_t arena_block_count = arena_size >> min_block_size_log2;
	
	// ptr is not from this allocator
  if (ptr < arena || ptr >= ((char*)arena + arena_size))
    SBB_FATAL("ptr is not from this sbb allocator");
  
	// convert to an offset in nodes (i.e. # min_block_size's)
	const size_t byte_offset = size_t((char*)ptr - (char*)arena);
	size_t offset = byte_offset >> min_block_size_log2;

	// set up a mask for traversing the Morton number interpretation of the offset:
	size_t mask = arena_block_count >> 1;

	size_t block_size = arena_size;

	// start at the root
	sbb_node_t node = 1;
	while (mask)
	{
		sbb_page_t node_mask;
		auto node_page = sbb_page(bookkeeping, node, &node_mask);

		if (sbb_used(node_page, node_mask))
		{
			if (node < num_nodes)
			{
				sbb_page_t children_mask;
				auto children_page = sbb_children_page(bookkeeping, node, &children_mask);

				if (children_page && (*children_page & children_mask) == children_mask)
					return block_size;

				#ifdef oDEBUG
				else if ((*children_page & children_mask) != 0)
					SBB_FATAL("corrupt heap b010 or b001");
				#endif
			}
		}

		#ifdef oDEBUG
		else
		{
			sbb_page_t children_mask;
			auto children_page = sbb_children_page(bookkeeping, node, &children_mask);
			if ((*children_page & children_mask) == 0)
				SBB_FATAL("corrupt heap b100");
		}
		#endif

		block_size >>= 1;

		// move to the left or right node
		node += node + (offset & mask) ? 1 : 0;
		mask >>= 1;
	}

	SBB_FATAL("pointer not allocated (must be dangling)");
}

size_t sbb_min_block_size(sbb_t sbb)
{
	return ((sbb_bookkeeping_t*)sbb)->min_block_size;
}

size_t sbb_max_free_block_size(sbb_t sbb)
{
	// find the first node that is marked free and both children are marked free

	sbb_bookkeeping_t* bookkeeping = (sbb_bookkeeping_t*)sbb;

	size_t block_size = bookkeeping->arena_bytes;
	sbb_node_t node = 1; // start at root
	sbb_page_t mask, children_mask, children_flags, children_flags_test;
	sbb_page_t* page, *children_page;
	while (true)
	{
		page = sbb_page(bookkeeping, node, &mask);
		if (sbb_used(page, mask))
			break;

		children_page = sbb_children_page(bookkeeping, node, &children_mask);

		// at leaves
		if (!children_page)
			return block_size;
		
		children_flags = *children_page & children_mask;
		
		if (children_flags == children_mask)
			return block_size;
		else if (children_flags == 0)
			SBB_FATAL("heap corrupt");

		// ok we've culled the both-free and both-used cases, so which one is marked free?
		children_flags_test = children_flags ^ children_mask;
		if (children_flags_test > children_flags) // this means the right is available
			node = cbtree_right_child(node);
		else // left is available
			node = cbtree_left_child(node);

		block_size >>= 1;
	}

	return 0;
}

static inline size_t sbb_num_free_blocks_recursive(sbb_bookkeeping_t* bookkeeping, sbb_node_t node)
{
	sbb_page_t mask;
	auto page = sbb_page(bookkeeping, node, &mask);
	if (sbb_used(page, mask))
		return 0;
	sbb_page_t children_mask;
	auto children_page = sbb_children_page(bookkeeping, node, &children_mask);
	if (children_page && *children_page & children_mask)
		return sbb_num_free_blocks_recursive(bookkeeping, cbtree_left_child(node)) + sbb_num_free_blocks_recursive(bookkeeping, cbtree_right_child(node));
	return 1; // unused leaf node counts as free block
}

size_t sbb_num_free_blocks(sbb_t sbb)
{
	return sbb_num_free_blocks_recursive((sbb_bookkeeping_t*)sbb, 1);
}

size_t sbb_overhead(sbb_t sbb)
{
	sbb_bookkeeping_t* bookkeeping = (sbb_bookkeeping_t*)sbb;
	return sbb_bookkeeping_size(bookkeeping->arena_bytes, bookkeeping->min_block_size);
}

void sbb_walk_heap(sbb_t sbb, sbb_walker walker, void* user)
{
	SBB_FATAL("not yet implemented");
	/*
	NLR
	000 hint, suballoctions exist, recurse
	001 invalid
	010 invalid
	011 this is a used allocation, call walker
	100 invalid
	101 recurse both
	110 recurse both
	111 
	*/
}
