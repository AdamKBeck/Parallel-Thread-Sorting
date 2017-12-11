/* This project splits an array into slices, the number of slices is defined
 * by the user. In parallel, threads sort the slices, and then the slices are 
 * merged together into a sorted array. The user specifies what type of sorting
 * each slice does (quicksort, insertion, or bubble), but since the project will
 * run on the HPC, I will manually run many different sizes and types of sorting so
 * I can graph my solution.
 *
 * The project is written in a mix of ANSI C and C99, as I confused myself midway
 * through the development with what compiler the HPC has, and what statements are and
 * aren't allowed in C99.
 */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <limits.h>

int global_array[] = {23,45,23,576,34,23,56,34,123,23,234,12,34,46,67,789,23,145,76,45};

// Each thread uses this struct to sort a specified slice of the global array
struct range {
	int start_index;
	int end_index;
};

// Each thread passes through the sort function to set up a range struct
void* sort(void* ptr);

// Threads are then delegated to a sorting function depending on user input
void insertion_sort(struct range *r);
void bubble_sort(struct range *r);
void quick_sort(struct range *r);
void quick_sort_array(int *array, int size);

int sorting_choice;
sem_t mutex; // Used when each thread prints out info in the sort() function

int main(int argc, char *argv) {
	// Save the global array size, as this will be needed a lot
	int global_size = sizeof(global_array) / sizeof(int);
	sem_init(&mutex, 0, 1); // Used in the sort() function

	// Get number of threads from user
	printf("How many threads? ");
	int num_threads;
	scanf("%d", &num_threads);
	
	// Force the user to enter in a thread count between 1 and 16, inclusive
	while (!(0 < num_threads && num_threads < 17)) {
		printf("Thread count must be greater than 0, and less than 17\n");
		printf("How many threads? ");
		scanf("%d", &num_threads);
	}

	// Get the type of sorting from the user.
	printf("What type of sorting? (1 -> insertion, 2 -> bubble, 3 -> quicksort) ");
	scanf("%d", &sorting_choice);

	// Force the user to enter in a sorting number between 1 and 3, inclusive
	while (!(0 < sorting_choice && sorting_choice < 4)) {
		printf("Sorting choice must be greater than 0, and less than 4\n");
		printf("What type of sorting? (1 -> insertion, 2 -> bubble, 3 -> quicksort)\n");
		scanf("%d", &sorting_choice);
	}

	//  Calculate the slice size each thread needs to work on in the global array
	double interval_range = global_size / (double)num_threads;

	// Print out the global array, its size, number of threads, and the slice range
	printf("\n[");
	for (int i = 0; i < global_size; i++) {
		if (i == global_size -1) {
			printf("%d", global_array[i]);
		}
		else {
			printf("%d,", global_array[i]);
		}
	}
	printf("]\n");
	printf("Global size: %d\n", global_size);
	printf("Number of threads: %d\n", num_threads);
	printf("Interval range: %f\n", interval_range);
	printf("Sorting choice: %d\n\n", sorting_choice);

	// Create as many threads as the user specified
	pthread_t threads[num_threads];
	struct range r[num_threads];

	// Create n-1 threads and start sorting slices of the array
	int farthest_range = 0; // A helper variable to help assign slices to threads
	for (int i = 0; i < num_threads-1; i++) {
		r[i].start_index = farthest_range;
		
		/* Floor the interval_range for the first n-1 threads. The n-th thread
		 * will pick up the remainder
		 * We subtract 1 from interval range because the start_index acts as the
		 * first element in our slice, so our range represents the total amount
		 * of elements for the slice, not how much we add to the start_index to 
		 * assign for the end_index
		 */
		r[i].end_index = r[i].start_index + ((int)interval_range - 1);

		farthest_range = r[i].end_index + 1; 
		
		pthread_attr_t attr;
		pthread_attr_init(&attr);
		pthread_create(&threads[i], &attr, sort, &r[i]);
	}
	
	// Create the last thread, so we have n in total, to sort the last potentially uneven slice
	r[num_threads-1].start_index = farthest_range;
	r[num_threads-1].end_index = global_size - 1; // the end index is the end of the array

	pthread_create(&threads[num_threads-1], NULL, sort, &r[num_threads -1]);
	
	// Wait for all the threads to exit
	for (int i = 0; i < num_threads; i++) {
		pthread_join(threads[i], NULL);
	}

	/* We have an array of structs representing slices in the array.
	 * Instead of using a list library to realloc() and dealloc() a list of structs,
	 * I will iterate through each struct and find the minimum value, to help build my
	 * overall solution of the global sorted array. When the struct.start_index is greater than 
	 * the struct.right_index, I ignore looking at this struct when findining the next global minimum.
	 */
	int solution[global_size];
	// Zero out our solution
	for (int i = 0; i < global_size; i++) {
		solution[i] = 0;
	}

	int min = INT_MAX;
	int min_struct = 0;

	// We need to place 'global_size' elements in order. So loop this many of times
	for (int k = 0; k < global_size; k++) {
		// Out of all the start indices for the structs, find the minimum one.
		for (int i = 0; i < num_threads; i++) {
			if (r[i].start_index > r[i].end_index) {
				continue;
			}					
			else if (global_array[r[i].start_index] < min){
				min = global_array[r[i].start_index];
				min_struct = i; 
			}
		}	
		
		// Place this element in the global array to build the solution
		solution[k] = min;
		r[min_struct].start_index++;
		min = INT_MAX;
	}
	
	printf("\nSolution indices after sorting: [");
	for (int i = 0; i < global_size; i++) {
		printf("%d, ", solution[i]);
	}
	printf("]\n\n");

	printf("Finished!");

	return 0; }

