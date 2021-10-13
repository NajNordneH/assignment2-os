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
		void read_from_log();
	private:
		mutex log_lock;
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
		void write_to_buffer(int input);
		void read_from_log();
		void remove_from_buffer();
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
	buffer_lock.lock();
	if (bounded) {
		if (bound == index + 1) {
			log.write_to_log("Unable to add to buffer: bound has been reached");
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
		catch (exception e) {
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
	buffer.resize(bound);

	buffer_lock.unlock();
}

void Buffer::unset_bound() {
	buffer_lock.lock();

	// Critical section
	bounded = false;

	buffer_lock.lock();
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


	t1.join();
	t2.join();
	t3.join();
	t4.join();
	t5.join();
	t6.join();
	t7.join();
	new_buff.read_from_log();
}
