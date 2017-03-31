/* UZ SE NEPOUZIVA */
#ifndef LIST_H_
#define LIST_H_

#include <stdlib.h>

// struktura reprezentujici datovou strukturu list
struct list {
  struct list_item *last; // ukazatel na posledni prvek
  struct list_item *first; // ukazatel na prvni prvek
  struct list_item *iterator; // iterator;
  unsigned int count;
};

// vnitrni list struktura
struct list_item {
  void* value; // samotny item (ukazatel na hodnotu)
  struct list_item *next; // ukazatel na predeslou strukturu s itemem
  struct list_item *prev; // ukazatel na dalsi strukturu s itemem
};

struct list * newList();
void insert(struct list *list, void* value);
void* remove_first(struct list *list);
void* remove_last(struct list *list);
void* get_last(struct list *list);
int get_count(struct list *list);
void list_dealloc(struct list *list);


#endif
