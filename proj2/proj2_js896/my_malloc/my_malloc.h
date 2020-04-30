/*function definitions for all*_malloc() and *_free() functions*/
//#ifndef _MY_MALLOC_H
//#define _MY_MALLOC_H

#include <pthread.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

//Thread Safe malloc/free: locking version
void * ts_malloc_lock(size_t size);
void ts_free_lock(void * ptr);

//Thread Safe malloc/free: non-locking version
void * ts_malloc_nolock(size_t size);
void ts_free_nolock(void * ptr);

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

void rm(MDLL * gone);
void add(MDLL * neo);
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

//#endif
