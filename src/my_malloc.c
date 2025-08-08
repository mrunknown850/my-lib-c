#include "my_malloc.h"
#include <stdio.h>
#include <sys/mman.h>

ChunkPtr chunk_head = NULL;
const size_t CHUNK_META_SIZE = sizeof(struct ChunkNode);

/**
 * @brief Rouding up to next power of two
 *
 * @param _t Number to round up to
 * @return Number after rounding
 */
static size_t round_up(const size_t *_t) {
  size_t ret = *_t;
  ret -= 1;
  ret |= ret >> 1;
  ret |= ret >> 2;
  ret |= ret >> 4;
  ret |= ret >> 8;
  ret |= ret >> 16;
  ret += 1;
  return ret;
}

/**
 * @brief Allocating block of memory through sys/mmap(...)
 *
 * @param _init_size Size available after mmap
 */
static void *allocate_block(size_t _init_size) {
  void *addr = mmap(NULL, _init_size + CHUNK_META_SIZE, PROT_READ | PROT_WRITE,
                    MAP_ANON | MAP_PRIVATE, -1, 0);
  ChunkPtr chunk = (ChunkPtr)(addr);
  chunk->is_free = true;
  chunk->size = _init_size;
  return addr;
}

/**
 * @brief Allocating chunk
 *
 * @param size Allocation size
 */
static void *allocate(size_t size) {
  ChunkPtr cur = chunk_head;

  // Cycle through chunks
  while (true) {
    if (cur->is_free) {
      // In case size fully encapsulate the free chunk,
      // immediately allocate entire chunk
      if (size == cur->size) {
        cur->is_free = false;
        return (cur + 1);
      }
      // In case when the free chunk has enough space to store `Obj`
      // with space to spare for the next block. Do so.
      if (size + CHUNK_META_SIZE < cur->size) {
        size_t prev_size = cur->size;
        cur->is_free = false;
        cur->size = size;

        ChunkPtr remaining_free =
            (ChunkPtr)((char *)cur + CHUNK_META_SIZE + size);
        remaining_free->size = prev_size - size - CHUNK_META_SIZE;
        remaining_free->is_free = true;
        remaining_free->next_chunk = cur->next_chunk;
        remaining_free->prev_chunk = cur;

        cur->next_chunk = remaining_free;

        return (cur + 1);
      }
    }

    if (cur->next_chunk == NULL)
      break;
    cur = cur->next_chunk;
  }

  // Current mmap block is fully filled. Ask system for more
  ChunkPtr new_block = (ChunkPtr)allocate_block(round_up(&size));
  new_block->prev_chunk = cur;
  new_block->next_chunk = NULL;
  cur->next_chunk = new_block;

  return allocate(size);
}

/**
 * @brief Basic heap memory allocation
 *
 * @param size Allocation size
 */
void *MyMalloc(size_t size) {
  // Initialize ChunkNode linked-lists
  if (chunk_head == NULL) {
    ChunkPtr head = allocate_block(round_up(&size));
    chunk_head = head;
  }
  return allocate(size);
}

void MyFree(void *_ptr) {}
