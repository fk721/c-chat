#include <stdlib.h>
#include <string.h>
#include "Queue.h"

MyQueue* init_queue() {
    MyQueue* queue = (MyQueue*) malloc(sizeof(MyQueue)); 
    queue->capacity = QUEUE_CAPACITY; 
    queue->front = queue->size = 0;
    return queue;
} 
  
int is_empty(MyQueue* queue) {
    return queue->size == 0;
}

int is_full(MyQueue* queue) {
    return queue->size == queue->capacity;
}
  
void enqueue(MyQueue* queue, char* elem) {
    if (is_full(queue)) return;
    int back = (queue->front + queue->size) % queue->capacity;
    memset(&queue->array[back], 0, MAXLINE);
    strcpy(queue->array[back], elem);
    queue->size = queue->size + 1; 
} 

char* dequeue(MyQueue* queue) {
    if (is_empty(queue)) return NULL;
    char* elem = queue->array[queue->front];
    queue->front = (queue->front + 1) % queue->capacity; 
    queue->size = queue->size - 1;
    return elem; 
}
