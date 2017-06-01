#include "jiffle.h"
#include <assert.h>

namespace jiffle {

	void tokenize_test() {

		// internal state -----------------------------------------------------
		std::vector<data> _tdata;
		size_t _index;

		// methods ------------------------------------------------------------
		auto set = [&](const char* input) {
			_tdata = tokenize(input);
			_index = 0;
		};
		auto assert_primitive = [&](primitive primitive, pos p) {
			assert(_tdata[_index].category == Primitive);
			assert(_tdata[_index].primitive == primitive);
			auto pos = _tdata[_index].pos;
			assert(pos.ch == p.ch && pos.col == p.col && pos.len == p.len && pos.ln == p.ln);
			_index++;
		};
		auto assert_error = [&](error error, pos p) {
			assert(_tdata[_index].category == Error);
			assert(_tdata[_index].error == error);
			auto pos = _tdata[_index].pos;
			assert(pos.ch == p.ch && pos.col == p.col && pos.len == p.len && pos.ln == p.ln);
			_index++;
		};
		auto assert_particle = [&](particle particle, pos p) {
			assert(_tdata[_index].category == Particle);
			assert(_tdata[_index].particle == particle);
			auto pos = _tdata[_index].pos;
			assert(pos.ch == p.ch && pos.col == p.col && pos.len == p.len && pos.ln == p.ln);
			_index++;
		};
		auto assert_end = [&]() {
			assert(_index >= _tdata.size());
		};

		// tests --------------------------------------------------------------

		// empty
		set(""); assert_end();
		// whitespace
		set("\v\f\r\t ");
		assert_particle(Whitespace,				{ 0,5,0,0 });
		assert_end();
		// structure particles
		set(",\n()={}[]"); 
		assert_particle(Separator,				{ 0,1,0,0 });
		assert_particle(SeparatorImplicit,		{ 1,1,0,1 });
		assert_particle(SequenceStart,			{ 2,1,1,0 });
		assert_particle(SequenceEnd,			{ 3,1,1,1 });
		assert_particle(DefinitionAssign,		{ 4,1,1,2 });
		assert_particle(DefinitionStart,		{ 5,1,1,3 });
		assert_particle(DefinitionEnd,			{ 6,1,1,4 });
		assert_particle(ParameterStart,			{ 7,1,1,5 });
		assert_particle(ParameterEnd,			{ 8,1,1,6 });
		assert_end();
		// comment
		set("# hello comment"); 
		assert_primitive(Comment,				{ 0,15,0,0 });
		assert_end();
		// identifiers
		set("null true false foo");
		assert_primitive(Null,					{ 0,4,0,0 });
		assert_particle(Whitespace,				{ 4,1,0,4 });
		assert_primitive(True,					{ 5,4,0,5 });
		assert_particle(Whitespace,				{ 9,1,0,9 });
		assert_primitive(False,					{ 10,5,0,10 });
		assert_particle(Whitespace,				{ 15,1,0,15 });
		assert_primitive(Symbol,				{ 16,3,0,16 });
		assert_end();
		// numbers
		set("0x 0 999999999999 0XF 0b111 0o111 0x111 3.14 6.02e-23 0.0e-1");
		assert_error(SyntaxError,				{ 0,2,0,0 });
		assert_particle(Whitespace,				{ 2,1,0,2 });
		assert_primitive(Number,				{ 3,1,0,3 });
		assert_particle(Whitespace,				{ 4,1,0,4 });
		assert_primitive(Number,				{ 5,12,0,5 });
		assert_particle(Whitespace,				{ 17,1,0,17 });
		assert_primitive(Number,				{ 18,3,0,18 });
		assert_particle(Whitespace,				{ 21,1,0,21 });
		assert_primitive(Number,				{ 22,5,0,22 });
		assert_particle(Whitespace,				{ 27,1,0,27 });
		assert_primitive(Number,				{ 28,5,0,28 });
		assert_particle(Whitespace,				{ 33,1,0,33 });
		assert_primitive(Number,				{ 34,5,0,34 });
		assert_particle(Whitespace,				{ 39,1,0,39 });
		assert_primitive(Number,				{ 40,4,0,40 });
		assert_particle(Whitespace,				{ 44,1,0,44 });
		assert_primitive(Number,				{ 45,8,0,45 });
		assert_particle(Whitespace,				{ 53,1,0,53 });
		assert_primitive(Number,				{ 54,6,0,54 });
		assert_end();
		// string
		set("'\v\f\r\t ,=(){}'");
		assert_primitive(String,				{ 0,13,0,0 });
		assert_end();
		// user error
		set("`\v\f\r\t ,=(){}`");
		assert_error(UserError,					{ 0,13,0,0 });
		assert_end();
		// operators
		set("(+3*-3)/(1%2+3-4)");
		assert_particle(SequenceStart,			{ 0,1,0,0 });
		assert_particle(OpAddition,				{ 1,1,0,1 }); // unary
		assert_primitive(Number,				{ 2,1,0,2 });
		assert_particle(OpMultiply,				{ 3,1,0,3 });
		assert_particle(OpSubtraction,			{ 4,1,0,4 }); // unary
		assert_primitive(Number,				{ 5,1,0,5 });
		assert_particle(SequenceEnd,			{ 6,1,0,6 });
		assert_particle(OpDivide,				{ 7,1,0,7 });
		assert_particle(SequenceStart,			{ 8,1,0,8 });
		assert_primitive(Number,				{ 9,1,0,9 });
		assert_particle(OpModulus,				{ 10,1,0,10 });
		assert_primitive(Number,				{ 11,1,0,11 });
		assert_particle(OpAddition,				{ 12,1,0,12 });
		assert_primitive(Number,				{ 13,1,0,13 });
		assert_particle(OpSubtraction,			{ 14,1,0,14 });
		assert_primitive(Number,				{ 15,1,0,15 });
		assert_particle(SequenceEnd,			{ 16,1,0,16 });
		assert_end();
	}
		
	
}