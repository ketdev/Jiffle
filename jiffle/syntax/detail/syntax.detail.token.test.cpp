#include "token.h"
#include <assert.h>

namespace syntax {
	namespace detail {

		static void single_token_test(const char* input, enum token::type type) {
			auto p = tokenize(input);
			assert(p.size() == 1);
			auto c = p[0];
			assert(c.type == type);
			assert(c.pos.ch == 0);
			assert(c.pos.len == strlen(input));
			assert(c.pos.ln == 0);
			assert(c.pos.col == 0);
		}

		void tokenize_test() {
			{ // primitives
				assert(tokenize("").empty()); // empty
				assert(tokenize("\v\f\r\t ").empty()); // whitespace
				single_token_test("# hello comment", token::Comment);
				single_token_test("`", token::SyntaxError);
				single_token_test("hello", token::Symbol);
				single_token_test(token::KeywordNull, token::Null);
				single_token_test(token::KeywordTrue, token::True);
				single_token_test(token::KeywordFalse, token::False);
				single_token_test("0x", token::SyntaxError);
				single_token_test("0", token::Integer);
				single_token_test("999999999999", token::Integer);
				single_token_test("0b111", token::Integer); // 7 binary
				single_token_test("0o111", token::Integer); // 73 octal
				single_token_test("0x111", token::Integer); // 273 hexadecimal
				single_token_test("3.14", token::Real);
				single_token_test("6.02e23", token::Real);
				single_token_test("0.0e-1", token::Real);
				single_token_test("'hello!\n'", token::String);
				single_token_test("`invalid`", token::UserError);
				
				// combined test
				auto t = tokenize(" foo  0xF 0.0e-1 \t  true  '' `` #");
				assert(t.size() == 7);
				assert(t[0].type == token::Symbol);
				assert(t[1].type == token::Integer);
				assert(t[2].type == token::Real);
				assert(t[3].type == token::True);
				assert(t[4].type == token::String);
				assert(t[5].type == token::UserError);
				assert(t[6].type == token::Comment);
			}
			{ // particles
				single_token_test(
					std::string(1,token::TokenSequenceSeparator).c_str(),
					token::SequenceSeparator);
				single_token_test(
					std::string(1, token::TokenSequenceSeparatorImplicit).c_str(),
					token::SequenceSeparatorImplicit);
				single_token_test(
					std::string(1, token::TokenDefinition).c_str(),
					token::Definition);
				single_token_test(
					std::string(1, token::TokenTupleStart).c_str(),
					token::TupleStart);
				single_token_test(
					std::string(1, token::TokenTupleEnd).c_str(),
					token::TupleEnd);
				single_token_test(
					std::string(1, token::TokenDefinitionSequenceStart).c_str(),
					token::DefinitionSequenceStart);
				single_token_test(
					std::string(1, token::TokenDefinitionSequenceEnd).c_str(),
					token::DefinitionSequenceEnd);
			}
		}

	}
}