#include "token.h"

namespace syntax {
	namespace detail {
		
		static bool isWhitespace(char token) {
			return (token == ' '
				|| token == 0xB
				|| token == 0xC
				|| token == 0xD // CR
				|| token == 0x9 // TAB
				);
		}
		static bool isLetter(char token) {
			return (token >= 'A' && token <= 'Z')
				|| (token >= 'a' && token <= 'z')
				|| (token == '_');
		}
		static bool isDigit(char token, int base = 10) {
			switch (base) {
			case 2:
				return token >= '0' && token <= '1';
			case 8:
				return token >= '0' && token <= '7';
			case 16:
				return (token >= '0' && token <= '9')
					|| (token >= 'A' && token <= 'F')
					|| (token >= 'a' && token <= 'f');
			default:
				return token >= '0' && token <= '9';
			}
		}


#define C (src[cur.ch])
#define GOOD (cur.ch < length)

		std::vector<token> tokenize(const std::string& code) {
			std::vector<token> tokens;
			
			struct detail::pos cur = {}, start = {};
			const char* src = &code[0];
			size_t length = code.length();

			auto shift = [&]() {
				cur.ch++;
				cur.col++;
				if (cur.ch < length && code[cur.ch] == token::Newline) {
					cur.col = 0;
					cur.ln++;
				}
			};
			auto reset = [&]() {
				start = cur;
			};
			auto push = [&](enum token::type type) {
				token t = {};
				t.type = type;
				t.pos = start;
				t.pos.len = cur.ch - start.ch;
				tokens.push_back(t);
				reset();
			};

			while (GOOD) {
				// skip whitespace --------------------------------------------
				while (GOOD && isWhitespace(C)) {
					shift();
					reset();
				}

				// particles --------------------------------------------------
				if (GOOD) {
					bool handled = true;
					switch (C) {
					case token::TokenSequenceSeparator:
						shift();
						push(token::SequenceSeparator);
						break;
					case token::TokenSequenceSeparatorImplicit:
						shift();
						push(token::SequenceSeparatorImplicit);
						break;
					default:
						handled = false;
						break;
					}
					if (handled)
						continue;
				}

				// comment ----------------------------------------------------
				if (GOOD && C == token::TokenComment) {
					shift();
					while (GOOD && C != token::Newline)
						shift();
					push(token::Comment);
					continue;
				}

				// identifier -------------------------------------------------
				if (GOOD && isLetter(C)) {
					while (length && (isLetter(C) || isDigit(C)))
						shift();
					// keywords -----------------------------------------------
					auto str = code.substr(start.ch, cur.ch - start.ch);
					if (str == token::KeywordNull)
						push(token::Null);
					else if (str == token::KeywordTrue)
						push(token::True);
					else if (str == token::KeywordFalse)
						push(token::False);
					else
						push(token::Symbol);
					continue;
				}

				// number -----------------------------------------------------
				if (GOOD && isDigit(C)) {
					auto real = false; // integer by default
					auto base = 10; // default base
					// detect base
					if (C == '0') {
						shift();
						if (GOOD) {
							if (C == 'X' || C == 'x')
								base = 16; // hex						
							if (C == 'O' || C == 'o')
								base = 8; // oct						
							if (C == 'B' || C == 'b')
								base = 2; // bin						
							if (base != 10) {
								shift();
							}
							// syntax error
							if (!GOOD || !(isDigit(C, base) || C == '.' || C == 'e' || C == 'E')) {
								push(token::SyntaxError);
								continue;
							}
						}
					}
					// consume digits
					while (GOOD && isDigit(C, base))
						shift();
					// real decimal point
					if (base == 10 && GOOD && C == '.') {
						shift();
						real = true;
						while (GOOD && isDigit(C))
							shift();
					}
					// real exponent
					if (base == 10 && GOOD && (C == 'e' || C == 'E')) {
						shift();
						real = true;
						if (GOOD && (C == '-' || C == '+'))
							shift();
						while (GOOD && isDigit(C))
							shift();
					}

					// integer 
					if (!real) {
						auto s = (base == 10) ? start.ch : start.ch + 2;
						auto str = code.substr(s, cur.ch - s);
						push(token::Integer);
						if (!str.empty())
							tokens.back().number.integer = std::stoll(str, NULL, base);
						continue;
					}
					// real 
					else {
						auto str = code.substr(start.ch, cur.ch - start.ch);
						push(token::Real);
						tokens.back().number.real = std::stold(str, NULL);
						continue;
					}
				}

				// string -----------------------------------------------------
				if (GOOD && C == token::TokenString) {
					shift();
					while (GOOD && C != token::TokenString)
						shift();

					// TODO: escape sequences

					if (GOOD) {
						shift();
						push(token::String);
						continue;
					}
				}

				// error (user) -----------------------------------------------
				if (GOOD && C == token::TokenError) {
					shift();
					while (GOOD && C != token::TokenError)
						shift();
					if (GOOD) {
						shift();
						push(token::UserError);
						continue;
					}
				}

				// error (default) --------------------------------------------
				if (GOOD && cur.ch == start.ch)
					shift();
				if(cur.ch != start.ch)
					push(token::SyntaxError);
			}

			return tokens;
		}

	}
}