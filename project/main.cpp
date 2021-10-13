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
		// Adds string msg to the vector of logs
		void write_to_log(string msg);
		// Prints all of the messages in the log so far
		void read_from_log();
	private:
		// This is the mutex we used to keep the different threads using the log synchronized
		mutex log_lock;
		// This is the vector containing all of the logs
		vector<string> messages;
};

void Log::write_to_log(string msg) {
	log_lock.lock();
	// Critical section
	messages.push_back(msg);

	log_lock.unlock();
}

void Log::read_from_log() {
	log_lock.lock();
	// Critical section
	for (string str : messages) {
		cout << str << "\n";
	}
	log_lock.unlock();
}


class Buffer {
	public:
		// Adds integer input to the buffer
		void write_to_buffer(int input);
		// Prints all of the messages from the log attached to this buffer
		void read_from_log();
		// Removes an element from the buffer
		void remove_from_buffer();
		// Sets a bound on the buffer of size bound
		void set_bound(int bound);
		// Removes the bound from the buffer
		void unset_bound();

	private:
		// Used to track if the buffer is bounded or not, initally the buffer is not bounded
		bool bounded = false;
		// Stores the size that was input for the bound
		int bound;
		// Tracks the index of where the next input should go, if the buffer is bounded
		int index;
		// The mutex that is used to synchronize the buffer
		mutex buffer_lock;
		// The log that this buffer sends messages to
		Log log;
		// The vector that stores all of the inputs
		vector<int> buffer;

};


void Buffer::write_to_buffer(int input) {
	buffer_lock.lock();
	if (bounded) {
		if (bound == index) {
			log.write_to_log("Unable to add to buffer: bound has been reached");
			buffer_lock.unlock();
			return;
		}
		else {
			// Critical section (when bounded)
			buffer[index] = input;
			index += 1;

			return;
		}
	}
	else {
		try {
			// Critical exception (when unbounded)
			buffer.push_back(input);

		}
		catch (const exception& e) {
			log.write_to_log("Unable to write to buffer: error " + (string) e.what());
			buffer_lock.unlock();
		}
	}
	log.write_to_log("Added " + to_string(input) + " to the buffer");
	buffer_lock.unlock();
}

void Buffer::read_from_log() {
	buffer_lock.lock();

	log.read_from_log();

	buffer_lock.unlock();
}

void Buffer::remove_from_buffer() {
	buffer_lock.lock();

	if (buffer.empty()) {
		log.write_to_log("Unable to remove from empty buffer");
	}
	else {
		if (bounded) {
			// Critcal section (when bounded)
			buffer[index - 1] = 0;
			index -= 1;
		}
		else {
			// Critical section (when unbounded)
			buffer.pop_back();
		}
		log.write_to_log("Removed item from buffer");
	}
	buffer_lock.unlock();
}

void Buffer::set_bound(int bound) {
	buffer_lock.lock();

	// Critical section
	index = buffer.size();
	this->bound = bound;
	bounded = true;
	if (!buffer.empty())
		buffer.resize(bound);

	log.write_to_log("Added a bound to the buffer. Bound is " + to_string(bound));

	buffer_lock.unlock();
}

void Buffer::unset_bound() {
	buffer_lock.lock();

	// Critical section
	bounded = false;

	log.write_to_log("Removed the bound");

	buffer_lock.unlock();
}

int main(int argc, char* argv[]) {
	Buffer new_buff;

	thread t1(&Buffer::write_to_buffer,&new_buff,1);
	thread t2(&Buffer::write_to_buffer,&new_buff,4);
	thread t3(&Buffer::write_to_buffer,&new_buff,8);
	thread t4(&Buffer::set_bound,&new_buff,3);
	thread t8(&Buffer::unset_bound,&new_buff);
	thread t5(&Buffer::write_to_buffer,&new_buff,100);
	thread t6(&Buffer::write_to_buffer,&new_buff,9);
	thread t7(&Buffer::write_to_buffer,&new_buff,0);


	t1.join();
	t2.join();
	t3.join();
	t4.join();
	t5.join();
	t6.join();
	t7.join();

	new_buff.read_from_log();
}
