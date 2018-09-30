#include <stdio.h>
#include <stdlib.h>

#define MAX_SIZE 5


typedef struct Queue{
    int size;
    int first;
    int last;
    int items[MAX_SIZE];
} Queue;

Queue* createQueue() {
    Queue *queue = (Queue*)malloc(sizeof(Queue));
    queue->size = 0;
    queue->first = 0;
    queue->last = MAX_SIZE - 1;

    return queue;
}

void Enqueue(Queue *queue, int item) {
    if(queue->size >= MAX_SIZE) {
        printf("Queue cheia!");
    }

    else {
        queue->last = (queue->last + 1) % MAX_SIZE;
        queue->items[queue->last] = item;
        queue->size++;
    }
}

void Dequeue(Queue *queue) {
    if(queue->size <= 0) {
        printf("Queue vazia!");
    }

    else {
        queue->first = (queue->first + 1) % MAX_SIZE;
        queue->size--;
    }
}

int main(void) {

    int i;

    Queue *queue = createQueue();

    Enqueue(queue, 1);
    Enqueue(queue, 5);
    Enqueue(queue, 8);
    Enqueue(queue, 9);

    for ( int i = 0; i < queue->size; i++ ) {
      printf( "Item na posicao %d eh %d\n", i, queue->items[ i ] );
    }

    return 0;
}
