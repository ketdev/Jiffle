#include "syntax.h"
#include <sstream>
#include <string>
#include <stack>

namespace syntax {

	bool isWhitespace(char token) {
		return (token == ' '
			|| token == 0xB
			|| token == 0xC
			|| token == 0xD // CR
			|| token == 0x9 // TAB
			);
	}
	bool isLetter(char token) {
		return (token >= 'A' && token <= 'Z')
			|| (token >= 'a' && token <= 'z')
			|| (token == '_');
	}
	bool isDigit(char token, int base) {
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

	static const inline void shift(const char *& src, size_t & length) {
		length--;
		src++;
	}

	value parseError(const char *& src, size_t & length) {
		value v;
		v.type = value::TError;
		v.data.error.user = false;
		v.data.error.text = src;
		v.data.error.length = 0;
		if (length && *src == syntax::PError) {
			shift(src, length);
			auto start = src;
			while (length && *src != syntax::PError) {
				shift(src, length);
			}
			if (length && *src == syntax::PError) {
				v.data.error.user = true;
				v.data.comment.text = start;
				v.data.comment.length = src - start;
				shift(src, length);
			}
		}
		return v;
	}

	value parseWord(const char*& src, size_t& length) {
		value v;
		v.type = value::TError;
		v.data.error.user = false;
		v.data.error.text = src;
		v.data.error.length = 0;

		// reference
		bool reference = false;
		if (length && *src == syntax::PReference) {
			reference = true;
			shift(src, length);
		}

		auto start = src;
		size_t len = 0;
		if (length && isLetter(*src)) {
			while (length && (isLetter(*src) || isDigit(*src))) {
				shift(src, length);
			}
			len = src - start;
		}

		// null
		if (len == 4 && !std::memcmp("null", start, len)) {
			v.type = value::TNull;
		}
		// bool: true
		else if (len == 4 && !std::memcmp("true", start, len)) {
			v.type = value::TBoolean;
			v.data.boolean.value = true;
		}
		// bool: false
		else if (len == 5 && !std::memcmp("false", start, len)) {
			v.type = value::TBoolean;
			v.data.boolean.value = false;
		}

		// cannot reference non-symbols
		if (reference && v.type != value::TError) {
			v.type = value::TError;
			v.data.error.user = false;
			v.data.error.text = start - 1;
			v.data.error.length = len + 1;
		}

		// reference
		if (len && reference){
			v.type = value::TReference;
			v.data.reference.name = start;
			v.data.reference.length = len;
		}
		// symbol
		else if (len) {
			v.type = value::TSymbol;
			v.data.symbol.name = start;
			v.data.symbol.length = len;
		}

		return v;
	}

	value parseNumber(const char*& src, size_t& length) {
		value v;
		v.type = value::TError;
		v.data.error.user = false;
		v.data.error.text = src;
		v.data.error.length = 0;

		auto start = src;
		if (length && isDigit(*src)) {
			// integer by default
			bool real = false;
			int base = 10;
			// detect hex/oct/bin
			if (*src == '0') {
				shift(src, length);
				if (length) {
					if (*src == 'X' || *src == 'x') {
						base = 16; // hex
					}
					if (*src == 'O' || *src == 'o') {
						base = 8; // oct
					}
					if (*src == 'B' || *src == 'b') {
						base = 2; // bin
					}
					if (base != 10) {
						shift(src, length);
						start += 2;
					}
				}
			}
			// consume digits
			while (length && isDigit(*src, base)) {
				shift(src, length);
			}
			// real decimal point
			if (base == 10 && length && *src == '.') {
				shift(src, length);
				real = true;
				while (length && isDigit(*src)) {
					shift(src, length);
				}
			}
			// real exponent
			if (base == 10 && length && (*src == 'e' || *src == 'E')) {
				shift(src, length);
				real = true;
				if (length && (*src == '-' || *src == '+')) {
					shift(src, length);
				}
				while (length && isDigit(*src)) {
					shift(src, length);
				}
			}
			
			// syntax error
			auto rep = std::string(start, src - start);
			if (!rep.length()) {
				auto nstart = (base != 10) ? start - 2 : start;
				v.data.error.text = nstart;
				v.data.error.length = src - nstart;
			}

			// integer
			else if (!real) {
				v.type = value::TInteger;
				v.data.integer.value = std::stoll(rep, NULL, base);
			}

			// real
			else {
				v.type = value::TReal;
				v.data.real.value = std::stold(rep, NULL);
			}
		}
		return v;
	}

	value parseString(const char*& src, size_t& length) {
		value v;
		v.type = value::TError;
		v.data.error.user = false;
		v.data.error.text = src;
		v.data.error.length = 0;
		if (length && *src == syntax::PString) {
			shift(src, length);

			// TODO: escape sequences

			auto start = src;
			while (length && *src != syntax::PString) {
				shift(src, length);
			}
			if (length && *src == syntax::PString) {
				v.type = value::TString;
				v.data.string.text = start;
				v.data.string.length = src - start;
				shift(src, length);
			}
		}
		return v;
	}

	memory parse(const char*& src, size_t& length) {
		memory code;
		memory seq, eval;
		value::evaluation evalInfo;
		value::sequence seqInfo;
		value val;

		evalInfo.count = 0;
		seqInfo.count = 0;

		bool explicitSequence = false;
		bool needsEvaluation = false;

		// parse loop
		while (true) {

			//-----------------------------------------------------------------
			// skip whitespaces -----------------------------------------------
			if (syntax::isWhitespace(*src)) {
				shift(src, length);
				continue;
			}
			// comment --------------------------------------------------------
			if (length && *src == syntax::PComment) {
				shift(src, length);
				auto start = src;
				while (length && *src != syntax::Newline) {
					shift(src, length);
				}
				val.type = value::TComment;
				val.data.comment.afterCode = false;
				val.data.comment.text = start;
				val.data.comment.length = src - start;
				*eval.alloc() = val;
				// not counted in evaluation
				continue;
			}

			//-----------------------------------------------------------------
			// param
			bool param = false;
			if (length && *src == syntax::PParamBegin) {
				param = true;
				shift(src, length);
			}
			// reference
			bool reference = false;
			if (length && *src == syntax::PReference) {
				reference = true;
				shift(src, length);
			}
			// identifier
			auto start = src;
			size_t len = 0;
			if (length && isLetter(*src)) {
				while (length && (isLetter(*src) || isDigit(*src))) {
					shift(src, length);
				}
				len = src - start;
			}

			// null -----------------------------------------------------------
			if (len == 4 && !std::memcmp("null", start, len)) {
				val.type = value::TNull;
			}
			// bool: true -----------------------------------------------------
			else if (len == 4 && !std::memcmp("true", start, len)) {
				val.type = value::TBoolean;
				val.data.boolean.value = true;
			}
			// bool: false ----------------------------------------------------
			else if (len == 5 && !std::memcmp("false", start, len)) {
				val.type = value::TBoolean;
				val.data.boolean.value = false;
			}

			// non-symbols cannot be references or params
			if ((param||reference) && val.type != value::TError) {
				val.type = value::TError;
				val.data.error.user = false;
				val.data.error.text = start - 1;
				val.data.error.length = len + 1;
				*eval.alloc() = val;
				evalInfo.count++;
				continue;
			}

			// reference ------------------------------------------------------
			if (len && reference) {
				val.type = value::TReference;
				val.data.reference.name = start;
				val.data.reference.length = len;
				*eval.alloc() = val;
				evalInfo.count++;
				continue;
			}
			
			//-----------------------------------------------------------------
			// symbol
			else if (len) {
				val.type = value::TSymbol;
				val.data.symbol.name = start;
				val.data.symbol.length = len;
			}




			// words: reference, symbol, null, true, false
			if ((val = syntax::parseWord(src, length)).type != value::TError) {
				*eval.alloc() = val;
				evalInfo.count++;
				continue;
			} else if (val.data.error.length) { // words syntax error
				*eval.alloc() = val;
				evalInfo.count++;
				continue;
			}

			// param
			if (length && *src == syntax::PParamBegin) {
				shift(src, length);
				
				//PParamBegin
				//PParamEnd
			}

			// numbers
			if ((val = syntax::parseNumber(src, length)).type != value::TError) {
				*eval.alloc() = val;
				evalInfo.count++;
				if (val.type == value::TSymbol)
					needsEvaluation = true;
				continue;
			} else if (val.data.error.length){ // number syntax error
				*eval.alloc() = val;
				evalInfo.count++;
				continue;
			}

			// strings
			if ((val = syntax::parseString(src, length)).type != value::TError) {
				*eval.alloc() = val;
				evalInfo.count++;
				continue;
			}

			// sub sequence
			if (length && *src == syntax::PSequenceBegin) {
				shift(src, length);
				auto sub = parse(src, length);
				for each (auto v in sub.values) {
					*eval.alloc() = v;
				}
				// null if empty
				if (!sub.values.size()) {
					val.type = value::TNull;
					*eval.alloc() = val;
				}
				evalInfo.count++;
				continue;
			}

			// end of evaluation term
			if (!length 
				|| *src == syntax::PSequenceEnd
				|| (*src == syntax::PSequenceDivML || *src == syntax::PSequenceDivSL)) {
				if (*src == syntax::PSequenceDivSL)
					explicitSequence = true;

				// finished parsing
				bool finished = (!length || *src == syntax::PSequenceEnd);

				if(length)
					shift(src, length);
				
				// copy eval to seq with added prefix if needed
				if (needsEvaluation || evalInfo.count > 1) {
					val.type = value::TEvaluation;
					val.data.evaluation = evalInfo;
					*seq.alloc() = val;
				}
				for each (auto v in eval.values) {
					*seq.alloc() = v;
				}
				if(evalInfo.count)
					seqInfo.count++;

				// reset eval
				needsEvaluation = false;
				eval.values.clear();
				evalInfo.count = 0;

				if (finished) break;
				continue;
			}

			// abstraction
			if (length && *src == syntax::PAbstraction) {
				// lhs is evaluation sequence of symbols only (including input symbols)
				bool validLHS = evalInfo.count;
				for each (auto s in eval.values) {
					if (s.type != value::TSymbol && s.type != value::TComment) {
						validLHS = false;
					}
				}

				if (validLHS) {
					shift(src, length);

					// copy abstraction to seq with added prefix
					val.type = value::TAbstraction;
					val.data.abstraction.count = evalInfo.count;
					*seq.alloc() = val;
					for each (auto v in eval.values) {
						*seq.alloc() = v;
					}
					// takes next value as declaration, so no seq count increase

					// reset eval
					eval.values.clear();
					evalInfo.count = 0;
					continue;
				}
			}

			// errors
			if (!(val = syntax::parseError(src, length)).data.error.user) {
				val.data.error.text = src;
				val.data.error.length = 1;
				shift(src, length);
			}
			*eval.alloc() = val;
			evalInfo.count++;
			continue;
		}

		// copy seq to code with added prefix if needed
		if (explicitSequence || seqInfo.count > 1) {
			val.type = value::TSequence;
			val.data.sequence = seqInfo;
			*code.alloc() = val;
		}
		for each (auto v in seq.values) {
			*code.alloc() = v;
		}
		return code;
	}
	std::string assemble(const memory & code) {
		std::stringstream output("");
		
		std::stack<int> evalCount;
		std::stack<int> seqCount;

		for each (auto &val in code.values) {
			if (val.type != value::TComment) {
				if (!evalCount.size() || !evalCount.top()) output << std::endl;
				if (evalCount.size()) {
					evalCount.top()--;
					if (!evalCount.top()) {
						evalCount.pop();
					}
				}
			}

			switch (val.type) {
			case value::TComment:
				if (!val.data.comment.afterCode)
					output << std::endl;
				output << '#' << std::string(val.data.comment.text, val.data.comment.length);
				break;
			case value::TSymbol:
				output << std::string(val.data.symbol.name, val.data.symbol.length) << ' ';
				break;
			case value::TReference:
				output << "&" << std::string(val.data.reference.name, val.data.reference.length) << ' ';
				break;
			case value::TNull:
				output << "null ";
				break;
			case value::TBoolean:
				output << (val.data.boolean.value ? "true " : "false ");
				break;
			case value::TInteger:
				output << val.data.integer.value << ' ';
				break;
			case value::TReal:
				output << val.data.real.value << ' ';
				break;
			case value::TString:
				output << '"' << std::string(val.data.string.text, val.data.string.length) << '"' << ' ';
				break;
			case value::TEvaluation:
				evalCount.push(val.data.evaluation.count);
				break;
			case value::TSequence:
				output << "(";
				seqCount.push(val.data.sequence.count);
				break;
			case value::TError:
				output << '`' << std::string(val.data.error.text, val.data.error.length) << '`' << ' ';
				break;
			default:
				break;
			}

			if (val.type != value::TComment) {
				if (seqCount.size()) {
					seqCount.top()--;
					if (!seqCount.top()) {
						seqCount.pop();
						output << ")";
					}
				}
			}

		}
		return output.str();
	}
}