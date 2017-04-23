#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <chrono>

#include "data.h"
#include "syntax.h"

#include "eval.h"

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
	str.reserve(input.tellg());
	input.seekg(0, std::ios::beg);

	// extra parenthesis is essential
	str.assign((std::istreambuf_iterator<char>(input)),
		std::istreambuf_iterator<char>());

	input.close();
	return str;
}

// entry point

int main(int argc, char* argv[]) {
	if (argc != 2) {
		std::cout << "Usage: jiffle <source.jfl>" << std::endl;
		return 1;
	}

	// 1. Load input(s)
	auto timer = clockTime();
	std::string sourcecode = loadInput(argv[1]);
	std::cout << "Loader: " << toMilli(clockMeasure(timer)) << " ms" << std::endl;
	const char* src = &sourcecode[0];
	size_t length = sourcecode.length();

	// 2. Parser
	timer = clockTime();
	auto code = syntax::parse(src,length);
	std::cout << "Parse: " << toMilli(clockMeasure(timer)) << " ms" << std::endl;
	
	// 2b. Assemble back to source code
	std::cout << syntax::assemble(code) << std::endl;

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

	getchar();
	return 0;
}