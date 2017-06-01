#include "jiffle.h"
#include <assert.h>

namespace jiffle {

#define START(op,...) op(true,__VA_ARGS__)
#define EXIT(op,...) op(false,__VA_ARGS__)

	void generate_test() {

		// internal state -------------------------------------------------
		std::vector<data> _tokens;
		std::vector<data> _modules;
		std::vector<frame> _frames;
		size_t _frameIndex;
		size_t _memIndex, _startIndex, _exitIndex;
			
		// methods --------------------------------------------------------
		auto set = [&](const std::string& input) {
			_tokens = tokenize(input);
			_modules.clear();
			_modules.push_back(structurize("",_tokens));
			_frames = generate(_modules);
			_frameIndex = 0;
			_memIndex = 0;
			_startIndex = 0;
			_exitIndex = 0;
		};
		auto nextFrame = [&]() {
			assert(_frameIndex < _frames.size());
			assert(_memIndex >= _frames[_frameIndex].memory.size());
			assert(_startIndex >= _frames[_frameIndex].start.size());
			assert(_exitIndex >= _frames[_frameIndex].exit.size());
			_frameIndex++;
			_memIndex = 0;
			_startIndex = 0;
			_exitIndex = 0;
		};
		auto end = [&]() {
			assert(_frameIndex >= _frames.size());
		};
		// frame memory data
		// frame instructions
		auto store = [&](bool _start, primitive value, address address ) {
			assert(_frameIndex < _frames.size());
			size_t &index = _start ? _startIndex : _exitIndex;
			std::vector<data> &ins = _start ? _frames[_frameIndex].start : _frames[_frameIndex].exit;
			assert(index < ins.size());
			assert(ins[index].category == Instruction);
			assert(ins[index].instruction == STORE);
			assert(ins[index].items.size() == 2);
			assert(ins[index].items[0].category == Primitive);
			assert(ins[index].items[0].primitive == value);
			assert(ins[index].items[1].category == Primitive);
			assert(ins[index].items[1].primitive == Address);
			assert(ins[index].items[1].text == address);
		};

		// tests ----------------------------------------------------------

		{ // empty
			set("");
			end();
		}
		{ // primitives
			set("null");
			// --
			START(store, Null, "::0");
			nextFrame();
			end();
		}


	}

}