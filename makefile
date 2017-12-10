all:
	gcc -o ParallelSorting ParallelSorting.c -lpthread -lrt -std=c99

clean:
	rm -f ParallelSorting
