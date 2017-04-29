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
		
		auto pushExpr = [&](expr::ptr& val) {
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
			pushExpr(tmp);
		};
		constexpr int bracketTypeMap[] = {
			syntax::SequenceEnd,
			syntax::DefinitionSequenceEnd,
			syntax::ParameterEnd
		};

		while (length > 0) {
			
			// skip comments --------------------------------------------------
			while (length && code->type == token::Comment) { shift(); }

			// constants ------------------------------------------------------
			if (length 
				&& (code->type == token::Constant || code->type == token::Error)) {
				tmp = expr::make<value>();
				tmp->error = code->type == token::Error;
				if (S.top()->flags & sequence::Parameters)
					tmp->error = true;				
				expr::as<value>(tmp)->token = code;
				pushExpr(tmp);
			}

			// objects --------------------------------------------------------
			else if (length && code->type == token::Symbol) {
				tmp = expr::make<object>();
				expr::as<object>(tmp)->symbol = &code->data.symbol;
				pushExpr(tmp);
			}

			// particles ------------------------------------------------------
			else if (length && code->type == token::Particle) {
				switch (code->data.particle) {
					
				// Definition -------------------------------------------------
				case syntax::DefinitionSequenceBegin:
				case syntax::Definition:					
				{					
					tmp = expr::make<definition>();
					auto def = expr::as<definition>(tmp);

					if (!E.top()) {
						def->error = true;
					}
					else { 
						// new definition
						// [optional] start with symbol
						auto head = expr::as<object>(E.top()->terms.front());
						int index = 0;
						if (head) {
							def->symbol = head->symbol;
							S.top()->symtable.push_back(def);
							index++;
						}
						
						// then parameter sequences follow
						for (; index < E.top()->terms.size(); index++) {
							auto ptr = E.top()->terms[index];
							auto seq = expr::as<sequence>(ptr);
							if (!seq || !(seq->flags & sequence::Parameters)) {
								ptr->error = true;
								def->error = true;
							}
							def->parameters.push_back(ptr);
						}

						E.top()->terms.clear();
					}
					pushExpr(tmp);

					def->content = expr::make<sequence>();
					if (code->data.particle == syntax::DefinitionSequenceBegin) {
						S.push(expr::as<sequence>(def->content));
						S.top()->flags |= sequence::Definition;
						E.push(nullptr);
					} else {
						tmp = expr::make<evaluation>();
						expr::as<sequence>(def->content)->items.push_back(tmp);
						E.top() = expr::as<evaluation>(tmp);
					}
				}
				break;

				// Sub Sequence -----------------------------------------------
				case syntax::SequenceBegin:
				case syntax::ParameterBegin:
					tmp = expr::make<sequence>();
					pushExpr(tmp);
					S.push(expr::as<sequence>(tmp));
					E.push(nullptr);
					// set sequence type
					if (code->data.particle == syntax::ParameterBegin)
						S.top()->flags |= sequence::Parameters;				
					break;

				case syntax::SequenceEnd:
				case syntax::ParameterEnd:
				case syntax::DefinitionSequenceEnd:					
					if (
						// stack must always have an item remaining
						S.size() == 1
						// match begin-end bracket type
						|| code->data.particle
						!= bracketTypeMap[S.top()->flags & sequence::BracketMask] 
					) {
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