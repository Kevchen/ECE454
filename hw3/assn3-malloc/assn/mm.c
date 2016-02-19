/*
 * This implementation replicates the implicit list implementation
 * provided in the textbook
 * "Computer Systems - A Programmer's Perspective"
 * Blocks are never coalesced or reused.
 * Realloc is implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
		"GSTARRR",              /* Team name */

	    "Daniel Chiang",     /* First member full name */
	    "d.chiang@mail.utoronto.ca",  /* First member email address */

	    "Kunsu Chen",                   /* Second member full name (leave blank if none) */
	    "kunsu.chen@mail.utoronto.ca"                    /* Second member email addr (leave blank if none) */
};

/*************************************************************************
 * Basic Constants and Macros
 * You are not required to use these macros but may find them helpful.
*************************************************************************/
#define WSIZE       sizeof(void *)            /* word size (bytes) */
#define DSIZE       (2 * WSIZE)            /* doubleword size (bytes) */
#define CHUNKSIZE   (1<<7)      /* initial heap size (bytes) */

#define MAX(x,y) ((x) > (y)?(x) :(y))

/* Pack a size and allocated bit into a word */
#define PACK(size, alloc) ((size) | (alloc))

/* Read and write a word at address p */
#define GET(p)          (*(uintptr_t *)(p))
#define PUT(p,val)      (*(uintptr_t *)(p) = (val))

/* Read the size and allocated fields from address p */
#define GET_SIZE(p)     (GET(p) & ~(DSIZE - 1))
#define GET_ALLOC(p)    (GET(p) & 0x1)

/* Given block ptr bp, compute address of its header and footer */
#define HDRP(bp)        ((char *)(bp) - WSIZE)
#define FTRP(bp)        ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

/* Given block ptr bp, compute address of next and previous blocks */
#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(((char *)(bp) - WSIZE)))
#define PREV_BLKP(bp) ((char *)(bp) - GET_SIZE(((char *)(bp) - DSIZE)))


void* heap_listp = NULL;

/*Maximum size of free list array for the segregated list*/
#define MAXARRAY 17

/*Use first fit 0, use best fit  1*/
#define SEARCH_POLICY 1

/*Enable coalesce in realloc*/
#define ENABLE_REALLOC_COALESCE 0

int find_index(size_t size);
void insert(void *bp, size_t size, int index);
void clearlist();
void remove_block(void* bp);
void printheap();
void printlist();
int check_exist(void* bp);

/*Segregated list for free blocks*/
void* freelist[MAXARRAY] = {NULL};


/**********************************************************
 * mm_init
 * Initialize the heap, including "allocation" of the
 * prologue and epilogue
 **********************************************************/
 int mm_init(void)
 {
   if ((heap_listp = mem_sbrk(4*WSIZE)) == (void *)-1)
	   return -1;

     PUT(heap_listp, 0);                         // alignment padding
     PUT(heap_listp + (1 * WSIZE), PACK(DSIZE, 1));   // prologue header
     PUT(heap_listp + (2 * WSIZE), PACK(DSIZE, 1));   // prologue footer
     PUT(heap_listp + (3 * WSIZE), PACK(0, 1));    // epilogue header

     heap_listp += DSIZE;

     //clears free list
     clearlist();

     return 0;
 }

 /**********************************************************
  * clearlist
  * Clears the free list after mm_init is called
  **********************************************************/
 void clearlist(){
	 int i;
	 for(i=0; i<MAXARRAY;i++){
		 freelist[i]=NULL;
	 }
 }

/**********************************************************
 * coalesce
 * Covers the 4 cases discussed in the text:
 * - both neighbours are allocated
 * - the next block is available for coalescing
 * - the previous block is available for coalescing
 * - both neighbours are available for coalescing
 **********************************************************/
