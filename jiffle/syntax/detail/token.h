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
				SequenceSeparator, // ','
				SequenceSeparatorImplicit, // '\n'
			};

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