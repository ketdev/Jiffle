#include <iostream>
#include <stack>
#include "eval.h"

memory eval(const value*& code, size_t& length) {
	memory result;

	bool isSequence = false;

	auto makeSequence = [&]() {
		if (isSequence) {
			value seq;
			seq.type = value::type::TParticle;
			seq.data.particle = value::particle::PSequenceBegin;
			result.values.emplace(result.values.begin(), seq);
			seq.data.particle = value::particle::PSequenceEnd;
			result.values.emplace(result.values.end(), seq);
		}
	};
	auto writeParticle = [&](value::particle particle) {
		value *p = result.alloc();
		p->type = value::type::TParticle;
		p->data.particle = particle;
	};
	
	// Interpret loop
	for (; length > 0; length--, code++) {

		switch (code->type) {

		// ------------------------------------------------------------
		// Interpret particles

		case value::type::TParticle:
			switch (code->data.particle) {

			// Sequences
			case value::particle::PSequenceBegin:
				{
					// write sequence start
					code++; length--;
					auto subVal = eval(code, length);
					for each (auto &var in subVal.values)
						*result.alloc() = var;
				}
				break;
			case value::particle::PSeparatorSL:
			case value::particle::PSeparatorML:
				isSequence = true;
				auto prev = result.values.back();
				if (prev.type != value::type::TParticle || (prev.data.particle != value::particle::PSeparatorSL
					&& prev.data.particle != value::particle::PSeparatorML)) {
					writeParticle(value::particle::PSeparatorSL);					
				}
				break;
			case value::particle::PSequenceEnd:
				makeSequence();
				return result; // early exit
				


			default:
				std::cerr << "Unknown particle: " << code->data.particle << std::endl;
				break;
			}
			break;

		// ------------------------------------------------------------
		// Interpret values

		case value::type::TComment:
			// ignore
			break;
		case value::type::TError:
		case value::type::TString:
		case value::type::TNull:
		case value::type::TBoolean:
		case value::type::TNumber:
		case value::type::TReal:
			*result.alloc() = *code;
			break;

		case value::type::TSymbol:
			*result.alloc() = *code;
			// TODO: resolve symbol and invoke evaluation
			break;

		default:
			std::cerr << "Unknown value type: " << code->type << std::endl;
			break;
		}
	}

	makeSequence();
	return result;
}
