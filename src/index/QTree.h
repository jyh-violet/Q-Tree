//
// Created by jyh_2 on 2021/5/14.
//

#ifndef QTREE_QTREE_H
#define QTREE_QTREE_H

#include <unistd.h>
#include "../query/QueryMeta.h"
#include "../query/QueryRange.h"
#include "../holder/RangeHolder.h"
#include "../Tool/ArrayList.h"
#define maxDepth 32
#define Border  65

#define stackPop(stack, index)  stack[ --index]
#define stackPush(stack, index, elem)  stack[index ++] = elem
#define stackEmpty(stack, index)  (index==0)

#define NodeIsLeaf(node)  (((Node*)node)->id >= 0)

extern int checkQueryMeta;
extern int setKeyCount;
extern int markDelete;
extern int WorkEnd;
extern int RefactorThreadEnd;
extern pthread_t RefactorThread;

extern int maxValue;
extern int removePoint;
extern int Qid;

typedef struct InternalNode InternalNode;
typedef struct LeafNode LeafNode;
typedef struct Node Node;

#define MaxThread 100
#define RemovedQueueSize 8
extern BoundKey RemovedKey[MaxThread][RemovedQueueSize];
extern u_int32_t clockFlag[MaxThread];
extern int clockIndex[MaxThread];

extern int useBFPRT;


extern OptimizationType optimizationType;

#define KeyType QueryRange
#define ValueType   QueryMeta
#define MaxBatchCount (Border/2 - 1)
#define batchMissThreshold  100

//extern pthread_key_t threadId;

typedef struct NodesStack{
    InternalNode*   stackNodes[maxDepth];
    int             stackNodesIndex;
}NodesStack;

typedef struct IntStack{
    int          stackSlots[maxDepth];
    int          stackSlotsIndex;
}IntStack;

typedef struct QueryData{
    KeyType key;
    ValueType *value;
}QueryData;

typedef struct QTree {
     int elements;
     int height;
     int maxNodeID;
    Node *root;
    pthread_spinlock_t removeLock;
    int     batchCount[MaxThread];
    int     batchMissCount[MaxThread];
    BoundKey batchSearchKey[MaxThread];
    QueryData batch[MaxThread][MaxBatchCount];
    size_t leafSplitCount;
    size_t internalSplitCount;
    size_t funcCount;
    size_t whileCount;
}QTree;


#define NodeIsValid(node)  (((Node*)node)->allocated >= 0)

#define LockType u_int64_t

typedef struct Node{
    LockType removeLock; // work as the lock for findAndRemove
    LockType insertLock; // work as the lock for insert
    int id ;
    int allocated ;
    int allowSplit;
    BoundKey maxValue ;
    BoundKey minValue;
    BoundKey nextNodeMin;
    Node *right;
    QTree* tree;
}Node;


struct LeafNode {
    Node  node;
    QueryData data[Border];
};

struct InternalNode {
    Node node;
    KeyType  keys[Border];  // array of key
    Node* childs[Border + 1];
};


void QTreeConstructor(QTree* qTree,  int BOrder);
void QTreeDestroy(QTree* qTree);
void printQTree( QTree* qTree);
int  QTreeAllocNode(QTree* qTree, BOOL isLeaf);
void QTreeMakeNewRoot(QTree* qTree, Node* splitedNode);
LeafNode* QTreeFindLeafNode(QTree* qTree, KeyType * key, NodesStack* nodesStack, int threadId);
void QTreeInsert(QTree* qTree, ValueType * value, int threadId);
void QTreeInvalid(QTree* qTree, BoundKey attribute, Arraylist* removedQuery, int threadId);
void QTreePutBatch(QTree* qTree, QueryData * batch, int batchCount, int threadId);
void QTreeCheckBatch(QTree* qTree, BoundKey attribute, Arraylist* removedQuery);
void QTreePutOne(QTree* qTree, QueryRange* key, QueryMeta* value, int threadId);
Node* checkInternalNode(QTree* qTree, InternalNode* nodeInternal,  KeyType* key, NodesStack *nodesStack, IntStack* slotStack, int threadId);
void checkLeafNode(QTree* qTree, LeafNode* leafNode, BoundKey* removedMax, BoundKey* removedMin, BoundKey attribute, Arraylist* removedQuery, int threadId);
Node* getAnotherNode(QTree* qTree, KeyType* key, BoundKey *removedMax, BoundKey *removedMin, Arraylist* removedQuery,
                     NodesStack *nodesStack, IntStack* slotStack, int threadId);
