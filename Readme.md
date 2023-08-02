# Introduction

Q-Tree is an index designed to infer query predicates from a given tuple.  The items to be indexed in a Q-Tree are simple intervals, represented as a lower bound and an upper bound, which are transformed from the query predicates. A range predicate in a range query is an interval by itself.
A point query predicate can be regarded as a special interval, whose lower and upper bounds are identical.  To verify if a tuple satisfies a predicate, we only need to verify if its value falls into the corresponding intervals.

When applied to cache invalidation, Q-Tree faces workloads that are drastically different from that of a general-purpose index, such as B+ Tree or Binary Search Trees. 
To an index structure in a database system, the majority of workloads are point and range queries. Insertion and deletion are usually less frequent. To a Q-Tree, however, the majority of the workload is insertion and deletion.

In cache invalidation, Q-Tree faces 3 main types of workload:
* **insertion**. When an entry is added to a cache, a set of intervals from its predicates will be inserted into the Q-Tree for its template.
* **eviction**. When a cache entry is evicted because of the fulling of the cache (and based on some cache replacement strategies, such as LRU, etc.), the corresponding intervals will be deleted from the Q-Tree.
* **invalidation**. When an update is performed on the database, the cache server will look up the Q-Tree for predicates to invalidate. If a matching predicate is found, a follow-up eviction will be performed.

In a typical web application, reads usually dominate writes. As a result, insertions and evictions should occur much more frequently than invalidation, which needs extra optimizations.

# Overview
Q-Tree is a variant of Interval Tree, which was designed to lookup intervals that cover a given key.
However, the original Interval Tree is a binary tree, which is not suitable for large-scale datasets. Therefore, we extend Interval Tree to a B+ Tree-like structure, so that it can work as a balanced n-ary tree.

A 3-ary Q-Tree is illustrated in the figure. Physically, it is a B+ tree consisting of a root node (*R*) and multiple internal (*S*) and leaf (*A*) nodes.
The index keys of this tree are the lower bounds of the intervals.
Each node (except the root) has a variable number of children, whose amount is between *m/2* and *m*.
In the figure, *m* is 3.
Children of the leaf nodes are intervals to be indexed.

![这是图片](pics/Q-Tree.png "Illustration of a 3-ray Q-Tree")


Besides the index keys, each node *X* maintains *X.max*, which is the upper bound of intervals stored in its subtree.
*X.max* is employed by Interval Tree to locate matching intervals.
During a search, we traverse the Q-Tree using the B+ tree algorithm. As the index keys are the lower bounds of the intervals, the original B+ tree algorithm only allows us to find intervals whose lower bounds are smaller than the search key. We still need to filter out the intervals whose upper bounds are smaller than the search key. This is where *X.max* comes into play. The concrete search algorithm is similar to that of Interval Tree.


# Performance Optimization
Due to the different workloads, the traditional implementation of B+ Trees no longer suits Q-Tree. Thus, we came up with a series of techniques to enhance Q-Tree's performance in cache invalidation.

## Merge Lock

B-link Tree is a typical technique to improve the concurrency of B+ Tree.
It complements each internal node of B+ Tree with additional links that point to its sibling nodes. These links glue the broken structures of B+ Tree together during the split or merge process so that we can substantially shorten lock duration and thus increase concurrency.

Q-Tree preserves the right links of B-link Tree to facilitate insertion. However, it removes the left link on each node and resorts to locking for safeguarding merge operations. When a deletion or insertion operation traverses the Q-Tree, it places a shared merge lock on each node it encounters. When a deletion operation is about to perform a merge, it places exclusive merge locks on the nodes to be merged. This merge lock simply blocks other operations from accessing a node that is being merged, making the merge process much simpler than that of B-link Tree. Our experiments show that it could significantly improve the performance of Q-Tree under an insertion-deletion-intensive workload.

## Lazy Deletion

In principle, when a cache entry is evicted, its related predicates should be immediately removed from the Q-Tree.
Practically, there is no actual harm if they still stay in the index, as long as we mark them as *deleted*.
This allows us to perform lazy deletion on a Q-Tree.
As a Q-Tree is often faced with interleaved insertion and deletion, the slot of a *deleted* item can be quickly filled up by a subsequent insertion. Thus, lazy deletion allows us to avoid a significant number of unnecessary merges and splits.

Actual deletion will be done if the item's slot can be reused or the number of deleted items in the whole Q-Tree reaches a certain threshold. To prevent too many accumulated *deleted* items, a background thread will periodically call a refactor procedure, which will traverse the whole tree to remove all  *deleted* and perform all the merges in a batch.

## Invalidation Operation
**Insertion** on Q-Trees is similar to that on B+ Tree. 
The following algorithm shows how to find and drop queries based on a given key.
We use *modifyLock* to represent the original node lock of concurrent B-link Tree.
Another read-write lock *mergeLock* is used to safeguard the merge process.

```
QuerySet QueryRetrieval(key){
    querySet={}
    Travel(root, key, querySet);
    return querySet;
}

void Travel(node, key, querySet){
    if(node.isLeaf()){
        for(query in node.queries){
            if(query.cover(key)){
                querySet.add(query);
                node.remove(query);
            }
        }
    }else{ // internal node
        for(child in node.childs){
            if(child.cover(key)){
                Travel(child, key, querySet);
            }
        }
        if(node.rebalancedChildNum > 0 and node.formatBit.cas(0,1) == True){
            node.modifyLock.addWriteLock();
            for(child in node.childs){
                if(child.IsUnderflow()){
                    add write lock for mergeLock of used nodes ;
                    do re-balance;
                    unlock mergeLock for used nodes ;
                }
            }
            node.modifyLock.unLock();
            node.formatBit.cas(1,0)
        }
    }
    node.recalculateBoundary()
}

```

Similar to B+ Tree, rebalancing is the process to make sure the fanouts of all nodes (except the root) are between *m/2* and *m*.
It is usually the heaviest process in B+ Tree's operations.
In the algorithm , we try to solve 2 problems:
* Let as less threads be blocked to wait to do the rebalance work as possible, as it is time-consuming and exclusive.
* Avoid deadlocks, as two locks could be applied on the same node by different operations, i.e., insertion and invalidation.   

First of all, one invalidation operation may invalidate multiple cache entries and then cause multiple droppings in the index.
It is very possible that a node and its siblings both need to be rebalanced, successively.
Then, instead of doing the rebalance immediately after finishing searching on a node, we check nodes' fanouts and do the rebalance after finishing searching on all siblings.
Further, before a node is merged and deleted, an exclusive merge lock will be applied to it to make sure no other threads access it.
Meanwhile, multiple concurrent invalidation operations may access the same node and find its child needed to be rebalanced.
In fact, only one thread is needed to perform the job.
A *formatBit* is used to prevent this wasteful contention.
Atomic operations(*cas*) on *formatBit* could make sure only one node would do the rebalance for its children.

As shown in the algorithm, a node and its child nodes should be locked at the same time when its child node needs to be rebalanced.
And for the insertion operation in B-link Tree, they also need to be locked at the same time (on *modifyLock*), in reverse order to the above algorithm, i.e. locking the child node first, and then the parent node.
If a parent and a child node are applied with the same lock, there can be a deadlock. Fortunately, applying the $mergeLock$ on child nodes in Algorithm 1 avoids this scenario.



