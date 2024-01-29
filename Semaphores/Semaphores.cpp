#include <cstdio>
#include <iostream>
#include <pthread.h>
#include <semaphore.h>
#include <queue>
#include <signal.h>

#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KNRM  "\x1B[0m"

#define N 10
#define BUFFER_SIZE 10


using namespace std;

sem_t mutex1, mutex2, full, emp; 
int counter = 0;
queue<int> buffer;
int collector_index = 0, monitor_index = 0;

void* Counter_thread(void *args)
{
	int thread_no = *(int*)args;
   	free(args); 
	while (1){
		
		// Sleep Before
		int rand_time = rand() % 20 + 15;
		sleep(rand_time);
		cout << KGRN <<"\nCounter Thread " << thread_no <<": recieved a message\n";
		
		cout << KGRN <<"\nCounter Thread " << thread_no <<": waiting to write\n";
		// Wait
		sem_wait(&mutex1); 

		// Critical Section
		counter++;
		cout << KGRN <<"\nCounter Thread " << thread_no <<": now adding to counter, counter value = " << counter << "\n";
		
		// Signal
		sem_post(&mutex1);
			
	}
}

void* Monitor_thread(void *args)
{
	while (1){
		//sleep Before
		int rand_time = rand() % 5 + 1;
		sleep(rand_time);
	
		int sem_value_mon;
		sem_getvalue(&full,&sem_value_mon);
		if (sem_value_mon == BUFFER_SIZE)
			cout << KYEL << "\nMonitor thread: Buffer full!!\n";
		
		cout << KYEL <<"\nMonitor Thread: waiting to read counter\n";
		// Wait for counter
		sem_wait(&mutex1); 
		
		// Critical Section
		int c = counter;
		cout << KYEL <<"\nMonitor Thread: reading a count value of " << counter << "\n";
		counter = 0;
		
		// Signal
		sem_post(&mutex1); 
		
		
		// Wait
		sem_wait(&emp);
		
		// Wait to write on buffer
		sem_wait(&mutex2);
		
		// Critical Section
		buffer.push(c);
		cout << KYEL << "\nMonitor thread: writing to buffer at position " << monitor_index % BUFFER_SIZE << endl;
		monitor_index = (monitor_index + 1) % BUFFER_SIZE; 
		
		// Signal 
		sem_post(&mutex2); 

		
		// Signal 
		sem_post(&full); 
		
		
	}
}

void* Collector_thread(void *args) 
{ 
	while (1){
	
		// Sleep Before 
		int rand_time = rand() % 15 + 5;
		sleep(rand_time);
		
		int sem_value_col;
		sem_getvalue(&emp,&sem_value_col);
		if (sem_value_col == BUFFER_SIZE)
			cout << KRED << "\nCollector thread: nothing is in the buffer!" << endl; 
		
		// Wait
		sem_wait(&full);
		
		// Wait
		sem_wait(&mutex2);
		
		// Critical Section 
		buffer.pop();
		cout << KRED << "\nCollector thread: reading from the buffer at position " << collector_index % BUFFER_SIZE << endl;
		collector_index = (collector_index + 1) % BUFFER_SIZE;
		
		// Signal
		sem_post(&mutex2); 
	
		// Signal
		sem_post(&emp); 
		
		
	}
	
} 

void intHandler(int dummy) {
	// set the noramal color back
	cout << KNRM << endl << "EXIT" << endl;
	// Destroy the semaphore 
	sem_destroy(&mutex1);
	sem_destroy(&mutex2); 
	sem_destroy(&full); 
	sem_destroy(&emp); 
	exit(0);
}

int main()
{
	signal(SIGINT, intHandler);
	
 	pthread_t mCounter[N], mMonitor, mCollector;
 
	sem_init(&mutex1, 0, 1); 
	sem_init(&mutex2, 0, 1); 
	sem_init(&full, 0, 0); 
	sem_init(&emp, 0, BUFFER_SIZE); 
	
	for(int i=1; i<=N; i++){
		int* arg = new int(i);
		pthread_create(&mCounter[i-1], NULL, Counter_thread, arg);
		//sleep(2);	
	}
	pthread_create(&mMonitor,NULL,Monitor_thread, NULL);
	//sleep(2);
	pthread_create(&mCollector,NULL,Collector_thread, NULL);
	
	for(int i=0; i<N; i++)
		pthread_join(mCounter[i],NULL);
	pthread_join(mMonitor,NULL); 
	pthread_join(mCollector,NULL); 
	
	return 0; 


}
