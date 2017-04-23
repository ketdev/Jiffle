#include <iostream>
#include <stack>
#include "eval.h"

memory eval(const value*& code, size_t& length) {
	memory result;

	bool isSequence = false;
		
	// Interpret loop
	for (; length > 0; length--, code++) {
		switch (code->type) {
		case value::type::TComment:
			// ignore
			break;
		case value::type::TError:
		case value::type::TString:
		case value::type::TNull:
		case value::type::TBoolean:
		case value::type::TInteger:
		case value::type::TReal:
			*result.alloc() = *code;
			break;

		case value::type::TSymbol:
			*result.alloc() = *code;
			// TODO: resolve symbol and invoke evaluation
			break;

		default:
			std::cerr << "Unknown value type: " << code->type << std::endl;
			break;
		}
	}

	return result;
}
