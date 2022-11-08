// Reference
// project2.html for adding the deletion resilient queue_iterate test

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"

#define TEST_ASSERT(assert)             \
do {                                    \
    printf("ASSERT: " #assert " ... "); \
    if (assert) {                       \
        printf("PASS\n");               \
    } else  {                           \
        printf("FAIL\n");               \
        exit(1);                        \
    }                                   \
} while(0)

/* Create queue */
void test_create(void)
{
    // arrange
    fprintf(stderr, "\n*** TEST queue create ***\n");
    // act
    queue_t q = queue_create();
    // assert
    TEST_ASSERT(q != NULL);
}

/* Destroy */
void test_destroy(void)
{
    // arrane
    int data = 3;
    queue_t emptyQ = queue_create();
    fprintf(stderr, "\n*** TEST queue destroy ***\n");
    // act
    int retValDestroy = queue_destroy(emptyQ);
    int retValDestroyInvalid1 = queue_destroy(NULL);
    queue_t nonEmptyQ = queue_create();
    queue_enqueue(nonEmptyQ, &data);
    int retValDestroyInvalid2 = queue_destroy(nonEmptyQ);
    // assert
    // destroy empty q
    TEST_ASSERT(retValDestroy == 0);
    // destroy null q
    TEST_ASSERT(retValDestroyInvalid1 == -1);
    // destroy non-empty q
    TEST_ASSERT(retValDestroyInvalid2 == -1);
}
/* Enqueue/Dequeue */
void test_queue_simple(void)
{
    // arrane
    int data = 3, *dataPtr;
    queue_t q = queue_create();
    fprintf(stderr, "\n*** TEST queue simple for enqueue/dequeue ***\n");
    // act
    queue_enqueue(q, &data);
    queue_dequeue(q, (void**)&dataPtr);
    // assert
    TEST_ASSERT(*dataPtr == 3);
    TEST_ASSERT(dataPtr == &data);
}

/* Test FIFO order and length */
struct mytype {
    int a, b;
    char str[6];
};
typedef struct mytype* mytype_t;
void test_queue_fifo_length(void)
{
    // arrane
    int num = 1, *numPtr;
    long lnum = 10L, *lnumPtr;
    bool boo = false, *booPtr;
    struct mytype data;
    data.a = 3;
    data.b = 4;
    strcpy(data.str, "Hello");
    mytype_t dataPtr;
    queue_t q = queue_create();
    fprintf(stderr, "\n*** TEST queue FIFO for enqueue/dequeue and length ***\n");
    // act
    int count0 = queue_length(q);
    queue_enqueue(q, &num);
    int count1 = queue_length(q);
    queue_enqueue(q, &lnum);
    int count2 = queue_length(q);
    queue_enqueue(q, &boo);
    int count3 = queue_length(q);
    queue_enqueue(q, &data);
    int count4 = queue_length(q);
    queue_dequeue(q, (void**)&numPtr);
    int count5 = queue_length(q);
    queue_dequeue(q, (void**)&lnumPtr);
    int count6 = queue_length(q);
    queue_dequeue(q, (void**)&booPtr);
    int count7 = queue_length(q);
    queue_dequeue(q, (void**)&dataPtr);
    int count8 = queue_length(q);

    // assert FIFO order
    TEST_ASSERT(numPtr == &num);
    TEST_ASSERT(*numPtr == 1);
    TEST_ASSERT(lnumPtr == &lnum);
    TEST_ASSERT(*lnumPtr == 10L);
    TEST_ASSERT(*booPtr == false);
    TEST_ASSERT(booPtr == &boo);
    TEST_ASSERT(dataPtr->a == 3);
    TEST_ASSERT(dataPtr->b == 4);
    TEST_ASSERT(strcmp(dataPtr->str,  "Hello") == 0);
    TEST_ASSERT(dataPtr == &data);

    // assert size is correct
    TEST_ASSERT(count0 == 0);
    TEST_ASSERT(count1 == 1);
    TEST_ASSERT(count2 == 2);
    TEST_ASSERT(count3 == 3);
    TEST_ASSERT(count4 == 4);
    TEST_ASSERT(count5 == 3);
    TEST_ASSERT(count6 == 2);
    TEST_ASSERT(count7 == 1);
    TEST_ASSERT(count8 == 0);
}

void test_enqueue_invalid_input(void)
{
    // arrange
    void *nullPtr = NULL;
    int data = 3;
    fprintf(stderr, "\n*** TEST queue invalid input for enqueue ***\n");
    // act
    queue_t q = queue_create();
    int retValEnqueue1 = queue_enqueue(nullPtr, &data);
    int retValEnqueue2 = queue_enqueue(q, nullPtr);
    int retValEnqueue3 = queue_enqueue(nullPtr, nullPtr);
    // assert
    TEST_ASSERT(retValEnqueue1 == -1);
    TEST_ASSERT(retValEnqueue2 == -1);
    TEST_ASSERT(retValEnqueue3 == -1);
}

void test_dequeue_invalid_input(void)
{
    // arrange
    void *nullPtr = NULL;
    int *dataPtr;
    fprintf(stderr, "\n*** TEST queue invalid input for dequeue ***\n");
    // act
    queue_t q = queue_create();
    int retValDequeue1 = queue_dequeue(nullPtr, (void**)&dataPtr);
    int retValDequeue2 = queue_dequeue(q, nullPtr);
    int retValDequeue3 = queue_dequeue(nullPtr, nullPtr);
    // assert
    TEST_ASSERT(retValDequeue1 == -1);
    TEST_ASSERT(retValDequeue2 == -1);
    TEST_ASSERT(retValDequeue3 == -1);
}

void test_delete_single(void)
{
    // arrane
    struct mytype data;
    data.a = 3;
    data.b = 4;
    strcpy(data.str, "Hello");
    queue_t q = queue_create();
    fprintf(stderr, "\n*** TEST queue delete with single elem queue ***\n");
    // act
    queue_enqueue(q, &data);
    queue_delete(q, &data);
    int count = queue_length(q);
    // assert
    TEST_ASSERT(count == 0);
}

/* Test delete together with enqueue/dequeue/length */
void test_delete_comprehensive(void)
{
    // arrane
    int num = 1;
    long lnum = 10L, *lnumPtr;
    short snum = 3;
    struct mytype data;
    data.a = 3;
    data.b = 4;
    strcpy(data.str, "Hello");
    mytype_t dataPtr;
    queue_t q = queue_create();
    bool boo = false;
    char ch = 'a', *chPtr;
    fprintf(stderr, "\n*** TEST delete used together with other methods ***\n");
    // act
    queue_enqueue(q, &num);
    queue_enqueue(q, &lnum);
    queue_enqueue(q, &snum);
    queue_enqueue(q, &data);
    queue_enqueue(q, &boo);
    queue_delete(q, &num);  // delete first
    queue_delete(q, &boo);  // delete last
    queue_enqueue(q, &ch);
    queue_delete(q, &snum); // delete middle
    int count1 = queue_length(q);
    queue_dequeue(q, (void**)&lnumPtr);
    queue_dequeue(q, (void**)&dataPtr);
    queue_dequeue(q, (void**)&chPtr);
    int count2 = queue_length(q);

    // assert two elem after 5 enqueue and 3 deletes
    TEST_ASSERT(lnumPtr == &lnum);
    TEST_ASSERT(*lnumPtr == 10L);
    TEST_ASSERT(dataPtr->a == 3);
    TEST_ASSERT(dataPtr->b == 4);
    TEST_ASSERT(strcmp(dataPtr->str,  "Hello") == 0);
    TEST_ASSERT(chPtr == &ch);
    TEST_ASSERT(*chPtr == 'a');

    // assert size is correct
    TEST_ASSERT(count1 == 3);
    TEST_ASSERT(count2 == 0);
}

void test_delete_invalid(void)
{
    // arrange
    void *nullPtr = NULL;
    int data = 3;
    fprintf(stderr, "\n*** TEST queue invalid input for delete ***\n");
    // act
    queue_t q = queue_create();
    int retValDelete1 = queue_delete(nullPtr, &data);
    int retValDelete2 = queue_delete(q, nullPtr);
    int retValDelete3 = queue_delete(nullPtr, nullPtr);
    // assert
    TEST_ASSERT(retValDelete1 == -1);
    TEST_ASSERT(retValDelete2 == -1);
    TEST_ASSERT(retValDelete3 == -1);
}

/* Iterate and double the number if it's multiple of 5 */
static void doulbeMultipleOfThree(queue_t q, void *data)
{
    int factor = 3;
    if (*((int*)data) % factor == 0) {
        *((int*)data) = 2 * *((int*)data) ;
    }
    // this part is added as I'm forced to use q to get rid of compile error
    // queue_tester_example.c:245:43: error: unused parameter ‘q’ [-Werror=unused-parameter]
    if (*((int**)data) == 0) {
        *((int*)data) = queue_length(q);
    }
}

void test_iterate_simple(void) {
    // arrange
    int nums[7] = {0, 1, 2, 3, 4, 5, 6};
    queue_t q = queue_create();
    for (int i = 0; i < 7; i++) {
        queue_enqueue(q, &nums[i]);
    }
    fprintf(stderr, "\n*** TEST queue iterate ***\n");
    // act
    queue_iterate(q, doulbeMultipleOfThree);
    // assert
    TEST_ASSERT(nums[0] == 0);
    TEST_ASSERT(nums[1] == 1);
    TEST_ASSERT(nums[2] == 2);
    TEST_ASSERT(nums[3] == 6);
    TEST_ASSERT(nums[4] == 4);
    TEST_ASSERT(nums[5] == 5);
    TEST_ASSERT(nums[6] == 12);
}

// Reference: Project2 Sample for iteration
/* Iterate and increment, if it's */
static void iterator_inc(queue_t q, void *data)
{
    int *a = (int*)data;
    if (*a == 42) {
        // if target found, delete from queue
        queue_delete(q, data);
    } else {
        // otherwise increment the value
        *a += 1;
    }
}

void test_iterate_resilient_to_delete(void)
{
    // arrange
    queue_t q;
    int data[] = {1, 2, 3, 4, 5, 42, 6, 7, 8, 9};
    int *dequeueData;
    size_t i;
    q = queue_create();
    for (i = 0; i < sizeof(data) / sizeof(data[0]); i++){
        queue_enqueue(q, &data[i]);
    }
    fprintf(stderr, "\n*** TEST queue interate is deletion safe ***\n");
    // act
    queue_iterate(q, iterator_inc);
    // original array should be increment
    TEST_ASSERT(data[0] == 2);
    TEST_ASSERT(data[1] == 3);
    TEST_ASSERT(data[2] == 4);
    TEST_ASSERT(data[3] == 5);
    TEST_ASSERT(data[4] == 6);
    TEST_ASSERT(data[5] == 42);
    TEST_ASSERT(data[6] == 7);
    TEST_ASSERT(data[7] == 8);
    TEST_ASSERT(data[8] == 9);
    TEST_ASSERT(data[9] == 10);
    assert(queue_length(q) == 9);
    queue_dequeue(q, (void**)&dequeueData);
    TEST_ASSERT(*dequeueData == 2);
    queue_dequeue(q, (void**)&dequeueData);
    TEST_ASSERT(*dequeueData == 3);
    queue_dequeue(q, (void**)&dequeueData);
    TEST_ASSERT(*dequeueData == 4);
    queue_dequeue(q, (void**)&dequeueData);
    TEST_ASSERT(*dequeueData == 5);
    queue_dequeue(q, (void**)&dequeueData);
    TEST_ASSERT(*dequeueData == 6);
    queue_dequeue(q, (void**)&dequeueData);
    TEST_ASSERT(*dequeueData == 7);
    queue_dequeue(q, (void**)&dequeueData);
    TEST_ASSERT(*dequeueData == 8);
    queue_dequeue(q, (void**)&dequeueData);
    TEST_ASSERT(*dequeueData == 9);
    queue_dequeue(q, (void**)&dequeueData);
    TEST_ASSERT(*dequeueData == 10);
    TEST_ASSERT(queue_length(q) == 0);
}

int main(void)
{
    test_create();
    test_destroy();
    test_queue_simple();
    test_queue_fifo_length();
    test_enqueue_invalid_input();
    test_dequeue_invalid_input();
    test_delete_single();
    test_delete_comprehensive();
    test_delete_invalid();
    test_iterate_simple();
    test_iterate_resilient_to_delete();
    return 0;
}
