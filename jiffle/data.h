#pragma once
#include <vector>


struct syntax {
	static const char Newline = '\n';

	enum particle : char {
		PComment				= '#',	// declares a comment from this token until newline
		PError					= '`',	// `...` (verbatim) declares an error, can also be produced from bad syntax
		PString					= '\'',	// '...' (escaped and formated)

		PSequenceBegin			= '(',	// value sequence starting token (each scope is an implicit sequence)
		PSequenceEnd			= ')',	// value sequence ending token
		PSequenceSeparatorSL	= ',',	// value sequence separator within same line
		PSequenceSeparatorML	= '\n',	// value sequence separator on multiple lines


		// '=' abstraction (value)
		// '{}' abstraction (sequence) (equivalent to '=()')
		// '<-' forked abstraction (for each in sequence, original order is maintained)
		// '|' variance (polymorphism)
		// '..' range (variance for numbers and characters) (sequence for list indices)
		// ':' specification
		// ' ' evaluation structure
		// '.' composition
		// '[]' dependency (input)
		// '->' flow mapping (if lhs is evaluated, then compute rhs)
		// '@' get index in sequence
		// '()' explicitly ordered evaluation
		// '\' escaped evaluation (pass the symbol without evaluating) (can be used to pass newline without separating)
		// '~' negation (variation inversion)
		// '$' reflexive (updateable state)
	};
};

// memory item meaning
struct value {
	enum type {
		TParticle,		// builtin tokens, see below (interpreted by eval)

		TComment,		// '#' until newline, user comments and documentations (ignored by eval) 
		TError,			// `...` (verbatim) errors are part of code, can also be produced from bad syntax
		TString,		// '...' (escaped and formated)
		TNull,			// 'null'
		TBoolean,		// 'true', 'false'
		TSymbol,		// symbol: [a-zA-Z_] [a-zA-Z0-9_]*
		TNumber,		// number: [0-9]+ | 0[xX][0-9A-F]+ | 0[oO][0-7]+ | 0[bB][0-1]+
		TReal,			// real: [0-9]+'.'[0-9]+ | [0-9]+('.'[0-9]+])?e['-''+']?[0-9]+
		
		TSequence,		// (1,2,3) multiple terms with separator between them


		TEvaluation,	// (1 + 3) terms without separator, to be evaluated

		TObject,		// symbol to value mapping
		TFunction,		// symbol to value mapping with input dependencies
	};
	enum particle : char {
		PSequenceBegin	= '(',	// value sequence starting token (each scope is an implicit sequence)
		PSequenceEnd	= ')',	// value sequence ending token
		PSeparatorSL	= ',',	// value sequence separator within same line
		PSeparatorML	= '\n',	// value sequence separator on multiple lines

		// '=' abstraction (value)
		// '{}' abstraction (sequence) (equivalent to '=()')
		// '<-' forked abstraction (for each in sequence, original order is maintained)
		// '|' variance (polymorphism)
		// '..' range (variance for numbers and characters) (sequence for list indices)
		// ':' specification
		// ' ' evaluation structure
		// '.' composition
		// '[]' dependency (input)
		// '->' flow mapping (if lhs is evaluated, then compute rhs)
		// '@' get index in sequence
		// '()' explicitly ordered evaluation
		// '\' escaped evaluation (pass the symbol without evaluating) (can be used to pass newline without separating)
		// '~' negation (variation inversion)
		// '$' reflexive (updateable state)
	};
	struct error {
		int index;
		bool user; // if explicitly generated
		const char* text;
		unsigned int length;
	};
	struct comment {
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
		// TODO: different bit sizes
	};
	struct real {
		long double value;
		// TODO: different bit sizes
	};
	union data {
		value::error error;
		value::particle particle;
		value::comment comment;
		value::string string;
		value::symbol symbol;
		value::boolean boolean;
		value::number number;
		value::real real;
		// ...
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