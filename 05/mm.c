/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 */
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>


#include "mm.h"
#include "memlib.h"

/* single word (4) or double word (8) alignment */
/* things I want to consider
  I need to do split anc combine blocks after malloc and free calls
*/
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)

#define SIZE_T_SIZE ALIGN(sizeof(size_t))
#define BLK_HDR_SIZE ALIGN(sizeof(FR_BLK_HDR))
#define BLK_PTR_SIZE ALIGN(sizeof(AL_BLK_PTR))
#define FREE_BLK_SIZE ALIGN(sizeof(FR_BLK_HDR)+ sizeof(AL_BLK_PTR))

/* we need this because size_t might varry from a 64-bit
  machine to a 32-bit machine.
  SIZE_T_SIZE gives allignfor size 4 which is 8
  and that is what we used in class for headers
*/
typedef struct header FR_BLK_HDR;
struct header{
  size_t size;
  struct header *next;
  struct header *prior;
};
typedef struct footer AL_BLK_PTR;
struct footer{
  size_t size;
};

//prototypes
void *find_best_fit(size_t size);
void print_heap();
void coalesce(FR_BLK_HDR *bp);
void split(FR_BLK_HDR *bp, size_t newsize,AL_BLK_PTR *bpf);
size_t resize (size_t size);
size_t nextPowerOf2(size_t n);

/* Finds next power of two for n. If n itself
   is a power of two then returns n*/
size_t nextPowerOf2(size_t n)
{
    n--;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    n++;
    return n;
}
//helper function that makes sure that the size of a block is at least 48
size_t resize (size_t size){
  if (size < 48){
    return 48;
  }
  if(size+64 >= nextPowerOf2(size))
    size = nextPowerOf2(size);
  return size;
}
/*
 * mm_init - initialize the malloc package.
 */
int mm_init(void){
  FR_BLK_HDR *bp = mem_sbrk(FREE_BLK_SIZE);
  AL_BLK_PTR *bpf= (AL_BLK_PTR *)((char *)bp + BLK_HDR_SIZE);
  bp->size = FREE_BLK_SIZE;
  bpf->size = FREE_BLK_SIZE;
  bp->next = bp;
  bp->prior = bp;
  return 0;
}
//maybe change this to find the best hit after a certain number of interatipns

void *find_best_fit(size_t size){
  FR_BLK_HDR *bp;
  for(bp=((FR_BLK_HDR *)mem_heap_lo())->next;
      bp != mem_heap_lo() && bp->size < size;
      bp = bp->next);
  if (bp != mem_heap_lo())
      return bp;
  else
      return NULL;
      }
/*
void *find_best_fit(size_t size){
  FR_BLK_HDR *bp;
  FR_BLK_HDR *min_blk_ptr = NULL;
  size_t min_blk_size = ~1;
  //for loop to go thought our explicit-list
  for(bp=((FR_BLK_HDR *)mem_heap_lo())->next;
      bp != mem_heap_lo();
      bp = bp->next){
    if (bp->size >= size && bp->size < min_blk_size && bp != (FR_BLK_HDR *)mem_heap_lo()) {
      min_blk_ptr = bp;
      min_blk_size = bp->size;
    }
    //this was new
    if(bp->size +64 >= pow(2,ceil(log(size)/log(2))) && bp->size >= size){
      return bp;
    }
    //end of new
  }
  //this will return the begining of the free block
  return min_blk_ptr;
}
*/
/*
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
 void print_heap() {
  FR_BLK_HDR *bp = mem_heap_lo();
  while (bp < (FR_BLK_HDR *)mem_heap_hi()) {
    printf("%s block at %p, size %d\n",
           (bp->size&1)?"allocated":"free",
           bp,
           (int)(bp->size & ~1L));
    bp = (FR_BLK_HDR *)((char *)bp + (bp->size & ~1L));
  }
}
void *mm_malloc(size_t size){
  //I just added the resize and it made everything work
  size = resize(size);
  size_t newsize = ALIGN((size + BLK_PTR_SIZE + BLK_PTR_SIZE));
  FR_BLK_HDR *bp = find_best_fit(newsize);
  AL_BLK_PTR *bpf = NULL ;
  if (bp == NULL){
    bp = mem_sbrk(newsize);
    if ((long)bp==-1){
          //if mem_sbrk fails
          return NULL;
    }else{//if mem_sbrk works
          //set the allocated bit to 1
          bp->size = newsize | 1;
          bpf = (AL_BLK_PTR *)((char *)bp + newsize - BLK_PTR_SIZE);
          bpf->size = newsize | 1;
    }
  }else{//if find_best_fit works
        //check if we have an extra space in the block so we can split
    if(bp->size > (newsize +FREE_BLK_SIZE+FREE_BLK_SIZE)){
        split(bp,newsize,bpf);
  }else{
    //if we can't split the block
    bpf = (AL_BLK_PTR *)((char*)bp + bp->size - BLK_PTR_SIZE);
    //set the allocated bit for both the header and footer to zero
    bp->size = bp->size | 1;
    bpf->size = bpf->size | 1;
    //remove bp from the explicit free list
    bp->next->prior = bp->prior;
    bp->prior->next = bp->next;
  }
}
  return (char *)bp + BLK_PTR_SIZE ;
}

/*
 * mm_free - free a block by adding it to our explicit-list
 */
