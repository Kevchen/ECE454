
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>


#include "defs.h"
#include "hash_element.h"

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
  pthread_mutex_t mutex;

  sample(unsigned the_key){my_key = the_key; count = 0; pthread_mutex_init( &(mutex), NULL);};
  unsigned key(){return my_key;}
  void print(FILE *f){printf("%d %d\n",my_key,count);}
  void lock_mutex(){pthread_mutex_lock(&mutex);}
  void unlock_mutex(){pthread_mutex_unlock(&mutex);}
};



// This instantiates an empty hash table
// it is a C++ template, which means we define the types for
// the element and key value here: element is "class sample" and
// key value is "unsigned".
hash<sample,unsigned> h;


void* thread_function_1_seed(void* seed);
void* thread_function_2_seeds(void* _seed);
static pthread_mutex_t mutex[RAND_NUM_UPPER_BOUND];
//static pthread_mutex_t *mutex_head;

int
main (int argc, char* argv[]){
  int i,j,k;
  int rnum;
  unsigned key;
  sample *s;

  pthread_t thread1, thread2, thread3;
  int lock_count;
  for(lock_count=0;lock_count<RAND_NUM_UPPER_BOUND;lock_count++){
  	  //Need to initialize mutex locks here
  	  pthread_mutex_init(&mutex[lock_count],NULL);
    }

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

  // initialize a 16K-entry (2**14) hash of empty lists
  h.setup(14);

  int seed[4] = {0,1,2,3};

  if(num_threads==1){
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
			if (!(s = h.lookup(key))){
		        //lock hash table access
				pthread_mutex_lock( &mutex[key] );

				// insert a new element for it into the hash table
				s = new sample(key);
				h.insert(s);

				pthread_mutex_unlock( &mutex[key] );
			}
			// increment the count for the sample
			s->lock_mutex();
			s->count++;
			s->unlock_mutex();
	      }
	    }
  }
  else if(num_threads==2){
	  pthread_create(&thread1, NULL, thread_function_2_seeds, &seed[2]);
	  thread_function_2_seeds(&seed[0]);

	  //wait for all threads
	  pthread_join(thread1, NULL);
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
  }


  // print a list of the frequency of all samples
  h.print();
}

void* thread_function_1_seed(void* _seed){
	int seed = *((int *) _seed);
	//printf("1 seed thread %d\n", seed);

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

			// if this sample has not been counted before
			if (!(s = h.lookup(key))){
				//lock hash table access
				pthread_mutex_lock( &mutex[key] );
				// insert a new element for it into the hash table
				s = new sample(key);
				h.insert(s);
				pthread_mutex_unlock( &mutex[key] );
			}

			//we don't need to lock the list for increments, we only need to lock the individual element.
			//however, we still have to lock the list during insert() otherwise 2 identical samples
			//might be inserted between lookup() and insert() from different threads.

			// increment the count for the sample
			s->lock_mutex();
			s->count++;
			s->unlock_mutex();

		}
}

void* thread_function_2_seeds(void* _seed){
	int seed = *((int *) _seed);
	int rnum = seed;

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
			if (!(s = h.lookup(key))){
				// insert a new element for it into the hash table
				//lock hash table access
				pthread_mutex_lock( &mutex[key] );
				s = new sample(key);
				h.insert(s);
				pthread_mutex_unlock( &mutex[key] );
			}
			// increment the count for the sample
			s->lock_mutex();
			s->count++;
			s->unlock_mutex();
			}
	}
}

