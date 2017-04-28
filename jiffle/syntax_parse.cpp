#include "syntax.h"
#include <stack>
#include <iostream>

namespace syntax {

#define shift() do{ length--; code++; }while(0);

	expr::ptr parse(const tokens& sourcecode) {
		const token* code = &sourcecode[0];
		size_t length = sourcecode.size();
		
		auto entry = expr::make<sequence>();
		std::stack<sequence*> S;
		std::stack<evaluation*> E;
		S.push(expr::as<sequence>(entry));
		E.push(nullptr);
		expr::ptr tmp;
		
		auto push2eval = [&](expr::ptr& val) {
			if (!E.top()) {
				auto e = expr::make<evaluation>();
				E.top() = expr::as<evaluation>(e);
				S.top()->items.push_back(e);
			}
			E.top()->terms.push_back(val);
			if (val->error)
				E.top()->error = true;
		};

		auto pushErr = [&]() {
			tmp = expr::make<value>();
			tmp->error = true;
			expr::as<value>(tmp)->token = code;
			push2eval(tmp);
		};

		while (length > 0) {
			
			// skip comments --------------------------------------------------
			if (length && code->type == token::Comment) { }

			// expression -----------------------------------------------------
			else if (length && code->type != token::Particle) {
				tmp = expr::make<value>();
				tmp->error = code->type == token::Error;
				expr::as<value>(tmp)->token = code;
				push2eval(tmp);
			}

			// particles ------------------------------------------------------
			else if (length && code->type == token::Particle) {
				switch (code->data.particle) {

				// Abstraction ------------------------------------------------
				case syntax::AbstractionSequenceBegin:
				case syntax::Abstraction:					
				{					
					tmp = expr::make<abstraction>();
					auto abs = expr::as<abstraction>(tmp);
					abs->content = expr::make<evaluation>();

					if (!E.top()) {
						tmp->error = true;
					}
					else {
						// set name and params
						for each (auto t in E.top()->terms) {
							auto val = expr::as<value>(t);

							// validate
							if (!expr::is<value>(t)
								|| val->token->type != token::Symbol
								|| (!val->token->data.symbol.isParam 
									&& abs->symbol)) {
								tmp->error = true;
								continue;
							}

							if (val->token->data.symbol.isParam)
								abs->params.push_back(&val->token->data.symbol);
							else
								abs->symbol = &expr::as<value>(t)->token->data.symbol;							
						}
						E.top()->terms.clear();
					}
					push2eval(tmp);
					E.top() = expr::as<evaluation>(abs->content);
				}
				if (code->data.particle == syntax::Abstraction)
					break;
				// continue to subsequence

				// Sub Sequence -----------------------------------------------
				case syntax::SequenceBegin:
					tmp = expr::make<sequence>();
					push2eval(tmp);
					S.push(expr::as<sequence>(tmp));
					E.push(nullptr);
					if (code->data.particle == syntax::AbstractionSequenceBegin)
						S.top()->flags |= sequence::Abstraction;
					break;
				case syntax::AbstractionSequenceEnd:
				case syntax::SequenceEnd:
					if (S.size() == 1
						|| ((S.top()->flags & sequence::Abstraction) > 0)
						!= (code->data.particle == syntax::AbstractionSequenceEnd)) {
						pushErr();
						break;
					}
					S.pop();
					E.pop();
					break;

				// Sequence ---------------------------------------------------
				case syntax::SequenceDivHard:
					S.top()->flags |= sequence::Explicit;
				case syntax::SequenceDivSoft:
					E.top() = nullptr;
					break;

				default:
					pushErr();
					break;
				}
			}
			
			shift();
		}

		return entry;
	}

}