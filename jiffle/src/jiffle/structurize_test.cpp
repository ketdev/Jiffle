#include "jiffle.h"
#include <assert.h>
#include <stack>

namespace jiffle {

	void structurize_test() {

		// internal state -----------------------------------------------------
		std::vector<data> _tokens;
		data _module;
		std::stack<data*> _dfs;

		// methods ------------------------------------------------------------
		auto set = [&](const std::string& input) {
			_tokens = tokenize(input);
			_module = structurize("module",_tokens);
			assert(_module.text == "module");
			_dfs.push(&_module);
		};
		auto next = [&]() {
			auto top = _dfs.empty() ? nullptr : _dfs.top();
			_dfs.pop();
			if (top) {
				for (auto it = top->items.rbegin(); it != top->items.rend(); it++) {
					_dfs.push(&(*it));
				}
			}
		};
		auto assert_structure = [&](structure type, pos p, int flags = 0) {
			assert(_dfs.top()->category == Structure);
			assert(_dfs.top()->structure == static_cast<structure>(static_cast<int>(type) | flags));
			auto pos = _dfs.top()->pos;
			assert(pos.ch == p.ch && pos.col == p.col && pos.len == p.len && pos.ln == p.ln);
			next();
		};
		auto assert_primitive = [&](primitive type, pos p) {
			assert(_dfs.top()->category == Primitive);
			assert(_dfs.top()->primitive == type);
			auto pos = _dfs.top()->pos;
			assert(pos.ch == p.ch && pos.col == p.col && pos.len == p.len && pos.ln == p.ln);
			next();
		};
		auto assert_error = [&](error type, pos p) {
			assert(_dfs.top()->category == Error);
			assert(_dfs.top()->error == type);
			auto pos = _dfs.top()->pos;
			assert(pos.ch == p.ch && pos.col == p.col && pos.len == p.len && pos.ln == p.ln);
			next();
		};
		auto assert_end = [&]() {
			assert(_dfs.empty());
		};

		// tests --------------------------------------------------------------

		// empty
		set("");
		assert_structure(Module,				{ 0,0,0,0 });
		assert_end();
		
		// comment
		set("# hello comment");
		assert_structure(Module,				{ 0,15,0,0 });
		assert_end();

		// primitives
		set("null true false 123456 123456.0 'hello world!' foo `err` 0x # comment");
		assert_structure(Module,				{ 0,69,0,0 });
		/**/assert_structure(Evaluation,		{ 0,59,0,0 });
		/****/assert_primitive(Null,			{ 0,4,0,0 });
		/****/assert_primitive(True,			{ 5,4,0,5 });
		/****/assert_primitive(False,			{ 10,5,0,10 });
		/****/assert_primitive(Number,			{ 16,6,0,16 });
		/****/assert_primitive(Number,			{ 23,8,0,23 });
		/****/assert_primitive(String,			{ 32,14,0,32 });
		/****/assert_primitive(Symbol,			{ 47,3,0,47 });
		/****/assert_error(UserError,			{ 51,5,0,51 });
		/****/assert_error(SyntaxError,			{ 57,2,0,57 });
		assert_end();
		
		{ // separators 
			// explicit
			set("1,2,a b,`ok`");
			assert_structure(Module,			{ 0,12,0,0 });
			/**/assert_structure(Sequence,		{ 0,12,0,0 }, ExplicitFlag);
			/****/assert_structure(Evaluation,	{ 0,1,0,0 });
			/******/assert_primitive(Number,	{ 0,1,0,0 });
			/****/assert_structure(Evaluation,	{ 2,1,0,2 });
			/******/assert_primitive(Number,	{ 2,1,0,2 });
			/****/assert_structure(Evaluation,	{ 4,3,0,4 });
			/******/assert_primitive(Symbol,	{ 4,1,0,4 });
			/******/assert_primitive(Symbol,	{ 6,1,0,6 });
			/****/assert_structure(Evaluation,	{ 8,4,0,8 });
			/******/assert_error(UserError,		{ 8,4,0,8 });
			assert_end();

			// implicit
			set("1\n2");
			assert_structure(Module,				{ 0,3,0,0 });
			/**/assert_structure(Sequence,			{ 0,3,0,0 });
			/****/assert_structure(Evaluation,		{ 0,1,0,0 });
			/******/assert_primitive(Number,		{ 0,1,0,0 });
			/****/assert_structure(Evaluation,		{ 2,1,1,0 });
			/******/assert_primitive(Number,		{ 2,1,1,0 });
			assert_end();
		}
	}

#if 0
	void parse_test() {

		// internal state -----------------------------------------------------
		std::vector<data> _src;
		data _ast, *_node;
		std::stack<data*> _dfs;

		// methods ------------------------------------------------------------
		auto set = [&](const std::string& input) {
			_src = tokenize(input);
			_ast = parse(_src, input);
			_dfs.push(&_ast);
			_node = _dfs.top();
		};
		auto next = [&]() {
			_dfs.pop();
			if (_node && (_node->type & STRUCTURE_BIT)) {
				for (auto it = _node->items.rbegin(); it != _node->items.rend(); it++) {
					_dfs.push(&(*it));
				}
			}
			_node = _dfs.empty() ? nullptr : _dfs.top();
		};
		auto assert_expr = [&](expr::type t, syntax::pos p, unsigned char flags = expr::flags::Default) {
			auto pos = _node->pos;
			assert(_node->type == t);
			assert(pos.ch == p.ch && pos.col == p.col && pos.len == p.len && pos.ln == p.ln);
			assert(_node->flags == flags);
			next();
		};
		auto assert_string = [&](const std::string& v, syntax::pos p) {
			assert(_node->text == v);
			assert_expr(expr::String, p);
		};
		auto assert_symbol = [&](const std::string& v, syntax::pos p) {
			assert(_node->text == v);
			assert_expr(expr::Object, p);
		};
		auto assert_error = [&](const std::string& v, syntax::pos p) {
			assert(_node->text == v);
			assert_expr(expr::Error, p);
		};
		auto assert_end = [&]() {
			assert(_node == nullptr);
		};

		// tests --------------------------------------------------------------

		// empty
		set("");
		assert_expr(expr::Module,			{ 0,0,0,0 });
		assert_end();

		// comment
		set("# hello comment");
		assert_expr(expr::Module,			{ 0,15,0,0 });
		assert_end();

		// primitives
		set("null true false 123456 123456.0 'hello world!' foo `err` 0x # comment");
		assert_expr(expr::Module,			{ 0,69,0,0 });
		/**/assert_expr(expr::Evaluation,	{ 0,69,0,0 });
		/****/assert_expr(expr::Null,		{ 0,4,0,0 });
		/****/assert_expr(expr::True,		{ 5,4,0,5 });
		/****/assert_expr(expr::False,		{ 10,5,0,10 });
		/****/assert_expr(expr::Integer,	{ 16,6,0,16 });
		/****/assert_expr(expr::Real,		{ 23,8,0,23 });
		/****/assert_string("hello world!", { 32,14,0,32 });
		/****/assert_symbol("foo",			{ 47,3,0,47 });
		/****/assert_error("err",			{ 51,5,0,51 });
		/****/assert_expr(expr::SyntaxError,{ 57,2,0,57 });
		assert_end();
			
			
		{ // separators 
			// explicit
			set("1,2,a b,`ok`");
			assert_expr(expr::Module,			{ 0,12,0,0 }, flags::ExplicitStructure);
			/**/assert_expr(expr::Evaluation,	{ 0,1,0,0 });
			/****/assert_expr(expr::Integer,	{ 0,1,0,0 });
			/**/assert_expr(expr::Evaluation,	{ 2,1,0,2 });
			/****/assert_expr(expr::Integer,	{ 2,1,0,2 });
			/**/assert_expr(expr::Evaluation,	{ 4,3,0,4 });
			/****/assert_symbol("a",			{ 4,1,0,4 });
			/****/assert_symbol("b",			{ 6,1,0,6 });
			/**/assert_expr(expr::Evaluation,	{ 8,4,0,8 });
			/****/assert_error("ok",			{ 8,4,0,8 });
			assert_end();

			// implicit
			set("1\n2");
			assert_expr(expr::Module,				{ 0,3,0,0 });
			/**/assert_expr(expr::Evaluation,		{ 0,1,0,0 });
			/****/assert_expr(expr::Integer,		{ 0,1,0,0 });
			/**/assert_expr(expr::Evaluation,		{ 2,1,1,0 });
			/****/assert_expr(expr::Integer,		{ 2,1,1,0 });
			assert_end();
		}
			
		{ // sequences
			// basic
			set("1(2)3");
			assert_expr(expr::Module,					{ 0,5,0,0 });
			/**/assert_expr(expr::Evaluation,			{ 0,5,0,0 });
			/****/assert_expr(expr::Integer,			{ 0,1,0,0 });
			/****/assert_expr(expr::Sequence,			{ 1,3,0,1 });
			/******/assert_expr(expr::Evaluation,		{ 2,1,0,2 });
			/********/assert_expr(expr::Integer,		{ 2,1,0,2 });
			/****/assert_expr(expr::Integer,			{ 4,1,0,4 });
			assert_end();

			// nested sequences
			set("1,(2,(3,)),(4)");
			assert_expr(expr::Module,					{ 0,14,0,0 }, flags::ExplicitStructure);
			/**/assert_expr(expr::Evaluation,			{ 0,1,0,0 });
			/****/assert_expr(expr::Integer,			{ 0,1,0,0 });
			/**/assert_expr(expr::Evaluation,			{ 2,8,0,2 });
			/****/assert_expr(expr::Sequence,			{ 2,8,0,2 }, flags::ExplicitStructure);
			/******/assert_expr(expr::Evaluation,		{ 3,1,0,3 });
			/********/assert_expr(expr::Integer,		{ 3,1,0,3 });
			/******/assert_expr(expr::Evaluation,		{ 5,4,0,5 });
			/********/assert_expr(expr::Sequence,		{ 5,4,0,5 }, flags::ExplicitStructure);
			/**********/assert_expr(expr::Evaluation,	{ 6,1,0,6 });
			/************/assert_expr(expr::Integer,	{ 6,1,0,6 });
			/**/assert_expr(expr::Evaluation,			{ 11,3,0,11 });
			/****/assert_expr(expr::Sequence,			{ 11,3,0,11 });
			/******/assert_expr(expr::Evaluation,		{ 12,1,0,12 });
			/********/assert_expr(expr::Integer,		{ 12,1,0,12 });
			assert_end();
			
			// sequence missing start
			set("a ) b");
			assert_expr(expr::Module,					{ 0,5,0,0 });
			/**/assert_expr(expr::Evaluation,			{ 0,1,0,0 });
			/****/assert_symbol("a",					{ 0,1,0,0 });
			/**/assert_expr(expr::Evaluation,			{ 2,3,0,2 });
			/****/assert_expr(expr::SyntaxError,		{ 2,1,0,2 });
			/****/assert_symbol("b",					{ 4,1,0,4 });
			assert_end();

			// sequence missing stop
			set("a ( b");
			assert_expr(expr::Module,					{ 0,5,0,0 });
			/**/assert_expr(expr::Evaluation,			{ 0,5,0,0 });
			/****/assert_symbol("a",					{ 0,1,0,0 });
			/****/assert_expr(expr::Sequence,			{ 2,3,0,2 });
			/******/assert_expr(expr::Evaluation,		{ 4,1,0,4 });
			/********/assert_symbol("b",				{ 4,1,0,4 });
			/****/assert_expr(expr::SyntaxError,		{ 5,0,0,5 });
			assert_end();
		}
		{ // definition
			set("f=3,4");
			assert_expr(expr::Module,						{ 0,5,0,0 }, flags::ExplicitStructure);
			/**/assert_expr(expr::Evaluation,				{ 0,3,0,0 });
			/****/assert_symbol("f",						{ 0,3,0,0 });
			/******/assert_expr(expr::Definition,			{ 1,2,0,1 });
			/********/assert_expr(expr::Evaluation,			{ 2,1,0,2 });
			/**********/assert_expr(expr::Integer,			{ 2,1,0,2 });
			/**/assert_expr(expr::Evaluation,				{ 4,1,0,4 });
			/****/assert_expr(expr::Integer,				{ 4,1,0,4 });
			assert_end();

			// sequence
			set("f{3,4},5");
			assert_expr(expr::Module,						{ 0,8,0,0 }, flags::ExplicitStructure);
			/**/assert_expr(expr::Evaluation,				{ 0,6,0,0 });
			/****/assert_symbol("f",						{ 0,6,0,0 });
			/******/assert_expr(expr::DefinitionSequence,	{ 1,5,0,1 }, flags::ExplicitStructure);
			/********/assert_expr(expr::Evaluation,			{ 2,1,0,2 });
			/**********/assert_expr(expr::Integer,			{ 2,1,0,2 });
			/********/assert_expr(expr::Evaluation,			{ 4,1,0,4 });
			/**********/assert_expr(expr::Integer,			{ 4,1,0,4 });
			/**/assert_expr(expr::Evaluation,				{ 7,1,0,7 });
			/****/assert_expr(expr::Integer,				{ 7,1,0,7 });
			assert_end();
		}
		{ // parameters
			set("f[x]=x");
			assert_expr(expr::Module,						{ 0,6,0,0 });
			/**/assert_expr(expr::Evaluation,				{ 0,6,0,0 });
			/****/assert_symbol("f",						{ 0,6,0,0 });
			/******/assert_expr(expr::Parameter,			{ 1,3,0,1 });
			/********/assert_expr(expr::Evaluation,			{ 2,1,0,2 });
			/**********/assert_symbol("x",					{ 2,1,0,2 });
			/******/assert_expr(expr::Definition,			{ 4,2,0,4 });
			/********/assert_expr(expr::Evaluation,			{ 5,1,0,5 });
			/**********/assert_symbol("x",					{ 5,1,0,5 });
			assert_end();

			// parameter definition ordering
			set("f{}[x]");
			assert_expr(expr::Module,						{ 0,6,0,0 });
			/**/assert_expr(expr::Evaluation,				{ 0,6,0,0 });
			/****/assert_symbol("f",						{ 0,3,0,0 });
			/******/assert_expr(expr::DefinitionSequence,	{ 1,2,0,1 });
			/****/assert_symbol("",							{ 3,3,0,3 });
			/******/assert_expr(expr::Parameter,			{ 3,3,0,3 });
			/********/assert_expr(expr::Evaluation,			{ 4,1,0,4 });
			/**********/assert_symbol("x",					{ 4,1,0,4 });
			assert_end();
				
		}

	}
#endif

}