#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <chrono>

#include "syntax.h"
#include "ansicolor.h"

// helper functions

std::chrono::time_point<std::chrono::steady_clock> clockTime() {
	return std::chrono::steady_clock::now();
}
std::chrono::duration<long long, std::nano> clockMeasure(const std::chrono::time_point<std::chrono::steady_clock>& start) {
	return clockTime() - start;
}
double toMilli(const std::chrono::duration<long long, std::nano>& duration) {
	return std::chrono::duration<double, std::milli>(duration).count();
}
std::string loadInput(const char* source) {
	std::string str;
	std::ifstream input(source);

	input.seekg(0, std::ios::end);
	str.reserve((size_t)input.tellg());
	input.seekg(0, std::ios::beg);

	// extra parenthesis is essential
	str.assign((std::istreambuf_iterator<char>(input)),
		std::istreambuf_iterator<char>());

	input.close();
	return str;
}

// entry point

namespace syntax { void test(); }

int main(int argc, char* argv[]) {
	syntax::test();
	
	if (argc != 2) {
		std::cout << "Usage: jiffle <source.jfl>" << std::endl;
		return 1;
	}

	// 1. Load input(s)
	auto timer = clockTime();
	std::string input = loadInput(argv[1]);
	std::cout << "Loader: " << toMilli(clockMeasure(timer)) << " ms" << std::endl;

	// 2. Tokenize
	timer = clockTime();
	auto sourcecode = syntax::tokenize(input);
	std::cout << "Tokenize: " << toMilli(clockMeasure(timer)) << " ms" << std::endl;

	// 3. Parse
	timer = clockTime();
	auto ast = syntax::parse(sourcecode);
	std::cout << "Parse: " << toMilli(clockMeasure(timer)) << " ms" << std::endl;


	// 3b. Dump
	ansi_fputs_out(syntax::dump(ast).c_str());

	// 4. Evaluate
	timer = clockTime();
	auto res = syntax::eval(ast);
	std::cout << "Evaluate: " << toMilli(clockMeasure(timer)) << " ms" << std::endl;

	// 5. Print
	timer = clockTime();
	ansi_fputs_out(syntax::dump(res).c_str());
	std::cout << "Print: " << toMilli(clockMeasure(timer)) << " ms" << std::endl;




	/*
	// 2b. Assemble back to source code
	//std::cout << syntax::assemble(code) << std::endl;

	// Built-in environment


	// 3. Evaluate
	timer = clockTime();
	const value *codeptr = &code.values[0];
	size_t codelen = code.values.size();
	auto output = eval(codeptr, codelen);
	std::cout << "Evaluate: " << toMilli(clockMeasure(timer)) << " ms" << std::endl;
	
	// 4. Assemble output (to code structure)
	timer = clockTime();
	auto assm = syntax::assemble(output);
	std::cout << assm << std::endl;
	std::cout << "Assemble: " << toMilli(clockMeasure(timer)) << " ms" << std::endl;
	*/

	getchar();
	return 0;
}