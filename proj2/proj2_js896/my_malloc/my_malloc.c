/*implementations*/
#include "my_malloc.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

MDLL * head = NULL;
MDLL * tail = NULL;
__thread MDLL * tls_head = NULL;
__thread MDLL * tls_tail = NULL;

unsigned long total = 0;
//unsigned long allocated = 0;

/* MDLL * FirstFit(MDLL * head, size_t sz) { */
/*   MDLL * curr = head; */
/*   while (((curr->free) == 0) || (curr->size) < (sizeof(MDLL) + sz)) { */
/*     curr = curr->next; */
/*     if (curr == NULL) { */
/*       break; */
/*     } */
/*   } */
/*   return curr; */
/* } */

MDLL * BestFit(size_t sz) {
  MDLL * curr = head;
  MDLL * bf = NULL;
  while (curr != NULL) {
    if ((curr->size) >= sz) {
      if (bf == NULL) {
        bf = curr;
        curr = curr->next;
        continue;
      }
      if (curr->size == sz) {
        bf = curr;
        return bf;
      }
      if ((curr->size) < (bf->size)) {
        bf = curr;
      }
    }
    curr = curr->next;
  }
  return bf;
}

MDLL * TLS_BestFit(size_t sz) {
  MDLL * curr = tls_head;
  MDLL * bf = NULL;
  while (curr != NULL) {
    if ((curr->size) >= sz) {
      if (bf == NULL) {
        bf = curr;
        curr = curr->next;
        continue;
      }
      if (curr->size == sz) {
        bf = curr;
        return bf;
      }
      if ((curr->size) < (bf->size)) {
        bf = curr;
      }
    }
    curr = curr->next;
  }
  return bf;
}

MDLL * alloc(size_t sz) {
  MDLL * start = sbrk(0);
  void * new_start = sbrk(sizeof(MDLL) + sz);
  total += sizeof(MDLL) + sz;
  //void * real = sbrk(0);
  //printf("\n start: %p\n new_start: %p\n real: %p\n", start, new_start, real);
  /*if sbrk fails, return nullptr*/
  if (new_start == (void *)-1) {
    return NULL;
  }
  start->free = 0;
  start->size = sz;
  start->DataPtr = start + 1;
  return new_start;
}

MDLL * TLS_alloc(size_t sz) {
  pthread_mutex_lock(&lock);
  MDLL * start = sbrk(0);
  void * new_start = sbrk(sizeof(MDLL) + sz);
  pthread_mutex_unlock(&lock);
  total += sizeof(MDLL) + sz;
  //void * real = sbrk(0);
  //printf("\n start: %p\n new_start: %p\n real: %p\n", start, new_start, real);
  /*if sbrk fails, return nullptr*/
  if (new_start == (void *)-1) {
    return NULL;
  }
  start->free = 0;
  start->size = sz;
  start->DataPtr = start + 1;
  return new_start;
}

void add(MDLL * neo) {
  if (head == NULL && tail == NULL) {
    //printf("\n head == NULL");
    head = neo;
    tail = neo;
    neo->prev = NULL;
    neo->next = NULL;
    return;
  }
  MDLL * curr = head;
  while ((curr != NULL) && curr < neo) {
    curr = curr->next;
  }
  //Append to FreedTail
  if (curr == NULL) {
    //printf("\n append to tail");
    tail->next = neo;
    neo->prev = tail;
    neo->next = NULL;
    tail = neo;
    //need to merge
    if ((char *)(tail->prev->DataPtr) + tail->prev->size == (char *)tail) {
      tail->prev->size += sizeof(MDLL) + tail->size;
      tail->prev->next = NULL;
      tail = tail->prev;
    }
    return;
  }
  //Append to FreedHead
  else if (curr == head) {
    //printf("\n append to head");
    head->prev = neo;
    neo->prev = NULL;
    neo->next = head;
    head = neo;
    //need to merge
    if ((char *)(head->DataPtr) + head->size == (char *)head->next) {
      head->size += sizeof(MDLL) + head->next->size;
      if (head->next->next != NULL) {
        head->next->next->prev = head;
      }
      head->next = head->next->next;
    }
    return;
  }
  //Append in the middle
  else {
    //printf("\n append to mid");
    curr->prev->next = neo;
    neo->prev = curr->prev;
    neo->next = curr;
    curr->prev = neo;
    MDLL * temp = curr->prev;
    if ((char *)(temp->prev->DataPtr) + temp->prev->size == (char *)temp) {
      temp->prev->size += sizeof(MDLL) + temp->size;
      temp->prev->next = curr;
      curr->prev = temp->prev;
    }
    return;
  }
}

