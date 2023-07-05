//
// Created by jyh_2 on 2021/5/14.
//
#include <stdlib.h>
#include <string.h>
#include "../common.h"
#include "QueryRange.h"


#ifndef QTREE_QUERYMETA_H
#define QTREE_QUERYMETA_H

extern int queryCount;
#define queryIdLen 20
typedef struct QueryMeta {
    char    queryId[queryIdLen];
    char    deleted;
    QueryRange  dataRegion;  //data region of the query
}QueryMeta;

char* getQueryId(QueryMeta* queryMeta);
BOOL QueryMetaCover(QueryMeta* queryMeta, BoundKey value);
void QueryMetaConstructor(QueryMeta* queryMeta);
void QueryMetaConstructorWithPara(QueryMeta* queryMeta, char* id, BoundKey lower, BoundKey upper);
BOOL QueryIdCmp(void *queryMeta1, void * queryMeta2);
void QueryMetaDestroy(QueryMeta* queryMeta);
void printQueryMeta(QueryMeta* queryMeta);
void QuerySetDeleteFlag(QueryMeta* queryMeta);
BOOL QueryIsDeleted(QueryMeta* queryMeta);
#endif //QTREE_QUERYMETA_H
