#pragma once
#include "ThreadSafeClasses.h"
#include <complex>
#include <string>

enum class DataType { // all the valid types for data units
	integer,
	floating,
	complex_num,
	NUM_DATA_TYPES
};

class Data {
public:
	DataType type; // type of data
	complex<double> num; // the data itself. the most inclusive type to represent all types is complex<double>

	// return random data with random type and random number according to type
	static shared_ptr<Data> get_random(shared_ptr<ThreadSafeRand> rnd) {
		shared_ptr<Data> dat(new Data());
		dat->type = DataType(rnd->rand_int() % int(DataType::NUM_DATA_TYPES));
		if (dat->type == DataType::integer) {
			dat->num = double(rnd->rand_int());
		}
		else if (dat->type == DataType::floating) {
			dat->num = rnd->rand_double();
		}
		else if (dat->type == DataType::complex_num) {
			dat->num = complex(double(rnd->rand_int()), double(rnd->rand_int()));
		}
		return dat;
	}

	// checks the type and string the data accordignly
	string to_string() {
		switch (type) {
		case DataType::integer:
			return std::to_string(int(num.real()));
		case DataType::floating:
			return std::to_string(num.real());
		}

		return std::to_string(int(num.real())) + " + " + std::to_string(int(num.imag())) + "i";
	} 
};