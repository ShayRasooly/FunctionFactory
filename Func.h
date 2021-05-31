#pragma once
#include "ThreadSafeClasses.h"
#include "Data.h"
#include <string>
#include <complex>

using namespace std;

// enums and structs
enum class Operation { // represents arithmetical operation
	add,
	sub,
	mul,
	divs,
	NUM_OPERATIONS
};

enum class OperandType { // operand types
	variable,
	constnat,
	NUM_OP_TYPES
};

struct Operand { // represnt opearnd within a function
	OperandType type;
	Data value;
};

class Func{//class Func : public Unit {
public:
	Operand operands[2]; // the 2 operands of the function
	Operation operation; // the arithmetical operation

	// return random func with 2 random operators, first randomize if constant, and only if constant randomize data
	static shared_ptr<Func> get_random(shared_ptr<ThreadSafeRand> rnd) {
		shared_ptr<Func> func(new Func());
		for (int i = 0; i < 2; i++) {
			func->operands[i].type = OperandType(rnd->rand_int() % int(OperandType::NUM_OP_TYPES));
			if (func->operands[i].type == OperandType::constnat) {
				shared_ptr<Data> dat = Data::get_random(rnd);
				func->operands[i].value = *dat;
			}
		}
		func->operation = Operation(rnd->rand_int() % int(Operation::NUM_OPERATIONS));
		return func;
	}

	// solve the function. throws DivByZero if the function divide by zero
	Data get_result() {
		Data res;
		res.type = max(operands[0].value.type, operands[1].value.type);
		switch (operation) {
		case Operation::add:
			res.num = operands[0].value.num + operands[1].value.num;
			break;
		case Operation::sub:
			res.num = operands[0].value.num - operands[1].value.num;
			break;
		case Operation::mul:
			res.num = operands[0].value.num * operands[1].value.num;
			break;
		case Operation::divs:
			res.num = operands[0].value.num / operands[1].value.num;
		}
		return res;
	}

	// solve the function and string it.
	string to_string() {
		bool div_by_zero = ((operation == Operation::divs) && (operands[1].value.num == complex(0.0)));
		bool is_first_var;
		string func_str = "Function: {";
		string param_str = "; parameters: ";
		string res_str = "; result: ";

		if (operands[0].type == OperandType::variable) {
			func_str += "x";
			param_str += operands[0].value.to_string();
			is_first_var = true;
		}
		else {
			func_str += operands[0].value.to_string();
			is_first_var = false;
		}

		switch (operation) {
		case Operation::add:
			func_str += " + ";
			break;
		case Operation::sub:
			func_str += " - ";
			break;
		case Operation::mul:
			func_str += " * ";
			break;
		case Operation::divs:
			func_str += " / ";
		}

		if (operands[1].type == OperandType::variable) {
			if (is_first_var) {
				func_str += "y}";
				param_str += ", ";
			}
			else {
				func_str += "x}";
			}
			param_str += operands[1].value.to_string();
		}
		else {
			func_str += operands[1].value.to_string() + "}";
		}

		if (div_by_zero) {
			res_str += "NO RESULT: DIVISION BY ZERO";
		}
		else {
			res_str += get_result().to_string();
		}

		return func_str + param_str + res_str;
	}
};

