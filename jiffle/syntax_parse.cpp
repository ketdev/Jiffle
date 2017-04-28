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

		auto evalpush = [&](expr::ptr& val) {
			if (!E.top()) {
				auto e = expr::make<evaluation>();
				E.top() = expr::as<evaluation>(e);
				S.top()->items.push_back(e);
			}
			E.top()->terms.push_back(val);
		};

		while (length > 0) {
			
			// skip comments --------------------------------------------------
			if (length && code->type == token::Comment) { }

			// expression -----------------------------------------------------
			else if (length && code->type != token::Particle) {
				tmp = expr::make<value>();
				expr::as<value>(tmp)->token = code;
				evalpush(tmp);
			}

			// particles ------------------------------------------------------
			else if (length && code->type == token::Particle) {
				switch (code->data.particle) {

				// Abstraction ------------------------------------------------
				//case syntax::Abstraction:
				//	break;
				//case syntax::ParamBegin:
				//	break;
				//case syntax::ParamEnd:
				//	break;

				// Sequence ---------------------------------------------------
				case syntax::SequenceDivHard:
					S.top()->isExplicit = true;
				case syntax::SequenceDivSoft:
					E.top() = nullptr;
					break;

				// Sub Sequence -----------------------------------------------
				case syntax::SequenceBegin:
					tmp = expr::make<sequence>();
					evalpush(tmp);
					S.push(expr::as<sequence>(tmp));
					E.push(nullptr);
					break;
				case syntax::SequenceEnd:
					if (S.size() == 1) {
						std::cerr << "Unexpected end of list" << std::endl;
						break;
					}
					S.pop();
					E.pop();
					break;

				default:
					std::cerr << "UNHANDLED PARTICLE: " << code->data.particle << std::endl;
					break;
				}
			}
			
			shift();
		}

		return entry;
	}

}