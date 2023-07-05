//
// Created by jyh_2 on 2021/6/18.
//

#ifndef QTREE_QUERYRANGE_H
#define QTREE_QUERYRANGE_H

#include "../common.h"


#define QueryRangeLT(queryRange1, queryRange2) \
(BOOL)(((QueryRange)queryRange1).searchKey < ((QueryRange)queryRange2).searchKey)

#define QueryRangeGT(queryRange1, queryRange2)  \
(BOOL)(((QueryRange)queryRange1).searchKey > ((QueryRange)queryRange2).searchKey)

#define QueryRangeMaxGE(queryRange1, queryRange2)  \
(BOOL)(((QueryRange)queryRange1).upper >= ((QueryRange)queryRange2).upper)

#define QueryRangeMinGT(queryRange1, queryRange2) \
(BOOL)(((QueryRange)queryRange1).lower > ((QueryRange)queryRange2).lower)

#define QueryRangeCover(queryRange, value)  \
(BOOL)(((QueryRange)(queryRange)).lower <= (int)value && ((QueryRange)(queryRange)).upper >= (int)value)

#define RAND_RANGE(N) ((double)rand() / ((double)RAND_MAX + 1) * (N))

#define BoundKey int
//#define MAXKey   0xFFFFFFFFFFFFFFFF
#define MAXKey   RAND_MAX
typedef struct QueryRange{
    BoundKey lower; // lower bound of the region
    BoundKey upper;  // upper bound of the region
    BoundKey searchKey; // in [lower, upper], used as the search key in the tree
}QueryRange;


void QueryRangeConstructor(QueryRange *queryRange);
void QueryRangeConstructorWithPara(QueryRange *queryRange, BoundKey startKey, BoundKey endKey);
void printQueryRange(QueryRange* queryRange);
#endif //QTREE_QUERYRANGE_H
