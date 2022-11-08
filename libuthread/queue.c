// Reference:
// - Single linked list in C: https://www.learn-c.org/en/Linked_lists

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

/* FIFO queue underlying data structure: a double linked list, with size */
typedef void* data_t;

// type for the node of double linked list
typedef struct node {
    data_t data;           // storing the pointer to the actaully data
    struct node* prevNode; // pointing to next elem in the linkedlist
    struct node* nextNode; // pointing to prev elem in the linkedlist
} node;
typedef node* node_t;

// type for the queue
typedef struct queue {
    int size;         // number of elements in the queue
    node_t dummyHead; // a dummyHead with empty data, points to the real head
    node_t tail;      // last node, if queue is empty, it's same as dummyHead
} queue;

/* Util function for generating a new linked list node */
node_t create_node(data_t data, node_t prev, node_t next)
{
    node_t n = malloc(sizeof(node));
    if (n != NULL) { // malloca success, reset pointer
        n->data = data;
        n->prevNode = prev;
        n->nextNode = next;
    } // else malloc failure, n is NULL
    return n;
}

/*
 * Allocate space for queue, aslo empty queue comes with a dummyHead empty node,
 * dummyHead is for easy dealing with logic for enqueue and dequeue.
 * dummyHead->nextNode points to the actually head of list
 */
queue_t queue_create(void)
{
    // create heap space for the queue struct
    queue_t q = malloc(sizeof(queue));
    // initialize with a dummyHead with no content as a starter
    if (q != NULL) {
        node_t dummyHead = create_node(NULL, NULL, NULL);
        q->size = 0;
        q->dummyHead = dummyHead;
        q->tail = dummyHead;
    } // else malloca failure q is NULL
    return q;
}

/* Destroy the dummyHead as well as the queue struct memory */
int queue_destroy(queue_t q)
{
    if (q == NULL || q->size != 0) {
        // return -1 for non-empty or null queue
        return -1;
    } else {
        // otherwise release dummyHead and queue space
        free(q->dummyHead);
        free(q);
        return 0;
    }

}

/* Enqueue place the new data at the end of the linkedlist */
int queue_enqueue(queue_t q, void *data)
{
    // input validation
    if (q == NULL || data == NULL) {
        return -1;
    }
    // enqueue by allocating a new pace for node, and append at tail
    node_t n = create_node(data, q->tail, NULL);
    if (n == NULL) {
        // fail to create node
        return -1;
    } else {
        // successfuly create node, update q's data structure
        q->tail->nextNode = n;
        q->tail = n;
        q->size += 1;
        return 0;
    }
}

/* FIFO queue, dequeue from head */
int queue_dequeue(queue_t q, void **data)
{
    // input validation
    if (q == NULL || q->size == 0 || data == NULL) {
        return -1;
    }
    // dequeue from head of linked list
    assert(q->size > 0);
    // get head node and node after head
    node_t head = q->dummyHead->nextNode;
    node_t newHead = head->nextNode;
    // adjust tail
    if (q->tail == head) {
        q->tail = q->dummyHead;
    }
    // remove old head
    q->dummyHead->nextNode = newHead;
    if (newHead != NULL) {
        newHead->prevNode = q->dummyHead;
    }
    // adjust size
    q->size -= 1;
    // store return data
    *data = head->data;
    free(head);
    return 0;
}

/* Find the data and delete it, iterate from head to tail */
int queue_delete(queue_t q, void *data)
{
    // input validation
    if (q == NULL || data == NULL) {
        return -1;
    }
    // interate from dummyHead to tail
    node_t iter = q->dummyHead;
    while (iter != NULL) {
        if (iter->nextNode != NULL && iter->nextNode->data == data) {
            // found delete target, delete and return
            node_t deleteTarget = iter->nextNode;
            iter->nextNode = deleteTarget->nextNode;
            if (deleteTarget->nextNode != NULL) {
                deleteTarget->nextNode->prevNode = iter;
            }
            // update q vars
            q->tail = (q->tail == deleteTarget) ? iter : q -> tail;
            q->size -= 1;
            // delete
            free(deleteTarget);
            return 0;
        } else {
            // target not found, move iter cursor to next node
            iter = iter->nextNode;
        }
    }
    return -1; // no target found
}

/* Iterate from head to tail and apply function to each elem in queue */
int queue_iterate(queue_t q, queue_func_t func)
{
    // input validation
    if (q == NULL || func == NULL) {
        return -1;
    }
    // iterate and apply func, start from head node
    node_t iter = q->dummyHead->nextNode;
    while (iter != NULL) {
        node_t nextIter = iter->nextNode;
        func(q, iter->data);
        iter = nextIter;
    }
    return 0;
}

/* Return the queue size depending on whether queue is NULL or not */
int queue_length(queue_t q)
{
    return (q == NULL) ? -1 : q->size;
}
