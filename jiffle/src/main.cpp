
#include "ansicolor.h"
#include "jiffle\jiffle.h"

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

	//if (argc != 2) {
	//	std::cout << "Usage: jiffle <source.jfl>" << std::endl;
	//	return 1;
	//}
	
	auto timer = clockTime();

	jiffle::tokenize_test();
	jiffle::structurize_test();
	//jiffle::generate_test();
	//jiffle::evaluate_test();

	auto metric = clockMeasure(timer);
	std::cout << "Tests: " << toSecs(metric) << " sec" << std::endl;
	getchar();
	return 0;	

}