void mm_free(void *ptr){
  FR_BLK_HDR *bp = (FR_BLK_HDR *)((char*)ptr - BLK_PTR_SIZE);
  bp->size &= ~1L;
  AL_BLK_PTR *bpf = (AL_BLK_PTR *)((char *)bp + bp->size - BLK_PTR_SIZE);
  FR_BLK_HDR *head= mem_heap_lo();
  bpf->size &= ~1L;
  // set the aloocated bit to 0
  bp->next = head->next;
  bp->prior= head;
  head->next = bp;
  bp->next->prior = bp;
  //try to coalesce if the free block has a free block next or before it
  coalesce(bp);
}
//spliter helper function
void split(FR_BLK_HDR* bp, size_t newsize,AL_BLK_PTR *bpf){
  //set the header and footer for the free blocks
  int is_bp_allocated = bp->size & 1;
  FR_BLK_HDR *extraFR_BLK_HDR =(FR_BLK_HDR *)((char *)bp + newsize);
  extraFR_BLK_HDR->size = bp->size - newsize;
  AL_BLK_PTR *extraAL_BLK_PTR = (AL_BLK_PTR *)((char *)extraFR_BLK_HDR + extraFR_BLK_HDR->size - BLK_PTR_SIZE);
  extraAL_BLK_PTR->size = bp->size - newsize;
  //set the footer for the allocatedblock
  bpf = (AL_BLK_PTR *)((char *)extraFR_BLK_HDR - BLK_PTR_SIZE);
  // set the allocated bit to 0 for the extra free block
  extraFR_BLK_HDR->size &= ~1L;
  extraAL_BLK_PTR->size &= ~1L;
  // set the allocated bit to 1 for the allocatedblock
  bp->size = newsize | 1;
  bpf->size = newsize | 1;
  if (!is_bp_allocated){
    //remove bp from the explicit list for free blocks
    bp->next->prior = bp->prior;
    bp->prior->next = bp->next;
  }
  //add the extra free block to the explicit free list
  FR_BLK_HDR *head= mem_heap_lo();
  extraFR_BLK_HDR->next = head->next;
  extraFR_BLK_HDR->prior= head;
  head->next = extraFR_BLK_HDR;
  extraFR_BLK_HDR->next->prior = extraFR_BLK_HDR;
}
//coalesce only when we free
void coalesce(FR_BLK_HDR *bp) {
  FR_BLK_HDR  *next,*prev;
  int next_alloc, prev_alloc;
  //check if the previous block was free
  if (((char *)mem_heap_lo()+FREE_BLK_SIZE) <= (char *)((char *)bp - (((AL_BLK_PTR *)((char *)bp-BLK_PTR_SIZE))->size & ~1L))) {
    prev =(FR_BLK_HDR *)((char *)bp - (((AL_BLK_PTR *)((char *)bp-BLK_PTR_SIZE))->size & ~1L));
    prev_alloc = prev->size & 1;
  } else {
    prev_alloc = 1; // sane choice
  }
  //check if the next block is free
  //I might have an issue here, double check this
  if ((char *)mem_heap_hi()>(char *)((char *)bp + (bp->size & ~1L))) {
    next =(FR_BLK_HDR *)((char *)bp + (bp->size & ~1L));
    next_alloc = next->size & 1;
  } else {
    next_alloc = 1; // sane choice
  }
  if (prev_alloc && next_alloc) {
    return ;
  }
  else if (!prev_alloc && next_alloc) {
    // adjust header size of the previous block
    prev->size += bp->size;
    //adjust footer of the free block size
    ((AL_BLK_PTR *)((char *)prev + prev->size - BLK_PTR_SIZE))->size =  prev->size;
      //remove the current block from the free list
    bp->next->prior = bp->prior;
    bp->prior->next = bp->next;
    return;
  }
  else if (prev_alloc && !next_alloc) {
     // adjust header size
    bp->size += next->size;
    // adjust footer size
    ((AL_BLK_PTR *)((char *)next + next->size - BLK_PTR_SIZE))->size = bp->size;
    //remove the next block from the free list
    next->next->prior = next->prior;
    next->prior->next = next->next;
    return ;
  }
  else if (!prev_alloc && !next_alloc){
    //adjust the size of the header of the previous block
    prev->size+=(bp->size+next->size);
    // adjust the size of the footer
    ((AL_BLK_PTR *)((char *)prev + prev->size - BLK_PTR_SIZE))->size = prev->size;
    //remove the current block from the explict list
    bp->next->prior = bp->prior;
    bp->prior->next = bp->next;
    //remove the next block form the explicit list
    next->next->prior = next->prior;
    next->prior->next = next->next;
    return;
  }
}
/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
/*
 void *mm_realloc(void *ptr, size_t size)
{
  AL_BLK_PTR *bp = ptr - BLK_PTR_SIZE;
  // if ( bp->size > size){
  //   printf("WE ARE SHRINING OMG. ABORT ABORT\n");
  //   printf("old size is %zd , current size %zd",bp->size, size);
  //   return NULL;
  // }
  void *newptr = mm_malloc(size);
  if (newptr == NULL)
    return NULL;
  int copySize = bp->size-BLK_PTR_SIZE;
  if (size < copySize)
    copySize = size;
  memcpy(newptr, ptr, copySize);
  mm_free(ptr);
  return newptr;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                  REALLOC                                                     //
//                                                                                                              //
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//should we copy the memory over for all realloc cases??
//or we only do it when we move to a new place?
//what about if we are just extending the size?
*/
 void *mm_realloc(void *ptr, size_t size){
   FR_BLK_HDR *next_bp = NULL;
   AL_BLK_PTR *bpf= NULL;
   int next_alloc;
    //if we are given a null pointer then we call malloc
   if (!ptr) {
     return mm_malloc(size);
   }
   //if size was 0 then this is a call to free
   if (size == 0) {
     mm_free(ptr);
     return NULL;
   }
   AL_BLK_PTR *bp = ptr - BLK_PTR_SIZE;
   if(((bp)->size& ~1L) == size){
     return ptr;
   }
     size_t newsize = ALIGN((size + BLK_PTR_SIZE + BLK_PTR_SIZE));
     size_t diff = newsize - (bp->size & ~1L);
     //we are checking the next block
     next_bp =(FR_BLK_HDR *)((char *)bp + (bp->size & ~1L));
     if ((char *)mem_heap_hi()>(char *)((char *)bp + (bp->size & ~1L))) {
       next_alloc = next_bp->size & 1;
     } else {
       next_alloc = 1; // sane choice
     }
     //if the next block is a free block
     if (!next_alloc) {
        // adjust header size
       bp->size = ((bp->size & ~1L) + (next_bp->size & ~1L)) | 1;
       // adjust footer size
       bpf = (AL_BLK_PTR *)((char *)bp + (bp->size & ~1L) - BLK_PTR_SIZE);
       bpf->size = bp->size;
       return (char*)bp + BLK_PTR_SIZE;
     }//if the next block points to the end of the heap so we extend the heap
     if ((char*)next_bp>=(char *)mem_heap_hi()){
       next_bp = mem_sbrk(diff);
       if ((long)next_bp==-1){// if we can't mem_sbrk
         return NULL;
       }else{//if mem_sbrk works
         //fix the footer pointer and size
         bp->size = newsize | 1;
         bpf = (AL_BLK_PTR*)((char *)bp + newsize- BLK_PTR_SIZE);
         //set the allocated bit to 1
         bpf->size = newsize | 1;
         return (char*) bp + BLK_PTR_SIZE;
       }
     }else{// if we CANNOT extend the heap and the next block is NOT free
       void *newptr = mm_malloc(size);
       if (newptr == NULL)
         return NULL;
       size_t copySize = (bp->size & ~1L) - 2*BLK_PTR_SIZE;
       memcpy(newptr, ptr, copySize);
       mm_free(ptr);
       return newptr ;
     }
}
