#include "syntax.h"
#include <iostream>
#include <sstream>

namespace syntax {
	
	std::string dump(const token& t) {
		std::stringstream ss;
		std::string p;
		
		switch (t.type) {
		case token::Particle:
			p = std::string(1,static_cast<char>(t.data.particle));
			if (p[0] == '\n') p = "\\n";
			ss << "\033[36;22mPARTICLE \033[22;37m[" << p << "]";
			break;
		case token::Comment:
			p = std::string(t.data.comment.text, t.data.comment.length);
			ss << "\033[36;22mCOMMENT \033[22;37m[" << p << "]";
			break;
		case token::Symbol:
			p = std::string(t.data.symbol.name, t.data.symbol.length);
			ss << "\033[36;22mSYMBOL \033[22;37m[" << p << "]";
			break;
		case token::Constant:
			ss << "\033[36;22mCONSTANT \033[22;37m[";
			switch (t.data.constant.type) {
			case token::constant::Null:
				ss << "null";
				break;
			case token::constant::Boolean:
				ss << (t.data.constant.data.boolean?"true":"false");
				break;
			case token::constant::Integer:
				ss << t.data.constant.data.integer;
				break;
			case token::constant::Real:
				ss << t.data.constant.data.real;
				break;
			case token::constant::String:
				p = std::string(t.data.constant.data.string.text, t.data.constant.data.string.length);
				ss << p;
				break;
			default:
				break;
			}
			ss << "]";
			break;
		case token::Error:
			p = std::string(t.data.error.text, t.data.error.length);
			ss << "\033[36;22m" << (t.data.error.user ? "USERERR" : "SYSERR") << " \033[22;37m[" << p << "]";
			break;
		default:
			break;
		}
		return ss.str();
	}

	std::string dump(expr::ptr e, int level) {
		std::stringstream ss("");

		ss << std::string(level * 4, ' ');
		if (e->error) 
			ss << "\033[31;01mERROR ";		

		if (expr::is<value>(e)) {
			auto val = expr::as<value>(e);
			ss << dump(*val->token) << std::endl;
		}
		else if (expr::is<abstraction>(e)) {
			auto abs = expr::as<abstraction>(e);
			ss << "\033[33;01mABSTRACTION ";
			if (abs->symbol)
				ss << "\033[22;37m[" << std::string(abs->symbol->name, abs->symbol->length) << "] ";
			else
				ss << "(ANONYMOUS) ";
			ss << "\033[22;37m[";
			for (size_t i = 0; i < abs->params.size(); i++){
				if (i > 0) ss << ", ";
				auto p = abs->params[i];
				ss << std::string(p->name, p->length);
			}
			ss << "]" << std::endl;
			ss << dump(abs->content, level + 1);
		}
		else if (expr::is<evaluation>(e)) {
			auto eval = expr::as<evaluation>(e);
			ss << "\033[34;01mEVALUATION " << std::endl;
			for each (auto e in eval->terms)
				ss << dump(e,level+1);
		}
		else if (expr::is<sequence>(e)) {
			auto seq = expr::as<sequence>(e);
			ss << "\033[32;22mSEQUENCE ";
			if (seq->flags & sequence::Explicit)
				ss << "(EXPLICIT) ";
			if (seq->flags & sequence::Abstraction)
				ss << "(ABSTRACTION) ";
			ss << std::endl;
			for each (auto e in seq->items)
				ss << dump(e, level + 1);
		}

		return ss.str();
	}

}