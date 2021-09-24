// TwoSensorsOneController.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <thread>
#include <fstream>
#include <cstdlib>
#include <stdlib.h> 
#include <filesystem>
#include <vector>
#include <chrono>
#include <atomic>
#include <regex>

class Thread_Object {
public: //some defines to make the class easier to read
#define log_buffer (*object_data_->log_buffer_)
#define out_buffer (*object_data_->output_buffer_)
#define time_now std::chrono::high_resolution_clock::now
#define timems_t std::chrono::milliseconds
#define timepoint_t std::chrono::high_resolution_clock::time_point
#define duration std::chrono::duration_cast
	struct Object_Info {
		timepoint_t launch_tpoint_;
		timepoint_t sync_tpoint_;
		timepoint_t schedual_tpoint_;
		timepoint_t current_tpoint_;
		timems_t schedual_duration_;
		timems_t total_duration_;
		timems_t running_duration_limit_;
		
		std::atomic<timems_t> schedual_timer_;
		std::thread* thread_;
		std::atomic<bool> active_; //control condition
		std::atomic<bool> shutdown_; //control condition
		std::atomic<bool> is_active_; //response condition from thread
		std::atomic<bool> is_shutdown_;	//response condition from thread
		std::atomic<bool> is_ready_; // response condition from thread
		std::atomic<bool> dual_output_;
		std::stringstream* output_buffer_;
		std::stringstream* log_buffer_;
	};

protected:
	Object_Info* object_data_;
	
	void null_data() {
		object_data_->active_.store(false);
		object_data_->shutdown_.store(false);
		object_data_->is_active_.store(false);
		object_data_->is_shutdown_.store(true);
		object_data_->is_ready_.store(false);
		object_data_->dual_output_.store(false);
	}
	void print_log_buffer() {
		std::clog << object_data_->log_buffer_->rdbuf()->str();
		if (object_data_->dual_output_.load() == true) {
			std::cout << log_buffer.rdbuf()->str();
		}
		object_data_->log_buffer_->rdbuf()->str(std::string());
	}
	void print_out_buffer() {
		std::fstream outputfile("buffer.txt", std::ios::app | std::ios::out);
		if (outputfile.is_open()) {
			log_buffer << "Object " << std::this_thread::get_id() << " prints to buffer: " << out_buffer.rdbuf()->str() << "\n";
			print_log_buffer();
			outputfile << out_buffer.rdbuf()->str();
			out_buffer.rdbuf()->str(std::string());
			outputfile.close();
		}
		else {
			log_buffer << "Object " << std::this_thread::get_id() << " could not open buffer file.\n";
			print_log_buffer();
		}
	}
public:
	Thread_Object() {
		object_data_ = new Object_Info();
		object_data_->output_buffer_ = new std::stringstream();
		object_data_->log_buffer_ = new std::stringstream();
		null_data();
	}
	~Thread_Object() {
		while (is_shutdown() == false);
		delete (object_data_->output_buffer_);
		delete (object_data_->log_buffer_);
		delete object_data_;
	}
	bool is_shutdown() {
		return object_data_->is_shutdown_.load();
	}
	bool is_active() {
		return object_data_->is_active_.load();
	}
	bool is_ready() {
		return object_data_->is_ready_.load();
	}
	//ensures shutdown state is set
	void shutdown() {
		while (object_data_->shutdown_.load() == false)
			object_data_->shutdown_.store(true);
	}
	//ensures active state is set
	void activate(timepoint_t syncArg) {
		object_data_->sync_tpoint_ = syncArg;;
		while (object_data_->active_.load() == false)
			object_data_->active_.store(true);
	}
	void initialize(bool doArg, timems_t schArg, timems_t limitArg) {
		null_data();
		object_data_->launch_tpoint_ = time_now();
		object_data_->running_duration_limit_ = limitArg;
		object_data_->schedual_timer_.store(schArg);
		object_data_->dual_output_.store(doArg);
		while (object_data_->is_shutdown_.load() == true) {
			object_data_->is_shutdown_.store(false);
		}
		object_data_->thread_ = new std::thread(&Thread_Object::program,this);
		//object_data_->thread_->detach();
	}
protected:
	//ensures thread communicates its "I am shutdown" state
	void shutdown_response() {
		while (object_data_->is_shutdown_.load() == false)
			object_data_->is_shutdown_.store(true);
	}
	//ensures thread communicates its "I am active" state
	void activate_response() {
		while (object_data_->is_active_.load() == false)
			object_data_->is_active_.store(true);
	}
	//ensures thread communicates its "I am ready" state
	void ready_response() {
		while (object_data_->is_ready_.load() == false)
			object_data_->is_ready_.store(true);
	}

