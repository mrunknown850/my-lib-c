#include "my_malloc.h"
#include <stdio.h>
#include <sys/mman.h>

ChunkPtr chunk_head = NULL;
const size_t CHUNK_META_SIZE = sizeof(struct ChunkNode);

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

static void *allocate(size_t size) {
  ChunkPtr cur = chunk_head;
  // Traverse through linked list and break at the last chunk
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
  void *new_mmap = mmap(NULL, round_up(&size), PROT_READ | PROT_WRITE,
                        MAP_PRIVATE | MAP_ANON, -1, 0);
  if (new_mmap == (void *)-1)
    return NULL;

  ChunkPtr new_block = (ChunkPtr)(new_mmap);
  new_block->is_free = false;
  new_block->size = size;
  new_block->prev_chunk = cur;
  new_block->next_chunk = NULL;

  cur->next_chunk = new_block;

  ChunkPtr free_block = (ChunkPtr)((char *)cur + CHUNK_META_SIZE + size);
  free_block->size = round_up(&size) - size - CHUNK_META_SIZE;
  free_block->is_free = true;
  free_block->next_chunk = NULL;
  free_block->prev_chunk = new_block;

  new_block->next_chunk = free_block;

  return (new_block + 1);
}

int initialize(size_t _init_size) {
  void *start_addr = mmap(NULL, _init_size, PROT_READ | PROT_WRITE,
                          MAP_PRIVATE | MAP_ANON, -1, 0);

  if (start_addr == (void *)-1)
    return 1;

  // Adding ChunkHead into allocated mem region
  ChunkPtr head_ptr = (ChunkPtr)(start_addr);
  head_ptr->is_free = true;
  head_ptr->size = _init_size - CHUNK_META_SIZE;
  head_ptr->prev_chunk = NULL;
  head_ptr->next_chunk = NULL;

  // Set head
  chunk_head = head_ptr;

  return 0;
}

void *MyMalloc(size_t size) {
  // Initialize ChunkNode linked-lists
  if (chunk_head == NULL) {
    // Round requested mem to the next power of two
    size_t syscall_size = round_up(&size);
    // Return error code
    if (initialize(syscall_size))
      return (void *)-1;
  }
  return allocate(size);
}
