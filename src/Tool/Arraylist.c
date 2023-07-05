//
// Created by workshop on 9/5/2021.
//
#include "ArrayList.h"


Arraylist *ArraylistCreate(size_t initCapacity) {
    /* Allocate Memory */
    struct _arraylist *list = (struct _arraylist *)malloc(sizeof(struct _arraylist));
    assert(list != NULL);
    list->size = 0;
    list->capacity = initCapacity;
    list->data = (void**)malloc(sizeof(void *) * list->capacity);
    assert(list->data != NULL);
    list->data[0] = NULL;
    return list;
}


void ArraylistAdd(struct _arraylist *list, void *elem) {
    /* Adds one element of generic pointer type to the internal array */
    if(list->size < (list->capacity - 1)){
        list->data[list->size ++] = elem;
    } else{
        list->capacity *= 2;
        void *olddata = list->data;
        list->data = (void**)malloc(sizeof (void *) * list->capacity);
        memcpy(list->data, olddata, list->size * sizeof (void *));
        free(olddata);
        list->data[list->size ++] = elem;
    }
}

void *ArraylistGet(struct _arraylist *list, int index) {
    /* Gets an member of the array at an index */
    return list->data[index];
}

size_t ArraylistGetSizeof(struct _arraylist *list) {
    /* Returns the size of the internal array in memory */
    return sizeof(*list->data);
}
size_t ArraylistGetsize(struct _arraylist *list) {
    /* Returns the number of elements in the arraylist */
    return list->size;
}

void ArraylistRemove(struct _arraylist *list, int index) {
    /* Removes one element at and index */
    if (index > list->size - 1)
        return;
    if (list->size == 1) {
        ArraylistClear(list);
        return;
    }
    for ( int i = index; i < list->size; ++i ) {
        if (i == list->size - 1)
            list->data[i] = NULL;
        else
            list->data[i] = list->data[i + 1];
    }
    --list->size;
}

void ArraylistClear(struct _arraylist *list) {
    /* Clears the internal array */
    list->size = 0;
    memset(list->data, 0, list->capacity * sizeof (void *));
}

void ArraylistDestroy(struct _arraylist *list) {
    /* Clears the internal array */
    list->size = 0;
    free(list->data);
    list->data = NULL;
}

void ArraylistDeallocate(struct _arraylist *list) {
    /* De-allocates the arraylist from memory
    No usage of the arraylist is allowed after this function call */
    if (list->data != NULL)
        free(list->data);
    free(list);
}

int ArraylistGetindex(struct _arraylist *list, void *elem) {
    /* Looks for elem in list and returns the index or -1 if not found */
    for(int i = 0; i < list->size; ++i){
        if (elem == ArraylistGet(list, i))
            return i;
    }
    return -1;
}