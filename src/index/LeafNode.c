//
// Created by workshop on 9/5/2021.
//
#include "QTree.h"

inline void LeafNodeConstructor(LeafNode* leafNode, QTree *tree){
    memset(leafNode, 0, sizeof (LeafNode));
    NodeConstructor((Node*)leafNode, tree);
//    leafNode->values = malloc(sizeof (ValueType *) * tree->Border);
//    memset(leafNode->values,0, sizeof (ValueType *) * Border);
}
void LeafNodeDestroy(LeafNode* leafNode){
    free(leafNode);
}

void LeafNodeMerge(LeafNode* leafNode, InternalNode* nodeParent, int slot,
                   Node* nodeFROMx){
    LeafNode* nodeFROM = (LeafNode*) nodeFROMx;
    LeafNode* nodeTO = leafNode;
    int sizeTO = nodeTO->node.allocated;
    int sizeFROM = nodeFROM->node.allocated;
    if(sizeFROM > 0){
        // copy keys from nodeFROM to nodeTO
        memcpy(nodeTO->data + sizeTO , nodeFROM->data, sizeFROM * sizeof(QueryData ));

        nodeTO->node.allocated += sizeFROM; // keys of FROM and key of nodeParent
        if(sizeTO == 0 || (sizeFROM > 0 && nodeTO->node.maxValue < nodeFROM->node.maxValue)){
            nodeTO->node.maxValue = nodeFROM->node.maxValue;
        }
        if(sizeTO == 0 || (sizeFROM > 0 && (nodeTO->node.minValue) > (nodeFROM->node.minValue))){
            nodeTO->node.minValue = nodeFROM->node.minValue;
        }
    }
    nodeTO->node.right = nodeFROMx->right;
    nodeTO->node.nextNodeMin = nodeFROMx->nextNodeMin;
    // remove key from nodeParent
    InternalNodeRemove(nodeParent, slot);
    // Free nodeFROM
//    vmlog(MiXLog, "LeafNodeMerge, rm node:%d, pointer:%lx", nodeFROM->node.id, nodeFROM);
    nodeFROM->node.allocated = -1;
    if(nodeFROMx->insertLock == 0){
//        free((Node*)nodeFROM);
    }
}

void * LeafNodRemove(LeafNode* leafNode, int slot) {
    return NULL;
}

BOOL LeafNodeAdd(LeafNode* leafNode, int slot, KeyType * newKey, ValueType * newValue){
    if(slot >= Border){

        printLeafNode(leafNode);
        vmlog(ERROR, "LeafNodeAdd ERROR!! slot:%d\n", slot);
    }
    BOOL restMaxMin = FALSE;
    if (slot < leafNode->node.allocated) {
        memcpy(leafNode->data + slot + 1, leafNode->data + slot, (leafNode->node.allocated - slot) * sizeof(QueryData ));
    }

    leafNode->data[slot].key = *newKey;
    leafNode->data[slot].value = newValue;
    if(leafNode->node.allocated == 0 || (newKey->upper > NodeGetMaxValue((Node*)leafNode))){
        restMaxMin = TRUE;
        leafNode->node.maxValue = newKey->upper;
    }
    if(leafNode->node.allocated == 0 || ((NodeGetMinValue((Node*)leafNode)) >  newKey->lower)){
        restMaxMin = TRUE;
        leafNode->node.minValue = newKey->lower;
    }
    ++leafNode->node.allocated;
    return restMaxMin;
}
BOOL LeafNodeAddBatch(LeafNode* leafNode, int slot, QueryData batch[], int batchCount, BoundKey *min, BoundKey* max){
    BOOL restMaxMin = FALSE;
    if(leafNode->node.allocated + batchCount > Border){
        vmlog(WARN, "LeafNodeAddBatch, batch too large. allocated:%d, batchCount:%d\n", leafNode->node.allocated, batchCount);
        return FALSE;
    }
    if (slot < leafNode->node.allocated) {
        memcpy(leafNode->data + slot + batchCount, leafNode->data + slot, (leafNode->node.allocated - slot) * sizeof(QueryData ));
    }
    memcpy(leafNode->data + slot, batch, batchCount * sizeof (QueryData));

    BoundKey localMin = batch[0].key.lower, localMax = batch[0].key.upper;
    for (int i = 1; i < batchCount; ++i) {
        if( (batch[i].key.upper >  localMax)){
            localMax = batch[i].key.upper;
        }
        if( (batch[i].key.lower <  localMin)){
            localMin = batch[i].key.lower;
        }
    }
    if(leafNode->node.allocated == 0 || leafNode->node.maxValue < localMax){
        restMaxMin = TRUE;
        leafNode->node.maxValue = localMax;
    }
    if(leafNode->node.allocated == 0 || leafNode->node.minValue > localMin){
        restMaxMin = TRUE;
        leafNode->node.minValue = localMin;
    }
    if(localMin < *min){
        *min = localMin;
    }
    if(localMax > *max){
        *max = localMax;
    }
    leafNode->node.allocated += batchCount;
    return restMaxMin;
}

