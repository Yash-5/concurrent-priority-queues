#include <iostream>
#include <cstdio>
#include <thread>
#include <atomic>
#include <fstream>
#include <string>
#include <cstdlib>
#include <time.h>
#include <chrono>
#include <random>
#include <unistd.h>
#include <cmath>
#include "lockfree_skiplists.h"

using namespace std;
using namespace chrono;

int n_add, n_remove, k_add, k_remove, lamda_add, lambda_remove;
long long microseconds_add = 0, microseconds_remove = 0;
atomic<bool> add_created, remove_created;
skipListQueue<int> *S;

char* currentTime(time_t curr_time) {
	struct tm* local_time = localtime(&curr_time);
	char *ret_time = new char[100];
	strftime(ret_time, 100, "%T", local_time);
	return ret_time; 
}

int delay(int x) {
	int sd = chrono::system_clock::now().time_since_epoch().count();
	default_random_engine random_gen(sd);
	exponential_distribution <double> dist_1(1.0/x);
	return (int)dist_1(random_gen);
}

void removeTh(int id) {
	while (!remove_created.load()) {};
	int th_id = id;
	int value;
	for (int i = 0; i < k_remove; i++) {
		auto start = std::chrono::high_resolution_clock::now();
		value = S->removeMin();
		time_t finish_time = time(NULL);
		auto wait_time = std::chrono::high_resolution_clock::now() - start;
		if (value == INT_MIN) {
			printf("At %s : EMPTY QUEUE! thread %d - trial number %d - failed removing min element\n", 
				currentTime(finish_time), th_id, i+1);
			fflush(stdout);
		}
		else{
			printf("At %s : thread %d - trial number %d - removed min element %d \n", 
				currentTime(finish_time), th_id, i+1, value);
			fflush(stdout);
		}
		
		microseconds_remove += std::chrono::duration_cast<std::chrono::microseconds>(wait_time).count();
		sleep(delay(lambda_remove));
	}
	pthread_exit(NULL);

}

void addTh(int id) {
	while (!add_created.load()) {};
	int th_id = id;
	bool success = false;
	for (int i = 0; i < k_add; i++) {
		int value = rand()%16;
		auto start = std::chrono::high_resolution_clock::now();
		success = S->insert(value);
		time_t finish_time = time(NULL);
		auto wait_time = std::chrono::high_resolution_clock::now() - start;
		if (success)
		{
			printf("At %s: thread %d - trial number %d - inserted %d in the queue\n", 
				currentTime(finish_time), th_id, i, value);
		}
		else
		{
			printf("At %s: DUPLICATE! thread %d - trial number %d - failed inserting %d\n",
				currentTime(finish_time), th_id, i, value);
		}

		microseconds_add += std::chrono::duration_cast<std::chrono::microseconds>(wait_time).count();
		sleep(delay(lamda_add));
	}
	pthread_exit(NULL);
}

int main() {

	srand(time(NULL));
	freopen("inp-params.txt", "r", stdin);
	freopen("result-output.txt", "w", stdout);
	cin >> n_add >> n_remove >> k_add >> k_remove >> lamda_add >> lambda_remove;
	
	setMaxHeight((int)log2(n_add * k_add));
	S = new skipListQueue<int>();
	add_created.store(false);
	remove_created.store(false);
	thread *arr_add = new thread[n_add];
	for (int i=0; i<n_add; i++) {
		arr_add[i] = thread(addTh, i);
	}
	thread *arr_remove = new thread[n_remove];
	for (int j=0; j<n_remove; j++) {
		arr_remove[j] = thread(removeTh, j);
	}
	add_created.store(true);
	remove_created.store(true);
	for(int i = 0; i < n_add; i++)
		arr_add[i].join();
	for (int j=0; j<n_remove; j++) 
		arr_remove[j].join();
	printf("Average execution time for Insert is %lld\n", microseconds_add);
	fflush(stdout);
	printf("Average execution time for removeMin is %lld\n", microseconds_remove);
	fflush(stdout);
	return 0;
}

