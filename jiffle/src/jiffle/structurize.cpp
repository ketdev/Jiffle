#include "jiffle.h"

#include <stack>
#include <queue>
#include <memory>

namespace jiffle {
	
	static int _particlePrecedence(particle particle) {
		switch (particle)
		{
			// structure declarators
		case jiffle::DefinitionStart:
			break;
		case jiffle::DefinitionEnd:
			break;
		case jiffle::ParameterStart:
			break;
		case jiffle::ParameterEnd:
			break;
		case jiffle::SequenceStart:
			break;
		case jiffle::SequenceEnd:
			break;
		case jiffle::Whitespace:
			break;

			// operators
		//case jiffle::OpUnaryPlus:
		//	break;
		//case jiffle::OpUnaryMinus:
		//	break;
		case jiffle::OpMultiply:
			break;
		case jiffle::OpDivide:
			break;
		case jiffle::OpModulus:
			break;
		case jiffle::OpAddition:
			break;
		case jiffle::OpSubtraction:
			break;

			// structure dividers
		case jiffle::DefinitionAssign:
			break;
		case jiffle::Separator:
			break;
		case jiffle::SeparatorImplicit:
			break;
		default:
			break;
		}
		return 0;
	}

	// parsers ----------------------------------------------------------------


	namespace {

		typedef std::shared_ptr<data> expr;

		// Parser function object
		// @brief	Handles parsing a stream of expressions, optionally passing
		//			the expression or a modification of it to the next parser 
		//			in the list.
		// @args	expression - the input expression, to be modified by parser
		//			can be null indicating end of file.
		//			consume - output, if the parsing pipeline should continue 
		//			to the next token or stay on same (not yet consumed).
		// @return	true if the expression should be passed to the next parser
		struct __parser {
			virtual bool operator()(expr& expression, bool& consume) = 0;
		};

		struct _ignored : public __parser {
			virtual bool operator()(expr& expression, bool& consume) {
				consume = true; // always consume
				return 
					// end of file
					expression == nullptr 
					// anything except comments and whitespaces
					|| ((expression->category != Primitive || expression->primitive != Comment)
					&& (expression->category != Particle || expression->primitive != Whitespace));
			}
		};

		struct _evaluation : public __parser {
			// internal evaluation build-up, passed on once completed
			expr eval = nullptr;

			/* // Shunting-yard algorithm //
			While there are tokens to be read:
			Read a token.
			If the token is a number, then push it to the output queue.
			If the token is a function token, then push it onto the stack.
			If the token is a function argument separator (e.g., a comma):
				Until the token at the top of the stack is a left parenthesis, pop operators off the stack onto the output queue. If no left parentheses are encountered, either the separator was misplaced or parentheses were mismatched.
			If the token is an operator, o1, then:
				while there is an operator token o2, at the top of the operator stack and either
					o1 is left-associative and its precedence is less than or equal to that of o2, or
					o1 is right associative, and has precedence less than that of o2,
						pop o2 off the operator stack, onto the output queue;
				at the end of iteration push o1 onto the operator stack.
			If the token is a left parenthesis (i.e. "("), then push it onto the stack.
			If the token is a right parenthesis (i.e. ")"):
				Until the token at the top of the stack is a left parenthesis, pop operators off the stack onto the output queue.
				Pop the left parenthesis from the stack, but not onto the output queue.
				If the token at the top of the stack is a function token, pop it onto the output queue.
				If the stack runs out without finding a left parenthesis, then there are mismatched parentheses.
			When there are no more tokens to read:
				While there are still operator tokens in the stack:
					If the operator token on the top of the stack is a parenthesis, then there are mismatched parentheses.
					Pop the operator onto the output queue.
			Exit.
			*/

			virtual bool operator()(expr& expression, bool& consume) {
				// consume by default
				consume = true;

				// add to evaluation: primitives, errors & operators
				if (expression != nullptr 
					&& (expression->category == Primitive 
						|| expression->category == Error
						|| (expression->category == Particle 
							&& expression->particle & OPERATOR_BIT))) {

					// init evaluation if missing
					if (!eval) {
						eval.reset(new data);
						eval->category = Structure;
						eval->structure = Evaluation;
						eval->pos = expression->pos;
					}
					
					// push the token & update position
					eval->items.push_back(*expression);
					eval->pos.len = expression->pos.ch + expression->pos.len - eval->pos.ch;
					return false; // consumed
				}

				// anything else closes the evaluation
				if (eval) {
					// pass the evaluation to the next parser first
					// then continue with the next token
					consume = false;
					expression = eval;
					eval = nullptr;
				}
				return true;
			}
		};

		struct _sequence : public __parser {
			// internal sequence build-up, passed on once completed
			std::stack<expr> _seqStack;