	virtual void program() = 0;
};

class Sensor : public Thread_Object {
protected:

	void prepair_data() {
		float value = (((rand() % 19901) + 100) * .01f);  //1-200.00 
		std::string file = "";
		int counter = 0;
		int lottery = (rand() % 5) + 1;
		int corruption = (rand() % 5);//normal output string has a big comment that takes half the string
		while (counter != lottery) {
			for (const auto& entry : std::filesystem::directory_iterator(".")) {
				if (entry.is_regular_file() == true) {
					if (counter == lottery) {
						file = entry.path().filename().string();
						break;
					}
					counter++;
				}
			}
		}
		out_buffer << "% Sensor " << std::this_thread::get_id() << " outputs:%" << (float)value << ";" << file << "\n";
		if (corruption == 0) {
			std::string corruption_values = "%;\".?!|][&*%$^ ~`'}{><";//some mean looking values
			std::streampos loc = out_buffer.tellp();
			for (size_t i = 0; i < rand() % 25; i++) {
				out_buffer.seekp((rand() % (out_buffer.str().size() - 2)));
				out_buffer << corruption_values.at(rand() % corruption_values.size());
			}
			out_buffer.seekp(loc);
		}
	}

	virtual void program() override {
		/*Do stuff needed before returning ready response*/
		bool data_prepaired = false;
		srand((unsigned)time(0));

		/*--------------------------------------------------*/
		log_buffer << "Sensor " << std::this_thread::get_id() << " is now ready.\n";
		print_log_buffer();
		ready_response();
		while (object_data_->active_.load() == false) {
			std::this_thread::sleep_for(timems_t(1));
		}
		/*Do stuff needed before returning activate response*/
		object_data_->schedual_tpoint_ = object_data_->sync_tpoint_;

		/*--------------------------------------------------*/
		log_buffer << "Sensor " << std::this_thread::get_id() << " is now active.\n";
		print_log_buffer();
		activate_response();
		while (object_data_->shutdown_.load() == false) {
			object_data_->current_tpoint_ = time_now();
			object_data_->schedual_duration_ = duration<timems_t>(object_data_->current_tpoint_ - object_data_->schedual_tpoint_);
			object_data_->total_duration_ = duration<timems_t>(object_data_->current_tpoint_ - object_data_->launch_tpoint_);
			
			if (object_data_->schedual_duration_ >= object_data_->schedual_timer_.load()) { //output data phase
				object_data_->schedual_tpoint_ += timems_t(object_data_->schedual_timer_.load());
				object_data_->schedual_duration_ = timems_t(0);
				log_buffer << "Sensor " << std::this_thread::get_id() << " is sending data.\n";
				print_log_buffer();
				print_out_buffer();
				data_prepaired = false;
			}
			else { //prepair data phase
				if (data_prepaired == false) {
					log_buffer << "Sensor " << std::this_thread::get_id() << " is prepairing data.\n";
					print_log_buffer();
					prepair_data();
					data_prepaired = true;
				}
				object_data_->current_tpoint_ = time_now();
				object_data_->schedual_duration_ = duration<timems_t>(object_data_->current_tpoint_ - object_data_->schedual_tpoint_);
				
				log_buffer << "Sensor " << std::this_thread::get_id() << " is waiting for next schedualed update in " << (object_data_->schedual_timer_.load() - object_data_->schedual_duration_).count() << "ms. \n";
				print_log_buffer();
				std::this_thread::sleep_for(object_data_->schedual_timer_.load() - object_data_->schedual_duration_); // sleep for remaining time until next output
			}
			if (object_data_->schedual_tpoint_ >= (object_data_->sync_tpoint_ + object_data_->running_duration_limit_)) { // timer shutdown event
				shutdown();
			}
		}
		/*Do stuff needed before returning shutdown response*/

		/*--------------------------------------------------*/
		log_buffer << "Sensor " << std::this_thread::get_id() << " is now shutdown after running " << object_data_->total_duration_.count() << "ms.\n";
		print_log_buffer();
		shutdown_response();
	}
};
class Controller : public Thread_Object {
protected:
	virtual void program() override {
		/*Do stuff needed before returning ready response*/
		log_buffer << "Controller " << std::this_thread::get_id() << " is clearing old buffer.\n";
		print_log_buffer();
		std::fstream my_file("buffer.txt", std::ios::out | std::ios::trunc);//erase old data
		my_file.close();
		std::vector<std::string> saved_lines;
		std::streampos read_position = std::ios::beg;
		int verified_counter = 0;
		int corruption_counter = 0;
		bool data_prepaired = false;
		std::regex comments_expr("(%[^%]*%)+");
		std::regex detect_fields("[[:digit:]]+[\.]?[[:digit:]]*;([[:alnum:]]*[\.][[:alnum:]]+)+");
		std::regex detect_invalid_tokens("[^[:digit:][:alpha:]\.;]+");	

		/*--------------------------------------------------*/
		log_buffer << "Controller " << std::this_thread::get_id() << " is now ready.\n";
		print_log_buffer();
		ready_response();
		while (object_data_->active_.load() == false) {
			std::this_thread::sleep_for(timems_t(1));
		}
		/*Do stuff needed before returning activate response*/
		object_data_->schedual_tpoint_ = object_data_->sync_tpoint_;
		
		/*--------------------------------------------------*/
		log_buffer << "Controller " << std::this_thread::get_id() << " is now active.\n";
		print_log_buffer();
		activate_response();
		while (object_data_->shutdown_.load() == false) {
			object_data_->current_tpoint_ = time_now();
			object_data_->schedual_duration_ = duration<timems_t>(object_data_->current_tpoint_ - object_data_->schedual_tpoint_);
			object_data_->total_duration_ = duration<timems_t>(object_data_->current_tpoint_ - object_data_->launch_tpoint_);

			if (object_data_->schedual_duration_ >= object_data_->schedual_timer_.load()) { //output data phase
				object_data_->schedual_tpoint_ += timems_t(object_data_->schedual_timer_.load());
				object_data_->schedual_duration_ = timems_t(0);
				log_buffer << "Controller " << std::this_thread::get_id() << " is reading data.\n";
				print_log_buffer();

				verified_counter = 0;
				my_file.open("buffer.txt", std::ios::in | std::ios::out);//read & copy old data
				std::string current_line;
				while (std::getline(my_file, current_line)) {
					/*some temp variables for determining useable info*/
					bool corruption = false;
					std::smatch reg_results;
					std::string check_line = current_line;
					/*Line processing*/
					check_line = std::regex_replace(check_line, comments_expr, ""); // remove comments, incase there is a comment inside the fields

					if (std::regex_search(check_line, reg_results, detect_fields)) { // isolate fields
						check_line = reg_results.str();
						if (!std::regex_search(check_line, reg_results, detect_invalid_tokens)) { // check for invalid tokens in fields, could be more strict however if it works let the line output
							verified_counter++;
							current_line += "\n";
							saved_lines.push_back(current_line); //could check if the file exists I guess but who cares, should be fine
						}
						else {
							corruption_counter++;
						}
					}
					else {
						corruption_counter++;
					}
				}
				my_file.close();
				my_file.open("buffer.txt", std::ios::out | std::ios::trunc); //erase old data
				my_file.close();
				log_buffer << "Controller " << std::this_thread::get_id() << " is writing verified data.\n";
				print_log_buffer();
				my_file.open("buffer.txt", std::ios::out | std::ios::app); //writes to file will be sent while sensor writes
				if (my_file.is_open()) {

					for (size_t i = 0; i < saved_lines.size(); i++) {
						
						log_buffer << "writing verified line: " << saved_lines.at(i);
						print_log_buffer();

						my_file << saved_lines.at(i);
					}
				}
				my_file.close();
				saved_lines.clear();
			}
			else { //wait for next update
				log_buffer << "Controller " << std::this_thread::get_id() << " is waiting for next schedualed update in " << (object_data_->schedual_timer_.load() - object_data_->schedual_duration_).count() << "ms. \n";
				print_log_buffer();
				std::this_thread::sleep_for(object_data_->schedual_timer_.load() - object_data_->schedual_duration_); // sleep for remaining time until next output
			}
			if (object_data_->schedual_tpoint_ >= (object_data_->sync_tpoint_ + object_data_->running_duration_limit_)) { // timer shutdown event
				shutdown();
			}
		}
		/*Do stuff needed before returning shutdown response*/
		log_buffer << "Controller " << std::this_thread::get_id() << " has detected " << corruption_counter << " invalid entries\n";
		log_buffer << "from the " << verified_counter + corruption_counter << " entries that have been written (" << ((float)corruption_counter/(corruption_counter+verified_counter))*100.00f << "%).\n";
		print_log_buffer();

		/*--------------------------------------------------*/
		log_buffer << "Controller " << std::this_thread::get_id() << " is now shutdown after running " << object_data_->total_duration_.count() << "ms.\n";
		print_log_buffer();
		shutdown_response();
	}
};

