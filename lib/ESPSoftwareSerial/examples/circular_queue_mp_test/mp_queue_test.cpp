// circular_mp_test.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include "circular_queue/circular_queue_mp.h"

struct qitem
{
	// produer id
	int id;
	// monotonic increasing value
	int val = 0;
};

constexpr int TOTALMESSAGESTARGET = 60000000;
// reserve one thread as consumer
const auto THREADS = std::thread::hardware_concurrency() / 2 - 1;
const int MESSAGES = TOTALMESSAGESTARGET / THREADS;
circular_queue<std::thread> threads(THREADS);
circular_queue_mp<qitem> queue(threads.capacity()* MESSAGES / 10);
std::vector<int> checks(threads.capacity());

int main()
{
	using namespace std::chrono_literals;
	std::cerr << "Utilizing " << THREADS << " producer threads" << std::endl;
	for (int i = 0; i < threads.capacity(); ++i)
	{
		threads.push(std::thread([i]() {
			for (int c = 0; c < MESSAGES;)
			{
				// simulate some load
				auto start = std::chrono::system_clock::now();
				while (std::chrono::system_clock::now() - start < 1us);
				if (queue.push({ i, c }))
				{
					++c;
				}
				else
				{
					//std::cerr << "queue full" << std::endl;
					//std::this_thread::sleep_for(10us);
				}
				//if (0 == c % 10000) std::this_thread::sleep_for(10us);
			}
			}));
	}
	for (int o = 0; o < threads.available() * MESSAGES; ++o)
	{
		auto now = std::chrono::system_clock::now();
		while (!queue.available())
		{
			auto starvedFor = std::chrono::system_clock::now() - now;
			if (starvedFor > 20s) std::cerr << "queue starved for > 20s" << std::endl;
			//std::this_thread::sleep_for(20ms);
		}
		auto item = queue.pop();
		if (checks[item.id] != item.val)
		{
			std::cerr << "item mismatch" << std::endl;
		}
		checks[item.id] = item.val + 1;
		if (0 == item.val % 1000) std::this_thread::sleep_for(100us);
	}
	while (threads.available())
	{
		auto thread = threads.pop();
		thread.join();
	}
	return 0;
}
