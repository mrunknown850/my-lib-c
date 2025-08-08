#ifndef MYLLOC_H
#define MYLLOC_H

#include <stdbool.h>
#include <stddef.h>

/* Structure for storing metadata of a single my-alloc node
 * NOT CIRCULAR
 */
struct ChunkNode {
  size_t size;
  bool is_free;

  struct ChunkNode *next_chunk;
  struct ChunkNode *prev_chunk;
};

typedef struct ChunkNode *ChunkPtr;

void *MyMalloc(size_t size);
void MyFree(void *ptr);

#endif