BOOL LeafNodeAddLast(LeafNode* leafNode, KeyType * newKey, ValueType * newValue){
    BOOL restMaxMin = FALSE;
    leafNode->data[leafNode->node.allocated].key = *newKey;
    leafNode->data[leafNode->node.allocated].value = newValue;
    leafNode->node.allocated++;
    if(leafNode->node.allocated == 1 || (newKey->upper >  NodeGetMaxValue((Node*)leafNode))){
        restMaxMin = TRUE;
        leafNode->node.maxValue = newKey->upper;
    }
    if(leafNode->node.allocated == 1 || ((NodeGetMinValue((Node*)leafNode)) >  newKey->lower)){
        restMaxMin = TRUE;
        leafNode->node.minValue = newKey->lower;
    }
    return restMaxMin;
}

BOOL LeafNodeAddLastBatch(LeafNode* leafNode, QueryData* data, int count, BoundKey *min, BoundKey* max){
    BOOL restMaxMin = FALSE;
    if(leafNode->node.allocated + count > Border){
        vmlog(WARN, "LeafNodeAddLastBatch batch too many");
        return FALSE;
    }
    memcpy(leafNode->data + leafNode->node.allocated, data, count * sizeof (QueryData));
    BoundKey localMin = data[0].key.lower, localMax = data[0].key.upper;
    for (int i = 1; i < count; ++i) {
        if( (data[i].key.upper >  localMax)){
            localMax = data[i].key.upper;
        }
        if( (data[i].key.lower <  localMin)){
            localMin = data[i].key.lower;
        }
    }
    if(leafNode->node.allocated == 0 || leafNode->node.maxValue < localMax){
        restMaxMin = TRUE;
        leafNode->node.maxValue = localMax;
    }
    if(leafNode->node.allocated == 0 || leafNode->node.minValue > localMin){
        restMaxMin = TRUE;
        leafNode->node.minValue = localMin;
    }
    if(localMin < *min){
        *min = localMin;
    }
    if(localMax > *max){
        *max = localMax;
    }
    leafNode->node.allocated+= count;
    return restMaxMin;
}


inline void LeafNodeAllocId(LeafNode* leafNode) {
    leafNode->node.id = QTreeAllocNode(leafNode->node.tree, (TRUE));
}

inline void LeafNodeResetMaxValue(LeafNode* leafNode) {
    if (leafNode->node.allocated == 0) {
        return;
    }
    leafNode->node.maxValue = leafNode->data[0].key.upper;
    for (int i = 1; i < leafNode->node.allocated; i++) {
        if ((leafNode->data[i].key.upper) > (leafNode->node.maxValue)) {
            leafNode->node.maxValue = leafNode->data[i].key.upper;
        }
    }
}


