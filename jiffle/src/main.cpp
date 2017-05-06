
#include "ansicolor.h"
#include "jiffle\syntax.h"
#include "jiffle\expr.h"

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <chrono>

int main(int argc, char* argv[]) {
	// methods ----------------------------------------------------------------
	
	auto clockTime = [](){
		return std::chrono::steady_clock::now();
	};
	auto clockMeasure = [&](const std::chrono::time_point<std::chrono::steady_clock>& start) {
		return clockTime() - start;
	};
	auto toMilli = [](const std::chrono::duration<long long, std::nano>& duration) {
		return std::chrono::duration<double, std::milli>(duration).count();
	};
	auto toSecs = [](const std::chrono::duration<long long, std::nano>& duration) {
		return std::chrono::duration<double, std::ratio<1, 1>>(duration).count();
	};
	auto loadInput = [](const char* source) {
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
	};

	// entry ------------------------------------------------------------------

	{
		auto timer = clockTime();

		jiffle::syntax::tokenize_test();
		jiffle::expr::parse_test();

		auto metric = clockMeasure(timer);
		std::cout << "Tests: " << toSecs(metric) << " sec" << std::endl;
		getchar();
		return 0;
	}

	//if (argc != 2) {
	//	std::cout << "Usage: jiffle <source.jfl>" << std::endl;
	//	return 1;
	//}
	//
	//// 1. Load input(s)
	//auto timer = clockTime();
	//std::string input = loadInput(argv[1]);
	//std::cout << "Loader: " << toMilli(clockMeasure(timer)) << " ms" << std::endl;
	//
	//// 2. Tokenize
	//timer = clockTime();
	//auto sourcecode = syntax::tokenize(input);
	//std::cout << "Tokenize: " << toMilli(clockMeasure(timer)) << " ms" << std::endl;
	//
	//// 3. Parse
	//timer = clockTime();
	//auto ast = syntax::parse(sourcecode);
	//std::cout << "Parse: " << toMilli(clockMeasure(timer)) << " ms" << std::endl;
	//
	//
	//// 3b. Dump
	//ansi_fputs_out(syntax::dump(ast).c_str());
	//
	//// 4. Evaluate
	//timer = clockTime();
	//auto res = syntax::eval(ast);
	//std::cout << "Evaluate: " << toMilli(clockMeasure(timer)) << " ms" << std::endl;
	//
	//// 5. Print
	//timer = clockTime();
	//ansi_fputs_out(syntax::dump(res).c_str());
	//std::cout << "Print: " << toMilli(clockMeasure(timer)) << " ms" << std::endl;

	getchar();
	return 0;
}