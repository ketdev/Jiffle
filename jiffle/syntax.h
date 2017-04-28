#pragma once

#include "data.h"

namespace syntax {

	static const char Newline = '\n';

	// builtin tokens (interpreted by parse)
	enum particle : char {
		PComment				= '#',	// declares a comment from this token until newline
		PString					= '\'',	// '...' (escaped and formated)
		PParamBegin				= '[',	// start of abstraction parameter
		PParamEnd				= ']',	// end of abstraction parameter
		PReference				= '&',  // reference a symbol without evaluating
		PSequenceBegin			= '(',	// value sequence starting token (each scope is an implicit sequence)
		PSequenceEnd			= ')',	// value sequence ending token
		PSequenceDivSL			= ',',	// value sequence divider within same line
		PSequenceDivML			= '\n',	// value sequence divider on multiple lines
		
		PAbstraction			= '=',	// (value) transforms evaluation sequence to abstraction declaration

		PError					= '`',	// `...` (verbatim) declares an error, can also be produced from bad syntax
		
		// '\' escape newline separator
		// '{}' abstraction (sequence) (equivalent to '=()')
		// '<-' forked abstraction (for each in sequence, original order is maintained)
		// '|' variance (polymorphism)
		// '..' range (variance for numbers and characters) (sequence for list indices)
		// ':' specification
		// '.' composition
		// '[]' dependency (input)
		// '->' flow mapping (if lhs is evaluated, then compute rhs)
		// '@' get index in sequence
		// '()' explicitly ordered evaluation
		// '~' negation (variation inversion)
		// '$' reflexive (updateable state)
		// '_'  non-stored abstraction target, for sinking tupled abstraction assignments
	};

	bool isWhitespace(char token); // ' ' evaluation structure
	bool isLetter(char token); // [a-zA-Z_]
	bool isDigit(char token, int base=10); // 2:(0-1), 8:(0-7), 10:(0-9), 16:(0-F) 

	value parseError(const char*& src, size_t& length);
	value parseWord(const char*& src, size_t& length); // symbol, null or boolean
	value parseNumber(const char*& src, size_t& length); // integer, real
	value parseString(const char*& src, size_t& length);

	memory parse(const char*& src, size_t& length);

	std::string assemble(const memory& code);
}