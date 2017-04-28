#include "token.h"
#include <string>

namespace syntax {

	template<typename T>
	static token makeToken(enum token::type type, T data) {
		token t;
		t.type = type;
		*reinterpret_cast<T*>(&t.data) = data;
		return t;
	}
#define shift() do{ length--; src++; }while(0);
#define push(type,data) tokens.push_back(makeToken(type,data));

	tokens tokenize(const std::string& input) {
		const char* src = &input[0];
		size_t length = input.length();
		tokens tokens;
		
		while (length > 0) {
			// skip whitespaces -----------------------------------------------
			if (syntax::isWhitespace(*src)) {
				shift();
				continue;
			}

			// comment --------------------------------------------------------
			if (length && *src == '#') {
				shift();
				auto start = src;
				while (length && *src != syntax::Newline)
					shift();
				push(token::Comment, (token::comment{ false, start, src - start }));
				continue;
			}

			// identifier -----------------------------------------------------
			if (length && isLetter(*src)) {
				auto start = src;
				while (length && (isLetter(*src) || isDigit(*src)))
					shift();				
				auto len = src - start;
				token::constant c;
				// null
				if (len == 4 && !std::memcmp("null", start, len)) {					
					c.type = token::constant::Null;
				}
				// bool: true
				else if (len == 4 && !std::memcmp("true", start, len)) {
					c.type = token::constant::Boolean;
					c.data.boolean = true;
				}
				// bool: false
				else if (len == 5 && !std::memcmp("false", start, len)) {
					c.type = token::constant::Boolean;
					c.data.boolean = false;
				}
				// symbol
				else {
					push(token::Symbol, (token::symbol{ start, src - start }));
					continue;
				}
				push(token::Constant, c);
				continue;
			}

			// string ---------------------------------------------------------
			if (length && *src == '\'') {
				shift();

				// TODO: escape sequences

				auto start = src;
				while (length && *src != '\'')
					shift();
				
				if (length) {
					token::constant cstr;
					cstr.type = token::constant::String;
					cstr.data.string.text = start;
					cstr.data.string.length = src - start;
					push(token::Constant, cstr);
					shift();
					continue;
				}
			}

			// number ---------------------------------------------------------
			if (length && isDigit(*src)) {
				auto start = src;
				// integer by default
				bool real = false;
				int base = 10;
				// detect hex/oct/bin
				if (*src == '0') {
					shift();
					if (length) {
						if (*src == 'X' || *src == 'x')
							base = 16; // hex						
						if (*src == 'O' || *src == 'o')
							base = 8; // oct						
						if (*src == 'B' || *src == 'b')
							base = 2; // bin						
						if (base != 10) {
							shift();
							start += 2;
						}
					}
				}
				// consume digits
				while (length && isDigit(*src, base))
					shift();

				// real decimal point
				if (base == 10 && length && *src == '.') {
					shift();
					real = true;
					while (length && isDigit(*src))
						shift();
				}
				// real exponent
				if (base == 10 && length && (*src == 'e' || *src == 'E')) {
					shift();
					real = true;
					if (length && (*src == '-' || *src == '+'))
						shift();					
					while (length && isDigit(*src))
						shift();					
				}

				// syntax error
				auto rep = std::string(start, src - start);
				if (!rep.length()) {
					auto nstart = (base != 10) ? start - 2 : start;
					push(token::Error, (token::error{ false, nstart, src - nstart }));
					continue;
				}
				// integer 
				else if (!real) {
					token::constant cint;
					cint.type = token::constant::Integer;
					cint.data.integer = std::stoll(rep, NULL, base);
					push(token::Constant, cint);
					continue;
				}
				// real 
				else {
					token::constant creal;
					creal.type = token::constant::Real;
					creal.data.real = std::stold(rep, NULL);
					push(token::Constant, creal);
					continue;
				}
			}

			// error ----------------------------------------------------------
			if (length && *src == '`') {
				shift();
				auto start = src;
				while (length && *src != '`')
					shift();
				push(token::Error, (token::error{ length > 0, start,src - start }));
				if (length)
					shift();				
				continue;
			}

			// particle -------------------------------------------------------
			switch (*src) {
			case syntax::ParamBegin:
			case syntax::ParamEnd:
			case syntax::SequenceBegin:
			case syntax::SequenceEnd:
			case syntax::SequenceDivSoft:
			case syntax::SequenceDivHard:
			case syntax::Abstraction:
				push(token::Particle, (particle)*src);
				break;
			default:
				push(token::Error, (token::error{ false, src, 1 }));
				break;
			}
			shift();
			continue;
		}

		return tokens;
	}

}
