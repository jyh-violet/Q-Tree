//
// Created by jyh_2 on 2021/5/14.
//

#include "QueryMeta.h"
int queryCount;
char* getQueryId(QueryMeta* queryMeta){
    return queryMeta->queryId;
}

BOOL QueryMetaCover(QueryMeta* queryMeta, BoundKey value){
    return  QueryRangeCover((queryMeta->dataRegion), (value));
}
void QueryMetaConstructor(QueryMeta* queryMeta){
    queryCount++;
    memset((void *)queryMeta, 0, sizeof (queryMeta));
    myItoa( queryCount, queryMeta->queryId);
    QueryRangeConstructor(&queryMeta->dataRegion);
}

void QueryMetaConstructorWithPara(QueryMeta* queryMeta, char* id, BoundKey lower, BoundKey upper){
    memset((void *)queryMeta, 0, sizeof (queryMeta));
    strncpy(queryMeta->queryId, id, queryIdLen - 1);
    QueryRangeConstructorWithPara(&queryMeta->dataRegion, lower, upper);
}

void QuerySetDeleteFlag(QueryMeta* queryMeta){
    queryMeta->deleted = 1;
}

BOOL QueryIsDeleted(QueryMeta* queryMeta){
    return queryMeta->deleted == 1;
}

BOOL QueryIdCmp(void *queryMeta1, void * queryMeta2){
    return (BOOL)strcmp(((QueryMeta*)queryMeta1)->queryId, ((QueryMeta*)queryMeta2)->queryId);
}

void QueryMetaDestroy(QueryMeta* queryMeta){
}

void printQueryMeta(QueryMeta* queryMeta){
    printf("Q[%s]", queryMeta->queryId);
    printQueryRange(&queryMeta->dataRegion);
}