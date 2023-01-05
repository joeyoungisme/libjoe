
#include <string.h>

#include "memory_manager.h"

static size_t prv_mini_block_size(memory_manager *mmr)
{
    return ((size_t)(mmr->struct_size << 1 ));
}

static void prv_insert_block_into_free_list(memory_manager *mmr, BlockLink_t *pxBlockToInsert )
{
    BlockLink_t *pxIterator;
    uint8_t *puc;

	/* Iterate through the list until a block is found that has a higher address
	than the block being inserted. */
	for( pxIterator = &(mmr->start_block); pxIterator->pxNextFreeBlock < pxBlockToInsert; pxIterator = pxIterator->pxNextFreeBlock )
	{
		/* Nothing to do here, just iterate to the right position. */
	}

	/* Do the block being inserted, and the block it is being inserted after
	make a contiguous block of memory? */
	puc = ( uint8_t * ) pxIterator;
	if( ( puc + pxIterator->xBlockSize ) == ( uint8_t * ) pxBlockToInsert )
	{
		pxIterator->xBlockSize += pxBlockToInsert->xBlockSize;
		pxBlockToInsert = pxIterator;
	}

	/* Do the block being inserted, and the block it is being inserted before
	make a contiguous block of memory? */
	puc = ( uint8_t * ) pxBlockToInsert;
	if( ( puc + pxBlockToInsert->xBlockSize ) == ( uint8_t * ) pxIterator->pxNextFreeBlock )
	{
		if( pxIterator->pxNextFreeBlock != (mmr->end_of_list) )
		{
			/* Form one big block from the two blocks. */
			pxBlockToInsert->xBlockSize += pxIterator->pxNextFreeBlock->xBlockSize;
			pxBlockToInsert->pxNextFreeBlock = pxIterator->pxNextFreeBlock->pxNextFreeBlock;
		}
		else
		{
			pxBlockToInsert->pxNextFreeBlock = (mmr->end_of_list);
		}
	}
	else
	{
		pxBlockToInsert->pxNextFreeBlock = pxIterator->pxNextFreeBlock;
	}

	/* If the block being inserted plugged a gab, so was merged with the block
	before and the block after, then it's pxNextFreeBlock pointer will have
	already been set, and should not be set here as that would make it point
	to itself. */
	if( pxIterator != pxBlockToInsert )
	{
		pxIterator->pxNextFreeBlock = pxBlockToInsert;
	}
}

static int32_t prv_heap_init(memory_manager *mmr)
{
    if (!mmr || !mmr->buff) {
        return MMR_BUFF_NO_ASSIGN;
    }

    BlockLink_t *pxFirstFreeBlock;
    uint8_t *pucAlignedHeap;
    size_t uxAddress;
    size_t xTotalHeapSize = mmr->buff_size;

	/* Ensure the heap starts on a correctly aligned boundary. */
	// uxAddress = ( size_t ) ucHeap;
	uxAddress = ( size_t ) mmr->buff;

	if( ( uxAddress & mmr->align_mask ) != 0 )
	{
		uxAddress += ( mmr->align_byte - 1 );
		uxAddress &= ~( ( size_t ) mmr->align_mask );
		xTotalHeapSize -= uxAddress - ( size_t ) mmr->buff;
	}

	pucAlignedHeap = ( uint8_t * ) uxAddress;

	/* (mmr->start_block) is used to hold a pointer to the first item in the list of free
	blocks.  The void cast is used to prevent compiler warnings. */
	(mmr->start_block).pxNextFreeBlock = ( void * ) pucAlignedHeap;
	(mmr->start_block).xBlockSize = ( size_t ) 0;

	/* (mmr->end_of_list) is used to mark the end of the list of free blocks and is inserted
	at the end of the heap space. */
	uxAddress = ( ( size_t ) pucAlignedHeap ) + xTotalHeapSize;
	uxAddress -= mmr->struct_size;
	uxAddress &= ~( ( size_t ) mmr->align_mask );
	(mmr->end_of_list) = ( void * ) uxAddress;
	(mmr->end_of_list)->xBlockSize = 0;
	(mmr->end_of_list)->pxNextFreeBlock = NULL;

	/* To start with there is a single free block that is sized to take up the
	entire heap space, minus the space taken by (mmr->end_of_list). */
	pxFirstFreeBlock = ( void * ) pucAlignedHeap;
	pxFirstFreeBlock->xBlockSize = uxAddress - ( size_t ) pxFirstFreeBlock;
	pxFirstFreeBlock->pxNextFreeBlock = (mmr->end_of_list);

	/* Only one block exists - and it covers the entire usable heap space. */
	mmr->mini_byte_left = pxFirstFreeBlock->xBlockSize;
	mmr->byte_left = pxFirstFreeBlock->xBlockSize;

	/* Work out the position of the top bit in a size_t variable. */
	(mmr->block_alloc_bit) = ( ( size_t ) 1 ) << ( ( sizeof( size_t ) * heapBITS_PER_BYTE ) - 1 );
}

