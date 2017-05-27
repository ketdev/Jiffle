#include "expr.h"

#include <stack>

namespace jiffle {
	namespace expr {

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

	}
}