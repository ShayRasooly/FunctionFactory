#pragma once
#include <mutex>
#include <shared_mutex>
#include <condition_variable>
#include <queue> 
#include <iostream>

using namespace std;

// rand() doesnt support concurrency, so this class provide random numbers safely
class ThreadSafeRand
{
private:
	mutable shared_mutex mut;
public:
	ThreadSafeRand() {
		srand((unsigned)time(0));
	}

	inline int rand_int() {
		unique_lock lock(mut);
		return rand();
	}

	// randomize the integer part and then the floating point part
	inline double rand_double() {
		unique_lock lock(mut);
		return double(rand()) + (double(rand()) / double(RAND_MAX));
	}
};

class ThreadSafeCounter
{
private:
	int counter;
	shared_mutex mut;
	condition_variable_any cv;
public:
	ThreadSafeCounter() { counter = 0; }

	inline void increase() {
		unique_lock lock(mut);
		++counter;
		cv.notify_all();
	}

	inline void wait_until_equales(int val) {
		shared_lock lock(mut);
		cv.wait(lock, [this, val] { return counter == val; });
	}
};

// cout doesnt support concurrency, so this class print safely
class ThreadSafePrinter 
{
private:
	mutex mut;
public:
	inline void print(string str) {
		unique_lock lock(mut);
		cout << str << endl;
	}
};

template<typename T>
class ThreadSafeQueue { // Thread Safe Queue
private:
	size_t capacity;
	int id; // uniqe id
	queue<T> _queue; // T elements queue
	mutable shared_mutex mut; // object mutex. lock specific queue
	condition_variable_any cv; // cv for notifying the conditions

public:
	ThreadSafeQueue() = delete;

	// constructor for infinite capacity
	ThreadSafeQueue(int id) {
		this->id = id;
		this->capacity = 0;
	}

	// constructor for limited capacity
	ThreadSafeQueue(int id, int capacity) {
		this->id = id;
		this->capacity = capacity;
	}

	// return the front element, if empty wait until an element is inserted.
	T front() {
		shared_lock lock(mut);
		cv.wait(lock, [this] {return !_queue.empty(); });
		return _queue.front();
	}

	// return the front element and remove it from queue, if empty wait until an element is inserted.
	T pop() {
		unique_lock lock(mut);
		cv.wait(lock, [this] {return !_queue.empty(); });
		T tmp = _queue.front();
		_queue.pop();
		cv.notify_all();
		return tmp;
	}

	// insert element to end of queue. if full wait until an element is removed.
	void push(const T& item) {
		unique_lock lock(mut);
		if (capacity > 0) {
			cv.wait(lock, [this] {return _queue.size() < capacity; });
		}
		_queue.push(item);
		cv.notify_all();
	}

	inline bool can_swap() {
		return !this->_queue.empty() && ((this->_queue.size() < this->capacity) || (this->capacity == 0));
	}

	// swaps between heads of two queues
	void swap_heads(shared_ptr<ThreadSafeQueue<T>> other) {
		unique_lock lock(mut);
		unique_lock other_lock(other->mut);
		cv.wait(lock, [this] {return this->can_swap(); });
		other->cv.wait(other_lock, [other] { return other->can_swap(); });

		T front = other->_queue.front();
		this->_queue.push(front);
		other->_queue.pop();

		front = this->_queue.front();
		other->_queue.push(front);
		this->_queue.pop();
	}
};