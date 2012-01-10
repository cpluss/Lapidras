#ifndef SHARED_MEMORY_H
#define SHARED_MEMORY_H
#include "types.h"

/*
 * Just to clarify the current usage of the shared memory
 * 		It is currently used to establish "pipes" or areas that two processes may access
 * 		It's meant to be very fast, and to communicate at a high level using it.
 * 			-> Mainly created for the current keyboard messaging ( too slow using regular signals )
 * */

#define SHARED_MEMORY_START 0xC5000000

//To specify a current memory region
//This is a block that lies in front of every region.
struct memory_region
{
	byte shared 	: 1;		//Shared region?
	byte locked		: 1;		//Is it locked?
	byte rw			: 1;		//Read-write?
	byte occupied	: 1; 		//Is it being used between two processes?
	byte recently_written : 1;	//Recently been written to or not?
	byte reserved	: 3;
	
	byte id;					//The block identifier
	byte size;					//Shared memory may never exceed 255 bytes ( 0xFF )
	
	uint start;					//The start of the actual memory
	
	volatile struct memory_region *next;	//Pointer to the next memory region in the linked list
} __attribute__((packed));
typedef struct memory_region memory_region_t;

typedef struct shared_memory_pool
{
	volatile memory_region_t *first;	//The first memory region in the linked list
	
	uint memory_total_size;				//The total size of the memory regions altogether
	byte region_size;					//The size of every region
	uint region_count;					//Total count of memory regions
	
	uint free_region_count;				//Count of free memory regions -> that aren't used
	uint occupied_region_count;			//Count of memory regions currently in use
} shared_memory_pool_t;

//Create a pool of memory regions
void init_shared_memory(int regions, byte size);
//Occupy a free region of memory for own usage
memory_region_t *reserve_shared_memory();
//This region is no longer needed, why occupy it any longer?
void free_shared_memory(memory_region_t *region);

//Prepare a shared memory region for writing
void lock_shared_memory(memory_region_t *region);
void unlock_shared_memory(memory_region_t *region);

#endif
