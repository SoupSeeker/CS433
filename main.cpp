#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include "buffer.h"

int insert_item(buffer_item item);
int remove_item(buffer_item *item);
void display_buffer();

sem_t emptySem, fullSem;
pthread_mutex_t mutex;

buffer_item item;

/*Producer Thread*/
void* producer(void* param) {
	buffer_item prodItem = item;

	while (true) {
		//sleep for random time
		sleep(rand() % 10);
		//generate random number
		prodItem = rand() % 100;

		sem_wait(&emptySem);		//aquire the semaphore
		pthread_mutex_lock(&mutex);

		if (insert_item(prodItem))								//critical section
			std::cout << "error producing item" << '\n';
		else {
			std::cout << "producer pMroduced item: " << prodItem << '\n';
			display_buffer();
		}

		pthread_mutex_unlock(&mutex);
		sem_post(&fullSem);		//release the semaphore

	}
}

/*Consumer Thread*/
void* consumer(void* param) {
	buffer_item item;

	while (true) {
		//sleep for random time
		sleep(rand() % 10);
		//generate random number

		sem_wait(&fullSem);		//aquire the semaphore
		pthread_mutex_lock(&mutex);

		if (remove_item(&item))
			std::cout << "error consuming item" << '\n';			//critical section
		else {
			std::cout << "consumer consumed item: " << item << '\n';
			display_buffer();
			
		}

		pthread_mutex_unlock(&mutex);
		sem_post(&emptySem);		//release the semaphore
	}
}

int main(int argc, char* argv[]) {
	if (argc < 4) {
		std::cout << "Usage Error: Need more arguments: ./prog4 {sleeptime} {number of producers} {number of consumers}" << '\n';
		exit(1);
	}

	std::cout << "--------------------------------------------------" << std::endl;
	std::cout << "CS 433 Programming assignment 4" << std::endl;
	std::cout << "Author: Jake Tremblay, Danny Ha, Brian Richey" << std::endl;
	std::cout << "Date: 11/17/2021" << std::endl;
	std::cout << "Course: CS433 (Operating Systems)" << std::endl;
	std::cout << "Description : Producer Consumer Problem" << std::endl;
	std::cout << "--------------------------------------------------" << std::endl;
	std::cout << '\n';

	srand(time(NULL));
	
	//  1. Get CLI argv[1] argv[2] argv[3] -> sleep time, number of producers, number of consumers
	int sleepTime = atoi(argv[1]);
	int producerCount = atoi(argv[2]);
	int consumerCount = atoi(argv[3]);
	
	//	2. Init semaphores, mutex, buffers
	sem_init(&emptySem, 0, 0);
	sem_init(&fullSem, 0, BUFFER_SIZE);

	pthread_mutex_init(&mutex, NULL);

	pthread_t producers[producerCount];
	pthread_t consumers[consumerCount];
	
	//	3. Create producer threads
	for (int i = 0; i < producerCount; i++) {
		//create producer
		pthread_create(&producers[i], NULL, producer, NULL);		
	}
	
	//	4. Create consumer threads
	for (int i = 0; i < consumerCount; i++) {
		//create consumers
		pthread_create(&consumers[i], NULL, consumer, NULL);		
	}
	
	//	5. Sleep
	sleep(sleepTime);
	
	//	6. Exit
	return 0;
}