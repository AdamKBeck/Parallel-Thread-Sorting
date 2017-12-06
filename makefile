all:
	gcc -o ParallelSorting ParallelSorting.c -lpthread -std=c99

clean:
	rm -f ParallelSorting
