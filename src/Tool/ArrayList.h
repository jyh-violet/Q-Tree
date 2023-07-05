//
// Created by workshop on 9/5/2021.
//

#ifndef QTREE_ARRAYLIST_H
#define QTREE_ARRAYLIST_H
#include <stdlib.h>
#include <assert.h>
#include <string.h>

typedef struct _arraylist {
    size_t capacity;
    size_t size;
    void ** data;
}Arraylist;

Arraylist *ArraylistCreate(size_t initCapacity) ;
void ArraylistSetData(struct _arraylist *list, void ** data, int max, int clear_data);
void ArraylistAdd(struct _arraylist *list, void *elem);
void *ArraylistGet(struct _arraylist *list, int index);
size_t ArraylistGetSizeof(struct _arraylist *list);
size_t ArraylistGetsize(struct _arraylist *list);
void ArraylistRemove(struct _arraylist *list, int index);
void ArraylistClear(struct _arraylist *list);
void ArraylistDestroy(struct _arraylist *list);
void ArraylistDeallocate(struct _arraylist *list);
int ArraylistGetindex(struct _arraylist *list, void *elem);

#endif //QTREE_ARRAYLIST_H