BOOL QTreeCheckMaxMin(QTree* qTree);
void printQTreeStatistics(QTree * qTree);
BOOL CheckLeafNodeCover(LeafNode * leafNode, int i,  BoundKey attribute);
BOOL QTreeCheckKey(QTree* qTree);
void QTreeResetStatistics(QTree* qTree);
BOOL QTreeAddLockForFindLeaf(Node* node, int threadId);
void QTreeRmLockForFindLeaf(Node* node, int threadId);
BOOL QTreeModifyNodeMaxMin(Node* node, BoundKey min, BoundKey max);
Node* QTreeTravelRightLink(Node* node, KeyType * key, int threadId);
void QTreePropagateSplit(QTree* qTree, NodesStack* nodesStack, LeafNode* nodeLeaf, Node* splitedNode, BOOL restMaxMin, BoundKey min, BoundKey max, int threadId);
BOOL QTreeDeleteQuery(QTree* qTree, QueryMeta * queryMeta, int threadId);
Node* checkInternalNodeForDelete(QTree* qTree, InternalNode* nodeInternal,  KeyType* key, NodesStack *nodesStack, IntStack* slotStack, int threadId);
LeafNode* checkLeafNodeForDelete(QTree* qTree, LeafNode* leafNode, QueryMeta * queryMeta, int threadId);
void QTreePropagateMerge(QTree* qTree, Node* lastNode,  NodesStack *nodesStack, IntStack* slotStack, int threadId);

Node* checkInternalNodeForRefactor(QTree* qTree, InternalNode* nodeInternal, NodesStack *nodesStack, IntStack* slotStack, int threadId);
Node* getAnotherNodeForRefactor(QTree* qTree, BoundKey* removedMax, BoundKey* removedMin, NodesStack *nodesStack, IntStack* slotStack, int threadId);
void checkLeafNodeForRefactor(QTree* qTree, LeafNode* leafNode, BoundKey* removedMax, BoundKey* removedMin, int threadId);
void QTreeRefactor(QTree* qTree, int threadId);
BOOL QTreeEvict(QTree* qTree, QueryMeta* queryMeta);
void QTreeRefactorThread(QTree* qTree);

void checkLeafNodeWithoutRemove(QTree* qTree, LeafNode* leafNode, BoundKey* removedMax, BoundKey* removedMin, BoundKey attribute, Arraylist* removedQuery, int threadId);
Node* getAnotherNodeWithoutRemove(QTree* qTree, KeyType* key, BoundKey* removedMax, BoundKey* removedMin, Arraylist* removedQuery,
                                  NodesStack *nodesStack, IntStack* slotStack, int threadId);
void QTreeFindRelatedQueries(QTree* qTree, BoundKey attribute, Arraylist* removedQuery, int threadId);

void NodeCheckTree(Node* node);
void NodeConstructor(Node* node, QTree *tree);
void NodeDestroy(Node* node);
BOOL NodeIsUnderFlow(Node* node);
BOOL NodeCanMerge( Node* node, Node* other) ;
BOOL NodeIsFull(Node* node);
void setSearchKey(KeyType * key, int threadId);
void printNode(Node* node);
Node* NodeSplit(Node* node);
int NodeGetHeight(Node* node);
void NodeResetId(Node* node);
void NodeMerge(Node* node, InternalNode* nodeParent, int slot,
               Node* nodeFROM);
KeyType NodeSplitShiftKeysLeft(Node* node);
BOOL NodeCheckMaxMin(Node * node);
BOOL NodeCheckKey(Node * node);
BOOL NodeCheckLink(Node* node);


void LeafNodeConstructor(LeafNode* leafNode, QTree *tree);
void LeafNodeDestroy(LeafNode* leafNode);
void LeafNodeMerge(LeafNode* leafNode, InternalNode* nodeParent, int slot,
                   Node* nodeFROMx);
