#include "read.h"
#include "detail/token.h"

#include <stack>

namespace syntax {

	// gets the top of the construct stack
	// as the specified construct type
	// or creates it by adding to the current top 
	// and pushing to the stack if not the specified type
	// @note cstack should not be empty!
	template<typename C>
	static C* get(std::stack<construct*>& cstack, const detail::pos& pos) {
		// create
		auto top = cstack.top()->as<C>();
		if (top == nullptr) {
			auto newCons = expr::make<C>();
			// inherit pos
			newCons->pos = pos;
			// add to current top
			cstack.top()->items.push_back(newCons);
			// add to stack
			cstack.push(newCons->as<construct>());
			// return top
			top = cstack.top()->as<C>();
		}
		return top;
	}

	// pops from the stack and updates position
	static void pop(std::stack<construct*>& cstack) {
		auto top = cstack.top();
		cstack.pop();
		// update construct length
		cstack.top()->pos.len = top->pos.ch + top->pos.len - cstack.top()->pos.ch;
	}

	// pops from the stack until the given construct type
	// is at the top, updating positions
	template<typename C>
	static void pop(std::stack<construct*>& cstack) {
		while (cstack.top()->as<C>() == nullptr)
			pop(cstack);
	}

	// removes the top if it is the given construct type
	template<typename C>
	static void popIf(std::stack<construct*>& cstack) {
		if (cstack.top()->as<C>() != nullptr)
			pop(cstack);
	}

	// adds a new expression (T) to the top construct (C), 
	// aquired using get, so it might create it if not current top
	template<typename C, typename T>
	static T* add(std::stack<construct*>& cstack, const detail::pos& pos) {
		// create expression
		auto expr = expr::make<T>();
		expr->pos = pos;
		// get construct
		auto c = get<C>(cstack, pos);
		// inherit pos if first
		if (c->items.empty())
			c->pos = expr->pos;
		// add expr to construct
		c->items.push_back(expr);
		// update construct length
		c->pos.len = expr->pos.ch + expr->pos.len - c->pos.ch;
		// return expression as type
		return expr->as<T>();
	}


	expr::ptr read(const std::string & code) {
		expr::ptr o = expr::make<sequence>(); // module sequence wrapper
		o->as<sequence>()->type = sequence::Module;

		// construct to push onto
		std::stack<construct*> C;
		C.push(o->as<construct>());

		// tokenize input
		auto tokens = detail::tokenize(code);
		detail::token t = {};

		// parse tokens
		for (size_t i = 0; i < tokens.size(); i++) {
			t = tokens[i];

			switch (t.type) {
			case detail::token::Comment:
				// skip
				break;

			case detail::token::Null:
				popIf<object>(C);
				add<evaluation, value_null>(C, t.pos);
				break;

			case detail::token::True:
				popIf<object>(C);
				add<evaluation, value_bool>(C, t.pos)->value = true;
				break;

			case detail::token::False:
				popIf<object>(C);
				add<evaluation, value_bool>(C, t.pos)->value = false;
				break;

			case detail::token::Symbol:
			{
				popIf<object>(C);
				auto o = add<evaluation, object>(C, t.pos);
				o->name = code.substr(t.pos.ch, t.pos.len);
				C.push(o);
			}
			break;

			case detail::token::Integer:
				popIf<object>(C);
				add<evaluation, value_integer>(C, t.pos)->value = t.number.integer;
				break;

			case detail::token::Real:
				popIf<object>(C);
				add<evaluation, value_real>(C, t.pos)->value = t.number.real;
				break;

			case detail::token::String:
				popIf<object>(C);
				add<evaluation, value_string>(C, t.pos)->value = code.substr(t.pos.ch + 1, t.pos.len - 2);
				break;

			case detail::token::SequenceSeparator:
			case detail::token::SequenceSeparatorImplicit:
				pop<sequence>(C);
				C.top()->as<sequence>()->isExplicit = t.type == detail::token::SequenceSeparator;
				break;

			case detail::token::Definition:
			case detail::token::DefinitionSequenceStart:
			{
				// must follow an object (optionally with params)
				auto o = C.top()->as<object>();
				if (o != nullptr) {
					if (t.type == detail::token::Definition)
						C.push(add<object, evaluation>(C, t.pos));
					else { // definition sequence
						auto as = add<object, sequence>(C, t.pos);
						as->type = sequence::Abstraction;
						C.push(as);
					}
				}
				else {
					auto p = add<evaluation, value_error>(C, t.pos);
					p->type = value_error::Syntax;
					p->text = std::string("can only define identifiers");
				}
			}
			break;

			case detail::token::TupleStart:
				C.push(add<evaluation, sequence>(C, t.pos));
				break;

			case detail::token::DefinitionSequenceEnd:
			case detail::token::TupleEnd:
			{
				pop<sequence>(C);
				// match parenthesis type
				auto pt = C.top()->as<sequence>()->type;
				if ((t.type == detail::token::DefinitionSequenceEnd && pt == sequence::Abstraction)
					|| (t.type == detail::token::TupleEnd && pt == sequence::Tuple)) {
					pop(C);
					break;
				}
				else {
					auto p = add<evaluation, value_error>(C, t.pos);
					p->type = value_error::Syntax;
					p->text = std::string("no matching opening parenthesis");
				}
			}
			break;


			case detail::token::UserError:
			{
				popIf<object>(C);
				auto p = add<evaluation, value_error>(C, t.pos);
				p->type = value_error::Custom;
				p->text = code.substr(t.pos.ch + 1, t.pos.len - 2);
			}
			break;

			default: // SyntaxError
			{
				popIf<object>(C);
				auto p = add<evaluation, value_error>(C, t.pos);
				p->type = value_error::Syntax;
				p->text = std::string("invalid syntax \"")
					+ code.substr(t.pos.ch, t.pos.len)
					+ "\"";
			}
			break;

			} // end switch
		} // end for

		// close constructs
		while (C.size() > 1) {
			// missing construct closing
			if (!C.top()->as<evaluation>() && !C.top()->as<object>()) {
				pop(C);
				auto p = add<evaluation, value_error>(C, t.pos);
				p->type = value_error::Syntax;
				p->text = std::string("matching closing parenthesis");
			}
			pop(C);
		}

		return o;
	}

}