#ifndef QUEUE_H
#define QUEUE_H

// C Program to demonstrate how to Implement a queue
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>

#define MAX_SIZE 32000

// Defining the Queue structure
typedef struct {
    uint64_t items[MAX_SIZE];
    int front;
    int rear;
} Queue;

// Function to initialize the queue
void initializeQueue(Queue* q) {
    q->front = -1;  // -1 means the queue is empty
    q->rear = -1;
}

// Function to clear queue items (optional)
void destroyItems(Queue* q) {
    memset(q->items, 0, sizeof(q->items));
}

// Function to reset the queue
void resetQueue(Queue* q) {
    destroyItems(q);
    initializeQueue(q);
}

// Function to check if the queue is empty
bool isEmpty(Queue* q) { return q->front == -1; }

// Function to check if the queue is full
bool isFull(Queue* q) { return (q->rear + 1) % MAX_SIZE == q->front; }

// Function to add an element to the queue (Enqueue
// operation)
void enqueue(Queue* q, uint64_t value)
{
if (isFull(q)) {
        printf("Queue is full\n");
        return;
    }
    
    // If the queue is empty, set `front` to 0
    if (isEmpty(q)) {
        q->front = 0;
    }
    
    // Move `rear` to the next position in a circular manner
    q->rear = (q->rear + 1) % MAX_SIZE;
    q->items[q->rear] = value;
}

// Function to remove an element from the queue (Dequeue
// operation)
void dequeue(Queue* q)
{
    if (isEmpty(q)) {
        printf("Queue is empty\n");
        return;
    }

    // If `front` and `rear` are the same, the queue will be empty after this dequeue
    if (q->front == q->rear) {
        q->front = -1;
        q->rear = -1;
    } else {
        // Move `front` to the next position in a circular manner
        q->front = (q->front + 1) % MAX_SIZE;
    }
}

// Function to get the element at the front of the queue
// (Peek operation)
uint64_t peek(Queue* q) {
    if (isEmpty(q)) {
        printf("Queue is empty\n");
        return -1;  // Or some other error code or indicator
    }
    return q->items[q->front];
}


// Function to print the current queue
void printQueue(Queue* q)
{
    if (isEmpty(q)) {
        printf("Queue is empty\n");
        return;
    }

    printf("Current Queue: ");
    for (int i = q->front + 1; i < q->rear; i++) {
        printf("%04lx ", q->items[i]);
    }
    printf("\n");
}


#endif