#include "query/QueryRange.h"
#include "query/QueryMeta.h"
#include "index/QTree.h"
#include <libconfig.h>
#include <time.h>
#include <Tool/ArrayList.h>
#include <papi.h>
#include <pthread.h>

DataRegionType dataRegionType = Zipf;
DataPointType dataPointType = MidPoint;
int valueSpan = 30; // 2 ^valueSpan
int maxValue ;
SearchKeyType searchKeyType = RAND;
int Qid = 0;
int TOTAL = (int) 100, TRACE_LEN = 100000;
double insertRatio = 0;
double deleteRatio = 0;

u_int64_t checkLeaf = 0;
u_int64_t checkQuery = 0;
u_int64_t checkInternal = 0;
int removePoint = 0;
double zipfPara = 0.75;
int rangeWidth = 100;
extern __thread threadID;

_Atomic int insertNum = 0, removeNum = 0, deleteNum = 0;
int threadnum = 4;

typedef struct TaskRes{
    double usedTime;
    size_t size;
}TaskRes;
typedef struct ThreadAttributes{
    int threadId;
    QTree* qTree;
    QueryMeta* insertQueries ;
    QueryMeta* queries ;
    QueryMeta* removeQuery;
    double *mixPara;
    int start;
    int end;
    TaskRes  result;
}ThreadAttributes;

void testInsert(ThreadAttributes* attributes){
    threadID = attributes->threadId;
    struct timespec startTmp, endTmp;
    clock_gettime(CLOCK_REALTIME, &startTmp);
    for(    int i = attributes->start; i <  attributes->end; i ++){
        int index = (i - attributes->start) * threadnum + attributes->threadId;
        QTreeInsert(attributes->qTree, attributes->insertQueries + index, attributes->threadId);
    }
    clock_gettime(CLOCK_REALTIME, &endTmp);
    attributes->result.usedTime = (endTmp.tv_sec - startTmp.tv_sec) + (endTmp.tv_nsec - startTmp.tv_nsec) * 1e-9;
    attributes->result.size = attributes->end - attributes->start;
}

void testMix(ThreadAttributes* attributes){
    threadID = attributes->threadId;
    Arraylist* removedQuery = ArraylistCreate(attributes->end - attributes->start);
    struct timespec startTmp, endTmp;
    clock_gettime(CLOCK_REALTIME, &startTmp);
    for (int i = attributes->start; i <  attributes->end; ++i) {
        if(attributes->mixPara[i] < insertRatio){
            if(insertRatio < 1 && attributes->qTree->elements > TOTAL){
                //cache is full, delete first
                if(markDelete){
                    if(QTreeEvict(attributes->qTree, attributes->insertQueries + i) == TRUE){
                        attributes->result.size ++;
                    }
                } else{
                    if (QTreeDeleteQuery(attributes->qTree, attributes->insertQueries + i, attributes->threadId) == TRUE){
                        attributes->result.size ++;
                    }
                }
                deleteNum ++;
            }

            QTreeInsert(attributes->qTree, attributes->queries + i, attributes->threadId);
            insertNum ++;
        }else if(attributes->mixPara[i] < (insertRatio + deleteRatio)){

        } else{
            QTreeInvalid(attributes->qTree,
                                             (attributes->removeQuery[i].dataRegion.upper + attributes->removeQuery[i].dataRegion.lower) / 2,
                                             removedQuery,
                                             attributes->threadId);

            removeNum ++;
        }
    }
    clock_gettime(CLOCK_REALTIME, &endTmp);
    attributes->result.usedTime = (endTmp.tv_sec - startTmp.tv_sec) + (endTmp.tv_nsec - startTmp.tv_nsec) * 1e-9;
    attributes->result.size += removedQuery->size;
    ArraylistDeallocate(removedQuery);
}


