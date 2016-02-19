/*****************************************************************************
 * life.c
 * The original sequential implementation resides here.
 * Do not modify this file, but you are encouraged to borrow from it
 ****************************************************************************/
#include "life.h"
#include "util.h"
#include <pthread.h>

/*
 * Four threads will be used to divide up the workload for each cycle of the game for input > 32x32.
 * Neighbours are manually calculated without the need of modulo. Loops are interchanged by processing the rows
 * instead of columns.
 */

/**
 * Swapping the two boards only involves swapping pointers, not
 * copying values.
 */
#define SWAP_BOARDS( b1, b2 )  do { \
  char* temp = b1; \
  b1 = b2; \
  b2 = temp; \
} while(0)

#define BOARD( __board, __i, __j )  (__board[(__i) + LDA*(__j)])

//number of threads used
#define NUM_THREADS 4
void* thread_noswap(void * data);

//struct to hold arguments passing to the thread functions
struct info{
	int nrows;
	int ncols;
	char* inboard;
	char* outboard;
	int block;
};


    char*
sequential_game_of_life (char* outboard,
        char* inboard,
        const int nrows,
        const int ncols,
        const int gens_max)
{
    /* HINT: in the parallel decomposition, LDA may not be equal to
       nrows! */
    const int LDA = nrows;
    int curgen, i, j;

    pthread_t thread[NUM_THREADS];

    //allocate and initialize structures used to pass to the threads
    struct info data[NUM_THREADS];
    for(i=0;i<NUM_THREADS;i++){
		data[i].block = i;
		data[i].ncols = ncols;
		data[i].nrows = nrows;
		data[i].inboard = inboard;
		data[i].outboard = outboard;
    }

    //if the block size is larger than 32x32, use multi-thread mode.
    if(nrows > 32){
		for (curgen = 0; curgen < gens_max; curgen++)
		{
			//spawn 3 new threads
			pthread_create(&thread[1], NULL, thread_noswap, &data[1]);
			pthread_create(&thread[2], NULL, thread_noswap, &data[2]);
			pthread_create(&thread[3], NULL, thread_noswap, &data[3]);
			thread_noswap(&data[0]);

			//wait for all threads to complete
			pthread_join(thread[1], NULL);
			pthread_join(thread[2], NULL);
			pthread_join(thread[3], NULL);

			SWAP_BOARDS( outboard, inboard );

			//update inboard, outboard pointer accordingly
		    for(i=0;i<NUM_THREADS;i++){
				data[i].inboard = inboard;
				data[i].outboard = outboard;
		    }
		}
    }
    else {
    	//use the original algorithm for single thread mode
    	for (curgen = 0; curgen < gens_max; curgen++)
		{
			for (i = 0; i < nrows; i++)
			{
				for (j = 0; j < ncols; j++)
				{
					const int inorth = mod (i-1, nrows);
					const int isouth = mod (i+1, nrows);
					const int jwest = mod (j-1, ncols);
					const int jeast = mod (j+1, ncols);

					const char neighbor_count =
						BOARD (inboard, inorth, jwest) +
						BOARD (inboard, inorth, j) +
						BOARD (inboard, inorth, jeast) +
						BOARD (inboard, i, jwest) +
						BOARD (inboard, i, jeast) +
						BOARD (inboard, isouth, jwest) +
						BOARD (inboard, isouth, j) +
						BOARD (inboard, isouth, jeast);

					BOARD(outboard, i, j) = alivep (neighbor_count, BOARD (inboard, i, j));
				}
			}
			SWAP_BOARDS( outboard, inboard );
		}
    }
	 /*
	  * We return the output board, so that we know which one contains
	  * the final result (because we've been swapping boards around).
	  * Just be careful when you free() the two boards, so that you don't
	  * free the same one twice!!!
	  */
	 return inboard;

}
/*
 * thread_noswap(void* data)
 *
 * This function performs 1 cycle of game of life for a portion of board.
 * The entire board is split into 4 sub blocks. Each thread is
 * responsible for one of the rectangles shown below.
 *
 * 0_________________ n
 * |		0		 |
 * |_________________|
 * |		1 		 |
 * |_________________|
 * |		2		 |
 * |_________________|
 * |		3		 |
 * |_________________|
 * n
 *
 */
void* thread_noswap(void* _data){
	struct info* data = (struct info*)_data;
	int nrows = data->nrows;
	int ncols = data->ncols;
	char* inboard = data->inboard;
	char* outboard = data->outboard;
	int block = data->block;

	const int LDA = nrows;
	int i, j, istart, iend;
	int inorth, isouth, jwest, jeast;

	//calculate the height of each block, everything larger than 32x32 are divisible by 4
	int block_height = nrows/NUM_THREADS;

	//calculate start and end location
	istart = block_height*block;
	iend = block_height*(block+1);

	for(j = 0; j < ncols; j++)
	{
		//manually calculate modulo for corners to address overflow of coordinates.
		if(j==0){
			//at first column
			jwest = ncols-1;
			jeast = (j+1);
		}
		else if (j== ncols-1){
			//at last column
			jwest = (j-1);
			jeast = 0;
		}
		else{
			jwest = (j-1);
			jeast = (j+1);
		}

		for (i = istart; i < iend; i++)
		{
			//manually calculate modulo for corners to address overflow of coordinates.
			if(i==0){
				//at first row
				inorth = nrows-1;
				isouth = (i+1);
			}
			else if(i== nrows-1){
				//at last row
				inorth = (i-1);
				isouth = 0;
			}
			else{
				inorth = (i-1);
				isouth = (i+1);
			}

			const char neighbor_count =
			BOARD (inboard, inorth, jwest) +
			BOARD (inboard, inorth, j) +
			BOARD (inboard, inorth, jeast) +
			BOARD (inboard, i, jwest) +
			BOARD (inboard, i, jeast) +
			BOARD (inboard, isouth, jwest) +
			BOARD (inboard, isouth, j) +
			BOARD (inboard, isouth, jeast);

			BOARD(outboard, i, j) = alivep (neighbor_count, BOARD (inboard, i, j));
		}
	}

}