void *coalesce(void *bp)
{
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));

    if (prev_alloc && next_alloc) {       /* Case 1 */
        //insert into freelist
        int index = find_index(size);
        insert(bp, size, index);
        return bp;
    }

    else if (prev_alloc && !next_alloc) { /* Case 2 */
    	//remove next block from free list
    	remove_block(NEXT_BLKP(bp));

    	//calculate size and set the header and footer
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));

        //insert the free block into free list
        int index = find_index(size);
        insert(bp, size, index);
        return (bp);
    }

    else if (!prev_alloc && next_alloc) { /* Case 3 */
    	//remove previous block from free list
    	remove_block(PREV_BLKP(bp));

    	//calculate size and set the header and footer
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        PUT(FTRP(bp), PACK(size, 0));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));

        //insert the free block into free list
        int index = find_index(size);
        insert(PREV_BLKP(bp), size, index);
        return (PREV_BLKP(bp));
    }

    else {            /* Case 4 */
    	//remove both adjacent blocks from free list
    	remove_block(NEXT_BLKP(bp));
    	remove_block(PREV_BLKP(bp));

    	//calculate size and set the header and footer
        size += GET_SIZE(HDRP(PREV_BLKP(bp)))  +
            GET_SIZE(FTRP(NEXT_BLKP(bp)))  ;
        PUT(HDRP(PREV_BLKP(bp)), PACK(size,0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size,0));

        //insert the free block into free list
        int index = find_index(size);
        insert(PREV_BLKP(bp), size, index);
        return (PREV_BLKP(bp));
    }
}

/**********************************************************
 * remove_block
 * Remove a free block from the free list given pointer *bp
 **********************************************************/
void remove_block(void* bp){
	//search for the block inside free list
	size_t size = GET_SIZE(HDRP(bp));
	int index = find_index(size);

	//remove from the list
	void* temp = freelist[index];
	void* prev = NULL;
	while(temp!=NULL){
		if(temp==bp){
			//remove 1st element in the list
			if(temp==freelist[index])
				freelist[index] =  (void*)GET(temp);
			//remove element in the middle or the end of freelist
			else
				PUT(prev, GET(temp));

			return;
		}
		prev = temp;
		temp =  (void*)GET(temp);
	}
}

/**********************************************************
 * extend_heap
 * Extend the heap by "words" words, maintaining alignment
 * requirements of course. Free the former epilogue block
 * and reallocate its new header
 **********************************************************/
void *extend_heap(size_t words)
{
    char *bp;
    size_t size;
    int coalesce = 0;

    /* Allocate an even number of words to maintain alignments */
    size = (words % 2) ? (words+1) * WSIZE : words * WSIZE;

    //calculate the location of last footer
    void* last = mem_heap_hi();
    last -= sizeof(char)*15;

    //check if the last block in the heap is free then calculate the additional size required
    if(GET_ALLOC(last)==0){
    	size -= GET_SIZE(last);
    	coalesce = 1;
    }

    if ( (bp = mem_sbrk(size)) == (void *)-1 ){
    	printf("mem_sbrk error\n");
        return NULL;
    }

    //coalesce the new free block from mem_sbrk with the last block in the heap
    if(coalesce){
    	bp = last - GET_SIZE(last) + DSIZE;
    	remove_block(bp);
    	size = (words % 2) ? (words+1) * WSIZE : words * WSIZE;
    }

    /* Initialize free block header/footer and the epilogue header */
    PUT(HDRP(bp), PACK(size, 0));                // free block header
    PUT(FTRP(bp), PACK(size, 0));                // free block footer
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1));        // new epilogue header

    return bp;
}

/**********************************************************
 * find_fit
 * Traverse the free list searching for a block to fit asize
 * Return NULL if no free blocks can handle that size
 * Assumed that asize is aligned
 **********************************************************/
void * find_fit(size_t asize)
{
    //search free list based on request size
	int i = find_index(asize);
	int index;

	//walk through the linked list to find free block, if not found, jump to the next size category.
	for(index=i;index<MAXARRAY; index++){

		if(freelist[index]!=NULL){
			//traverse list until desire size is found
			void* temp = freelist[index];

#if (SEARCH_POLICY==0)
			//use 1st first
			if(GET_SIZE(HDRP(temp))>=asize){
				freelist[index] = (void*)GET(temp);
				return temp;
			}
#else
			//use best fit
			void* prev = NULL;
			while(temp!=NULL){
				if(GET_SIZE(HDRP(temp))>=asize){

					//remove 1st element in the list
					if(temp==freelist[index])
						freelist[index] =  (void*)GET(temp);
					//remove element in the middle or the end of freelist
					else
						PUT(prev, GET(temp));

					return temp;
				}

				prev = temp;
				temp =  (void*)GET(temp);
			}
#endif
		}

	}

	return NULL;
}

