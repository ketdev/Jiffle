#pragma once

#include <string>
#include <memory>
#include <vector>

#include "detail\pos.h"

namespace syntax {
	
	// base -------------------------------------------------------------------

	struct expr abstract {
		typedef std::shared_ptr<expr> ptr;

	public:
		detail::pos pos; // parsing info

	public:
		virtual ~expr() {}
		template<typename T> static ptr make() { return ptr(new T()); }
		template<typename T> T* as() { return dynamic_cast<T*>(this); }
	};

	// node types -------------------------------------------------------------
	
	// values

	struct value abstract : public expr { };
	struct value_error : public value {
		enum type {
			Syntax,
			Custom,
		};
		value_error::type type;
		std::string text;
	};
	struct value_symbol : public expr {
		std::string name;
	};
	struct value_null : public value { };
	struct value_bool : public value {
		bool value;
	};
	struct value_integer : public value {
		long long value;
	};
	struct value_real : public value {
		long double value;
	};
	struct value_string : public value {
		std::string value;
	};

	// constructs

	struct construct abstract : public expr {
		std::vector<expr::ptr> items;
	};

	struct evaluation : public construct {

	};

	struct sequence : public construct {
		enum type {
			Tuple,			// '()' default
			Abstraction,	// '{}' 
			Parameters,		// '[]'
		};

		sequence::type type;
		bool isExplicit; // has separator token
	};

	//struct object : public construct {
	//	std::string name;
	//};

	// read -------------------------------------------------------------------

	expr::ptr read(const std::string& code);

}