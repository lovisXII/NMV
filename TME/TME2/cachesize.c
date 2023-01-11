#include "hwdetect.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>


#define MEMORY_SIZE        (1ul << 21)
#define WARMUP             10000
#define PRECISION          1000000

#ifndef PARAM
#  define PARAM            1024
#endif


#define NBR_ACCESS 1000000
#define CACHE_SIZE PARAM 

static inline char *alloc(void)
{
	size_t i;
	char *ret = mmap(NULL, MEMORY_SIZE, PROT_READ | PROT_WRITE,
			 MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

	if (ret == MAP_FAILED)
		abort();

	for (i = 0; i < MEMORY_SIZE; i += PAGE_SIZE)
		ret[i] = 0;

	return ret;
}

static inline uint64_t detect(char *mem)
{
	// WARMUP : allow to cancel the MISS
	// since its load the cache with the data we want
	// If data are too big then it will still miss
	// when the 2nd loop start cause it will erase data

	for(size_t i = 0; i < NBR_ACCESS; i++)
		for(size_t j = 0; j < CACHE_SIZE; j++)
			writemem(mem + j);

	uint64_t start = now() ;

	for(size_t i = 0; i < NBR_ACCESS; i++)
		for(size_t j = 0; j < CACHE_SIZE; j++)
			writemem(mem + j);
	
	uint64_t end = now() ;

	return (end - start)/NBR_ACCESS;
}

int main(void)
{
	char *mem = alloc();
	uint64_t t = detect(mem);

	printf("%d %lu\n", PARAM, t);
	return EXIT_SUCCESS;
}
