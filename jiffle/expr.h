#pragma once

#include "token.h"
#include <memory>

namespace syntax {

	// expressions ------------------------------------------------------------

	struct expr {
		typedef std::shared_ptr<expr> ptr;		
		
	public:
		template<typename T>
		static ptr make() {
			return ptr(new T());
		}		
		
		template<typename T>
		static T* as(ptr p) {
			return dynamic_cast<T*>(p.get());
		}

		template<typename T>
		static bool is(ptr p) {
			return dynamic_cast<T*>(p.get()) != nullptr;
		}

	protected:
		expr() {}
		virtual ~expr() {};
	};
	
	struct value : public expr {
		const syntax::token *token;
	};
	struct abstraction : public expr {
		syntax::token::symbol name;
		std::vector<syntax::token::symbol> params;
		expr::ptr content;
	};
	struct evaluation : public expr {
		std::vector<expr::ptr> terms;
	};
	struct sequence : public expr {
		bool isExplicit;
		std::vector<expr::ptr> items;

		sequence() : isExplicit(false) {}
	};	

	// parse ------------------------------------------------------------------

	expr::ptr parse(const tokens& sourcecode);
}