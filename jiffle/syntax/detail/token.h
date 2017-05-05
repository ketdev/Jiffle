#pragma once

#include <string>
#include <vector>

#include "pos.h"

namespace syntax {
	namespace detail {

		struct token {
		public:
			constexpr static char Newline = '\n';

			constexpr static char* KeywordNull = "null";
			constexpr static char* KeywordTrue = "true";
			constexpr static char* KeywordFalse = "false";

			constexpr static char TokenComment = '#';
			constexpr static char TokenString = '\'';
			constexpr static char TokenError = '`';
			constexpr static char TokenSequenceSeparator = ',';
			constexpr static char TokenSequenceSeparatorImplicit = Newline;
			constexpr static char TokenDefinition = '=';
			constexpr static char TokenTupleStart = '(';
			constexpr static char TokenTupleEnd = ')';
			constexpr static char TokenDefinitionSequenceStart = '{';	// definition + sequence begin '=('
			constexpr static char TokenDefinitionSequenceEnd = '}';		// definition + sequence end ')'

		public:

			// semantic meaning
			enum type {
				// Primitives
				SyntaxError,
				UserError,
				Comment,
				Null,
				True,
				False,
				Symbol,
				Integer,
				Real,
				String,

				// Particles
				SequenceSeparator,			// ','
				SequenceSeparatorImplicit,	// '\n'
				Definition,					// '='
				TupleStart,					// '('
				TupleEnd,					// ')'
				DefinitionSequenceStart,	// '{'
				DefinitionSequenceEnd,		// '}'
			};

		//// Parameter
		//ParameterBegin = '[',	// start of argument group for definitions
		//ParameterEnd = ']',		// end of arguments group for definitions
		//
		//
		////Reference = '&',		// reference a symbol without evaluating
		//
		//// '\' escape newline separator
		//// '<-' forked abstraction (for each in sequence, original order is maintained)
		//// '|' variance (polymorphism)
		//// '..' range (variance for numbers and characters) (sequence for list indices)
		//// ':' specification
		//// '.' composition ('.foo' makes it private to scope)
		//// '->' flow mapping (if lhs is evaluated, then compute rhs)
		//// '@' get index in sequence
		//// '~' negation (variation inversion)
		//// '$' reflexive (updateable state)
		//// '%' abstract (extern)

		public:
			token::type type;
			detail::pos pos;
			union {
				long long integer;
				long double real;
			} number;
		};

		std::vector<token> tokenize(const std::string& code);

	}
}