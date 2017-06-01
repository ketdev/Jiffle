#include "jiffle.h"
#include <assert.h>

namespace jiffle {

	void evaluate_test() {

		// internal state -------------------------------------------------
		std::vector<data> _tokens;
		std::vector<data> _modules;
		std::vector<frame> _frames;
		std::vector<data> _output;
		int _index;

		// methods --------------------------------------------------------
		auto set = [&](const std::string& input) {
			_tokens = tokenize(input);
			_modules.clear();
			_modules.push_back(structurize("",_tokens));
			_frames = generate(_modules);
			_output = evaluate(_frames);
			_index = 0;
		};
		auto assert_end = [&]() {
			assert(_index >= _output.size());
		};
		auto assert_primitive = [&](primitive primitive) {
			assert(_index < _output.size());
			assert(_output[_index].category == Primitive);
			assert(_output[_index].primitive == primitive);
			_index++;
		};

		// tests ----------------------------------------------------------

		{ // empty
			set("");
			assert_end();
		}
		{ // primitives
			set("null"); assert_primitive(Null); assert_end();
			//set("false"); assert_primitive(False); assert_end();
			//set("true"); assert_primitive(True); assert_end();
			//set("0x1234567890"); assert_primitive(Number); assert_end();
			//set("'hello world!'"); assert_primitive(String); assert_end();			
		}
	}

}