void *sort(void* ptr) {
	// Dereference the void pointer into our struct so we can access its fields
 	struct range r = *((struct range *)(ptr));
	int i;
	int j;

	// Print out the range we are sorting for proof of correctness 
	// All of the ranges, combined, equal our total range of the global array
	// Note: Begin and End indices are inclusive. Account for this in the for loops
	sem_wait(&mutex);
	printf("Begin: %d \t", r.start_index);
	printf("End: %d \n", r.end_index);
	printf("Indices before sorting: [");
	for (i = r.start_index; i <= r.end_index; i++) {
		printf("%d,", global_array[i]);
	}
	printf("]\n");
	sem_post(&mutex);
	
	if (sorting_choice == 1) {
		insertion_sort(&r);
	}
	else if (sorting_choice == 2) {
		bubble_sort(&r);
	}
	else {
		quick_sort(&r);
	}
	
	sem_wait(&mutex);
	printf("Indices after sorting: [");
	for (i = r.start_index; i <= r.end_index; i++) {
		printf("%d,", global_array[i]);
	}
	printf("]\n");
	sem_post(&mutex);

	pthread_exit(0);
}

// Sorts a thread's slice, given the thread's range struct reference
void insertion_sort(struct range *r) {
	int i;
	int j;
	
	for (i = r->start_index; i <= r->end_index; i++) {
		j = i;
		while (j > r->start_index && global_array[j] < global_array[j-1]) {
			// Swap j-th index with the previous index
			int temp = global_array[j];
			global_array[j] = global_array[j-1];
			global_array[j-1] = temp;

			j--;
		}
	}
}

// Sorts a thread's slice, given the thread's range struct reference
void bubble_sort(struct range *r) { 
	int i;
	int j;

	for (i = r->start_index; i <= r->end_index; i++) {
		for (j = r->start_index; j < r->end_index - (i - r->start_index); j++) {
			if (global_array[j] > global_array[j+1]) {
				// Swap j-th index with the previous index
				int temp = global_array[j];
				global_array[j] = global_array[j+1];
				global_array[j+1] = temp;
			}
		}
	}
	
}

/* Sorts a thread's slice, given the thread's range struct reference.
 * Instead of recursive calls with range structs as the paramater, I'm going to
 * implement quicksort the traditional way, of recursively sorting an array with 
 * left and right indices
 */
void quick_sort(struct range *r) { 
	int *array = &global_array[r->start_index];
	quick_sort_array(array, r->end_index - r->start_index + 1);
}

void quick_sort_array(int *array, int size) {
	if (size <= 1) {
		return;
	}

	int pivot = array[size/ 2];

	int i, j;
	for (i = 0, j = size- 1; ; i++, j--) {
		while (array[i] < pivot){
			i++;
		} 

		while (array[j] > pivot){
			j--;
		}

		if (i >= j){
			break;
		}

		// Swap indices i and j for quicksort
		int temp = array[j];
		array[j] = array[i];
		array[i] = temp;

	}

	quick_sort_array(array, i);
	quick_sort_array(array + i, size - i);

}
