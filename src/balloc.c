/*
 *  ircd-hybrid: an advanced Internet Relay Chat Daemon(ircd).
 *  balloc.c: The ircd block allocator.
 *
 *  Copyright (C) 2002 by the past and present ircd coders, and others.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307
 *  USA
 *
 *   $Id: balloc.c,v 1.5 2002/01/06 09:20:35 a1kmm Exp $
 */

/* A note on the algorithm:
 *  - Unlike past block allocators, this one uses a single linked list
 *    for block segments. This makes garbage collection much slower but
 *    should drastically increase allocation and de-allocation times.
 *    This is okay, because garbage collection occurs much less
 *    frequently than allocation and de-allocation.
 * The following table is a rough guideline only...
 * n = number of blocks, m = elements per block.
 *|     Action      |  Estimated rel. freq. | Worst case bit complexity
 *|   Heap setup    |             1         |       O(1) + block alloc
 *|Block allocation |            10^3       |       O(n)
 *|Element allocate |            10^6       |       O(1)
 *|  Element free   |            10^6       |       O(1)
 *| Garbage collect |            10^3       |       O(nm)
 * - A1kmm
 */

/* 
 * About the block allocator
 *
 * Basically we have three ways of getting memory off of the operating
 * system. Below are this list of methods and the order of preference.
 *
 * 1. mmap() anonymous pages with the MMAP_ANON flag.
 * 2. mmap() via the /dev/zero trick.
 * 3. malloc() 
 *
 * The advantages of 1 and 2 are this.  We can munmap() the pages which will
 * return the pages back to the operating system, thus reducing the size 
 * of the process as the memory is unused.  malloc() on many systems just keeps
 * a heap of memory to itself, which never gets given back to the OS, except on
 * exit.  This of course is bad, if say we have an event that causes us to allocate
 * say, 200MB of memory, while our normal memory consumption would be 15MB.  In the
 * malloc() case, the amount of memory allocated to our process never goes down, as
 * malloc() has it locked up in its heap.  With the mmap() method, we can munmap()
 * the block and return it back to the OS, thus causing our memory consumption to go
 * down after we no longer need it.
 * 
 * Of course it is up to the caller to make sure BlockHeapGarbageCollect() gets
 * called periodically to do this cleanup, otherwise you'll keep the memory in the
 * process.
 *
 * Comment and system allocation interface by Androsyn.
 */

#include "memory.h"
#include "common.h"
#include <assert.h>

#ifndef NOBALLOC
/* First some structures. I put them here and not in balloc.h because
 * I didn't want them to be exposed outside this file, and I also don't
 * really want to add another header... -A1kmm
 */

/*
psuedo-struct BHElement
{
 dlink_node elementn;
 char data[elsize];
};
*/

struct BHBlock
{
  dlink_node blockn;
  int usedcount;
  /*
     pseudo-struct BHElement elements[elsperblock];
   */
};

void BlockHeapAddBlock(BlockHeap * bh);

/* --- Begin the system allocation interface --- */
static INLINE void free_block(void *ptr, size_t size);
static INLINE void* get_block(size_t size);

#ifdef HAVE_MMAP /* We've got mmap() that is good */
#include <sys/mman.h>

/*
 * static inline void free_block(void *ptr, size_t size)
 *
 * Inputs: The block and its size
 * Output: None
 * Side Effects: Returns memory for the block back to the OS
 */
static INLINE void
free_block(void *ptr, size_t size)
{
  munmap(ptr, size);
}

#ifndef MAP_ANON /* But we cannot mmap() anonymous pages */
		 /* So we mmap() /dev/zero, which is just as good */
static int zero_fd = -1;

/*
 * void initBlockHeap(void)
 * Note: This is the /dev/zero version of getting pages 
 * 
 * Inputs: None
 * Outputs: None
 * Side Effects: Opens /dev/zero and saves the file handle for
 *		 future allocations.
 */
 
void
initBlockHeap(void)
{
  zero_fd = open("/dev/zero", O_RDWR);

  if (zero_fd < 0)
    outofmemory();
  fd_open(zero_fd, FD_FILE, "Anonymous mmap()");
}

/*
 * static inline void *get_block(size_t size)
 * 
 * Note: This is the /dev/zero version
 * Input: Size of block to allocate
 * Output: Pointer to new block
 * Side Effects: None
 */
static GINLINE void*
get_block(size_t size)
{
  return (mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE, zero_fd, 0));
}
#else /* MAP_ANON */ 

/* 
 * void initBlockHeap(void)
 *
 * Note: This is the anonymous pages version: This is a placeholder
 * Input: None
 * Output: None
 */ 
void
initBlockHeap(void)
{
  return;
}

/*
 * static inline void *get_block(size_t size)
 * 
 * Note: This is the /dev/zero version
 * Input: Size of block to allocate
 * Output: Pointer to new block
 * Side Effects: None
 */

