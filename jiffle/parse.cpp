#include <iostream>
#include <vector>
#include <cstdlib>
#include "parse.h"

memory parse(const std::string& sourcecode) {
	memory code;
	value* v = nullptr;

	int length = sourcecode.size();
	int start = 0;
	int index = -1;
	char token, peek;

	// helper functions

	auto shift = [&]() {
		index++;
		token = sourcecode[index];
		peek = index + 1 < length ? sourcecode[index + 1] : 0;
	};
	auto end = [&]() {
		start = index;
	};
	auto pushParticle = [&]() {
		v = code.alloc();
		v->type = value::type::TParticle;
		v->data.particle = (value::particle)token;
		shift();
		end();
	};

	// parse loop
	
	// TODO: validate correct parenthesis

	shift();
	while (index < length) {
		// whitespaces
		if ( token == ' '
		  || token == 0xB
		  || token == 0xC
		  || token == 0xD // CR
		  || token == 0x9 // TAB
		  ) {
			shift();
			end();
			continue;
		}
		
		
		//if ( 
		//	// separator
		//	 token == value::particle::PSeparatorML 
		//  || token == value::particle::PSeparatorSL
		//	// sub sequence
		//  || token == value::particle::PSequenceBegin
		//  || token == value::particle::PSequenceEnd
		//  ) {
		//	pushParticle();
		//	continue;
		//}
		
		// comments
		if (token == syntax::PComment) {
			shift();
			end();
			// comment
			while (token != syntax::Newline && index < length)
				shift();
			v = code.alloc();
			v->type = value::type::TComment;
			v->data.comment.text = &sourcecode[start];
			v->data.comment.length = index - start;
			end();
			continue;
		}

		// user error
		if (token == '`') {
			shift();
			end();

			// error
			while (token != '`' && index < length)
				shift();
			v = code.alloc();
			v->type = value::type::TError;
			v->data.error.index = start;
			v->data.error.user = true;
			v->data.error.text = &sourcecode[start];
			v->data.error.length = index - start;
			end();

			shift();
			end();
			continue;
		}

		// strings
		if (token == '\'') {
			shift();
			end();

			// TODO: escape sequences

			// string
			while (token != '\'' && index < length)
				shift();
			v = code.alloc();
			v->type = value::type::TString;
			v->data.string.text = &sourcecode[start];
			v->data.string.length = index - start;
			end();

			shift();
			end();
			continue;
		}


		// identifier | null | bool
		if ((token >= 'A' && token <= 'Z')
			|| (token >= 'a' && token <= 'z')
			|| (token == '_')) {
			while (((token >= '0' && token <= '9')
				|| (token >= 'A' && token <= 'Z')
				|| (token >= 'a' && token <= 'z')
				|| (token == '_')) && index < length) {
				shift();
			}

			auto name = &sourcecode[start];
			auto length = index - start;

			// null
			if (length == 4 && std::memcmp("null", name, length) == 0) {
				v = code.alloc();
				v->type = value::type::TNull;
				end();
			}

			// bool: true
			else if (length == 4 && std::memcmp("true", name, length) == 0) {
				v = code.alloc();
				v->type = value::type::TBoolean;
				v->data.boolean.value = true;
				end();
			}

			// bool: false
			else if (length == 5 && std::memcmp("false", name, length) == 0) {
				v = code.alloc();
				v->type = value::type::TBoolean;
				v->data.boolean.value = false;
				end();
			}

			// symbol
			else {
				v = code.alloc();
				v->type = value::type::TSymbol;
				v->data.symbol.name = name;
				v->data.symbol.length = length;
				end();
			}			
			continue;
		}

		// number | real
		if (token >= '0' && token <= '9') {
			// number by default
			bool real = false;
			bool numParsed = false;
			int base = 10;
			// detect hex/oct/bin
			if (token == '0') { 
				shift();
				if ((token == 'X' || token == 'x')) { // hex
					shift();
					base = 16;
					while (((token >= '0' && token <= '9')
						|| (token >= 'A' && token <= 'F')
						|| (token >= 'a' && token <= 'f')) && index < length) {
						shift();
					}
					numParsed = true;
				}
				else if ((token == 'O' || token == 'o')) { // oct
					shift();
					base = 8;
					while ((token >= '0' && token <= '7') && index < length) {
						shift();
					}
					numParsed = true;
				}
				else if ((token == 'B' || token == 'b')) { // bin	
					shift();
					base = 2;
					while ((token >= '0' && token <= '1') && index < length) {
						shift();
					}
					numParsed = true;
				}
			}
			// consume digits
			while (!numParsed && (token >= '0' && token <= '9') && index < length) {
				shift();
			}
			// real decimal point
			if (!numParsed && token == '.') {
				real = true;
				shift();
				while ((token >= '0' && token <= '9') && index < length) {
					shift();
				}
			}
			// real exponent
			if (!numParsed && (token == 'e' || token == 'E')) {
				real = true;
				shift();
				if (token == '-' || token == '+') {
					shift();
				}
				while ((token >= '0' && token <= '9') && index < length) {
					shift();
				}
			}

			v = code.alloc();
			auto nstart = start + (base != 10 ? 2 : 0);
			auto rep = std::string(&sourcecode[nstart], index - nstart);

			// syntax error
			if (!rep.length()) {
				v->type = value::type::TError;
				v->data.error.index = start;
				v->data.error.user = false;
				v->data.error.text = &sourcecode[start];
				v->data.error.length = index - start;
			}

			// number
			else if (!real) {
				v->type = value::type::TNumber;
				v->data.number.value = std::stoll(rep, NULL, base);
			}

			// real
			else {
				v->type = value::type::TReal;
				v->data.real.value = std::stold(rep, NULL);
			}
			end();
			continue;
		}

		// syntax error
		shift();
		v = code.alloc();
		v->type = value::type::TError;
		v->data.error.index = start;
		v->data.error.user = false;
		v->data.error.text = &sourcecode[start];
		v->data.error.length = index - start;
		end();
		continue;
	}

	return code;
}
