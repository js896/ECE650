/*function definitions for all*_malloc() and *_free() functions*/
//#ifndef _MY_MALLOC_H
//#define _MY_MALLOC_H

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

//First Fit malloc/free
void * ff_malloc(size_t size);
void ff_free(void * ptr);
//Best Fit malloc/free
void * bf_malloc(size_t size);
void bf_free(void * ptr);



unsigned long get_data_segment_size();             //in bytes
unsigned long get_data_segment_free_space_size();  //in bytes

typedef struct MetaDataLinkedList {
  int free; /*1 if free, 0 otherwise*/
  size_t size;
  struct MetaDataLinkedList * prev;
  struct MetaDataLinkedList * next;
  void * DataPtr;
} MDLL;

void rm (MDLL * gone);
void add (MDLL * neo);
//#endif