			virtual bool operator()(expr& expression, bool& consume) {
				// consume by default
				consume = true;

				// init sequence if missing
				if (_seqStack.empty()) {
					expr seq(new data);
					seq->category = Structure;
					seq->structure = Sequence;
					seq->pos = expression->pos;
					_seqStack.push(seq);
				}

				// explicit separator
				if (expression != nullptr 
					&& expression->category == Particle 
					&& expression->particle == Separator) {
					_seqStack.top()->structure = static_cast<structure>(Sequence | ExplicitFlag);
				}

				// implicit separator
				if (expression != nullptr
					&& expression->category == Particle
					&& expression->particle == SeparatorImplicit) {
				}
				
				// add to sequence: other structures
				if (expression != nullptr && expression->category == Structure) {
					
					// push the structure & update position
					_seqStack.top()->items.push_back(*expression);
					_seqStack.top()->pos.len = expression->pos.ch + expression->pos.len - seq->pos.ch;
					return false; // consumed
				}
			}
		};

		struct _module : public __parser {
			// create module from expression
			virtual bool operator()(expr& expression, bool& consume) {
				consume = true; // always consume
				expr m(new data{});
				m->category = Structure;
				m->structure = Module;
				if (expression) {
					m->pos = expression->pos;
					m->items.push_back(*expression);
				}
				expression = m;
				return true;
			}
		};

	}
	
	////
	static void _parseAtom(data& structure, const data* tokens, int length) {

	}
	static void _parseGenericStructure(data& structure, const data* tokens, int length) {

	}
	static void _parseSequence(data& structure, const data* tokens, int length) {
		// parse generic structure, 
		// we will know what it is later on depending on particles
		data s = {};
		s.category = Structure;
		s.structure = Evaluation; // evaluation by default
		_parseGenericStructure(s, tokens, length);

	}

	// structurize ------------------------------------------------------------
	