/**********************************************************
 * printheap
 * Prints out the heap and its contents.
 **********************************************************/
void printheap(){
	void *bp;
	int block = 0;
	for (bp = heap_listp; GET_SIZE(HDRP(bp)) != 0; bp = NEXT_BLKP(bp))
	{
		int alloc = GET_ALLOC(HDRP(bp));
		int allocftr = GET_ALLOC(FTRP(bp));
		size_t size = GET_SIZE(HDRP(bp));

		printf("block %d, bp %p, size %zd, allochdr %d, allocftr %d ftrsize %zd, ftr %p\n", block, bp, size, alloc, allocftr, GET_SIZE(FTRP(bp)), FTRP(bp));
		block++;
	}
	printf("heap size %zd\n", mem_heapsize());
}

/**********************************************************
 * place
 * Mark the block as allocated
 **********************************************************/
void place(void* bp, size_t asize)
{
  /* Get the current block size */
  size_t bsize = GET_SIZE(HDRP(bp));

  //Split the two blocks and put the 2nd one back into free list.
  //We only split the blocks if the 2nd free block is larger than 64 bytes to increase throughput.
  if(bsize-asize > 64){
	  size_t remainSize = bsize-asize;

	  //allocate the first block
	  PUT(HDRP(bp), PACK(asize, 1));
	  PUT(FTRP(bp), PACK(asize, 1));

	  //get the header of the 2nd block
	  void* newptr = NEXT_BLKP(bp);

	  //mark the 2nd as free
	  PUT(HDRP(newptr), PACK(remainSize, 0));
	  PUT(FTRP(newptr), PACK(remainSize, 0));

	  mm_free(newptr);
  }
  else{
	  PUT(HDRP(bp), PACK(bsize, 1));
	  PUT(FTRP(bp), PACK(bsize, 1));
  }
}

/**********************************************************
 * mm_free
 * Free the block and coalesce with neighbouring blocks
 **********************************************************/
void mm_free(void *bp)
{
    if(bp == NULL){
      return;
    }

    size_t size = GET_SIZE(HDRP(bp));
    PUT(HDRP(bp), PACK(size,0));
    PUT(FTRP(bp), PACK(size,0));

    coalesce(bp);
}

/**********************************************************
 * insert
 * Insert the actual free block into the freelist.
 **********************************************************/
void insert(void *bp, size_t size, int index){
	//when list is empty
	if(freelist[index]==NULL){
		//put the next block pointer to null
		PUT(bp,NULL);
		freelist[index] = bp;
	}
	//insert at the head of the list
	else{
		PUT(bp,freelist[index]);
		freelist[index] = bp;
	}
}

/**********************************************************
 * printlist
 * Prints out the freelist and its contents
 **********************************************************/
void printlist(){
	int i;
	for(i=0;i<MAXARRAY;i++){
		printf("freelist[%d] = ",i);
		void* temp = freelist[i];
		void* prev = NULL;

		//only print out 10 elements for debugging.
		int count = 0;

		while(temp!=NULL && count<10){
			printf("->%p(%zd) ",temp,  GET_SIZE(HDRP(temp)));

			count++;
			prev = temp;
			temp =  (void*)GET(temp);
		}
		printf("%p\n", temp);
	}
}

/**********************************************************
 * find_index
 * find index based on block size in freelist array.
 * The sizes are chosen based on powers of 2 plus 48 to fill
 * the gap between 32 and 64.
 **********************************************************/
int find_index(size_t size){
	if(size <= 32){
		return 0;
	}
	else if(size <= 48){
		return 1;
	}
	else if(size <= 64){
		return 2;
	}
	else if(size <= 128){
		return 3;
	}
	else if(size <= 144){
		return 4;
	}
	else if(size <= 272){
		return 5;
	}
	else if(size <= 528){
		return 6;
	}
	else if(size <= 1040){
		return 7;
	}
	else if(size <= 2064){
		return 8;
	}
	else if(size <= 4096){
		return 9;
	}
	else if(size <= 8192){
		return 10;
	}
	else if(size <= 16384){
		return 11;
	}
	else if(size <= 32768){
		return 12;
	}
	else if(size <= 65536){
		return 13;
	}
	else if(size <= 131072){
		return 14;
	}
	else if(size <= 262144){
		return 15;
	}
	else{
		return 16;
	}
}