void TLS_add(MDLL * neo) {
  if (tls_head == NULL && tls_tail == NULL) {
    //printf("\n head == NULL");
    tls_head = neo;
    tls_tail = neo;
    neo->prev = NULL;
    neo->next = NULL;
    return;
  }
  MDLL * curr = tls_head;
  while ((curr != NULL) && curr < neo) {
    curr = curr->next;
  }
  //Append to FreedTail
  if (curr == NULL) {
    //printf("\n append to tail");
    tls_tail->next = neo;
    neo->prev = tls_tail;
    neo->next = NULL;
    tls_tail = neo;
    //need to merge
    if ((char *)(tls_tail->prev->DataPtr) + tls_tail->prev->size == (char *)tls_tail) {
      tls_tail->prev->size += sizeof(MDLL) + tls_tail->size;
      tls_tail->prev->next = NULL;
      tls_tail = tls_tail->prev;
    }
    return;
  }
  //Append to FreedHead
  else if (curr == tls_head) {
    //printf("\n append to head");
    tls_head->prev = neo;
    neo->prev = NULL;
    neo->next = tls_head;
    tls_head = neo;
    //need to merge
    if ((char *)(tls_head->DataPtr) + tls_head->size == (char *)tls_head->next) {
      tls_head->size += sizeof(MDLL) + tls_head->next->size;
      if (tls_head->next->next != NULL) {
        tls_head->next->next->prev = tls_head;
      }
      tls_head->next = tls_head->next->next;
    }
    return;
  }
  //Append in the middle
  else {
    //printf("\n append to mid");
    curr->prev->next = neo;
    neo->prev = curr->prev;
    neo->next = curr;
    curr->prev = neo;
    MDLL * temp = curr->prev;
    if ((char *)(temp->prev->DataPtr) + temp->prev->size == (char *)temp) {
      temp->prev->size += sizeof(MDLL) + temp->size;
      temp->prev->next = curr;
      curr->prev = temp->prev;
    }
    return;
  }
}

void rm(MDLL * gone) {
  if (gone == head && gone == tail) {
    head = NULL;
    tail = NULL;
  }
  else if (gone == head) {
    if (head->next != NULL) {
      head->next->prev = NULL;
    }
    head = head->next;
  }
  else if (gone == tail) {
    tail->prev->next = NULL;
    tail = tail->prev;
  }
  else {
    gone->prev->next = gone->next;
    gone->next->prev = gone->prev;
  }
}

void TLS_rm(MDLL * gone) {
  if (gone == tls_head && gone == tls_tail) {
    tls_head = NULL;
    tls_tail = NULL;
  }
  else if (gone == tls_head) {
    if (tls_head->next != NULL) {
      tls_head->next->prev = NULL;
    }
    tls_head = tls_head->next;
  }
  else if (gone == tls_tail) {
    tls_tail->prev->next = NULL;
    tls_tail = tls_tail->prev;
  }
  else {
    gone->prev->next = gone->next;
    gone->next->prev = gone->prev;
  }
}

void split(MDLL * FitSlot, size_t sz) {
  //  printf("split \n");
  MDLL * new = (void *)((char *)(FitSlot + 1) + sz);
  new->free = 1;
  add(new);
  new->size = (FitSlot->size) - sz - sizeof(MDLL);
  new->DataPtr = new + 1;
  FitSlot->free = 0;
  rm(FitSlot);
  FitSlot->size = sz;
  FitSlot->DataPtr = FitSlot + 1;
  //  allocated += FitSlot->size + sizeof(MDLL);
}