inline void LeafNodeResetMinValue(LeafNode* leafNode) {
    if (leafNode->node.allocated == 0) {
        return;
    }
    leafNode->node.minValue = leafNode->data[0].key.lower;
    for (int i = 1; i < leafNode->node.allocated; i++) {
        if (((leafNode->node.minValue)> leafNode->data[i].key.lower)) {
            leafNode->node.minValue = leafNode->data[i].key.lower;
        }
    }
}

inline void swap(KeyType arr[], int a, int b){
    KeyType tmp = arr[a];
    arr[a] = arr[b];
    arr[b] = tmp;
}

void InsertSortIndex(QueryData data[], int l, int r)
{
    for(int i = l + 1; i <= r; i++)
    {
        if(data[i - 1].key.searchKey > data[i].key.searchKey)
        {
            QueryData t = data[i];
            int j = i;
            while(j > l && data[j - 1].key.searchKey > t.key.searchKey)
            {
                data[j] = data[j - 1];
                j--;
            }
            data[j] = t;
        }
    }
}

int FindMidIndex(QueryData data[], int l, int r)
{
    if(l == r) return l;
    int i = 0;
    int n = 0;
    for(i = l; i < r - 5; i += 5)
    {
        InsertSortIndex(data, i, i + 4);
        n = i - l;
        QueryData temp = data[l + n / 5];
        data[l + n / 5] = data[i + 2];
        data[i + 2] = temp;
    }

    //处理剩余元素
    int num = r - i + 1;
    if(num > 0)
    {
        InsertSortIndex(data, i, i + num - 1);
        n = i - l;
        QueryData temp = data[l + n / 5];
        data[l + n / 5] = data[i + num / 2];
        data[i + num / 2] = temp;
    }
    n /= 5;
    if(n == l) return l;
    return FindMidIndex(data, l, l + n);
}

int PartionIndex(QueryData data[], int l, int r, int p)
{
    QueryData temp = data[p];
    data[p] = data[l];
    data[l] = temp;
    int i = l;
    int j = r;
    QueryData pivot = data[l];
    while(i < j)
    {
        while(data[j].key.searchKey >= pivot.key.searchKey && i < j)
            j--;
        data[i] = data[j];
        while(data[i].key.searchKey <= pivot.key.searchKey && i < j)
            i++;
        data[j] = data[i];
    }
    data[i] = pivot;
    return i;
}

int BFPRTSelect(QueryData data[], int l, int r, int k)
{
    int p = FindMidIndex(data,  l, r);    //寻找中位数的中位数
    int i = PartionIndex(data,  l, r, p);

    int m = i - l + 1;
    if(m == k) return i;
    if(m > k)  return BFPRTSelect(data, l, i - 1, k);
    return BFPRTSelect(data, i + 1, r, k - m);
}

inline void quickSelect(QueryData data[], int k, int s, int e){
    if(s > e || k < s || k > e){
        printf("invalid array range\n");
        return;
    }

    short i, j;
    QueryData pivot = data[s];
    if(s <= e){
        i = s;
        j = e;
        for(;;){
            while(data[j].key.searchKey >= pivot.key.searchKey && i<j){j--;}
            while(data[i].key.searchKey <= pivot.key.searchKey && i<j){i++;}
            if(i<j){
                QueryData temp = data[i];
                data[i] = data[j];
                data[j] = temp;
            }else{
                break;
            }
        }
        QueryData temp = data[i];
        data[i] = data[s];
        data[s] = temp;
        if(k<i)
            quickSelect(data, k, s, i-1);
        else if(k>i)
            quickSelect(data, k, i+1, e);
    }
}

Node* LeafNodeSplit_Sort(LeafNode* leafNode, LeafNode* newHigh) {


    int j = leafNode->node.allocated >> 1; // dividir por dos (libro)
    int newsize = leafNode->node.allocated - j;

    memcpy(newHigh->data, leafNode->data + j,  newsize * sizeof (QueryData ));

    memset(leafNode->data + j, 0, sizeof (QueryData) * newsize);

    newHigh->node.allocated = newsize;
    leafNode->node.allocated -= newsize;

    return (Node*)newHigh;
}

