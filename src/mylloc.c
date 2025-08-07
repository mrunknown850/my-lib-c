#include "mylloc.h"
#include <stdio.h>
#include <sys/mman.h>

ChunkPtr chunk_head = NULL;
const size_t CHUNK_SIZE = sizeof(struct ChunkNode);

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
  while (cur != NULL) {
    if (cur->is_free) {
      // In case size fully encapsulate the free chunk,
      // immediately allocate entire chunk
      if (size == cur->size) {
        cur->is_free = false;
        return (cur + 1);
      }
      // In case when the free chunk has enough space to store `Obj`
      // with space to spare for the next block. Do so.
      if (size + CHUNK_SIZE < cur->size) {
        size_t prev_size = cur->size;
        cur->is_free = false;
        cur->size = size;

        ChunkPtr remaining_free = (ChunkPtr)((char *)cur + CHUNK_SIZE + size);
        remaining_free->size = prev_size - size - CHUNK_SIZE;
        remaining_free->is_free = true;
        remaining_free->next_chunk = cur->next_chunk;
        remaining_free->prev_chunk = cur;

        cur->next_chunk = remaining_free;

        return (cur + 1);
      }
    }
    cur = cur->next_chunk;
  }
  return NULL;
}

int initialize(size_t _init_size) {
  void *start_addr = mmap(NULL, _init_size, PROT_READ | PROT_WRITE,
                          MAP_PRIVATE | MAP_ANON, -1, 0);

  if (start_addr == (void *)-1)
    return 1;

  // Adding ChunkHead into allocated mem region
  ChunkPtr head_ptr = (ChunkPtr)start_addr;
  head_ptr->is_free = true;
  head_ptr->size = _init_size - sizeof(*head_ptr);
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