int32_t memory_manager_init(memory_manager *mmr)
{
    return MMR_SUCCESS;
}
int32_t memory_manager_destroy(memory_manager *mmr)
{
    return MMR_SUCCESS;
}

// void *pvPortMalloc( size_t allocate_size )
void *memory_manager_allocate(memory_manager *mmr, size_t allocate_size)
{
    BlockLink_t *pxBlock, *pxPreviousBlock, *pxNewBlockLink;
    void *pvReturn = NULL;

	{
		/* If this is the first call to malloc then the heap will require
		initialisation to setup the list of free blocks. */
		if( (mmr->end_of_list) == NULL )
		{
			mmr->heap_init(mmr);
		}
		else
		{
			mtCOVERAGE_TEST_MARKER();
		}

		/* Check the requested block size is not so large that the top bit is
		set.  The top bit of the block size member of the BlockLink_t structure
		is used to determine who owns the block - the application or the
		kernel, so it must be free. */
		if( ( allocate_size & (mmr->block_alloc_bit) ) == 0 )
		{
			/* The wanted size is increased so it can contain a BlockLink_t
			structure in addition to the requested amount of bytes. */
			if( allocate_size > 0 )
			{
				allocate_size += mmr->struct_size;

				/* Ensure that blocks are always aligned to the required number
				of bytes. */
				if( ( allocate_size & mmr->align_mask ) != 0x00 )
				{
					/* Byte alignment required. */
					allocate_size += ( mmr->align_byte - ( allocate_size & mmr->align_mask ) );
					configASSERT( ( allocate_size & mmr->align_mask ) == 0 );
				}
				else
				{
					mtCOVERAGE_TEST_MARKER();
				}
			}
			else
			{
				mtCOVERAGE_TEST_MARKER();
			}

			if( ( allocate_size > 0 ) && ( allocate_size <= mmr->byte_left ) )
			{
				/* Traverse the list from the start	(lowest address) block until
				one	of adequate size is found. */
				pxPreviousBlock = &(mmr->start_block);
				pxBlock = (mmr->start_block).pxNextFreeBlock;
				while( ( pxBlock->xBlockSize < allocate_size ) && ( pxBlock->pxNextFreeBlock != NULL ) )
				{
					pxPreviousBlock = pxBlock;
					pxBlock = pxBlock->pxNextFreeBlock;
				}

				/* If the end marker was reached then a block of adequate size
				was	not found. */
				if( pxBlock != (mmr->end_of_list) )
				{
					/* Return the memory space pointed to - jumping over the
					BlockLink_t structure at its start. */
					pvReturn = ( void * ) ( ( ( uint8_t * ) pxPreviousBlock->pxNextFreeBlock ) + mmr->struct_size );

					/* This block is being returned for use so must be taken out
					of the list of free blocks. */
					pxPreviousBlock->pxNextFreeBlock = pxBlock->pxNextFreeBlock;

					/* If the block is larger than required it can be split into
					two. */
					if( ( pxBlock->xBlockSize - allocate_size ) > mmr->mini_block_size(mmr) )
					{
						/* This block is to be split into two.  Create a new
						block following the number of bytes requested. The void
						cast is used to prevent byte alignment warnings from the
						compiler. */
						pxNewBlockLink = ( void * ) ( ( ( uint8_t * ) pxBlock ) + allocate_size );
						configASSERT( ( ( ( size_t ) pxNewBlockLink ) & mmr->align_mask ) == 0 );

						/* Calculate the sizes of two blocks split from the
						single block. */
						pxNewBlockLink->xBlockSize = pxBlock->xBlockSize - allocate_size;
						pxBlock->xBlockSize = allocate_size;

						/* Insert the new block into the list of free blocks. */
                        mmr->insert_block_into_free_list(mmr, pxNewBlockLink);
					}
					else
					{
						mtCOVERAGE_TEST_MARKER();
					}

					mmr->byte_left -= pxBlock->xBlockSize;

					if( mmr->byte_left < mmr->mini_byte_left )
					{
						mmr->mini_byte_left = mmr->byte_left;
					}
					else
					{
						mtCOVERAGE_TEST_MARKER();
					}

					/* The block is being returned - it is allocated and owned
					by the application and has no "next" block. */
					pxBlock->xBlockSize |= (mmr->block_alloc_bit);
					pxBlock->pxNextFreeBlock = NULL;
				}
				else
				{
					mtCOVERAGE_TEST_MARKER();
				}
			}
			else
			{
				mtCOVERAGE_TEST_MARKER();
			}
		}
		else
		{
			mtCOVERAGE_TEST_MARKER();
		}

		traceMALLOC( pvReturn, allocate_size );
	}

	#if( configUSE_MALLOC_FAILED_HOOK == 1 )
	{
		if( pvReturn == NULL )
		{
			extern void vApplicationMallocFailedHook( void );
			vApplicationMallocFailedHook();
		}
		else
		{
			mtCOVERAGE_TEST_MARKER();
		}
	}
	#endif

	configASSERT( ( ( ( size_t ) pvReturn ) & ( size_t ) mmr->align_mask ) == 0 );
	return pvReturn;
}