BOOL LeafNodeCheckMinKey(LeafNode* leafNode){
    for(int i = 1; i < leafNode->node.allocated; i ++){
        if(QueryRangeLT(leafNode->data[i].key, leafNode->data[0].key)){
            return FALSE;
        }
    }
    return TRUE;
}

Node* LeafNodeSplit_NoSort(LeafNode* leafNode, LeafNode* newHigh) {
    int median = leafNode->node.allocated >> 1;
    int oldSize = leafNode->node.allocated;
    if(useBFPRT){
        BFPRTSelect(leafNode->data, 0, leafNode->node.allocated - 1, median);
    } else{
        quickSelect(leafNode->data, median, 0, leafNode->node.allocated - 1);
    }
    memcpy(newHigh->data, leafNode->data + median,  (oldSize - median) * sizeof (QueryData ));

    memset(leafNode->data + median, 0, sizeof (QueryData) * (oldSize - median));

    newHigh->node.allocated = (oldSize - median);
    leafNode->node.allocated = median;


    return (Node*)newHigh;
}

Node*  LeafNodeSplit(LeafNode* leafNode) {
    LeafNode* newHigh;
    int deleted = 0;
    for (int i = 0; i < leafNode->node.allocated; ++i) {
        if(QueryIsDeleted(leafNode->data[i].value) == FALSE){
            if(deleted > 0){
                leafNode->data[i - deleted] = leafNode->data[i];
            }
        } else{
            deleted ++;
        }
    }
    leafNode->node.allocated -= deleted;
    if(!NodeIsFull((Node*)leafNode)){
        return NULL;
    }

    if(leafNode->node.tree == NULL){
        printLeafNode(leafNode);
    }
    //    leafNode->node.tree->leafSplitCount ++;
    newHigh = (LeafNode*)malloc(sizeof (LeafNode));
    LeafNodeConstructor(newHigh, leafNode->node.tree);
    LeafNodeAllocId(newHigh);
    NodeAddInsertWriteLock((Node*)newHigh);
    switch (optimizationType) {
        case None:
        case Batch:
            LeafNodeSplit_Sort(leafNode, newHigh);
            break;
        case NoSort:
        case BatchAndNoSort:
            LeafNodeSplit_NoSort(leafNode,  newHigh);
            break;
        default:
            printf("LeafNodeSplit unsupported optimizationType:%d\n", optimizationType);
            return NULL;

    }
    LeafNodeResetMaxValue(leafNode);
    LeafNodeResetMaxValue(newHigh);
    LeafNodeResetMinValue(leafNode);
    LeafNodeResetMinValue(newHigh);
    newHigh->node.right = leafNode->node.right;
    leafNode->node.right = (Node*)newHigh;
    newHigh->node.nextNodeMin = leafNode->node.nextNodeMin;
    leafNode->node.nextNodeMin = newHigh->data[0].key.searchKey;
    return (Node*)newHigh;
}

inline void LeafNodeResetMinKey(LeafNode* leafNode){
    int minIndex = 0;
    for(int i = 1; i < leafNode->node.allocated; i ++){
        if(QueryRangeLT(leafNode->data[i].key, leafNode->data[minIndex].key)){
            minIndex = i;
        }
    }
    QueryData  temp = leafNode->data[0];
    leafNode->data[0] = leafNode->data[minIndex];
    leafNode->data[minIndex] = temp;
}

KeyType  LeafNodeSplitShiftKeysLeft(LeafNode* leafNode) {
//    if(optimizationType == NoSort || optimizationType == BatchAndNoSort){
//        LeafNodeResetMinKey(leafNode);
//    }
    return leafNode->data[0].key;
}

int LeafNodeGetId(LeafNode* leafNode) {
    return leafNode->node.id;
}

int LeafNodeGetHeight(LeafNode* leafNode) {
    return 1;
}

