#include "syntax.h"
#include <assert.h>

namespace jiffle {
	namespace syntax {

		void tokenize_test() {
			// internal state -------------------------------------------------
			std::vector<token> _tdata;
			size_t _index;

			// methods --------------------------------------------------------
			auto set = [&](const char* input) {
				_tdata = tokenize(input);
				_index = 0;
			};
			auto assert_token = [&](type t, pos p) {
				auto tok = _tdata[_index].type;
				auto pos = _tdata[_index].pos;
				_index++;
				assert(tok == t);
				assert(pos.ch == p.ch && pos.col == p.col && pos.len == p.len && pos.ln == p.ln);
			};
			auto assert_integer = [&](long long i, pos p) {
				assert_token(type::Integer, p);
				auto integer = _tdata[_index-1].value.integer;
				assert(integer == i);
			};
			auto assert_real = [&](long double r, pos p) {
				assert_token(type::Real, p);
				auto real = _tdata[_index-1].value.real;
				assert(real == r);
			};
			auto assert_end = [&]() {
				assert(_index >= _tdata.size());
			};

			// tests ----------------------------------------------------------

			// empty
			set(""); assert_end();
			// whitespace
			set("\v\f\r\t "); assert_end();
			// particles
			set(",\n()={}[]"); 
			assert_token(type::Separator,					{ 0,1,0,0 });
			assert_token(type::SeparatorImplicit,			{ 1,1,0,1 });
			assert_token(type::SequenceStart,				{ 2,1,1,0 });
			assert_token(type::SequenceEnd,					{ 3,1,1,1 });
			assert_token(type::Definition,					{ 4,1,1,2 });
			assert_token(type::DefinitionStart,				{ 5,1,1,3 });
			assert_token(type::DefinitionEnd,				{ 6,1,1,4 });
			assert_token(type::ParameterStart,				{ 7,1,1,5 });
			assert_token(type::ParameterEnd,				{ 8,1,1,6 });
			assert_end();
			// comment
			set("# hello comment"); 
			assert_token(type::Comment,						{ 0,15,0,0 });
			assert_end();
			// identifiers
			set("null true false foo");
			assert_token(type::Null,						{ 0,4,0,0 });
			assert_token(type::True,						{ 5,4,0,5 });
			assert_token(type::False,						{ 10,5,0,10 });
			assert_token(type::Symbol,						{ 16,3,0,16 });
			assert_end();
			// numbers
			set("0x 0 999999999999 0XF 0b111 0o111 0x111 3.14 6.02e-23 0.0e-1");
			assert_token(type::SyntaxError,					{ 0,2,0,0 });
			assert_integer(0,								{ 3,1,0,3 });
			assert_integer(999999999999,					{ 5,12,0,5 });
			assert_integer(15,								{ 18,3,0,18 });
			assert_integer(7,								{ 22,5,0,22 });
			assert_integer(73,								{ 28,5,0,28 });
			assert_integer(273,								{ 34,5,0,34 });
			assert_real(3.14,								{ 40,4,0,40 });
			assert_real(6.02e-23,							{ 45,8,0,45 });
			assert_real(0.0e-1,								{ 54,6,0,54 });
			assert_end();
			// string
			set("'\v\f\r\t ,=(){}'");
			assert_token(type::String,						{ 0,13,0,0 });
			assert_end();
			// user error
			set("`\v\f\r\t ,=(){}`");
			assert_token(type::UserError,					{ 0,13,0,0 });
			assert_end();
		}
		
	}
}