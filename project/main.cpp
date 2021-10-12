/**
  * Assignment: synchronization
  * Operating Systems
  * Jan Hendron (s1049777)
  */

/**
  Hint: F2 (or Control-klik) on a functionname to jump to the definition
  Hint: Ctrl-space to auto complete a functionname/variable.
  */

// function/class definitions you are going to use
#include <algorithm>
#include <exception>
#include <iostream>
#include <mutex>
#include <stdexcept>
#include <thread>
#include <vector>

// although it is good habit, you don't have to type 'std::' before many objects by including this line
using namespace std;

class Log {
	public:
		void write_to_log(string msg);
		string read_from_log();
		~Log();
	private:
		mutex log_lock;
		vector<string> messages;
};

Log::~Log() {
	// This ensures that the lock is definetly unlocked after the Log
	// is finished runnning
	log_lock.unlock();
}

void Log::write_to_log(string msg) {
	log_lock.lock();
	// Critical section
	messages.push_back(msg);

	log_lock.unlock();
}

string Log::read_from_log() {
	string msg;
	log_lock.lock();
	// Critical section
	msg = messages.front();

	log_lock.unlock();
	return msg;
}


class Buffer {
	public:
		void write_to_buffer(int input);
		void read_from_buffer();
		int read_from_buffer(int r_index);
		void set_bound(int bound);
		void unset_bound();
	private:
		bool bounded;
		int bound;
		int index;
		mutex buffer_lock;
		mutex bound_lock;
		Log log;
		vector<int> buffer;
};


void Buffer::write_to_buffer(int input) {
	bound_lock.lock();
	buffer_lock.lock();
	if (bounded) {
		if (bound == index + 1) {
			log.write_to_log("Unable to add to buffer: bound has been reached");

			return;
		}
		else {

			buffer[index] = input;
			index += 1;

			return;
		}
	}
	else {
		try {

			buffer.push_back(input);

		}
		catch (exception e) {
			log.write_to_log("Unable to write to buffer: error " + (string) e.what());
			buffer_lock.unlock();
		}
	}
	buffer_lock.unlock();
	bound_lock.unlock();
}

void Buffer::read_from_buffer() {
	bound_lock.lock();
	buffer_lock.lock();
	if (bounded) {
		for (int i = 0; i <= index; i++) {
			if (i == index) {
				cout << buffer[i] <<"\n";
			}
			else {
				cout << buffer[i] << ", ";
			}
		}
	}
	else {
		for (size_t i = 0; i < buffer.size(); i++) {
			if (i == buffer.size() - 1) {
				cout << buffer[i] << "\n";
			}
			else {
				cout << buffer[i] << ", ";
			}
		}
	}
	buffer_lock.unlock();
	bound_lock.unlock();
}

int Buffer::read_from_buffer(int r_index) {
	bound_lock.lock();
	buffer_lock.lock();
	int ans;

	if (bounded) {
		if (r_index > this->index) {
			log.write_to_log("Unable to read position: error outside of range");
		}
		else {
			ans = buffer.at(r_index);
		}
	}
	else {
		ans = buffer.at(r_index);
	}
	bound_lock.unlock();
	buffer_lock.unlock();
	return ans;
}

void Buffer::set_bound(int bound) {
	bound_lock.lock();

	index = buffer.size() - 1;
	this->bound = bound;
	bounded = true;
	buffer.resize(bound);

	bound_lock.unlock();
}

void Buffer::unset_bound() {
	bound_lock.lock();

	bounded = false;

	bound_lock.unlock();
}

int main(int argc, char* argv[]) {
	Buffer new_buff;
	int arr[] = {1,4,5,6,7,78};

	thread t1(&Buffer::write_to_buffer,&new_buff,1);
	thread t2(&Buffer::write_to_buffer,&new_buff,4);
	thread t3(&Buffer::write_to_buffer,&new_buff,8);
	thread t4(&Buffer::write_to_buffer,&new_buff,2);
	thread t5(&Buffer::write_to_buffer,&new_buff,100);
	thread t6(&Buffer::write_to_buffer,&new_buff,9);
	thread t7(&Buffer::write_to_buffer,&new_buff,0);

	new_buff.read_from_buffer();

	t1.join();
	t2.join();
	t3.join();
	t4.join();
	t5.join();
	t6.join();
	t7.join();
}