void TLS_split(MDLL * FitSlot, size_t sz) {
  //  printf("split \n");
  MDLL * new = (void *)((char *)(FitSlot + 1) + sz);
  new->free = 1;
  TLS_add(new);
  new->size = (FitSlot->size) - sz - sizeof(MDLL);
  new->DataPtr = new + 1;
  FitSlot->free = 0;
  TLS_rm(FitSlot);
  FitSlot->size = sz;
  FitSlot->DataPtr = FitSlot + 1;
  //  allocated += FitSlot->size + sizeof(MDLL);
}

void * bf_malloc(size_t sz) {
  MDLL * curr = head;
  if (curr == NULL) {
    curr = alloc(sz);
    //    allocated += sz + sizeof(MDLL);
    return curr->DataPtr;
  }
  else {
    curr = BestFit(sz);
    if (curr == NULL) {
      curr = alloc(sz);
      //      allocated += sz + sizeof(MDLL);
      return curr->DataPtr;
    }
    if (curr->size <= sz + 2 * sizeof(MDLL)) {
      rm(curr);
      //allocated += curr->size + sizeof(MDLL);
      return curr->DataPtr;
    }
    else {
      split(curr, sz);
      return curr->DataPtr;
    }
  }
}

void ff_free(void * ptr) {
  MDLL * curr = ptr;
  --curr;
  curr->free = 1;
  //allocated -= curr->size + sizeof(MDLL);
  add(curr);
  //printf("\n out of add");
  //merge();
  //printf("\n out of merge\n");
}

void bf_free(void * ptr) {
  ff_free(ptr);
}

void lts_bf_free(void * ptr) {
  MDLL * curr = ptr;
  --curr;
  curr->free = 1;
  TLS_add(curr);
}

void * ts_malloc_lock(size_t size) {
  pthread_mutex_lock(&lock);
  void * result = bf_malloc(size);
  pthread_mutex_unlock(&lock);
  return result;
}

void ts_free_lock(void * ptr) {
  pthread_mutex_lock(&lock);
  bf_free(ptr);
  pthread_mutex_unlock(&lock);
}

void * ts_malloc_nolock(size_t size) {
  MDLL * curr = tls_head;
  if (curr == NULL) {
    curr = TLS_alloc(size);
    //    allocated += sz + sizeof(MDLL);
    return curr->DataPtr;
  }
  else {
    curr = TLS_BestFit(size);
    if (curr == NULL) {
      curr = TLS_alloc(size);
      //      allocated += sz + sizeof(MDLL);
      return curr->DataPtr;
    }
    if (curr->size <= size + 2 * sizeof(MDLL)) {
      TLS_rm(curr);
      //allocated += curr->size + sizeof(MDLL);
      return curr->DataPtr;
    }
    else {
      TLS_split(curr, size);
      return curr->DataPtr;
    }
  }
}

void ts_free_nolock(void * ptr) {
  lts_bf_free(ptr);
}

unsigned long get_data_segment_size() {
  return total;
}

unsigned long get_data_segment_free_space_size() {
  //  return total - allocated;
  unsigned long freed = 0;
  MDLL * curr = head;
  while (curr != NULL) {
    freed += sizeof(MDLL) + curr->size;
    curr = curr->next;
  }
  return freed;
}

/* int main() { */
/*   printf("head: %p \n", head); */
/*   char * p = ff_malloc(sizeof(int)); */
/*   ff_free(p); */
/*   printf("head: %p \n", head); */
/*   char * p1 = ff_malloc(sizeof(int)); */
/*   char * p2 = ff_malloc(sizeof(int)); */
/*   char * p3 = ff_malloc(sizeof(int)); */
/*   printf("head0: %p \n", head); */
/*   ff_free(p2); */
/*   printf("head: %p \n", head); */
/*   ff_free(p3); */
/*   printf("head: %p \n", head); */
/*   ff_free(p1); */
/*   printf("head: %p \n", head); */
/*   printf("Fragmentation  = %f\n", */
/*          (float)get_data_segment_free_space_size() / (float)total); */

/*   return EXIT_SUCCESS; */
/* } */
