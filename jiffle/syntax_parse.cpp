#include "syntax.h"
#include <stack>
#include <map>
#include <iostream>

namespace syntax {
	
	int _uid = 0;

	class exprmap {
	public:
		template<typename T>
		void push(expr<T>& val) {
			auto &stack = _map[expr<T>::uid()];
			// restart item
			if (!stack.empty() && !stack.top()) {
				stack.top() = val;
			}
			else {
				stack.push(val);
			}
		}

		template<typename T>
		expr<T> getmake() {
			auto &stack = _map[expr<T>::uid()];
			if (stack.empty()) {
				stack.push(expr<T>());
			}
			return expr<T>(stack.top());
		}

		template<typename T>
		bool empty() {
			auto &stack = _map[expr<T>::uid()];
			return (stack.empty() || !stack.top());
		}

	private:
		// uid mapped current editing expressions
		std::map<int, std::stack<handle>> _map;
	};

	template<typename T>
	static void push(std::map<int, std::stack<node::ptr>>& map, expr<T>& val) {
		auto &stack = map[expr<T>::uid()];
		// restart item
		if (!stack.empty() && !stack.top()){
			stack.top() = val;
		}
		else {
			stack.push(val);
		}
	}
		
#define shift() do{ length--; code++; }while(0);

	node::ptr parse(const tokens& sourcecode) {
		const token* code = &sourcecode[0];
		size_t length = sourcecode.size();
		
		auto module = expr<sequence>();
		// uid mapped current editing expressions
		exprmap emap;
		emap.push(module);
		
		////////////////////////

		auto entry = node::make<_sequence>();
		std::stack<_sequence*>	S;
		std::stack<_evaluation*> E;

		S.push(node::as<_sequence>(entry));
		E.push(nullptr);
		node::ptr tmp;
		
		auto pushExpr = [&](node::ptr& val) {
			if (!E.top()) {
				auto e = node::make<_evaluation>();
				E.top() = node::as<_evaluation>(e);
				S.top()->items.push_back(e);
			}
			E.top()->terms.push_back(val);
			if (val->error)
				E.top()->error = true;
		};
		auto pushErr = [&]() {
			tmp = node::make<_value>();
			tmp->error = true;
			node::as<_value>(tmp)->token = code;
			pushExpr(tmp);
		};

		// match with sequence::flags bracket numberings
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
				
				// if first, make new evaluation in sequence
				if (emap.empty<evaluation>())
					emap.getmake<sequence>()->items.push_back(
						emap.getmake<evaluation>()
					);

				// add constant to current evaluation
				if (code->type == token::Error)
					emap.getmake<evaluation>()->terms.push_back(
						expr<error>(code)
					);				
				else // value
					emap.getmake<evaluation>()->terms.push_back(
						expr<value>(code)
					);
				
				///////////
				tmp = node::make<_value>();
				tmp->error = code->type == token::Error;
				if (S.top()->flags & _sequence::Parameters)
					tmp->error = true;				
				node::as<_value>(tmp)->token = code;
				pushExpr(tmp);
			}

			// objects --------------------------------------------------------
			else if (length && code->type == token::Symbol) {

				///////////
				tmp = node::make<_object>();
				node::as<_object>(tmp)->symbol = &code->data.symbol;
				pushExpr(tmp);
			}

			// particles ------------------------------------------------------
			else if (length && code->type == token::Particle) {
				switch (code->data.particle) {
					
				// Definition -------------------------------------------------
				case syntax::DefinitionSequenceBegin:
				case syntax::Definition:					
				{					
					tmp = node::make<_definition>();
					auto def = node::as<_definition>(tmp);

					if (!E.top()) {
						def->error = true;
					}
					else { 
						// new definition
						// [optional] start with symbol
						auto head = node::as<_object>(E.top()->terms.front());
						size_t index = 0;
						if (head) {
							def->symbol = head->symbol;
							S.top()->symtable.push_back(def);
							index++;
						}
						
						// then parameter sequences follow
						for (; index < E.top()->terms.size(); index++) {
							auto ptr = E.top()->terms[index];
							auto seq = node::as<_sequence>(ptr);
							if (!seq || !(seq->flags & _sequence::Parameters)) {
								ptr->error = true;
								def->error = true;
							}
							def->parameters.push_back(ptr);
						}

						E.top()->terms.clear();
					}
					pushExpr(tmp);

					def->content = node::make<_sequence>();
					if (code->data.particle == syntax::DefinitionSequenceBegin) {
						S.push(node::as<_sequence>(def->content));
						S.top()->flags |= _sequence::Definition;
						E.push(nullptr);
					} else {
						tmp = node::make<_evaluation>();
						node::as<_sequence>(def->content)->items.push_back(tmp);
						E.top() = node::as<_evaluation>(tmp);
					}
				}
				break;

				// Sub Sequence -----------------------------------------------
				case syntax::SequenceBegin:
				case syntax::ParameterBegin:
					tmp = node::make<_sequence>();
					pushExpr(tmp);
					S.push(node::as<_sequence>(tmp));
					E.push(nullptr);
					// set sequence type
					if (code->data.particle == syntax::ParameterBegin)
						S.top()->flags |= _sequence::Parameters;				
					break;

				case syntax::SequenceEnd:
				case syntax::ParameterEnd:
				case syntax::DefinitionSequenceEnd:					
					if (
						// stack must always have an item remaining
						S.size() == 1
						// match begin-end bracket type
						|| code->data.particle
						!= bracketTypeMap[S.top()->flags & _sequence::BracketMask] 
					) {
						pushErr();
						break;
					}
					S.pop();
					E.pop();
					break;

				// Sequence ---------------------------------------------------
				case syntax::SequenceDivHard:
					S.top()->flags |= _sequence::Explicit;
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