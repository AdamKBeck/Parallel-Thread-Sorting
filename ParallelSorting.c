#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

int global_array[] = {23,45,23,576,34,23,56,34,123,23,234,12,34,46,67,789,23,145,76,45};

struct range {
	int start_index;
	int end_index;
};

void* sort(void* ptr);

int main(int argc, char *argv) {
	printf("Global array size: %ld \n", sizeof(global_array) / sizeof(int));
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
		r[i].end_index = farthestRange + intervalRange;

		//TODO: Splits are incorrect for certain ratios. Fix for final release
		farthestRange = r[i].end_index; 
		
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
	// Dereference the void pointer into our struct so we can access its fields
 	struct range r = *((struct range *)(ptr));

	// Print out the range we are sorting for proof of correctness 
	// All of the ranges, combined, equal our total range of the global array
	printf("Begin: %d \t", r.start_index);
	printf("End: %d \n", r.end_index);

	printf("Indices before sorting: [");
	for (int i = r.start_index; i < r.end_index; i++) {
		printf("%d,", global_array[i]);
	}
	printf("]\n");




	// Sort the range
	// TODO: This is a bubble-sort example. Let user decide what type of sorting to do.
	printf("Indices after sorting: [");
	int i, j;
	int n = r.end_index - r.start_index + 1;
	for (int i = 0; i < n; i++) {
		for (j = 1; j < (n-i); j++) {
			if (global_array[j-1+r.start_index] > global_array[j+r.start_index]) {
				int temp = global_array[j-1+r.start_index];
				global_array[j-1+r.start_index] = global_array[j+r.start_index];
				global_array[j+r.start_index] = temp;
			}
		}
	}
	for (int i = r.start_index; i < r.end_index; i++) {
		printf("%d,", global_array[i]);
	}
	printf("]\n\n");

	pthread_exit(0);
}
