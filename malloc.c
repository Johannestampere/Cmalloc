#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>

struct contiguous {
  struct cnode *first;
  void *upper_limit;
};

struct cnode {
  size_t nsize;
  struct cnode *prev;
  struct cnode *next;
  struct contiguous *block;
};


////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////


// make_contiguous(size) creates a block including a buffer of size
struct contiguous *make_contiguous(size_t size) {

  // malloc size bytes for the block
  void *block = malloc(size);

  // struct contiguous init
  struct contiguous *cont = block;
  cont->first = NULL;
  // make the upper limit point to the first byte after the block

  cont->upper_limit = block + size;

  // assign void pointer to char type, the char block starts from the 
  // first empty byte of the block
  char *char_block = block + sizeof(struct contiguous);

  // iterate through each free byte and make it '$'
  for (size_t i = 0; i < size - sizeof(struct contiguous); i += sizeof(char)) {
    char_block[i] = '$';
  }

  // return pointer to the block
  return cont;
}

//destroy_contiguous(block) cleans up block.
void destroy_contiguous(struct contiguous *block) {
  if (block->first != NULL) {
    printf("Destroying non-empty block!\n");
  }
  free(block);
}

// cfree(p) removes the node for which p points to its data
void cfree(void *p) {
  assert(p);

  char *chunk_addy = p;
  char *node_ptr = chunk_addy - sizeof(struct cnode);

  void *node_start_2 = node_ptr;

  struct cnode *node_2 = node_start_2;

  struct cnode *node_2_prev = node_2->prev;
  struct cnode *node_2_next = node_2->next;
  struct contiguous *block_2 = node_2->block;

  if (node_2_prev != NULL) {
    node_2_prev->next = node_2_next;
  } else {
    block_2->first = node_2_next;  
  }

  if (node_2_next != NULL) {
    node_2_next->prev = node_2_prev;
  }
}


// cmalloc(block, size) make a region inside a block of size bytes, and
// returns a pointer to it.  If there is not enough space, it returns NULL.
void *cmalloc(struct contiguous *block, int size) {
  assert(block);
  assert(size >= 0);

  void *start_of_block = block;
  void *end_of_block = block->upper_limit;

  char *start = start_of_block;
  char *end = end_of_block;

  size_t block_size_in_bytes = end - start;
  size_t free_space_in_empty_block = block_size_in_bytes - sizeof(struct contiguous);

  int int_free_space_in_empty_block = free_space_in_empty_block;

  // check if there would even be enough space in the empty block
  if (int_free_space_in_empty_block < (size + sizeof(struct cnode))) {
    return NULL;
  }

  // if cont->first == NULL -> make the node and chunk right
  // below the cont

  if (block->first == NULL) {
    // address of start of block - start_of_block
    // we need to make the address of the next node the next free byte
    // after the struct contiguous
    char *start = start_of_block;

    char *first_free_byte = start + sizeof(struct contiguous);
    void *free_ptr = first_free_byte;
    struct cnode *new = free_ptr;

    new->nsize = size;
    new->block = block;
    new->prev = NULL;
    new->next = NULL;

    block->first = new;
  
    return free_ptr + sizeof(struct cnode);
  }

  // start the main func
  struct cnode *node = block->first; 

  while (node->next != NULL) {
    // calculate the address of the end of the chunk of cur node
    void *temp = node;
    char *start_of_cur_block = temp;
    char *end_of_cur_chunk = start_of_cur_block + sizeof(struct cnode) + node->nsize;

    // calculate the address of the start of the next block
    void *temp2 = node->next;
    char *start_of_next_block = temp2;

    // calculate if there is enough room between the end of the chunk
    // of the current node and the start of the next node.
    //    if yes, place the new node to the end of the current node's 
    //        chunk, (change pointers) and return that address
    //    if no, move on to the next node

    int gap = start_of_next_block - end_of_cur_chunk;

    int size_of_new_node_and_chunk = sizeof(struct cnode) + size;

    if (gap >= size_of_new_node_and_chunk) {
      void *start_of_new_node = end_of_cur_chunk;
      struct cnode *new_node = start_of_new_node;

      struct cnode *original_next = node->next;

      // two pointer switches
      node->next = new_node;

      new_node->nsize = size;
      new_node->prev = node;
      new_node->block = block;
      new_node->next = original_next;

      if (original_next != NULL) {
        original_next->prev = new_node;
      }

      return start_of_new_node + sizeof(struct cnode);
    } else {
        node = node->next;
    }
  }

  // handle the last node
  // if the gap between the last node's chunk's end and the upper limit 
  // of the block, place the node there, else return NULL
  void *temp3 = node;
  char *start_of_last_block = temp3;
  char *end_of_cur_chunk = start_of_last_block + sizeof(struct cnode) + node->nsize;
  char *end_of_block_char = end_of_block;

  int room_between_last_node_and_end_of_block = end_of_block_char - end_of_cur_chunk;
  int size_needed = sizeof(struct cnode) + size;

  if (room_between_last_node_and_end_of_block >= size_needed) {
    void *start_of_new_node = end_of_cur_chunk;
    struct cnode *new_node = start_of_new_node;

    node->next = new_node;
    new_node->nsize = size;
    new_node->prev = node;
    new_node->next = NULL;
    new_node->block = block;

    return start_of_new_node + sizeof(struct cnode);
  }

  return NULL;
}
