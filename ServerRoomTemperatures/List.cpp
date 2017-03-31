/* UZ SE NEPOUZIVA */
#include "List.h"
#include <Arduino.h>

// vytvori novy list
struct list * newList() {
  struct list *l = (struct list *) malloc (sizeof(struct list));
  l->last = NULL;
  l->first = NULL;
  l->iterator = NULL;
  l->count = 0;
  return l;
}

// vlozi item  na konec listu
void insert(struct list *list, void* value) {
  //vytvorim prvek
  struct list_item *item = (struct list_item *) malloc (sizeof(struct list_item));
  // nastavim prvku hodnotu
  item->value = value;
  // prvek nema naslednovnika
  item->next = NULL; 
  // nastavim dosadavdni posledni prvek jako predchozi noveho prvku
  item->prev = list->last;

  if (list->last) {
    list->last->next = item;
  } else {
    list->first = item;
  }
  // nastavim novy prvek jako posledni
  list->last = item;
  ++list->count;
}


// odebere item ze zacatku listu
void* remove_first(struct list *list) {
  struct list_item *first_item = list->first;
  if (first_item) {
    list->first = list->first->next;
    list->first->prev = NULL;
    void* val = first_item->value;
    free(first_item);
    --list->count;
    return val;
  } else {
    return NULL;
  }
}

// odebere item z konce listu
void* remove_last(struct list *list) {
  struct list_item *last_item = list->last;
  if (last_item) {
    list->last = list->last->prev;
    list->last->next = NULL;
    void* val = last_item->value;
    free(last_item);
    --list->count;
    return val;
  } else {
    return NULL;
  }
}

// ziska item z konce listu
void* get_last(struct list *list) {
  struct list_item *last_item = list->last;
  if (last_item) {
    return last_item->value;
  } else {
    return NULL;
  }
}

// zjisti, zda lze iterator inkrementovat (nastavit na dalsi prvek), pokud ne jsem na konci listu
bool iterator_hasNext(struct list *list) {
  return list->iterator->next != NULL;
}

// nastavi iterator na dalsi prvek
struct list_item * iterator_next(struct list *list) {
  return list->iterator = list->iterator->next;
}


// Odstrani z listu hodnotu, na kterou ukazuje iterator
void* iterator_remove(struct list *list) {
  // Pokud je napravo i nalevo neco
  if (list->iterator->next && list->iterator->prev) {
    list->iterator->prev->next = list->iterator->next;
    list->iterator->next->prev = list->iterator->prev;
    void* val = list->iterator->value;
    free(list->iterator);
    --list->count;
    return val;
  } else if (list->iterator->next) {
    return remove_last(list);
  } else {
    return remove_first(list);
  }
}

// odstrani z listu hodnotu na danem indexu
void* remove_index(struct list *list, unsigned int index) {
  list->iterator = list->last;
  while(list->iterator && index) {
    iterator_next(list);
    --index;
  }
  if (index) {
    return NULL;
  }
  iterator_remove(list);
}

// odsrani z listu danou hodnotu (prvni nalezenou)
void* remove_value(struct list *list, void* value) {
  list->iterator = list->last;
  while(list->iterator->value != value && list->iterator) {
    iterator_next(list);
  }
  if (list->iterator->value == value) {
    return iterator_remove(list);
  }
  return NULL;
}

// ziska pocet itemu v listu
int get_count(struct list *list) {
  return list->count;
}

// dealokuje cely list
void list_dealloc(struct list *list) {
  while (get_count(list)) {
    free(remove_last(list));
  }
  free(list);
}

