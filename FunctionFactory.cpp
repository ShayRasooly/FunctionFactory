#include "Data.h"
#include "Func.h"
#include "ThreadSafeClasses.h"

using namespace std;

const int  MAX_QUEUE = 100;

// constantly pushes new random data to the queue
void thread_generate_data(shared_ptr<ThreadSafeQueue<shared_ptr<Data>>> data_queue,
	                      shared_ptr<ThreadSafeRand> rnd) {
	while (true) {
		data_queue->push(Data::get_random(rnd));
	}
}

// constantly pushes new random funcs to the queue
void thread_generate_func(shared_ptr<ThreadSafeQueue<shared_ptr<Func>>> func_queue,
	                      shared_ptr<ThreadSafeRand> rnd) {
	while(true)
	{
		func_queue->push(Func::get_random(rnd));
	}
}

 // insert random values to the varibles, print the function and increase counter
void process_single_func(shared_ptr<Func> func,
	                     shared_ptr<ThreadSafeQueue<shared_ptr<Data>>> data_q,
	                     shared_ptr<ThreadSafePrinter> printer,
	                     shared_ptr<ThreadSafeCounter> func_counter) {
	for (int i = 0; i < 2; i++)
	{
		if (func->operands[i].type == OperandType::variable) {
			func->operands[i].value = *(data_q->pop());
		}
	}

	printer->print(func->to_string());
	func_counter->increase();
}


// 
void thread_process(
	shared_ptr<ThreadSafeRand> rnd,
	shared_ptr<ThreadSafeCounter> func_counter,
	shared_ptr<ThreadSafePrinter> printer,
	shared_ptr<vector<shared_ptr<ThreadSafeQueue<shared_ptr<Data>>>>> data_queues_vec,
	shared_ptr<vector<shared_ptr<ThreadSafeQueue<shared_ptr<Func>>>>> func_queues_vec) 
{	
	bool is_first_data, is_second_data;
	int num_of_data_queues = data_queues_vec->size();
	int num_of_func_queues = func_queues_vec->size();
	int total_num_of_queues = num_of_data_queues + num_of_func_queues;
	int first_queue_id, second_queue_id;
	
	while (true) {
		first_queue_id = (rnd->rand_int() % total_num_of_queues);
		do {
			second_queue_id = (rnd->rand_int() % total_num_of_queues);
		} while (second_queue_id == first_queue_id);

		is_first_data = first_queue_id < num_of_data_queues;
		is_second_data = second_queue_id < num_of_data_queues;
		if (!is_first_data) {
			first_queue_id -= num_of_data_queues;
		}
		if (!is_second_data) {
			second_queue_id -= num_of_data_queues;
		}

		if (is_first_data && is_second_data) {
			shared_ptr<ThreadSafeQueue<shared_ptr<Data>>> first_q = (*data_queues_vec)[first_queue_id];
			shared_ptr<ThreadSafeQueue<shared_ptr<Data>>> second_q = (*data_queues_vec)[second_queue_id];
			first_q->swap_heads(second_q);
		}
		else if(is_first_data || is_second_data){
			shared_ptr<Func> func;
			shared_ptr<ThreadSafeQueue<shared_ptr<Data>>> data_q;

			if(is_first_data) {
				func = (*func_queues_vec)[second_queue_id]->pop();
				data_q = (*data_queues_vec)[first_queue_id];
			}
			else {
				func = (*func_queues_vec)[first_queue_id]->pop();
				data_q = (*data_queues_vec)[second_queue_id];
			}

			process_single_func(func, data_q, printer, func_counter);
		}
	}
}

class arguments {
public:
	int num_of_data_threads,
		num_of_func_threads,
		num_of_proc_threads,
		total_num_of_threads,
		num_of_funcs_to_print;
	arguments(int num_of_data_threads,
		      int num_of_func_threads,
		      int num_of_proc_threads,
		      int total_num_of_threads,
		      int num_of_funcs_to_print)
	{
		this->num_of_data_threads = num_of_data_threads;
		this->num_of_func_threads = num_of_func_threads;
		this->num_of_proc_threads = num_of_proc_threads;
		this->total_num_of_threads = total_num_of_threads;
		this->num_of_funcs_to_print = num_of_funcs_to_print;
	}
};

