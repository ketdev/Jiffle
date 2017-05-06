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

	std::string dump(node::ptr e, int level) {
		if (!e) return "";
		std::stringstream ss("");
		auto indent = std::string(level * 4, ' ');

		ss << indent;
		if (e->error) 
			ss << "\033[31;01mERROR ";		

		if (node::is<_value>(e)) {
			auto val = node::as<_value>(e);
			ss << dump(*val->token) << std::endl;
		}
		else if (node::is<_object>(e)) {
			auto obj = node::as<_object>(e);
			ss << "\033[32;01mOBJECT ";
			ss << "\033[22;37m[" << std::string(obj->symbol->name, obj->symbol->length) << "] ";
			ss << std::endl;
		}
		else if (node::is<_definition>(e)) {
			auto def = node::as<_definition>(e);
			ss << "\033[33;01mDEFINITION ";
			if (def->symbol)
				ss << "\033[22;37m[" << std::string(def->symbol->name, def->symbol->length) << "] ";
			else
				ss << "(ANONYMOUS) ";
			ss << std::endl;
			for (size_t i = 0; i < def->parameters.size(); i++){
				auto p = def->parameters[i];
				ss << dump(p, level+1);
			}
			ss << dump(def->content, level + 1);
		}
		else if (node::is<_evaluation>(e)) {
			auto eval = node::as<_evaluation>(e);
			ss << "\033[34;01mEVALUATION " << std::endl;
			for each (auto e in eval->terms)
				ss << dump(e,level+1);
		}
		else if (node::is<_sequence>(e)) {
			auto seq = node::as<_sequence>(e);
			ss << "\033[35;01mSEQUENCE ";
			if (seq->flags & _sequence::Definition)
				ss << "(DEFINITION) ";
			if (seq->flags & _sequence::Parameters)
				ss << "(PARAMETERS) ";
			if (seq->flags & _sequence::Explicit)
				ss << "(EXPLICIT) ";
			ss << std::endl;
			ss << indent << "\033[35;22mSYMBOLS [";
			for each (auto def in seq->symtable) {
				ss << std::string(def->symbol->name, def->symbol->length) << " ";
			}
			ss << "]" << std::endl;
			for each (auto e in seq->items)
				ss << dump(e, level + 1);
		}

		return ss.str();
	}

}