Sensor sensorA;
Sensor sensorB;
Controller controller;

void wait_ready() {
	std::cout << "\nWaiting for sensors and controller to start.\n";
	bool ready = false;
	while (ready == false) {
		ready = true;
		if (sensorA.is_ready() == false || sensorB.is_ready() == false || controller.is_ready() == false) {
			ready = false;
			std::this_thread::sleep_for(timems_t(1));
		}
	}
}

void wait_active() {
	std::cout << "\nWaiting for sensors and controller to activate.\n";
	bool shutdown = false;
	while (shutdown == false) {
		shutdown = true;
		if (sensorA.is_active() == false || sensorB.is_active() == false || controller.is_active() == false) {
			shutdown = false;
			std::this_thread::sleep_for(timems_t(1));
		}
	}
}


void wait_shutdown() {
	std::cout << "\nWaiting for controller and sensors to shutdown.\n";
	bool shutdown = false;
	while (shutdown == false) {
		shutdown = true;
		if (sensorA.is_shutdown() == false || sensorB.is_shutdown() == false || controller.is_shutdown() == false) {
			shutdown = false;
			std::this_thread::sleep_for(timems_t(1));
		}
	}
}

bool dual_output() {
	std::regex reg_options_ex("^(([yY]{1}([eE][sS])?)|([nN]{1}[oO]?))$"); //detects y, yes, n, ignoring case sensistivity
	std::regex reg_options_answer_ex("^([yY]){1}([eE][sS])?$"); //detects yes for a boolean comparison of the answer
	std::string input;
	bool output_mode;
	std::cout << "Would you like to display additional log info? (y/n)";
	while (!std::regex_search(input, reg_options_ex)) {
		std::cin >> input;
		if (!std::regex_search(input, reg_options_ex)) {
			std::cout << "Invalid input. Please try again.";
		}
		else {
			output_mode = (std::regex_search(input, reg_options_answer_ex) ? true : false);
		}
	}
	return output_mode; //will always initialize
}

void initialize() {
	bool output_mode = dual_output();
	std::cout << "\nInitializing controller and sensors.\n";
	sensorA.initialize(output_mode, timems_t(50), timems_t(1000));
	sensorB.initialize(output_mode, timems_t(50), timems_t(1000));
	controller.initialize(output_mode, timems_t(125), timems_t(1000)); //all with the same time sync and end condition
}

void activate() {
	timepoint_t timeSync = time_now();
	sensorA.activate(timeSync);
	sensorB.activate(timeSync);
	controller.activate(timeSync + timems_t(1));
}

int main() {
	
	std::cout << "\nCreating Log.txt for additional program information.\n";
	std::ofstream log_file("log.txt"); //clear existing data, std::ios::trunc default
	log_file.close();
	log_file.open("log.txt", std::ios::out | std::ios::app); //all writes append
	std::streambuf* clog_buffer_backup = std::clog.rdbuf();
	std::clog.rdbuf(log_file.rdbuf());
	
	initialize();

	wait_ready();

	activate();

	wait_active();

	wait_shutdown();
	
	log_file.close();
	std::clog.rdbuf(clog_buffer_backup);
	std::system("pause");
	return 0;
}