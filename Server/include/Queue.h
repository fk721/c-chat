#ifndef MY_QUEUE_H
#define MY_QUEUE_H

#include "Macros.h"

typedef struct {
    int front, size, capacity;
    char array[QUEUE_CAPACITY][MAXLINE];
} MyQueue; 

/// Initializes the queue.
MyQueue* init_queue();

/// Returns 1 when the queue is empty.
int is_empty(MyQueue* queue);

/// Returns 1 when the queue is at maximum capacity (defined in `Macros.h`).
int is_full(MyQueue* queue);  

/// Insert item to the back of the queue.
void enqueue(MyQueue* queue, char* elem);

/// Remove item from the front of the queue.
char* dequeue(MyQueue* queue);

#endif