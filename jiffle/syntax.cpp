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

	value parseComment(const char *& src, size_t & length) {
		value v;
		v.type = value::type::TError;
		v.data.error.user = false;
		v.data.error.text = src;
		v.data.error.length = 0;
		if (length && *src == syntax::PComment) {
			shift(src, length);
			auto start = src;
			while (length && *src != syntax::Newline) {
				shift(src, length);
			}
			v.type = value::type::TComment;
			v.data.comment.afterCode = false;
			v.data.comment.text = start;
			v.data.comment.length = src - start;
		}
		return v;
	}

	value parseError(const char *& src, size_t & length) {
		value v;
		v.type = value::type::TError;
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
		v.type = value::type::TError;
		v.data.error.user = false;
		v.data.error.text = src;
		v.data.error.length = 0;

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
			v.type = value::type::TNull;
		}
		// bool: true
		else if (len == 4 && !std::memcmp("true", start, len)) {
			v.type = value::type::TBoolean;
			v.data.boolean.value = true;
		}
		// bool: false
		else if (len == 5 && !std::memcmp("false", start, len)) {
			v.type = value::type::TBoolean;
			v.data.boolean.value = false;
		}
		// symbol
		else if (len){
			v.type = value::type::TSymbol;
			v.data.symbol.name = start;
			v.data.symbol.length = len;
		}
		return v;
	}

	value parseNumber(const char*& src, size_t& length) {
		value v;
		v.type = value::type::TError;
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

			// number
			else if (!real) {
				v.type = value::type::TInteger;
				v.data.number.value = std::stoll(rep, NULL, base);
			}

			// real
			else {
				v.type = value::type::TReal;
				v.data.real.value = std::stold(rep, NULL);
			}
		}
		return v;
	}

	value parseString(const char*& src, size_t& length) {
		value v;
		v.type = value::type::TError;
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
				v.type = value::type::TString;
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
			// skip whitespaces
			if (syntax::isWhitespace(*src)) {
				shift(src, length);
				continue;
			}

			// comment
			if ((val = syntax::parseComment(src, length)).type != value::TError) {
				if (evalInfo.count)
					val.data.comment.afterCode = true;
				*eval.alloc() = val;
				// not counted in evaluation
				continue;
			}

			// words: symbol, null, true, false
			if ((val = syntax::parseWord(src, length)).type != value::TError) {
				*eval.alloc() = val;
				evalInfo.count++;
				if (val.type == value::TSymbol)
					needsEvaluation = true;
				continue;
			}

			// numbers
			if ((val = syntax::parseNumber(src, length)).type != value::TError) {
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
			if (val.type != value::type::TComment) {
				if (!evalCount.size() || !evalCount.top()) output << std::endl;
				if (evalCount.size()) {
					evalCount.top()--;
					if (!evalCount.top()) {
						evalCount.pop();
					}
				}
			}

			switch (val.type) {
			case value::type::TComment:
				if (!val.data.comment.afterCode)
					output << std::endl;
				output << '#' << std::string(val.data.comment.text, val.data.comment.length);
				break;
			case value::type::TSymbol:
				output << std::string(val.data.symbol.name, val.data.symbol.length) << ' ';
				break;
			case value::type::TNull:
				output << "null ";
				break;
			case value::type::TBoolean:
				output << (val.data.boolean.value ? "true " : "false ");
				break;
			case value::type::TInteger:
				output << val.data.number.value << ' ';
				break;
			case value::type::TReal:
				output << val.data.real.value << ' ';
				break;
			case value::type::TString:
				output << '"' << std::string(val.data.string.text, val.data.string.length) << '"' << ' ';
				break;
			case value::type::TEvaluation:
				evalCount.push(val.data.evaluation.count);
				break;
			case value::type::TSequence:
				output << "(";
				seqCount.push(val.data.sequence.count);
				break;
			case value::type::TError:
				output << '`' << std::string(val.data.error.text, val.data.error.length) << '`' << ' ';
				break;
			default:
				break;
			}

			if (val.type != value::type::TComment) {
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