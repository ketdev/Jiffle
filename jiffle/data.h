#pragma once
#include <vector>



// memory item meaning
struct value {
	enum type {
		TComment,		// '#' until newline, user comments and documentations (ignored by eval) 
		TSymbol,		// symbol: [a-zA-Z_] [a-zA-Z0-9_]* except: null,true,false
		TNull,			// 'null'
		TBoolean,		// 'true', 'false'
		TInteger,		// integer: [0-9]+ | 0[xX][0-9A-F]+ | 0[oO][0-7]+ | 0[bB][0-1]+
		TReal,			// real: [0-9]+'.'[0-9]+ | [0-9]+('.'[0-9]+])?e['-''+']?[0-9]+
		TString,		// '...' (escaped and formated)
		TEvaluation,	// (1 + 2) prefix for terms without separator, to be evaluated (does not count comments)
		TSequence,		// (1,2,3) prefix for multiple terms with separator between them (does not count comments)

		TError,			// `...` (verbatim) errors are part of code, can also be produced from bad syntax
		
		//TObject,		// symbol to value mapping
		//TFunction,		// symbol to value mapping with input dependencies
	};

	struct comment {
		bool afterCode;
		const char* text;
		unsigned int length;
	};
	struct string {
		const char* text;
		unsigned int length;
	};
	struct symbol {
		const char* name;
		unsigned int length;
	};
	struct boolean {
		bool value;
	};
	struct number {
		long long value;
	};
	struct real {
		long double value;
	};
	struct evaluation {
		unsigned int count;
	};
	struct sequence {
		unsigned int count;
	};
	struct error {
		bool user; // if explicitly generated
		const char* text;
		unsigned int length;
	};
	union data {
		value::comment comment;
		value::string string;
		value::symbol symbol;
		value::boolean boolean;
		value::number number;
		value::real real;
		value::evaluation evaluation;
		value::sequence sequence;
		// ...
		value::error error;
	};

	value::type type;
	value::data data;
};


// memory values container
struct memory {
	std::vector<value> values;

	// note: old pointers might be invalidated on next alloc
	value* alloc() {
		auto i = values.size();
		values.push_back(value());
		return &values[i];
	}
};