#ifndef MEM_H
#define MEM_H
#include "types.h"

#define KMEM_START          0xC0000000
#define KMEM_SIZE           0x00FFFFFF  //~10mb would be enough ..
#define KMEM_CHUNK_SIZE     0x1000      //4kb chunks to begin with
#define KERNEL_STACK		0xE0000000

#define FLAG_BLOCK_USED             0x1
#define FLAG_BLOCK_RW               0x2
#define FLAG_BLOCK_LINK             0x4
#define FLAG_BLOCK_OFFSET           0xFFFFFFF0
#define GET_BLOCK_LOCATION(x) (KMEM_START + ((x & FLAG_BLOCK_OFFSET) >> 4))

typedef struct
{
    uint address : 28; //28-bit address.. Should be enough to address it all .. ( TEST_MEM_START + address -> the block location )
    byte reserved : 1;
    
    byte link : 1; //To determine if this block is in a linked chain of blocks -> It's inactive in that case ..
    byte rw : 1;
    byte used : 1;
} __attribute__((packed)) mem_bit_t;

void mem_init();

void *alloc(uint sz);
void *ralloc(uint sz, int align);
void free(void *p);

uint kmalloc(uint size);
uint kmalloc_real(uint size, int align, uint *phys);

int valid_magic(void *addr);

#endif
