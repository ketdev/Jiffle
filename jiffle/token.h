#pragma once

#include <string>
#include <vector>

// TODO: rename file name to syntax
namespace syntax {

	// tokenize ---------------------------------------------------------------

	static const char Newline = '\n';
	bool isWhitespace(char token); // ' ' evaluation structure
	bool isLetter(char token); // [a-zA-Z_]
	bool isDigit(char token, int base = 10); // 2:(0-1), 8:(0-7), 10:(0-9), 16:(0-F) 
	
	// builtin syntax tokens
	enum particle : char {
		// Sequence
		SequenceDivHard = ',',	// value sequence divider within same line
		SequenceDivSoft = '\n',	// value sequence divider on multiple lines

		// Sub Sequence
		SequenceBegin = '(',	// value sequence starting token (each scope is an implicit sequence)
		SequenceEnd = ')',	// value sequence ending token

		// Abstraction
		Abstraction = '=',		// defines an abstraction declaration
		ParamBegin = '[',		// start of abstraction parameter
		ParamEnd = ']',			// end of abstraction parameter

		//Reference = '&',		// reference a symbol without evaluating


		// '\' escape newline separator
		// '{}' abstraction (sequence) (equivalent to '=()')
		// '<-' forked abstraction (for each in sequence, original order is maintained)
		// '|' variance (polymorphism)
		// '..' range (variance for numbers and characters) (sequence for list indices)
		// ':' specification
		// '.' composition
		// '->' flow mapping (if lhs is evaluated, then compute rhs)
		// '@' get index in sequence
		// '~' negation (variation inversion)
		// '$' reflexive (updateable state)
		// '%' abstract (extern)
	};

	// tokens are as equivalent to the source code, giving meaning
	// to different syntax constructs, for syntax highlighting and easier parsing
	struct token {
		enum type {
			Particle,	// builtin syntax characters, see above
			Comment,	// user comments and documentations
			Symbol,		// abstraction terms
			Constant,	// null, true, false, integers, reals, strings
			Error,		// user error type, or invalid tokens
		};
		struct comment {
			bool afterCode;
			const char* text;
			int length;
		};
		struct constant {
			enum type {
				Null,		// 'null'
				Boolean,	// 'true', 'false'
				Integer,	// integer: [0-9]+ | 0[xX][0-9A-F]+ | 0[oO][0-7]+ | 0[bB][0-1]+
				Real,		// real: [0-9]+'.'[0-9]+ | [0-9]+('.'[0-9]+])?e['-''+']?[0-9]+
				String,		// '...' (escaped and formated)
			};
			struct string {
				const char* text;
				int length;
			};
			union data {
				bool boolean;
				long long integer;
				long double real;
				constant::string string;
			};

			constant::type type;
			constant::data data;
		};
		struct symbol {
			const char* name;
			int length;
		};
		struct error {
			bool user; // if explicitly generated
			const char* text;
			int length;
		};
		union data {
			syntax::particle particle;
			token::comment comment;
			token::constant constant;
			token::symbol symbol;
			token::error error;
		};

		token::type type;
		token::data data;
	};

	typedef std::vector<token> tokens;
	tokens tokenize(const std::string& input);


}