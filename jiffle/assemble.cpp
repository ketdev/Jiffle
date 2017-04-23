#include <sstream>

#include "assemble.h"

std::string assemble(const memory& code) {
	std::stringstream output("");

	for each (auto &val in code.values) {
		switch (val.type) {
		case value::type::TParticle:
			output << (char)val.data.particle;
			break;
		case value::type::TComment:
			output << '#' << std::string(val.data.comment.text, val.data.comment.length);
			break;
		case value::type::TError:
			output << '`' << std::string(val.data.error.text, val.data.error.length) << '`' << ' ';
			break;
		case value::type::TString:
			output << '"' << std::string(val.data.string.text, val.data.string.length) << '"' << ' ';
			break;
		case value::type::TNull:
			output << "null ";
			break;
		case value::type::TBoolean:
			output << (val.data.boolean.value ? "true " : "false ");
			break;
		case value::type::TSymbol:
			output << std::string(val.data.symbol.name, val.data.symbol.length) << ' ';
			break;
		case value::type::TNumber:
			output << val.data.number.value << ' ';
			break;
		case value::type::TReal:
			output << val.data.real.value << ' ';
			break;
		default:
			break;
		}
	}
	return output.str();
}
