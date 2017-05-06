#pragma once

#include "syntax.h"

#include <list>

namespace jiffle {
	namespace expr {

		// expressions --------------------------------------------------------

		enum type : unsigned char {
			// special
			ERROR_BIT		= 0x40,
			STRUCTURE_BIT	= 0x80,

			// values
			Null			= 1,
			Bool			= 2,
			Integer			= 3,
			Real			= 4,
			String			= 5,

			// error values
			Error			= ERROR_BIT | 1,
			SyntaxError		= ERROR_BIT | 2,

			// structures
			Evaluation			= STRUCTURE_BIT | 1,
			Object				= STRUCTURE_BIT | 2,
			Definition			= STRUCTURE_BIT | 3, // '='
			Sequence			= STRUCTURE_BIT | 4, // '()' default
			DefinitionSequence	= STRUCTURE_BIT | 5, // '{}'
			Parameter			= STRUCTURE_BIT | 6, // '[]'
			Module				= STRUCTURE_BIT | 7, // topmost
		};

		enum flags : unsigned char {
			Default				= (0 << 0),
			ExplicitStructure	= (1 << 0),
		};

		// structures ---------------------------------------------------------

		struct node {
			type type;
			unsigned char flags;
			syntax::pos pos;
			std::string text;			// for string, error and objects
			std::list<node> items;		// for structures
			union value_union {
				bool boolean;
				long long integer;
				long double real;
			} value;
		};

		// functions ----------------------------------------------------------

		// converts tokens to an expression tree
		node parse(const std::vector<syntax::token>& tokens, const std::string& code);

		// tests --------------------------------------------------------------

		void parse_test();

	}
}