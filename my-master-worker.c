#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>


int BUFFER_SIZE;
int NUM_PRODUCERS;
int NUM_CONSUMERS;
int TOTAL_ITEMS_TO_PRODUCE;

// Shared buffer
int* buffer;
int count = 0; // Number of items in buffer
int in = 0;    // Points to the next free position
int out = 0;   // Points to the next item to consume
int cur_item = 0;

// Synchronization variables
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t not_empty = PTHREAD_COND_INITIALIZER;
pthread_cond_t not_full = PTHREAD_COND_INITIALIZER;


void print_produced(int num, int master) {

    printf("Produced %d by master %d\n", num, master);
}

void print_consumed(int num, int worker) {

    printf("Consumed %d by worker %d\n", num, worker);

}


void* produce_requests_loop(void* data){

    int thread_id = *((int*)data);

    while (1){


        pthread_mutex_lock(&mutex);

        if(cur_item >= TOTAL_ITEMS_TO_PRODUCE) {
          break;
        }

        while (count == BUFFER_SIZE) {
            pthread_cond_wait(&not_full, &mutex);
        }

        // Add item to buffer
        buffer[in] = cur_item;
        print_produced(cur_item, thread_id);
        cur_item++;
        in = (in + 1) % BUFFER_SIZE;
        count++;


        // TODO: Signal the consumers that buffer is not empty
        pthread_cond_broadcast(&not_empty);
        pthread_mutex_unlock(&mutex);

    }
    return NULL;

}


void* consume_requests_loop(void* data){

    int thread_id = *((int*)data);

    while (1){


        pthread_mutex_lock(&mutex);

        if (cur_item >= TOTAL_ITEMS_TO_PRODUCE){
            break;
        }

        while (count == 0) {
            pthread_cond_wait(&not_empty, &mutex);
        }

        // Add item to buffer
        int item = buffer[out];
        print_consumed(item, thread_id);
        out = (out + 1) % BUFFER_SIZE;
        count--;


        // TODO: Signal the producers that buffer is not full
        pthread_cond_broadcast(&not_full);
        pthread_mutex_unlock(&mutex);

    }
    return NULL;

}


// input is...
// M = how many numbers to produce
// N = the maximum size buffer in which the produced number should be stored
// C = the number of workers to consume
// P = the number of master threads to produce
int main(int argc, char* argv[])
{

    if (argc < 5) {
        printf("./master-worker #total_items #max_buf_size #num_workers #masters e.g. ./exe 10000 1000 4 3\n");
        exit(1);
    }
    else {
        NUM_PRODUCERS = atoi(argv[4]);
        NUM_CONSUMERS = atoi(argv[3]);
        TOTAL_ITEMS_TO_PRODUCE = atoi(argv[1]);
        BUFFER_SIZE = atoi(argv[2]);
    }

    pthread_t producers[NUM_PRODUCERS], consumers[NUM_CONSUMERS];
    int producer_ids[NUM_PRODUCERS], consumer_ids[NUM_CONSUMERS];

    buffer = (int*)malloc(sizeof(int)*BUFFER_SIZE);

    // Create producer threads
    for (int i = 0; i < NUM_PRODUCERS; i++) {
        producer_ids[i] = i;
        pthread_create(&producers[i], NULL, produce_requests_loop, &producer_ids[i]);
    }

    // Create consumer threads
    for (int i = 0; i < NUM_CONSUMERS; i++) {
        consumer_ids[i] = i;
        pthread_create(&consumers[i], NULL, consume_requests_loop, &consumer_ids[i]);
    }

    // Join producer threads
    for (int i = 0; i < NUM_PRODUCERS; i++) {
        pthread_join(producers[i], NULL);
    }

    // Join consumer threads
    for (int i = 0; i < NUM_CONSUMERS; i++) {
        pthread_join(consumers[i], NULL);
    }

    /*----Deallocating Buffers---------------------*/
    free(buffer);
    return 0;
}