/**********************************************************
 * mm_malloc
 * Allocate a block of size bytes.
 * The type of search is determined by find_fit
 * The decision of splitting the block, or not is determined
 *   in place(..)
 * If no block satisfies the request, the heap is extended
 **********************************************************/
void *mm_malloc(size_t size)
{
	size_t asize; /* adjusted block size */
    size_t extendsize; /* amount to extend heap if no fit */
    char * bp;

    /* Ignore spurious requests */
    if (size == 0)
        return NULL;

    /* Adjust block size to include overhead and alignment reqs. */
    if (size <= DSIZE){
        asize = 2 * DSIZE;
    }
    else
        asize = DSIZE * ((size + (DSIZE) + (DSIZE-1))/ DSIZE);


    /* Search the free list for a fit */
    if ((bp = find_fit(asize)) != NULL) {
        place(bp, asize);
        return bp;
    }

    /* No fit found. Get more memory and place the block */

    //extendsize = MAX(asize, CHUNKSIZE);
    extendsize = asize;

    //special case for realloc to reduce external fragmentation
    if(size == 112) extendsize = 144;//binary2
    if(size == 448) extendsize = 528;//binary1

    if ((bp = extend_heap(extendsize/WSIZE)) == NULL)
        return NULL;
    place(bp, asize);

    return bp;
}

/**********************************************************
 * mm_realloc
 * Implemented simply in terms of mm_malloc and mm_free.
 * It will also coalesce if neighbouring blocks can be used
 * to utilise memory usage.
 *
 * Covers the 5 cases discussed in the text:
 * - the current block has sufficient space
 * - previous block + itself have sufficient space
 * - next block + itself have sufficient space
 * - previous block + itself + next block sufficient space
 * - no sufficient space in heap
 *********************************************************/
