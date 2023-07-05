//
// Created by jyh_2 on 2021/6/18.
//

#include "QueryRange.h"

static int rangeCount;
extern  DataRegionType dataRegionType;
extern  DataPointType dataPointType;
extern int maxValue;
extern int removePoint;
extern int rangeWidth;


void QueryRangeConstructor(QueryRange *queryRange){

    int r, diff, randNum;

    switch (dataRegionType){
        case Same:
            queryRange->lower = rangeCount;
            queryRange->upper = rangeCount + 1;
            break;
        case  Random:
            r = (int) RAND_RANGE(maxValue);
            diff = (int) RAND_RANGE(rangeWidth);
            switch (dataPointType) {
                case RemovePoint:
                    queryRange->lower = r + rangeWidth - diff / 4 * 3;
                    queryRange->upper = r + rangeWidth + diff / 4 ;
                    break;
                case LowPoint:
                    queryRange->lower = r + rangeWidth ;
                    queryRange->upper = r + rangeWidth + diff ;
                    break;
                case MidPoint:
                    queryRange->lower = r + rangeWidth - diff / 2;
                    queryRange->upper = r + rangeWidth + diff / 2;
                    break;
            }

            break;
        case Zipf:
            randNum = zipf();
            diff = (int) RAND_RANGE(rangeWidth);
            switch (dataPointType) {
                case RemovePoint:
                    queryRange->lower = randNum + rangeWidth - diff / 4 * 3;
                    queryRange->upper = randNum + rangeWidth + diff / 4 ;
                    break;
                case  LowPoint:
                    queryRange->lower = randNum + rangeWidth ;
                    queryRange->upper = randNum + rangeWidth + diff ;
                    break;
                case  MidPoint:
                    queryRange->lower = randNum + rangeWidth - diff / 2;
                    queryRange->upper = randNum + rangeWidth + diff / 2;
                    break;
            }
            break;
        case Increase:
            queryRange->lower = rangeCount ++ ;
            queryRange->upper = rangeCount ++ ;
            break;
        case Remove:
            randNum = zipf();
            queryRange->lower =  removePoint  + randNum + rangeWidth ;
            queryRange->upper =  removePoint  + randNum + rangeWidth;
            break;
    }
//    queryRange->boundInclude = 3;
    queryRange->searchKey  = -1;
}

void QueryRangeConstructorWithPara(QueryRange *queryRange, BoundKey startKey, BoundKey endKey){
    queryRange->lower = startKey;
    queryRange->upper = endKey;
    queryRange->searchKey = -1;
}

void printQueryRange(QueryRange* queryRange){
    printf("{%d,%d}", queryRange->lower, queryRange->upper);
}