shared_ptr<arguments> set_args(int argc, char* argv[]) {
	int num_of_data_threads = 0,
		num_of_func_threads = 0,
		num_of_proc_threads = 0,
		total_num_of_threads = 0,
		num_of_funcs_to_print = 0;
	
	for (int i = 1; i < argc; i += 2) {
		string arg_name = argv[i];
		int arg_val = stoi(argv[i + 1]);
		if (arg_name == "ND") {
			num_of_data_threads = arg_val;
		}

		else if (arg_name == "NF") {
			num_of_func_threads = arg_val;
		}

		else if (arg_name == "NP") {
			num_of_proc_threads = arg_val;
		}

		else if (arg_name == "NA") {
			num_of_funcs_to_print = arg_val;
		}
	}

	total_num_of_threads = num_of_data_threads + num_of_func_threads + num_of_proc_threads;

	return shared_ptr<arguments>(new arguments(
		num_of_data_threads,
		num_of_func_threads,
		num_of_proc_threads,
		total_num_of_threads,
		num_of_funcs_to_print));
}

void create_queues(
	shared_ptr<arguments> args,
	shared_ptr<vector<shared_ptr<ThreadSafeQueue<shared_ptr<Data>>>>> data_queues_vec,
	shared_ptr<vector<shared_ptr<ThreadSafeQueue<shared_ptr<Func>>>>> func_queues_vec)
{
	for (int i = 0; i < args->num_of_data_threads; ++i)
	{
		(*data_queues_vec)[i] = shared_ptr<ThreadSafeQueue<shared_ptr<Data>>>(new ThreadSafeQueue<shared_ptr<Data>>(i, MAX_QUEUE));
	}

	for (int i = 0; i < args->num_of_func_threads; ++i)
	{
		(*func_queues_vec)[i] = shared_ptr<ThreadSafeQueue<shared_ptr<Func>>>(new ThreadSafeQueue<shared_ptr<Func>>(i + args->num_of_data_threads, MAX_QUEUE));
	}
}

shared_ptr<vector<shared_ptr<thread>>> create_threads(
	shared_ptr<arguments> args,
	shared_ptr<ThreadSafeRand> rnd,
	shared_ptr<ThreadSafeCounter> func_counter,
    shared_ptr<ThreadSafePrinter> printer,
	shared_ptr<vector<shared_ptr<ThreadSafeQueue<shared_ptr<Data>>>>> data_queues_vec,
	shared_ptr<vector<shared_ptr<ThreadSafeQueue<shared_ptr<Func>>>>> func_queues_vec)
{
	shared_ptr<vector<shared_ptr<thread>>> threads(new vector<shared_ptr<thread>>(args->total_num_of_threads));

	for (int i = 0; i < args->num_of_data_threads; ++i)
	{
		(*threads)[i] = shared_ptr<thread>(new thread(thread_generate_data, (*data_queues_vec)[i], rnd));
	}

	for (int i = 0; i < args->num_of_func_threads; ++i)
	{
		(*threads)[i + args->num_of_data_threads] = shared_ptr<thread>(new thread(thread_generate_func, (*func_queues_vec)[i], rnd));
	}

	for (int i = 0; i < args->num_of_proc_threads; ++i)
	{
		(*threads)[i + args->num_of_data_threads + args->num_of_func_threads] =
			shared_ptr<thread>(new thread(thread_process, rnd, func_counter, printer, data_queues_vec, func_queues_vec));
	}

	return threads;
}

int main(int argc, char* argv[]) {
	shared_ptr<arguments> args = set_args(argc, argv);
	shared_ptr<ThreadSafeRand> rnd(new ThreadSafeRand());
	shared_ptr<ThreadSafeCounter> func_counter(new ThreadSafeCounter());
	shared_ptr<ThreadSafePrinter> printer(new ThreadSafePrinter());
	shared_ptr<vector<shared_ptr<ThreadSafeQueue<shared_ptr<Data>>>>>
		data_queues_vec(new vector<shared_ptr<ThreadSafeQueue<shared_ptr<Data>>>>(args->num_of_data_threads));
	shared_ptr<vector<shared_ptr<ThreadSafeQueue<shared_ptr<Func>>>>>
		func_queues_vec(new vector<shared_ptr<ThreadSafeQueue<shared_ptr<Func>>>>(args->num_of_func_threads));

	create_queues(args, data_queues_vec, func_queues_vec);
	shared_ptr<vector<shared_ptr<thread>>> threads = create_threads(args, rnd, func_counter, printer, data_queues_vec, func_queues_vec);
	func_counter->wait_until_equales(args->num_of_funcs_to_print);
	for (shared_ptr <thread> t : *threads) {
		//pthread_cancel(t->native_handle());
		//t->~thread();
		t->detach();
	}
	return 0;
}