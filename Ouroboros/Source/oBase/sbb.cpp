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
	// todo: store log2i of min_block_size, log2i num_nodes, num_levels all in one word

  void* arena;
  size_t arena_bytes; // must be pow2
  size_t min_block_size; // must be pow2
  sbb_page_t* pages;
  sbb_node_t num_levels;
  sbb_node_t num_nodes;
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
	*out_childrenmask = bit | (bit>>1);
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
  {
    SBB_FATAL("arena must be a power of two");
  }
    
  if (!ispow2(min_block_size))
  {
    SBB_FATAL("min_block_size must be a power of two");
  }

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
  {
    SBB_FATAL("a valid bookkeeping memory must be specified");
  }
    
  if (!ispow2(arena_bytes))
  {
    SBB_FATAL("arena must be a power of two");
  }
    
  if (!ispow2(min_block_size))
  {
    SBB_FATAL("min_block_size must be a power of two");
  }

  sbb->arena = arena;
  sbb->arena_bytes = arena_bytes;
  sbb->min_block_size = min_block_size;
  sbb->pages = (sbb_page_t*)(sbb + 1);
  cbtree_metrics(sbb_node_t(arena_bytes / min_block_size), &sbb->num_nodes, &sbb->num_levels);
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

static void* sbb_allocate_recursive(sbb_bookkeeping_t* bookkeeping, sbb_node_t node, size_t size, size_t level_block_size, bool* out_mark_parent)
{
  sbb_page_t mask, sibling_mask;
  auto page = sbb_page(bookkeeping, node, &mask, &sibling_mask);

  // nodes are marked as used if ALL children nodes are used, so either
  // this was allocated and all child nodes are still free (1) or all
  // children are used (0).
  if (sbb_used(page, mask))
    return nullptr;
  
  // the block is available: if it's the best-fit mark it used and return
  if (size == level_block_size)
  {
		 // if this will be used then if its sibling is also used mark the parent as used as well
		*out_mark_parent = sbb_used(page, sibling_mask);
  
		sbb_mark_used(page, mask);
    return sbb_to_ptr(bookkeeping, node, level_block_size);
  }
  
  // recurse on children: try left and if not try right
  size_t child_level_block_size = level_block_size >> 1;
  bool mark_parent;
  int child = cbtree_left_child(node);
  void* ptr = sbb_allocate_recursive(bookkeeping, child, size, child_level_block_size, &mark_parent);
  if (!ptr)
    ptr = sbb_allocate_recursive(bookkeeping, child + 1, size, child_level_block_size, &mark_parent);
  
  // if allocation was successful and sibling was also used, flag parent
  // (this) as not having either child available.
  if (ptr && mark_parent)
	{
		 // if this is now used then if its sibling is also used mark the parent as used as well
		*out_mark_parent = sbb_used(page, sibling_mask);
		sbb_mark_used(page, mask);
	}

	else
	{
		// either this is a nullptr or not to mark the parent, so don't propagate
		*out_mark_parent = false;
	}

  return ptr;
}

void* sbb_memalign(sbb_t sbb, size_t align, size_t size)
{
  sbb_bookkeeping_t* bookkeeping = (sbb_bookkeeping_t*)sbb;
  size_t block_size = SBB_MAX(bookkeeping->min_block_size, nextpow2(size));
  size_t level_block_size = bookkeeping->arena_bytes;
  if (block_size > level_block_size)
    return nullptr;
  bool dummy; // no root sibling
  return sbb_allocate_recursive(bookkeeping, 1, block_size, level_block_size, &dummy);
}

void sbb_free(sbb_t sbb, void* ptr)
{
  if (!ptr)
    return;
   
  sbb_bookkeeping_t* bookkeeping = (sbb_bookkeeping_t*)sbb;

  void* arena = bookkeeping->arena;
  size_t arena_size = bookkeeping->arena_bytes;

	size_t arena_block_count = arena_size / bookkeeping->min_block_size; // do a log2 or constant thing
	
	// ptr is not from this allocator
  if (ptr < arena || ptr >= ((char*)arena + arena_size))
    SBB_FATAL("ptr is not from this sbb allocator");
  
	// convert to an offset in nodes (i.e. # min_block_size's)
	size_t byte_offset = size_t((char*)ptr - (char*)arena);
	size_t offset = byte_offset / bookkeeping->min_block_size; // change to a log2 thing

	// set up a mask for traversing the Morton number interpretation of the offce:
	size_t mask = arena_block_count >> 1;

	// start at the root
	sbb_node_t node = 1;
	while (mask)
	{
		// truth table for the state of a node and its children 
		// (Node LeftChild RightChild nodes):
		// NLR State desc 
		// 000 node hints that all sub-nodes are separately used but this itself is not an allocation
		// 001 invalid/corrupt heap: node says all subnodes are used but one is free
		// 010 invalid/corrupt heap: node says all subnodes are used but one is free
		// 011 node is an allocation and thus represents all its subnodes and can be freed
		// 100 invalid/corrupt heap: node says it is part of a free path but both children are used
		// 101 node hints that at least one subnode is available
		// 110 node hints that at least one subnode is available
		// 111 node hints that at least one subnode is available
	
		// for the purposes of this function we only care about 011: node is an allocation
		// so we want to grab both children at the same time and test against a 2-bit mask
		// and ensure that this node itself is used

		sbb_page_t node_mask;
		auto node_page = sbb_page(bookkeeping, node, &node_mask);

		if (sbb_used(node_page, node_mask))
		{
			if (node < bookkeeping->num_nodes)
			{
				sbb_page_t children_mask;
				auto children_page = sbb_children_page(bookkeeping, node, &children_mask);

				if (children_page && (*children_page & children_mask) == children_mask)
				{
					// go back up to the root marking the path as available
					do
					{
						sbb_mark_free(node_page, node_mask);
						node = cbtree_parent(node);
					} while (node);

					break;
				}

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

		// move to the left or right node
		node += node + (offset & mask) ? 1 : 0;
		mask >>= 1;
	}
}

// convert a node index to a page + bit index
void* sbb_malloc(sbb_t sbb, size_t size)
{
  sbb_bookkeeping_t* bookkeeping = (sbb_bookkeeping_t*)sbb;
  size_t default_alignment = SBB_MAX(16, bookkeeping->arena_bytes >> (bookkeeping->num_levels-1));
  return sbb_memalign(sbb, default_alignment, size);
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

size_t sbb_block_size(void* ptr)
{
	SBB_FATAL("not yet implemented");
}

size_t sbb_min_block_size(sbb_t sbb)
{
	return ((sbb_bookkeeping_t*)sbb)->min_block_size;
}

size_t sbb_max_free_block_size(sbb_t sbb)
{
	SBB_FATAL("not yet implemented");
}

size_t sbb_num_free_blocks(sbb_t sbb)
{
	SBB_FATAL("not yet implemented");
}

size_t sbb_overhead(sbb_t sbb)
{
	sbb_bookkeeping_t* bookkeeping = (sbb_bookkeeping_t*)sbb;
	return sbb_bookkeeping_size(bookkeeping->arena_bytes, bookkeeping->min_block_size);
}

void sbb_walk_pool(sbbpool_t pool, sbb_walker walker, void* user)
{
	SBB_FATAL("not yet implemented");
}
