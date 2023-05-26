#include "ResourcePool.h"

#include <atomic>
#include <chrono>
#include <csignal>
#include <stdio.h>
#include <thread>
#include <unistd.h>

struct TestResource
{
	unsigned int pid;
};
using TestPool = concrt::ResourcePool<TestResource, concrt::PC_Model::MPMC>;

static bool stay_alive = true;
static std::atomic<unsigned long long> total_consume_count;
static TestPool *pool;

void
produce(
	unsigned int pid)
{
	while (stay_alive)
	{
		TestResource *res;
		if (pool->acquire_busy(res,100) != 0)
		{
			continue;
		}

		res->pid = pid;
		pool->produce(res);
	}
}

void
consume(
	unsigned int cid)
{
	unsigned long consume_count = 0;
	while (stay_alive)
	{
		TestResource *res;
		if (pool->consume_busy(res,100) != 0)
		{
			continue;
		}

		consume_count++;
		pool->release(res);
	}
	printf("consumer %d consumed %ld resources\n",cid,consume_count);
	total_consume_count += consume_count;
}

void handle_sigint(int)
{
	stay_alive = false;
}

int main()
{
	signal(SIGINT,handle_sigint);
	pool = new TestPool(16);
	total_consume_count = 0;

	unsigned int n_pthreads = 1;
	unsigned int n_cthreads = 1;

	std::thread p_threads[n_pthreads];
	std::thread c_threads[n_cthreads];

	// spin up threads
	for (unsigned int i=0; i<n_cthreads; i++)
	{
		c_threads[i] = std::thread(&consume,i);
	}
	printf("Started %d consumer thread(s)\n",n_cthreads);
	for (unsigned int i=0; i<n_pthreads; i++)
	{
		p_threads[i] = std::thread(&produce,i);
	}
	printf("Started %d producer thread(s)\n",n_pthreads);

	// wait for threads to join
	std::chrono::time_point<std::chrono::high_resolution_clock> start = std::chrono::high_resolution_clock::now();
	sleep(20);
	stay_alive = false;
	for (unsigned int i=0; i<n_pthreads; i++)
	{
		p_threads[i].join();
	}
	for (unsigned int i=0; i<n_cthreads; i++)
	{
		c_threads[i].join();
	}
	std::chrono::nanoseconds dur = std::chrono::duration_cast<std::chrono::nanoseconds>(
		std::chrono::high_resolution_clock::now() - start);

	printf("total consume count = %lld\n", total_consume_count.load());
	printf("duration = %ldns\n", dur.count());
	printf("throughput = %0.2fres/sec\n", (float)total_consume_count.load()/dur.count()*1000000000.0);

	return 0;
}