// void vPortFree( void *pv )
int32_t memory_manager_free(memory_manager *mmr, void *pv)
{
    uint8_t *puc = ( uint8_t * ) pv;
    BlockLink_t *pxLink;

	if( pv != NULL )
	{
		/* The memory being freed will have an BlockLink_t structure immediately
		before it. */
		puc -= mmr->struct_size;

		/* This casting is to keep the compiler from issuing warnings. */
		pxLink = ( void * ) puc;

		/* Check the block is actually allocated. */
		configASSERT( ( pxLink->xBlockSize & (mmr->block_alloc_bit) ) != 0 );
		configASSERT( pxLink->pxNextFreeBlock == NULL );

		if( ( pxLink->xBlockSize & (mmr->block_alloc_bit) ) != 0 )
		{
			if( pxLink->pxNextFreeBlock == NULL )
			{
				/* The block is being returned to the heap - it is no longer
				allocated. */
				pxLink->xBlockSize &= ~(mmr->block_alloc_bit);

				{
					/* Add this block to the list of free blocks. */
					mmr->byte_left += pxLink->xBlockSize;
					traceFREE( pv, pxLink->xBlockSize );
					mmr->insert_block_into_free_list(mmr, ( ( BlockLink_t * ) pxLink ) );
				}
			}
			else
			{
				mtCOVERAGE_TEST_MARKER();
			}
		}
		else
		{
			mtCOVERAGE_TEST_MARKER();
		}
	}

    return 0;
}

size_t memory_manager_left(memory_manager *mmr)
{
    if (!mmr) {
        return 0;
    }

    return mmr->byte_left;
}

size_t memory_manager_remaining(memory_manager *mmr)
{
    return ((mmr->byte_left * 100) / mmr->buff_size);
}

int32_t memory_manager_static_init(memory_manager *mmr, uint8_t *buff, uint32_t buff_size)
{
    if (!mmr) {
        return -MMR_INVALID_ARGS;
    }

    // assign public method
    mmr->init = memory_manager_init;
    mmr->destroy = memory_manager_destroy;
    mmr->allocate = memory_manager_allocate;
    mmr->free = memory_manager_free;
    mmr->memory_left = memory_manager_left;
    mmr->remaining = memory_manager_remaining;

    // assign private method
    mmr->heap_init = prv_heap_init;
    mmr->insert_block_into_free_list = prv_insert_block_into_free_list;
    mmr->mini_block_size = prv_mini_block_size;

    // member init
    mmr->buff = buff;
    mmr->buff_size = buff_size;

    memset(mmr->buff, 0, mmr->buff_size);

    mmr->align_byte = 8;

    if (mmr->align_byte == 1) {
        mmr->align_mask = 0x0000;
    } else if (mmr->align_byte == 2) {
        mmr->align_mask = 0x0001;
    } else if (mmr->align_byte == 4) {
        mmr->align_mask = 0x0003;
    } else if (mmr->align_byte == 8) {
        mmr->align_mask = 0x0007;
    } else if (mmr->align_byte == 16) {
        mmr->align_mask = 0x000F;
    } else if (mmr->align_byte == 32) {
        mmr->align_mask = 0x001F;
    }

    mmr->struct_size = (sizeof(BlockLink_t) + ((size_t)(mmr->align_byte - 1)));
    mmr->struct_size &= ~((size_t)mmr->align_mask);

    mmr->byte_left = 0U;
    mmr->mini_byte_left = 0U;

    memset(&mmr->start_block, 0, sizeof(BlockLink_t));
    mmr->end_of_list = NULL;

    mmr->block_alloc_bit = 0;

    return MMR_SUCCESS;
}

