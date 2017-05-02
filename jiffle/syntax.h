#pragma once

#include <string>
#include <vector>
#include <memory>

namespace syntax {

	// tokenize ---------------------------------------------------------------

	bool isWhitespace(char token); // ' ' evaluation structure
	bool isLetter(char token); // [a-zA-Z_]
	bool isDigit(char token, int base = 10); // 2:(0-1), 8:(0-7), 10:(0-9), 16:(0-F) 
	
	// builtin syntax tokens
	enum particle : char {
		// Sequence
		SequenceDivHard = ',',	// value sequence divider within same line
		SequenceDivSoft = '\n',	// value sequence divider on multiple lines

		// Sub Sequence
		SequenceBegin = '(',	// value sequence starting token (each scope is an implicit sequence)
		SequenceEnd = ')',	// value sequence ending token

		// Definition
		Definition = '=',		// defines an abstraction 
		DefinitionSequenceBegin = '{',	// definition + sequence begin '=('
		DefinitionSequenceEnd = '}',	// definition + sequence end ')'

		// Parameter
		ParameterBegin = '[',	// start of argument group for definitions
		ParameterEnd = ']',		// end of arguments group for definitions
		

		//Reference = '&',		// reference a symbol without evaluating

		// '\' escape newline separator
		// '<-' forked abstraction (for each in sequence, original order is maintained)
		// '|' variance (polymorphism)
		// '..' range (variance for numbers and characters) (sequence for list indices)
		// ':' specification
		// '.' composition ('.foo' makes it private to scope)
		// '->' flow mapping (if lhs is evaluated, then compute rhs)
		// '@' get index in sequence
		// '~' negation (variation inversion)
		// '$' reflexive (updateable state)
		// '%' abstract (extern)
	};

	// tokens are as equivalent to the source code, giving meaning
	// to different syntax constructs, for syntax highlighting and easier parsing
	struct token {
		enum type {
			Particle,	// builtin syntax characters, see above
			Comment,	// user comments and documentations
			Symbol,		// abstraction terms
			Constant,	// null, true, false, integers, reals, strings
			Error,		// user error type, or invalid tokens
		};
		struct comment {
			static const char BeginToken = '#';
			static const char EndToken = '\n';

			bool afterCode;
			const char* text;
			int length;
		};
		struct constant {
			enum type {
				Null,		// 'null'
				Boolean,	// 'true', 'false'
				Integer,	// integer: [0-9]+ | 0[xX][0-9A-F]+ | 0[oO][0-7]+ | 0[bB][0-1]+
				Real,		// real: [0-9]+'.'[0-9]+ | [0-9]+('.'[0-9]+])?e['-''+']?[0-9]+
				String,		// '...' (escaped and formated)
			};
			struct string {
				static const char Token = '\'';

				const char* text;
				int length;
			};
			union data {
				bool boolean;
				long long integer;
				long double real;
				constant::string string;
			};

			constant::type type;
			constant::data data;
		};
		struct symbol {
			const char* name;
			int length;
		};
		struct error {
			static const char Token = '`';

			bool user; // if explicitly generated
			const char* text;
			int length;
		};
		union data {
			syntax::particle particle;
			token::comment comment;
			token::constant constant;
			token::symbol symbol;
			token::error error;
		};

		token::type type;
		token::data data;
	};

	typedef std::vector<token> tokens;
	tokens tokenize(const std::string& input);
	
	// expressions ------------------------------------------------------------
	
	extern int _uid;
	typedef std::shared_ptr<void> handle;
	template<typename T>
	struct expr {

		expr() : _ptr(new T){ }
		template<typename ...Args>
		expr(Args... args) : _ptr(new T(args...)) { }
		expr(handle ptr) : _ptr(ptr){}

		static int uid() {
			static int id = _uid++;
			return id;
		}
		T* operator->() {
			return reinterpret_cast<T*>(_ptr.get());
		}
		operator handle() {
			return _ptr;
		}
	private:
		handle _ptr;
	};

	// --

	struct error {
		const syntax::token *token;
		error(const syntax::token *token = nullptr) : token(token) {}
	};
	struct value {
		const syntax::token *token;
		value(const syntax::token *token = nullptr) : token(token) {}
	};
	struct sequence {
		enum type {
			Tuple,			// '()' default
			TupleExplicit,	// '(,)'
			Abstraction,	// '{}' 
			Parameters,		// '[]'
		};
		sequence::type type;
		std::vector<handle> items;

		sequence() : type(Tuple) {}
	};
	struct evaluation {
		std::vector<handle> terms;
	};


	////////////

	struct node {
		typedef std::shared_ptr<node> ptr;

		bool error;

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
		node() :error(false) {}
		virtual ~node() {};
	};


	struct _value : public node {
		const syntax::token *token;

		_value(const syntax::token *token = nullptr) :token(token) {}
	};
	struct _object : public node {
		const syntax::token::symbol *symbol;

		_object() :symbol(nullptr) {}
	};
	struct _evaluation : public node {
		std::vector<node::ptr> terms;
	};
	struct _definition : public node {
		const syntax::token::symbol *symbol;
		std::vector<node::ptr> parameters;
		node::ptr content;

		_definition() :symbol(nullptr) {}
	};
	struct _sequence : public node {
		enum flags{
			// bracket types are mutually exclusive
			None		= 0,	// uses '()' by default
			Definition  = 1,	// uses '{}' instead
			Parameters	= 2,	// uses '[]' instead
			BracketMask = 3,	// type of brackets

			// other properties
			Explicit = 4,		// has ','
		};
		int flags;
		std::vector<node::ptr> items;

		std::vector<const _definition*> symtable;

		_sequence() : flags(None) {}
	};
	
	// parse ------------------------------------------------------------------

	node::ptr parse(const tokens& sourcecode);

	// dump -------------------------------------------------------------------

	std::string dump(const token& t);
	std::string dump(node::ptr e, int level = 0);

	// evaluate ---------------------------------------------------------------

	node::ptr eval(const node::ptr& expr);

}