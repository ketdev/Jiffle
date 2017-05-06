#include "syntax.h"

#include <string>
#include <map>

namespace jiffle {
	namespace syntax {

		std::vector<token> tokenize(const std::string & code) {
			// internal state -------------------------------------------------
			std::vector<token> _tdata;
			pos _cur = {}, _start = {};
			char _c = code[0];
			std::string _buffer;
			std::map<std::string, type> _keywords;

			// keywords -------------------------------------------------------
			_keywords[KeywordNull] = Null;
			_keywords[KeywordTrue] = True;
			_keywords[KeywordFalse] = False;

			// methods --------------------------------------------------------

			// stateless
			auto isWhitespace = [](char c) {
				return (c == ' '
					|| c == 0xB
					|| c == 0xC
					|| c == 0xD // CR
					|| c == 0x9 // TAB
					);
			};
			auto isLetter = [](char c) {
				return (c >= 'A' && c <= 'Z')
					|| (c >= 'a' && c <= 'z')
					|| (c == '_');
			};
			auto isDigit = [](char c, int base = 10) {
				switch (base) {
				case 2:
					return c >= '0' && c <= '1';
				case 8:
					return c >= '0' && c <= '7';
				case 16:
					return (c >= '0' && c <= '9')
						|| (c >= 'A' && c <= 'F')
						|| (c >= 'a' && c <= 'f');
				default:
					return c >= '0' && c <= '9';
				}
			};
			auto isParticle = [](char c) {
				switch (static_cast<type>(c)) {
				case Separator:
				case SeparatorImplicit:
				case Definition:
				case SequenceStart:
				case SequenceEnd:
				case DefinitionStart:
				case DefinitionEnd:
				case ParameterStart:
				case ParameterEnd:
					return true;
				default:
					return false;
				}
			};

			// statefull
			auto shift = [&]() {
				_buffer.push_back(_c);
				_cur.ch++;
				_cur.col++;
				if (_c == syntax::Newline) {
					_cur.col = 0;
					_cur.ln++;
				}
				_c = _cur.ch < code.length() ? code[_cur.ch] : 0;
			};
			auto reset = [&]() {
				_buffer.clear();
				_start = _cur;
			};
			auto skip = [&]() {
				shift();
				reset();
			};
			auto push = [&](type t) {
				token td = {
					t,
					_start,
					{}
				};
				td.pos.len = _cur.ch - _start.ch;
				_tdata.push_back(td);
				reset();
			};
			auto pushInt = [&](long long i) {
				push(type::Integer);
				_tdata.back().value.integer = i;
			};
			auto pushReal = [&](long double r) {
				push(type::Real);
				_tdata.back().value.real = r;
			};
			

			// entry ----------------------------------------------------------
			while (true) {

				// skip whitespace --------------------------------------------
				while (isWhitespace(_c))
					skip();

				// particles --------------------------------------------------
				if (isParticle(_c)) {
					auto t = static_cast<type>(_c);
					shift();
					push(t);
					continue;
				}

				// comment ----------------------------------------------------
				if (_c == type::Comment) {
					shift();
					while (_c && _c != syntax::Newline)
						shift();
					push(type::Comment);
					continue;
				}

				// identifier -------------------------------------------------
				if (isLetter(_c)) {
					while (isLetter(_c) || isDigit(_c))
						shift();
					auto it = _keywords.find(_buffer);

					// keywords -----------------------------------------------
					if (it != _keywords.end())
						push(it->second);
					// symbol -----------------------------------------------------
					else
						push(type::Symbol);
					continue;
				}

				// number -----------------------------------------------------
				if (isDigit(_c)) {
					auto real = false; // integer by default
					auto base = 10; 
					// detect base
					if (_c == '0') {
						shift();
						if (_c == 'X' || _c == 'x')
							base = 16; // hex						
						if (_c == 'O' || _c == 'o')
							base = 8; // oct						
						if (_c == 'B' || _c == 'b')
							base = 2; // bin						
						if (base != 10) {
							shift();
							// syntax error if no followup
							if (!isDigit(_c, base) && _c != '.' && _c != 'e' && _c != 'E') {
								push(type::SyntaxError);
								continue;
							}
						}
					}
					// skip base in integer parsing
					_buffer.clear();

					// consume digits
					while (isDigit(_c, base))
						shift();

					// real decimal point
					if (base == 10 && _c == '.') {
						shift();
						real = true;
						while (isDigit(_c))
							shift();
					}
					// real exponent
					if (base == 10 && (_c == 'e' || _c == 'E')) {
						shift();
						real = true;
						if (_c == '-' || _c == '+')
							shift();
						while (isDigit(_c))
							shift();
					}

					// integer 
					if (!real) {
						auto i = _buffer.empty() ? 0 : std::stoll(_buffer, NULL, base);
						pushInt(i);
						continue;
					}
					// real 
					else {
						auto r = std::stold(_buffer, NULL);
						pushReal(r);
						continue;
					}
				}

				// string -----------------------------------------------------
				if (_c == type::String) {
					shift();
					while (_c && _c != type::String)
						shift();

					// TODO: escape sequences

					if (_c) {
						shift();
						push(type::String);
						continue;
					}
				}

				// error (user) -----------------------------------------------
				if (_c == type::UserError) {
					shift();
					while (_c && _c != type::UserError)
						shift();
					if (_c) {
						shift();
						push(type::UserError);
						continue;
					}
				}

				// error (default) --------------------------------------------
				if (_c && _cur.ch == _start.ch)
					shift();
				if (_cur.ch != _start.ch) {
					push(type::SyntaxError);
					continue;
				}

				// end
				break;
			}
			return _tdata;
		}

	}
}