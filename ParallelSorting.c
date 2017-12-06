#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

int global_array[] = {23,45,23,576,34,23,56,34,123,76,45};

struct range {
	int start_index;
	int end_index;
};

void* sort(void* ptr);

int main(int argc, char *argv) {
	// Get number of threads from user
	printf("How many threads? ");
	int num_threads;
	scanf("%d", &num_threads);
	
	//TODO: cardinality constraints. Can the user enter in -4 threads? etc.

	// Calculate the interval each thread needs to work on in the global array
	// This is an int, as the final thread will pick up the remaining interval slice,
	// knowing that most invervals aren't evenly sliced in the global array
	int intervalRange = (sizeof(global_array) / sizeof(int)) / num_threads;

	// Create as many threads as the user specified
	pthread_t threads[num_threads];
	struct range r[num_threads];
	int farthestRange = 0;


	// Create n-1 threads and start sorting slices of the array
	for (int i = 0; i < num_threads-1; i++) {
		r[i].start_index = farthestRange;
		r[i].end_index= farthestRange + intervalRange;

		farthestRange = r[i].end_index + 1;
		
		pthread_attr_t attr;
		pthread_attr_init(&attr);
		pthread_create(&threads[i], &attr, sort, &r[i]);
	}
	
	// Create the last thread, so we have n in total, to sort the last potentially uneven slice
	struct range range_instantiation;
	r[num_threads-1].start_index= farthestRange;
	r[num_threads-1].end_index = (sizeof(global_array) / sizeof(int)) -1; // end of array

	pthread_create(&threads[num_threads-1], NULL, sort, &r[num_threads -1]);
	
	// Wait for all the threads to exit
	for (int i = 0; i < num_threads; i++) {
		pthread_join(threads[i], NULL);
	}

	printf("Finished!");

	return 0;
}

void *sort(void* ptr) {
	printf("here \n");
	pthread_exit(0);
}
