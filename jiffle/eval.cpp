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
	memory result;

	// abstraction environment
	typedef std::map<value::symbol, value, symbolLess> env_map;
	env_map locals;
	env_map::iterator lookup;

	// skip comments
	while (length && code->type == value::TComment)
		shift(code,length);

	// primitives returned as is
	if (length) {
		switch (code->type) {
		case value::TReference:
		case value::TNull:
		case value::TBoolean:
		case value::TInteger:
		case value::TReal:
		case value::TString:
		case value::TError:
			*result.alloc() = *code;
			shift(code, length);
			return result;
		default:
			break;
		}
	}

	// symbol lookup
	if(length && code->type == value::TSymbol){
		lookup = locals.find(code->data.symbol);
		shift(code, length);
		if (lookup == locals.end()) {
			value val;
			val.type = value::TError;
			val.data.error.user = false;
			val.data.error.text = code->data.symbol.name;
			val.data.error.length = code->data.symbol.length;
			*result.alloc() = val;			
		}
		else {
			std::cout << "SYMBOL found!" << std::endl;
		}
		return result;
	}

	// symbol declaration
	if (length && code->type == value::TAbstraction) {
		lookup = locals.find(code->data.symbol);
		auto len = code->data.sequence.count;
		shift(code, length);
		if (lookup != locals.end()) {
			value val;
			val.type = value::TError;
			val.data.error.user = false;
			val.data.error.text = code->data.symbol.name;
			val.data.error.length = code->data.symbol.length;
			*result.alloc() = val;
		}
		else {
			// get symbol name
			value::symbol symb;
			for (size_t i = 0; i < len; i++) {
				// skip comments
				while (length && code->type == value::TComment)
					shift(code, length);

				// missing symbols
				if (!length || code->type != value::TSymbol) {
					value val;
					val.type = value::TError;
					val.data.error.user = false;
					val.data.error.text = code->data.symbol.name;
					val.data.error.length = code->data.symbol.length;
					*result.alloc() = val;
					return result;
				}

				// read symbol
				symb = code->data.symbol;
				// TODO: read symbol input params
				shift(code, length);
			}
			// skip symbol value
			eval(code, length);

			// return symbol reference
			value val;
			val.type = value::TReference;
			val.data.reference.length = symb.length;
			val.data.reference.name = symb.name;
			*result.alloc() = val;
		}
		return result;
	}

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

	// evaluation 
	switch (code->type) {
	case value::TEvaluation:
		shift(code, length);
		// TODO
		break;
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
