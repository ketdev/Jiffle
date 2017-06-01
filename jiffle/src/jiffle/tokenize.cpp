#include "jiffle.h"

#include <string>
#include <map>


namespace jiffle {
		
	static bool _isDigit(char c, int base = 10) {
		switch (base) {
		case 2:
			return c >= '0' && c <= '1';
		case 8:
			return c >= '0' && c <= '7';
		case 16:
			return (c >= '0' && c <= '9')
				|| (c >= 'A' && c <= 'F')
				|| (c >= 'a' && c <= 'f');
		default: // 10
			return c >= '0' && c <= '9';
		}
	};
	static bool _isLetter(char c) {
		return (c >= 'A' && c <= 'Z')
			|| (c >= 'a' && c <= 'z')
			|| (c == '_');
	}
				
	// parsers ----------------------------------------------------------------
		
	typedef bool(*try_parse_t)(const char* code, size_t length, const pos& cur, data& out);

	static bool _tpNull(const char* code, size_t length, const pos& cur, data& out) {
		static const char* const Keyword = "null";
		static const int Length = 4;

		if (length >= Length && memcmp(code, Keyword, Length) == 0) {
			out = {};
			out.category = Primitive;
			out.primitive = Null;
			out.pos = cur;
			out.pos.len = Length;
			return true;
		}
		return false;
	}
	static bool _tpFalse(const char* code, size_t length, const pos& cur, data& out) {
		static const char* const Keyword = "false";
		static const int Length = 5;

		if (length >= Length && memcmp(code, Keyword, Length) == 0) {
			out = {};
			out.category = Primitive;
			out.primitive = False;
			out.pos = cur;
			out.pos.len = Length;
			return true;
		}
		return false;
	}
	static bool _tpTrue(const char* code, size_t length, const pos& cur, data& out) {
		static const char* const Keyword = "true";
		static const int Length = 4;

		if (length >= Length && memcmp(code, Keyword, Length) == 0) {
			out = {};
			out.category = Primitive;
			out.primitive = True;
			out.pos = cur;
			out.pos.len = Length;
			return true;
		}
		return false;
	}
	static bool _tpNumber(const char* code, size_t length, const pos& cur, data& out) {
		if (length > 0 && _isDigit(code[0])) {
			int base = 10;

			// base
			if (length > 1 && code[0] == '0') {
				if (code[1] == 'X' || code[1] == 'x')
					base = 16; // hex						
				if (code[1] == 'O' || code[1] == 'o')
					base = 8; // oct						
				if (code[1] == 'B' || code[1] == 'b')
					base = 2; // bin			
			}
			// must have followup if base is defined
			if (base != 10 && (length < 3 || !_isDigit(code[2], base)))
				return false;
			
			// starting index
			size_t index = base != 10 ? 2 : 0;

			// consume digits
			while (index < length && _isDigit(code[index], base))
				index++;
				
			// floating point
			if (base == 10) {
				// decimal point
				if (index < length && code[index] == '.') {
					index++;
					// consume digits
					while (index < length && _isDigit(code[index]))
						index++;
				}
				// exponent
				if (index < length && (code[index] == 'e' || code[index] == 'E')) {
					index++;
					// sign
					if (index < length && (code[index] == '-' || code[index] == '+'))
						index++;
					// must have followup digits
					if (index >= length) 
						return false;
					// consume digits
					while (index < length && _isDigit(code[index]))
						index++;
				}
			}

			// set output
			out = {};
			out.category = Primitive;
			out.primitive = Number;
			out.pos = cur;
			out.pos.len = index;
			out.text = std::string(code, index);
			return true;
		}
		return false;
	}
	static bool _tpString(const char* code, size_t length, const pos& cur, data& out) {
		static const char StringDelim = '\'';
		size_t index = 0;
		// string has at least 2 characters: ''
		if (length > 1 && code[0] == StringDelim) {
			index++;
			// consume string
			while (index < length && code[index] != StringDelim)
				index++;

			// TODO: escape sequences

			if (index < length && code[index] == StringDelim) {
				index++;

				// set output
				out = {};
				out.category = Primitive;
				out.primitive = String;
				out.pos = cur;
				out.pos.len = index;
				out.text = std::string(code + 1, index - 2);
				return true;
			}
		}
		return false;
	}
	static bool _tpSymbol(const char* code, size_t length, const pos& cur, data& out) {
		size_t index = 0;
		if (index < length && _isLetter(code[index])) {
			// consume letters
			while (index < length && (_isLetter(code[index]) || _isDigit(code[index])))
				index++;

			// set output
			out = {};
			out.category = Primitive;
			out.primitive = Symbol;
			out.pos = cur;
			out.pos.len = index;
			out.text = std::string(code, index);
			return true;
		}
		return false;
	}
	static bool _tpComment(const char* code, size_t length, const pos& cur, data& out) {
		static const char CommentStart = '#';
		static const char CommentEnd = '\n';

		size_t index = 0;
		if (index < length && code[index] == CommentStart) {
			// consume comment line
			while (index < length &&  code[index] != CommentEnd)
				index++;

			// set output
			out = {};
			out.category = Primitive;
			out.primitive = Comment;
			out.pos = cur;
			out.pos.len = index;
			out.text = std::string(code + 1, index - 1);
			return true;
		}
		return false;
	}
	static bool _tpUserError(const char* code, size_t length, const pos& cur, data& out) {
		static const char ErrorDelim = '`';
		size_t index = 0;
		// string has at least 2 characters: ''
		if (length > 1 && code[0] == ErrorDelim) {
			index++;
			// consume error string
			while (index < length && code[index] != ErrorDelim)
				index++;

			if (index < length && code[index] == ErrorDelim) {
				index++;

				// set output
				out = {};
				out.category = Error;
				out.error = UserError;
				out.pos = cur;
				out.pos.len = index;
				out.text = std::string(code + 1, index - 2);
				return true;
			}
		}
		return false;
	}
	static bool _tpParticle(const char* code, size_t length, const pos& cur, data& out) {
		size_t index = 0;
		if (index < length) {

			// whitespace
			while (index < length && ( false 
				|| code[index] == ' '
				|| code[index] == '\v'
				|| code[index] == '\f'
				|| code[index] == '\r' // CR
				|| code[index] == '\t' // TAB
				))
				index++;
			if (index > 0) {
				out = {};
				out.category = Particle;
				out.particle = Whitespace;
				out.pos = cur;
				out.pos.len = index;
				return true;
			}

			particle p = {};

			// arithmetic operators
			index = 1;
			switch (code[0]) {
			case '*':	p = OpMultiply;		break;
			case '/':	p = OpDivide;		break;
			case '%':	p = OpModulus;		break;
			case '+':	p = OpAddition;		break;
			case '-':	p = OpSubtraction;	break;
			default:						break;
			}

			// output particle
			if (p != particle{}) {
				out = {};
				out.category = Particle;
				out.particle = p;
				out.pos = cur;
				out.pos.len = 1;
				return true;
			}


			p = static_cast<particle>(code[0]);
			switch (p) {
			case Separator:
			case SeparatorImplicit:
			case DefinitionAssign:
			case SequenceStart:
			case SequenceEnd:
			case DefinitionStart:
			case DefinitionEnd:
			case ParameterStart:
			case ParameterEnd:
				out = {};
				out.category = Particle;
				out.particle = p;
				out.pos = cur;
				out.pos.len = 1;
				return true;
			default:
				break;
			}
		}
		return false;			
	}

