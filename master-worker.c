#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
// #include <wait.h>
#include <pthread.h>

int item_to_produce, curr_buf_size, item_to_consume;
int total_items, max_buf_size, num_workers, num_masters;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t not_empty = PTHREAD_COND_INITIALIZER;
pthread_cond_t not_full = PTHREAD_COND_INITIALIZER;

int *buffer;
int BUFFER_SIZE;

void print_produced(int num, int master) {
    printf("Produced %d by master %d\n", num, master);
}

void print_consumed(int num, int worker) {
    printf("Consumed %d by worker %d\n", num, worker);
}


//produce items and place in buffer
//modify code below to synchronize correctly
void *generate_requests_loop(void *data)
{
    int thread_id = *((int *)data);

    while (1)
    {
        pthread_mutex_lock(&mutex);

        if (item_to_produce >= total_items)
        {
            pthread_mutex_unlock(&mutex);
            break;
        }

        while (curr_buf_size >= max_buf_size)
        {
            pthread_cond_wait(&not_full, &mutex);
        }

        buffer[curr_buf_size++] = item_to_produce;
        print_produced(item_to_produce, thread_id);
        item_to_produce++;

        pthread_cond_signal(&not_empty);
        pthread_mutex_unlock(&mutex);
    }

    return 0;
}

void *consumer_request_loop(void *data)
{
    int thread_id = *((int *)data);

    while (1)
    {
        pthread_mutex_lock(&mutex);

        while (curr_buf_size <= 0 && item_to_consume < total_items)
        {
            pthread_cond_wait(&not_empty, &mutex);
        }

        if (item_to_consume >= total_items)
        {
            pthread_mutex_unlock(&mutex);
            break;
        }

        int consumed_item = buffer[--curr_buf_size];
        print_consumed(consumed_item, thread_id);
        item_to_consume++;

        pthread_cond_signal(&not_full);
        pthread_mutex_unlock(&mutex);
    }

    return 0;
}

int main(int argc, char *argv[])
{
    int *master_thread_ids;
    pthread_t *master_threads;
    int *consumer_thread_ids;
    pthread_t *consumer_threads;
    item_to_produce = 0;
    item_to_consume = 0;
    curr_buf_size = 0;



    int i;

    if (argc < 5) {
        printf("./master-worker #total_items #max_buf_size #num_workers #masters e.g. ./exe 10000 1000 4 3\n");
        exit(1);
    }
    else {
        num_masters = atoi(argv[4]);
        num_workers = atoi(argv[3]);
        total_items = atoi(argv[1]);
        max_buf_size = atoi(argv[2]);
    }
    BUFFER_SIZE = max_buf_size;


    buffer = (int *)malloc (sizeof(int) * max_buf_size);

    //create master producer threads
    master_t./test-master-worker.sh 100 100 100 100hread_ids = (int *)malloc(sizeof(int) * num_masters);
    master_threads = (pthread_t *)malloc(sizeof(pthread_t) * num_masters);
    for (i = 0; i < num_masters; i++)
        master_thread_ids[i] = i;

    for (i = 0; i < num_masters; i++)
        pthread_create(&master_threads[i], NULL, generate_requests_loop, (void *)&master_thread_ids[i]);

    //create worker consumer threads
    consumer_thread_ids = (int *)malloc(sizeof(int) * num_workers);
    consumer_threads = (pthread_t *)malloc(sizeof(pthread_t) * num_workers);
    for (i = 0; i < num_workers; i++)
        consumer_thread_ids[i] = i;

    for (i = 0; i < num_workers; i++)
        pthread_create(&consumer_threads[i], NULL, consumer_request_loop, (void *)&consumer_thread_ids[i]);


    //wait for all threads to complete
    for (i = 0; i < num_masters; i++)
    {
        pthread_join(master_threads[i], NULL);
        printf("master %d joined\n", i);
    }

    // wait for all worker threads to complete
    for (i = 0; i < num_workers; i++)
    {
        pthread_join(consumer_threads[i], NULL);
        printf("consumer %d joined\n", i);
    }

    /*----Deallocating Buffers---------------------*/
    free(buffer);
    free(master_thread_ids);
    free(master_threads);
    free(consumer_thread_ids);
    free(consumer_threads);

    return 0;
}
