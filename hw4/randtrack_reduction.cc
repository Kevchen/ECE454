
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>


#include "defs.h"
#include "hash.h"

#define SAMPLES_TO_COLLECT   	 10000000
#define RAND_NUM_UPPER_BOUND   	   100000
#define NUM_SEED_STREAMS            4

/*
 * ECE454 Students:
 * Please fill in the following team struct
 */
team_t team = {
    "GSTARRR",                  /* Team name */

    "Daniel Chiang",                    /* First member full name */
    "998825534",                 /* First member student number */
    "d.chiang@mail.utoronto.ca",                 /* First member email address */

    "Kunsu Chen",                           /* Second member full name */
    "999069029",                           /* Second member student number */
    "kunsu.chen@mail.utoronto.ca"                            /* Second member email address */
};

unsigned num_threads;
unsigned samples_to_skip;

class sample;

class sample {
  unsigned my_key;
 public:
  sample *next;
  unsigned count;

  sample(unsigned the_key){my_key = the_key; count = 0;};
  unsigned key(){return my_key;}
  void print(FILE *f){printf("%d %d\n",my_key,count);}
};

// This instantiates an empty hash table
// it is a C++ template, which means we define the types for
// the element and key value here: element is "class sample" and
// key value is "unsigned".
hash<sample,unsigned> h[4];


void* thread_function_1_seed(void* seed);
void* thread_function_2_seeds(void* _seed);
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int
main (int argc, char* argv[]){
  int i,j,k;
  int rnum;
  unsigned key;
  sample *s;

  pthread_t thread1, thread2, thread3;

  // Print out team information
  printf( "Team Name: %s\n", team.team );
  printf( "\n" );
  printf( "Student 1 Name: %s\n", team.name1 );
  printf( "Student 1 Student Number: %s\n", team.number1 );
  printf( "Student 1 Email: %s\n", team.email1 );
  printf( "\n" );
  printf( "Student 2 Name: %s\n", team.name2 );
  printf( "Student 2 Student Number: %s\n", team.number2 );
  printf( "Student 2 Email: %s\n", team.email2 );
  printf( "\n" );

  // Parse program arguments
  if (argc != 3){
    printf("Usage: %s <num_threads> <samples_to_skip>\n", argv[0]);
    exit(1);
  }
  sscanf(argv[1], " %d", &num_threads); // not used in this single-threaded version
  sscanf(argv[2], " %d", &samples_to_skip);

  int seed[4] = {0,1,2,3};

  if(num_threads==1){
	  // initialize a 16K-entry (2**14) hash of empty lists
	  h[0].setup(14);

	  // process streams starting with different initial numbers
	    for (i=0; i<NUM_SEED_STREAMS; i++){
	      rnum = i;

	      // collect a number of samples
	      for (j=0; j<SAMPLES_TO_COLLECT; j++){

	        // skip a number of samples
	        for (k=0; k<samples_to_skip; k++){
	  	rnum = rand_r((unsigned int*)&rnum);
	        }

	        // force the sample to be within the range of 0..RAND_NUM_UPPER_BOUND-1
	        key = rnum % RAND_NUM_UPPER_BOUND;

	        // if this sample has not been counted before
	        if (!(s = h[0].lookup(key))){

	  	// insert a new element for it into the hash table
	  	s = new sample(key);
	  	h[0].insert(s);
	        }

	        // increment the count for the sample
	        s->count++;
	      }
	    }

  }
  else if(num_threads==2){
	  pthread_create(&thread1, NULL, thread_function_2_seeds, &seed[2]);
	  thread_function_2_seeds(&seed[0]);

	  //wait for all threads
	  pthread_join(thread1, NULL);

	  //merge hash table h[0],h[2]
	  h[0].merge(&h[2]);
  }
  else if(num_threads==4){
	  pthread_create(&thread1, NULL, thread_function_1_seed, &seed[1]);
	  pthread_create(&thread2, NULL, thread_function_1_seed, &seed[2]);
	  pthread_create(&thread3, NULL, thread_function_1_seed, &seed[3]);
	  thread_function_1_seed(&seed[0]);

	  //wait for all threads
	  pthread_join(thread1, NULL);
	  pthread_join(thread2, NULL);
	  pthread_join(thread3, NULL);

	  //merge hash table h[0],h[1],h[2], h[3]
	  //printf("merging\n");

	  h[0].merge(&h[1]);
	  h[0].merge(&h[2]);
	  h[0].merge(&h[3]);
  }


  // print a list of the frequency of all samples
  h[0].print();
}

void* thread_function_1_seed(void* _seed){
	int seed = *((int *) _seed);
	//printf("1 seed thread %d\n", seed);

	//setup hash table
	h[seed].setup(14);

	int j,k;
	int rnum = seed;
	unsigned key;
	sample *s;

		// collect a number of samples
		for (j=0; j<SAMPLES_TO_COLLECT; j++){

			// skip a number of samples
			for (k=0; k<samples_to_skip; k++){
			  rnum = rand_r((unsigned int*)&rnum);
			}

			// force the sample to be within the range of 0..RAND_NUM_UPPER_BOUND-1
			key = rnum % RAND_NUM_UPPER_BOUND;

			//lock hash table access
			pthread_mutex_lock( &mutex );

			// if this sample has not been counted before
			if (!(s = h[seed].lookup(key))){
				// insert a new element for it into the hash table
				s = new sample(key);
				h[seed].insert(s);
			}

			// increment the count for the sample
			s->count++;

			pthread_mutex_unlock( &mutex );
		}
}

void* thread_function_2_seeds(void* _seed){
	int seed = *((int *) _seed);
	int rnum = seed;

	//setup hash table
	h[seed].setup(14);

	//printf("2 seeds thread %d\n", seed);

	int i,j,k;
	unsigned key;
	sample *s;
	// process streams starting with different initial numbers
	for (i=0; i<2; i++){
	  rnum = i+seed;
	  //printf("seed %d\n", rnum);

		// collect a number of samples
		for (j=0; j<SAMPLES_TO_COLLECT; j++){

			// skip a number of samples
			for (k=0; k<samples_to_skip; k++){
			  rnum = rand_r((unsigned int*)&rnum);
			}

			// force the sample to be within the range of 0..RAND_NUM_UPPER_BOUND-1
			key = rnum % RAND_NUM_UPPER_BOUND;

			//lock hash table access
			pthread_mutex_lock( &mutex );

			// if this sample has not been counted before
			if (!(s = h[seed].lookup(key))){
				// insert a new element for it into the hash table
				s = new sample(key);
				h[seed].insert(s);
			}

			// increment the count for the sample
			s->count++;

			pthread_mutex_unlock( &mutex );
		}
	}
}