	// tokenize ---------------------------------------------------------------

	std::vector<data> tokenize(const std::string & code) {

		// parsing order
		static const try_parse_t parsers[] = {
			_tpParticle,	_tpComment,		_tpNull,
			_tpFalse,		_tpTrue,		_tpSymbol,
			_tpNumber,		_tpString,		_tpUserError,
		};

		const char* ptr = &code[0];
		const char* ptrend = ptr + code.length();

		std::vector<data> tokens;
		data d;
		pos cur = {};
			
		while (ptr < ptrend){

			// parse (error by default)
			d = {};
			d.category = Error;
			d.error = SyntaxError;
			d.pos = cur;
			for (size_t i = 0; i < sizeof(parsers) / sizeof(try_parse_t); i++) {
				if (parsers[i](ptr, ptrend - ptr, cur, d))
					break;
			}
			// error until next particle or eof
			if (d.pos.len == 0) {
				int len = 0;
				data p;
				while (ptr + len < ptrend
					&& !_tpParticle(ptr + len, ptrend - ptr - len, cur, p)) {
					len++;
				}
				d.pos.len = len;
				d.text = std::string(ptr, d.pos.len);
			}
				
			// update position
			auto shift = d.pos.len;
			cur.ch += shift;
			cur.col += shift;
			if (ptr < ptrend && *ptr == '\n') {
				// @note: we assume that the fact that a newline 
				// is a new (single char) token won't change
				// (veeery unlikely, not sure why I even wrote this comment)
				cur.ln++;
				cur.col = 0;
			}
			ptr += shift;
			
			// push 
			tokens.push_back(d);
		}
		return tokens;			
	}

}