int test() {
    useBFPRT = 0;
    double generateT = 0, putT = 0,  mixT = 0;
    maxValue = 1 << valueSpan;
    TRACE_LEN = 1000;
    srand((unsigned)time(NULL));
    initZipfParameter(TOTAL, zipfPara);
    clock_t   start,   finish;
    QTree qTree;
    QTreeConstructor(&qTree, 2);
    double *mixPara = (double *) malloc(sizeof (double ) * TOTAL);
    for (int i = 0; i < TOTAL; ++i) {
        int randNum = rand();
        mixPara[i] =    ((double )randNum) / ((double )RAND_MAX + 1);
    }

    QueryMeta* insertQueries = (QueryMeta*)malloc(sizeof(QueryMeta) * TOTAL );
    QueryMeta* queries = (QueryMeta*)malloc(sizeof(QueryMeta) * TOTAL );
    QueryMeta* removeQuery = (QueryMeta*)malloc(sizeof(QueryMeta) * TOTAL);

    start = clock();
    for(int i = 0; i < TOTAL ;i ++){
        QueryMetaConstructor(queries + i);
        QueryMetaConstructor(insertQueries + i);
    }
    DataRegionType  dataRegionTypeOld = dataRegionType;
    for(int i = 0; i < TOTAL;i ++){
        QueryMetaConstructor(removeQuery + i);
    }
    finish = clock();
    generateT = (double)(finish - start)/CLOCKS_PER_SEC;
    int perThread = TOTAL / threadnum;
    pthread_t thread[MaxThread];
    ThreadAttributes attributes[MaxThread];
    memset(attributes, 0, sizeof (ThreadAttributes) * MaxThread);
    for (int i = 0; i < threadnum; ++i) {
        attributes[i].threadId = i;
        attributes[i].start = i * perThread;
        attributes[i].end = i == (threadnum - 1)? TOTAL: (i + 1)* perThread;
        attributes[i].insertQueries = insertQueries;
        attributes[i].queries = queries;
        attributes[i].removeQuery = removeQuery;
        attributes[i].qTree = &qTree;
        attributes[i].mixPara = mixPara;
        pthread_create(&thread[i], 0, (void *(*)(void *))testInsert, (void *)&attributes[i]);
    }
    for (int i = 0; i < threadnum; ++i) {
        pthread_join(thread[i], NULL);
        putT += attributes[i].result.usedTime;
    }
    putT = putT / threadnum;

    size_t removed = 0;
    QTreeResetStatistics(&qTree);

    printLog = 1;

    perThread = TOTAL / threadnum;
    memset(attributes, 0, sizeof (ThreadAttributes) * MaxThread);
    for (int i = 0; i < threadnum; ++i) {
        attributes[i].threadId = i;
        attributes[i].start = i * perThread;
        attributes[i].end = i == (threadnum - 1)? TOTAL: (i + 1)* perThread;
        attributes[i].insertQueries = insertQueries;
        attributes[i].queries = queries;
        attributes[i].removeQuery = removeQuery;
        attributes[i].qTree = &qTree;
        attributes[i].mixPara = mixPara;
        pthread_create(&thread[i], 0, (void *(*)(void *))testMix, (void *)&attributes[i]);
    }

    for (int i = 0; i < threadnum; ++i) {
        pthread_join(thread[i], NULL);
        removed += attributes[i].result.size;
        mixT += attributes[i].result.usedTime;
    }
    mixT = mixT / threadnum;

    if(markDelete){
        WorkEnd = TRUE;
        while (RefactorThreadEnd == FALSE){
            WorkEnd = TRUE;
        }
    }
    QTreeDestroy(&qTree);

printf("%d,  %d,  %.3lf,%.3lf,%.3lf\n",
       Border,TOTAL,
           generateT, putT, mixT);
    free(queries) ;
    free(removeQuery);
    free(insertQueries);
    return 0;
}

int main(){
    const char ConfigFile[]= "config.cfg";

    config_t cfg;

    config_init(&cfg);

    /* Read the file. If there is an error, report it and exit. */
    if(! config_read_file(&cfg, ConfigFile))
    {
        fprintf(stderr, "%s:%d - %s\n", config_error_file(&cfg),
                config_error_line(&cfg), config_error_text(&cfg));
        config_destroy(&cfg);
        return(EXIT_FAILURE);
    }
    config_lookup_int(&cfg, "TOTAL", &TOTAL);
    config_lookup_int(&cfg, "dataRegionType", (int*)&dataRegionType);
    config_lookup_int(&cfg, "valueSpan", &valueSpan);
    config_lookup_float(&cfg, "insertRatio", &insertRatio);
    config_lookup_int(&cfg, "threadnum", &threadnum);
    config_lookup_float(&cfg, "zipfPara", &zipfPara);
    config_lookup_int(&cfg, "rangeWidth", &rangeWidth);
    maxValue = TOTAL;
    markDelete = 1;
    optimizationType = NoSort;

    test();
    return 0;
}