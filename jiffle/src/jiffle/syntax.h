#pragma once

#include <vector>
#include <string>

namespace jiffle {
	namespace syntax {

		// keywords -----------------------------------------------------------

		constexpr static char Newline = '\n';

		constexpr static char* KeywordNull = "null";
		constexpr static char* KeywordTrue = "true";
		constexpr static char* KeywordFalse = "false";

		//constexpr static char* KeywordFork = "<-";
		//constexpr static char* KeywordRange = "..";
		//constexpr static char* KeywordFlow = "->";

		// tokens -------------------------------------------------------------

		// semantic meaning
		enum type : char {
			// primitives
			Null,
			True,
			False,
			Symbol,
			Integer,
			Real,
			SyntaxError,
			String = '\'',
			Comment = '#',
			UserError = '`',

			// particles
			Separator = ',',				// explicit sequence separator
			SeparatorImplicit = '\n',		// implicit sequence separator
			SequenceStart = '(',			// start of subsequence 
			SequenceEnd = ')',				// end of subsequence
			Definition = '=',				// definition evaluation, can only be used after object
			DefinitionStart = '{',			// start of definition sequence, almost equivalent to '=('
			DefinitionEnd = '}',			// end of definition sequence, also ends evaluation
			ParameterStart = '[',			// start of argument group for definitions
			ParameterEnd = ']',				// end of arguments group for definitions

			// TODO: parse
			//Composition = '.',					// composition ('.foo' makes it private to scope)
			//Specification = ':',				// specification
			//Variance = '|',						// variance (polymorphism)
			//Reference = '&',					// reference a symbol without evaluating
			//Statefull = '$',					// updateable state
			//Escape = '\\',						// escape newline separator
			//Index = '@',						// get index in sequence
			//Fork = 7,							// '<-' forked abstraction (for each in sequence, original order is maintained)
			//Range = 8,							// '..' range (variance for numbers and characters) (sequence for list indices)
			//Flow = 9,							// '->' flow mapping (if lhs is evaluated, then compute rhs)
			//Negation = '~',						// negation (variation inversion)
			//Extern = '%',						// abstract (extern)
		};

		// structures ---------------------------------------------------------
		
		// expression position
		struct pos {
			size_t ch;
			size_t len;
			size_t ln;
			size_t col;
		};
		
		// combined token type and data output
		struct token {
			type type;
			pos pos;
		};

		// functions ----------------------------------------------------------

		// convert an input string to tokens
		std::vector<token> tokenize(const std::string & code);
		
		// tests --------------------------------------------------------------

		void tokenize_test();

	}
}