static INLINE void*
get_block(size_t size)
{
  return (mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0));
}

#endif /* MAP_ANON */

#else  /* HAVE_MMAP */
/* Poor os doesn't even have mmap() */

/* 
 * static inline void *get_block(size_t size)
 *
 * Note: This is the non-mmap() version
 * Input: Size of block
 * Output: Pointer to the memory
 */
static INLINE void*
get_block(size_t size)
{
  return malloc(size);
}

/*
 * static inline void free_block(void *ptr, size_t size)
 *
 * Inputs: The block and its size
 * Output: None
 * Side Effects: Returns memory for the block back to the malloc heap
 */

static INLINE void
free_block(void *ptr, size_t unused)
{
  free(ptr);
}

/* 
 * void initBlockHeap(void)
 *
 * Note: This is the malloc() version: This is a placeholder
 * Input: None
 * Output: None
 */ 

void
initBlockHeap(void)
{
}
#endif /* HAVE_MMAP */

/* Add a block to the blockheap... */
void
BlockHeapAddBlock(BlockHeap * bh)
{
  char *d;
  int i;
  struct BHBlock *bhb = get_block(bh->blocksize);
  dlinkAdd(bhb, &bhb->blockn, &bh->blocks);
  d = ((char *)bhb) + sizeof(struct BHBlock);
  bhb->usedcount = 0;
  /* On the front is the best because of memory caches/swap/paging. */
  for (i = 0; i < bh->elsperblock; i++)
  {
    dlinkAdd(bhb, (dlink_node *) d, &bh->f_elements);
    d += sizeof(dlink_node) + bh->elsize;
  }
}

/* Create a blockheap... */
BlockHeap *
BlockHeapCreate(int elsize, int elsperblock)
{
  BlockHeap *bh = MyMalloc(sizeof(*bh));
#ifdef MEMDEBUG
  /* Squeeze in the memory header too... -A1kmm */
  elsize += sizeof(MemoryEntry);
#endif
  memset(bh, 0, 2 * sizeof(dlink_list));
  bh->elsize = elsize;
  bh->elsperblock = elsperblock;
  bh->blocksize = elsperblock * (elsize + sizeof(dlink_node)) +
    sizeof(struct BHBlock);
  return bh;
}

/* Allocate an element from the free pool, making new blocks if needed.
 */
void *
_BlockHeapAlloc(BlockHeap * bh)
{
  char *d;
  if (bh->f_elements.head == NULL)
    BlockHeapAddBlock(bh);
  d = (char *)(bh->f_elements.head + 1);
  ((struct BHBlock *)bh->f_elements.head->data)->usedcount++;
  bh->f_elements.head = bh->f_elements.head->next;
  if (bh->f_elements.head == NULL)
    bh->f_elements.tail = NULL;
  else
    bh->f_elements.head->prev = NULL;
  /* No need to "frob" when debugging here, it is done on initial
   * MyMalloc and after each free. -A1kmm */
  return d;
}

/* Release an element back into the pool... */
void
_BlockHeapFree(BlockHeap * bh, void *el)
{
  dlink_node *dln = (el - sizeof(dlink_node));
#ifdef MEMDEBUG
  mem_frob(el, bh->elsize);
#endif
  ((struct BHBlock *)dln->data)->usedcount--;
  /* On the front is the best because of memory caches/swap/paging. 
   * It also should make garbage collection work better... -A1kmm */
  dlinkAdd(dln->data, dln, &bh->f_elements);
}

/* Destroy a blockheap... */
void
BlockHeapDestroy(BlockHeap * bh)
{
  struct BHBlock *bhb;
  for (bhb = (struct BHBlock *)bh->blocks.head; bhb;
       bhb = (struct BHBlock *)bhb->blockn.next)
    free_block(bhb, bh->blocksize);
  MyFree(bh);
}

/* Destroy empty blocks. Note that this is slow because we put off all
 * processing until this late to save speed in the frequently called
 * routines.
 * Okay, that is not really the case any more, so now the garbage
 * collector doesn't take 10s...
 **/
void
BlockHeapGarbageCollect(BlockHeap * bh)
{
  struct BHBlock *bhb, *bhbn;
  char *d;
  int i;
  for (bhb = (struct BHBlock *)bh->blocks.head; bhb; bhb = bhbn)
  {
    bhbn = (struct BHBlock *)bhb->blockn.next;
    if (bhb->usedcount != 0)
      continue;
    d = (char *)(bhb + 1);
    for (i = 0; i < bh->elsperblock; i++)
    {
      dlinkDelete((dlink_node *) d, &bh->f_elements);
      d += sizeof(dlink_node) + bh->elsize;
    }
    dlinkDelete(&bhb->blockn, &bh->blocks);
    free_block(bhb, bh->blocksize);
  }
}
#endif