	data structurize(const std::string& moduleName, const std::vector<data>& tokens) {

		// build module
		data module = {};
		module.text = moduleName;
		if (!tokens.empty())
			module.pos.len = tokens.back().pos.ch + tokens.back().pos.len;

		_parseSequence(module, &tokens[0], tokens.size());

		return module;

		////


		// structure parsing pipeline
		// @input	all tokens
		__parser* parser[] = {
			// @pipe	removed whitespaces and comments
			new _ignored, 

			// @pipe	evaluation structs and non-operator particles
			new _evaluation,

			// @pipe	combine expressions into sequences
			new _sequence,

			// ...

			// @output	module structure
			new _module,
		};

		expr expression;

		// structurize all tokens
		for (size_t t = 0;;) {
			expression = nullptr;

			// copy of current expression to push
			// can be modified by parsers before passing to the next parser
			if (t < tokens.size()) {
				expression.reset(new data);
				*expression = tokens[t];
			}

			// parser pipeline
			bool finished = true; // finish once module is reached (no break)
			bool consume = true;
			for (size_t p = 0; p < sizeof(parser) / sizeof(parser[0]); p++) {
				if (!(*parser[p])(expression, consume)) {
					finished = false;
					break;
				}
			}

			// advance if consuming
			if (consume) t++;

			// break if expression returned module
			if (finished) break;
		}
		
		// free memory
		for each (auto ptr in parser)
			delete ptr;

		// set module name & full file position
		expression->text = moduleName;
		expression->pos = {};
		if(!tokens.empty())
			expression->pos.len = tokens.back().pos.ch + tokens.back().pos.len;
		return *expression;
	}

#if 0
	node parse(const std::vector<syntax::token> & tokens, const std::string & code) {
		using namespace syntax;

		// internal state -------------------------------------------------
		node _module = { Module }, *_node = &_module;
		std::stack<node*> _stack;
		_stack.push(&_module);
		token _t;

		// methods --------------------------------------------------------
		auto updatePos = [&](pos p) {
			auto top = _stack.top();
			if (!(top->type & STRUCTURE_BIT))
				top->pos = p;
			else 
				top->pos.len = p.ch + p.len - top->pos.ch;
		};
		auto pop = [&](bool cond = true) {
			if (cond) {
				auto p = _stack.top()->pos;
				_stack.pop();
				updatePos(p);
				_node = _stack.top();
			}
		};
		auto popUntil = [&](expr::type structure) {
			while (_stack.top()->type != structure)
				pop();
		};
		auto isType = [&](expr::type t) {
			return _stack.top()->type == t;
		};
		auto add = [&](expr::type e) {
			_stack.top()->items.push_back(node{ e, flags::Default, _t.pos });
			_node = &_stack.top()->items.back();
		};
		auto push = [&](expr::type structure, expr::type value) {
			if (_stack.top()->type != structure) {
				add(structure);
				_stack.push(_node);
			}
			add(value);
			if (_node->type & STRUCTURE_BIT) {
				_stack.push(_node);
			}
		};

		// entry ----------------------------------------------------------
		for (size_t i = 0; i < tokens.size(); i++){
			_t = tokens[i];

			// implicit sequence stops ------------------------------------				
			while (true) {
				auto s = _stack.size();
				// object
				if (_t.type != syntax::Definition
					&& _t.type != syntax::DefinitionStart
					&& _t.type != syntax::ParameterStart) {
					pop(isType(expr::Object));
				}
				// evaluation
				if (_t.type == syntax::Separator
					|| _t.type == syntax::SeparatorImplicit
					|| _t.type == syntax::SequenceEnd
					|| _t.type == syntax::DefinitionEnd
					|| _t.type == syntax::ParameterEnd) {
					pop(isType(expr::Evaluation));

					// regular definition ('=') (and object)
					// also closed at end of evaluation
					pop(isType(expr::Definition));
				}

				// nothing changed
				if(s == _stack.size())
					break;
			}

			// update expression position ---------------------------------
			updatePos(_t.pos);
			switch (_t.type) {

			// values -----------------------------------------------------
			case syntax::Comment: 
				break;
			case syntax::Null:
				push(expr::Evaluation, expr::Null);
				break;
			case syntax::True:
				push(expr::Evaluation, expr::True);
				break;
			case syntax::False:
				push(expr::Evaluation, expr::False);
				break;
			case syntax::Integer:
				push(expr::Evaluation, expr::Integer);
				_node->text = code.substr(_t.pos.ch, _t.pos.len);
				break;
			case syntax::Real:
				push(expr::Evaluation, expr::Real);
				_node->text = code.substr(_t.pos.ch, _t.pos.len);
				break;
			case syntax::String:
				push(expr::Evaluation, expr::String);
				_node->text = code.substr(_t.pos.ch + 1, _t.pos.len - 2);
				break;
			case syntax::UserError:
				push(expr::Evaluation, expr::Error);
				_node->text = code.substr(_t.pos.ch + 1, _t.pos.len - 2);
				break;
			default: case syntax::SyntaxError:
				push(expr::Evaluation, expr::SyntaxError);
				_node->text = std::string("invalid syntax \"")
					+ code.substr(_t.pos.ch, _t.pos.len)
					+ "\"";
				break;

			// symbol -----------------------------------------------------
			case syntax::Symbol:
				push(expr::Evaluation, expr::Object);
				_node->text = code.substr(_t.pos.ch, _t.pos.len);
				break;

			// parameter --------------------------------------------------
			case syntax::ParameterStart:
				if (!isType(expr::Object)) // anonymous object
					push(expr::Evaluation, expr::Object);
				push(expr::Object, expr::Parameter);
				break;
			case syntax::ParameterEnd:
				if (!isType(expr::Parameter)) {
					push(expr::Evaluation, expr::SyntaxError);
					_node->text = "no matching opening bracket";
				}
				else {
					pop();
				}
				break;

			// definition -------------------------------------------------
			case syntax::Definition:
				if (!isType(expr::Object)) {
					push(expr::Evaluation, expr::SyntaxError);
					_node->text = "symbol missing";
				}
				else {
					push(expr::Object, expr::Definition);
				}
				break;
			case syntax::DefinitionStart:
				if (!isType(expr::Object)) // anonymous object
					push(expr::Evaluation, expr::Object);					
				push(expr::Object, expr::DefinitionSequence);
				break;
			case syntax::DefinitionEnd:
				if (!isType(expr::DefinitionSequence)) {
					push(expr::Evaluation, expr::SyntaxError);
					_node->text = "no matching opening curly bracket";
				} else {
					pop();
					pop(isType(expr::Object));
				}
				break;

			// separator --------------------------------------------------
			case syntax::Separator:
				_node->flags |= flags::ExplicitStructure;
			case syntax::SeparatorImplicit:
				break;

			// sequence ---------------------------------------------------
			case syntax::SequenceStart:
				push(expr::Evaluation, expr::Sequence);
				break;
			case syntax::SequenceEnd:
				if (!isType(expr::Sequence)) {
					push(expr::Evaluation, expr::SyntaxError);
					_node->text = "no matching opening parenthesis";
				} else {
					pop();
				}
				break;
			}
		}

		// close structures -----------------------------------------------
		while (_stack.size() > 1) {
			// error: missing construct closing
			if (!isType(expr::Evaluation) 
				&& !isType(expr::Object)
				&& !isType(expr::Definition)) {
				pop();
				push(expr::Evaluation, expr::SyntaxError);
				_node->pos.ch++;
				_node->pos.col++;
				_node->pos.len = 0;
				_node->text = "missing closing parenthesis";
			}
			pop();
		}

		return _module;
	}
#endif

}
