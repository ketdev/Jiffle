#include <iostream>
#include <stack>
#include <map>
#include "eval.h"


struct symbolLess {
	typedef value::symbol type;
	bool operator()(const type& l, const type& r) const {
		if (l.length != r.length)
			return l.length < r.length;
		return std::memcmp(l.name,r.name,l.length) < 0;
	}
};


static const inline void shift(const value*& code, size_t & length) {
	length--;
	code++;
}

memory eval(const value*& code, size_t& length) {
	typedef std::map<value::symbol, value, symbolLess> env_map;

	memory result;

	// abstraction environment
	env_map locals;
	env_map::iterator lookup;

	// skip comments
	while (length && code->type == value::TComment)
		shift(code,length);

	// primitives returned as is
	if (length) {
		switch (code->type) {
		case value::TNull:
		case value::TBoolean:
		case value::TInteger:
		case value::TReal:
		case value::TString:
		case value::TReference:
		case value::TSymbol:
		case value::TError:
			*result.alloc() = *code;
			shift(code, length);
			return result;
		default:
			break;
		}
	}

	// SEQUENCE TODO: iterate full sequence to get abstractions first (since order independent)
	// ABSTRACTION TODO: store abstractions in symbol table, with different scopes
	// EVALUATION TODO: lookup symbols in evaluation & call

	// first iteration: 
	//	- register abstraction definitions
	//	- build evaluation dependencies
	// second iteration:
	//  - resolve dependency tree, from leafs to root

	// sequence evaluates each item recursively
	if (length && code->type == value::TSequence) {
		// copy sequence prefix
		*result.alloc() = *code;
		auto len = code->data.sequence.count;
		shift(code,length);
		// copy items
		for (size_t i = 0; i < len; i++) {
			auto item = eval(code, length);
			for each (auto v in item.values) {
				*result.alloc() = v;
			}
		}
		return result;
	}
	
	// error value
	shift(code, length);
	value val;
	val.type = value::TError;
	val.data.error.user = false;
	val.data.error.text = nullptr;
	val.data.error.length = 0;
	*result.alloc() = val;
	return result;
}