void * LeafNodeRemove(LeafNode* leafNode, int slot);
BOOL LeafNodeAdd(LeafNode* leafNode, int slot, KeyType * newKey, ValueType * newValue);
BOOL LeafNodeAddLast(LeafNode* leafNode, KeyType * newKey, ValueType * newValue);
BOOL LeafNodeAddLastBatch(LeafNode* leafNode, QueryData* data, int count, BoundKey *min, BoundKey* max);
void LeafNodeAllocId(LeafNode* leafNode);
void LeafNodeResetMaxValue(LeafNode* node);
void LeafNodeResetMinValue(LeafNode* node);
Node* LeafNodeSplit(LeafNode* leafNode);
KeyType  LeafNodeSplitShiftKeysLeft(LeafNode* node);
int LeafNodeGetId(LeafNode* node) ;
int LeafNodeGetHeight(LeafNode* node);
void LeafNodeResetId(LeafNode* node);
void printLeafNode(LeafNode* leafNode);
void LeafNodeResetMinKey(LeafNode* leafNode);
BOOL LeafNodeCheckMaxMin(LeafNode * leafNode);
Node* LeafNodeSplit_NoSort(LeafNode* leafNode, LeafNode* newHigh) ;
Node* LeafNodeSplit_Sort(LeafNode* leafNode, LeafNode* newHigh);
BOOL LeafNodeCheckKey(LeafNode * leafNode);
BOOL LeafNodeAddBatch(LeafNode* leafNode, int slot, QueryData batch[], int batchCount, BoundKey *min, BoundKey* max);
int LeafNodeFindSlotByKey( LeafNode * node, KeyType* searchKey) ;
BOOL LeafNodeCheckLink(LeafNode* node);

void InternalNodeConstructor(InternalNode* internalNode, QTree* qTree);
void InternalNodeDestroy(InternalNode* internalNode);
BOOL InternalNodeAdd(InternalNode* node, int slot, KeyType * newKey, Node* child);
void InternalNodeResetMaxValue(InternalNode* node);
void InternalNodeResetMinValue(InternalNode* node);
void InternalNodeAllocId(InternalNode* node);
Node* InternalNodeSplit(InternalNode* node);
KeyType  InternalNodeSplitShiftKeysLeft(InternalNode* node);
int InternalNodeGetId(InternalNode* node);
int InternalNodeGetHeight(InternalNode* node);
void InternalNodeResetId(InternalNode* node);
BOOL InternalNodeCheckUnderflowWithRight(InternalNode* node, int slot);
void  InternalNodeRemove(InternalNode* node, int slot);
void InternalNodeMerge(Node* node, InternalNode* nodeParent, int slot, Node* nodeFROMx);
void printInternalNode(InternalNode* internalNode);
BOOL InternalNodeCheckMaxMin(InternalNode * internalNode);
BOOL InternalNodeCheckKey(InternalNode * internalNode);
int InternalNodeFindSlotByKey( InternalNode * node, KeyType* searchKey) ;
int InternalNodeFindSlotByNextMin( InternalNode* node, BoundKey nextMin);
BOOL InternalNodeCheckLink(InternalNode * node);
int InternalNodeFindSlotByChild( InternalNode* node, Node* child);
int InternalNodeFindSlotByChildWithRight( InternalNode* node, Node* child);
int InternalNodeFindMinSlotByKey( InternalNode* node, BoundKey key) ;

void quickSelect(QueryData data[], int k, int s, int e);
//void quickSelect(KeyType arr[], int k, int s, int e);
void swap(KeyType arr[], int a, int b);

int getThreadId();

BoundKey NodeGetMaxValue(Node* node);
BoundKey NodeGetMinValue(Node* node);
void NodeRmInsertReadLockNoLog(Node* node, int threadId);

void NodeAddInsertWriteLock(Node* node);
void NodeRmInsertWriteLock(Node* node);
void NodeAddRemoveReadInsertWriteLock(Node* node, int threadId);
void NodeRmRemoveReadInsertWriteLock(Node* node, int threadId);
void NodeRmRemoveReadLock(Node* node, int threadId);
void NodeAddRemoveReadLock(Node* node, int threadId);
void NodeAddInsertReadLock(Node* node, int threadId);
void NodeRmInsertReadLock(Node* node, int threadId);
BOOL NodeTryAddInsertWriteLock(Node* node);
BOOL NodeTryAddInsertReadLock(Node* node, int threadId);
void NodeDegradeInsertLock(Node* node, int threadId);
void NodeAddRemoveWriteLock(Node* node);
void NodeRmRemoveWriteLock(Node* node);
BOOL NodeTryAddRemoveWriteLock(Node* node);
BOOL NodeAddRemoveWriteLockNoWait(Node* node);
BOOL NodeTryAddInsertWriteLockForRemove(Node* node);
void NodeRmInsertWriteLockForRemove(Node* node);
#endif //QTREE_QTREE_H