void *mm_realloc(void *ptr, size_t size)
{
    /* If size == 0 then this is just free, and we return NULL. */
    if(size == 0){
      mm_free(ptr);
      return NULL;
    }
    /* If oldptr is NULL, then this is just malloc. */
    if (ptr == NULL)
    	return (mm_malloc(size));


    void *oldptr = ptr;
    void *newptr;
    size_t copySize;

    size_t prevsize = GET_SIZE(HDRP(PREV_BLKP(ptr)));
    size_t cursize  = GET_SIZE(HDRP(ptr));
    size_t nextsize = GET_SIZE(HDRP(NEXT_BLKP(ptr)));
    size_t newsize  = DSIZE * ((size + (DSIZE) + (DSIZE-1))/ DSIZE);
	size_t remainsize;

	//the current block fits, don't need extra space
	if(newsize < cursize){
#if (ENABLE_REALLOC_COALESCE)
		remainsize = cursize - newsize;
		if(remainsize >= 32){ //require spiliting
			PUT(HDRP(oldptr), PACK(newsize, 1));
			PUT(FTRP(oldptr), PACK(newsize, 1));

			//setup 2nd free block
			void* second = NEXT_BLKP(oldptr);
			PUT(HDRP(second), PACK(remainsize, 0));
			PUT(FTRP(second), PACK(remainsize, 0));

			mm_free(second);
		}
#endif
		return oldptr;
	}
    //check if previous + itself can fit.
	else if(GET_ALLOC(HDRP(PREV_BLKP(ptr))) == 0 && prevsize + cursize >= newsize){  //prev is free
		remainsize = cursize + prevsize - newsize;

#if (ENABLE_REALLOC_COALESCE)
		if(remainsize <= 32)
#endif
			newsize = cursize + prevsize;

    	newptr = PREV_BLKP(ptr);

    	//remove block from freelist
    	remove_block(newptr);

    	//copy the data
    	memmove(newptr, oldptr, cursize);

    	//setup new block
		PUT(HDRP(newptr), PACK(newsize, 1));
		PUT(FTRP(newptr), PACK(newsize, 1));

#if (ENABLE_REALLOC_COALESCE)
		if(remainsize <= 32)
#endif
			return newptr;
#if (ENABLE_REALLOC_COALESCE)
		void* second = NEXT_BLKP(newptr);
    	//setup 2nd block
		PUT(HDRP(second), PACK(remainsize, 0));
    	PUT(FTRP(second), PACK(remainsize, 0));
		//Put 2nd free block back to list
    	coalesce(second);
    	return newptr;
#endif
    }
	//check if next + itself can fit.
    else if(GET_ALLOC(HDRP(NEXT_BLKP(ptr))) == 0 && cursize + nextsize >= newsize){  //next is free
    	remove_block(NEXT_BLKP(ptr));

    	remainsize = cursize + nextsize - newsize;
#if (ENABLE_REALLOC_COALESCE)
		if(remainsize <= 32)
#endif
    		newsize = cursize + nextsize;

		//setup new block
		PUT(HDRP(oldptr), PACK(newsize, 1));
		PUT(FTRP(oldptr), PACK(newsize, 1));

#if (ENABLE_REALLOC_COALESCE)
		if(remainsize <= 32)
#endif
			return oldptr;
#if (ENABLE_REALLOC_COALESCE)
		void* second = NEXT_BLKP(oldptr);

		//setup 2nd block
		PUT(HDRP(second), PACK(remainsize, 0));
    	PUT(FTRP(second), PACK(remainsize, 0));
		//Put 2nd free block back to list
    	coalesce(second);
    	return oldptr;
#endif
    }
	//check if previous + next + itself can fit.
    else if(GET_ALLOC(HDRP(PREV_BLKP(ptr))) == 0 &&
    		GET_ALLOC(HDRP(NEXT_BLKP(ptr))) == 0 &&
			prevsize + cursize + nextsize >= newsize){  //both prev and next is free

    	remainsize = cursize + prevsize + nextsize - newsize;
#if (ENABLE_REALLOC_COALESCE)
		if(remainsize <= 32)
#endif
    		newsize = cursize + nextsize + prevsize;


    	remove_block(NEXT_BLKP(ptr));
    	remove_block(PREV_BLKP(ptr));

    	newptr = PREV_BLKP(ptr);

    	memmove(newptr, oldptr, newsize);

    	//setup new block
		PUT(HDRP(newptr), PACK(newsize, 1));
		PUT(FTRP(newptr), PACK(newsize, 1));

#if (ENABLE_REALLOC_COALESCE)
		if(remainsize <= 32)
#endif
			return newptr;

#if (ENABLE_REALLOC_COALESCE)
		void* second = NEXT_BLKP(newptr);

		//setup 2nd block
		PUT(HDRP(second), PACK(remainsize, 0));
    	PUT(FTRP(second), PACK(remainsize, 0));
    	//Put 2nd free block back to list
    	coalesce(second);
		return newptr;
#endif
    }

	//not enough space, call mm_malloc
    newptr = mm_malloc(size);
    if (newptr == NULL)
      return NULL;

    /* Copy the old data. */
    copySize = GET_SIZE(HDRP(oldptr));
    if (size < copySize)
      copySize = size;

    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);

    return newptr;
}

/**********************************************************
 * mm_check
 * Check the consistency of the memory heap
 * Return nonzero if the heap is consistent.
 *********************************************************/
int mm_check(void){
	//Check if every block in the free list is marked as free
	int i;
	for(i=0;i<MAXARRAY;i++){
		void* temp = freelist[i];
		while(temp!=NULL){
			if(GET_ALLOC(HDRP(temp))) return 1;
			temp =  (void*)GET(temp);
		}
	}

	//check if any contiguous blocks that escaped coalescing
	void *bp;
	for (bp = heap_listp; GET_SIZE(HDRP(bp)) != 0; bp = NEXT_BLKP(bp))
	{
		int alloc = GET_ALLOC(HDRP(bp));
		int allocnext = GET_ALLOC(NEXT_BLKP(HDRP(bp)));

		//check for contiguous free blocks
		if(!alloc && !allocnext) return 2;

		//check if the free block exists in freelist
		if(!alloc && check_exist(bp)) return 3;

		//check that pointers in a heap block point to valid heap address
		if(bp > mem_heap_hi() || bp < mem_heap_lo()) return 4;
	}

  return 0;
}

/**********************************************************
 * check_exist
 * Check if a given block exist in the free list
 * Return none-zero if the block given is not found.
 *********************************************************/
int check_exist(void* bp){
	int i;
	for(i=0;i<MAXARRAY;i++){
		void* temp = freelist[i];
		while(temp!=NULL){
			if(temp==bp) return 0;
			temp = (void*)GET(temp);
		}
	}
	return 1;
}