void LeafNodeResetId(LeafNode* leafNode){
    for (int i = 0; i < leafNode->node.allocated ; ++i) {
        myItoa(Qid, getQueryId(leafNode->data[i].value));
        Qid ++;
    }
}


void printLeafNode(LeafNode* leafNode){
    printf("[L%d](%d: L%d)(%d)(%d,%d){", leafNode->node.id,
           leafNode->node.nextNodeMin, leafNode->node.right== NULL? 0 :leafNode->node.right->id,
           leafNode->node.allocated, ((leafNode->node.minValue)), ((leafNode->node.maxValue)));
    for (int i = 0; i < leafNode->node.allocated; i++) {
        QueryRange * k = &leafNode->data[i].key;
        QueryMeta* v = leafNode->data[i].value;
//        printQueryRange(k);
        printf("%d:", k->searchKey);
        printQueryRange(k);
        printf("Q[%s] | ",v->queryId );
    }
    printf("}\n");
}

BOOL LeafNodeCheckMaxMin(LeafNode * leafNode){
    if(leafNode->node.allocated == 0){
        return TRUE;
    }
    int findMin = FALSE, findMax = FALSE;
    for(int i = 0; i < leafNode->node.allocated; i ++){
        if(leafNode->data[i].key.lower < leafNode->node.minValue){
            printf("LeafNodeCheckMaxMin ERROR:%d, data[%d].lower<node.min\n", leafNode->node.id, i);
            printLeafNode(leafNode);
            return FALSE;
        }
        if(leafNode->data[i].key.upper > leafNode->node.maxValue){
            printf("LeafNodeCheckMaxMin ERROR:%d, data[%d].upper>node.max\n", leafNode->node.id, i);
            printLeafNode(leafNode);
            return FALSE;
        }
        if(leafNode->data[i].key.lower == leafNode->node.minValue){
            findMin = TRUE;
        }
        if(leafNode->data[i].key.upper == leafNode->node.maxValue){
            findMax = TRUE;
        }
    }
    if(findMin == TRUE && findMax ==TRUE){
        return TRUE;
    } else{
//        printf("LeafNodeCheckMaxMin ERROR:%d, max or min not fount\n", leafNode->node.id);
//        printLeafNode(leafNode);
        return TRUE;
    }
}

BOOL LeafNodeCheckKey(LeafNode * leafNode){
    for (int i = 0; i < leafNode->node.allocated; ++i) {
        if(leafNode->data[i].key.upper != leafNode->data[i].value->dataRegion.upper || leafNode->data[i].key.lower != leafNode->data[i].value->dataRegion.lower){
            return FALSE;
        }
    }
    return TRUE;
}


int LeafNodeFindSlotByKey( LeafNode * node, KeyType* searchKey) {
    // return Arrays.binarySearch(keys, 0, allocated, searchKey);
    if(node->node.allocated == 0){
        return -1;
    }
    int low = 0;
    int high = node->node.allocated - 1;

    while (low <= high) {
        int mid = (low + high) >> 1;
        KeyType midVal = (node->data[mid].key);

        if (QueryRangeLT(midVal, *searchKey)) {
            low = mid + 1;
        } else if (QueryRangeGT(midVal, *searchKey)) {
            high = mid - 1;
        } else {
            return mid; // key found
        }
    }
    return -(low + 1);  // key not found.
}

BOOL LeafNodeCheckLink(LeafNode* node){
    if(node->node.allocated > Border){
        return FALSE;
    }
    if(node->node.right!= NULL){
        if(node->node.nextNodeMin <= ((LeafNode*)node->node.right)->data[0].key.searchKey){
            return TRUE;
        } else{
            printf("LeafNodeCheckLink ERROR:%d, node.nextNodeMin > right->data[0].searchKey\n", node->node.id);
            printLeafNode(node);
            printNode(node->node.right);
            return FALSE;
        }
    } else{
        return TRUE;
    }
}