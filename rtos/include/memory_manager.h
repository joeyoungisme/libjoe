#ifndef __MEMORY_MANAGER_H
#define __MEMORY_MANAGER_H

#include <string.h>
#include <stdint.h>

/* Block sizes must not get too small. */
#define heapMINIMUM_BLOCK_SIZE	( ( size_t ) ( xHeapStructSize << 1 ) )

/* Assumes 8bit bytes! */
#define heapBITS_PER_BYTE		( ( size_t ) 8 )

#define mtCOVERAGE_TEST_MARKER()
#define configASSERT(x)

#define traceFREE(addr, size)
#define traceMALLOC(addr, size)

typedef enum {
    MMR_SUCCESS,
    MMR_INVALID_ARGS,
    MMR_BUFF_NO_ASSIGN,
    MMR_ERROR_AMOUNT
} MMR_ERROR_CODE;

/* Define the linked list structure.
 * This is used to link free blocks in order of their memory address. */
typedef struct A_BLOCK_LINK {
	struct A_BLOCK_LINK *pxNextFreeBlock;	/*<< The next free block in the list. */
	size_t xBlockSize;						/*<< The size of the free block. */
} BlockLink_t;

typedef struct _memory_manager {

    // ---- PUBLIC METHOD
    int32_t (*init)(struct _memory_manager *);
    int32_t (*destroy)(struct _memory_manager *);

    void *(*allocate)(struct _memory_manager *, size_t);
    int32_t (*free)(struct _memory_manager *, void *);

    size_t (*memory_left)(struct _memory_manager *);
    size_t (*remaining)(struct _memory_manager *);

    // ---- PRIVATE METHOD
    int32_t (*heap_init)(struct _memory_manager *);
    void (*insert_block_into_free_list)(struct _memory_manager *, BlockLink_t *);
    size_t (*mini_block_size)(struct _memory_manager *);

    // ---- PRIVATE MEMBER

    uint8_t *buff;
    uint32_t buff_size;

    BlockLink_t start_block;
    BlockLink_t *end_of_list;

    size_t align_byte;
    size_t align_mask;
    size_t struct_size;

    size_t byte_left;
    size_t mini_byte_left;
    size_t block_alloc_bit;

} memory_manager;

// memory_manager *memory_manager_new(void);
int32_t memory_manager_static_init(memory_manager *, uint8_t *, uint32_t);

#endif
