#include "vm.h"
#include <assert.h>

namespace jiffle {
	namespace vm {

		typedef std::vector<uint8_t> bytes;

		template<int N>
		bytes b(const char str[N]) {
			auto v = bytes(N);
			memcpy(&v[0], str, N);
			return v;
		}

		void generate_test() {
			using namespace syntax;
			using namespace expr;

			// internal state -------------------------------------------------
			std::vector<token> _src;
			node _ast;
			std::vector<table> _tables;
			size_t _index;
			
			// methods --------------------------------------------------------
			auto gen = [&](const std::string& input) {
				_src = tokenize(input);
				_ast = parse(_src, input);
				_tables = generate(_ast);
			};
			auto nextTable = [&]() {
				assert(_index < _tables.size());
				_index++;
			};
			auto end = [&]() {
				assert(_index >= _tables.size());
			};

			auto assert_memory = [&](int offset, const bytes& data) {
				assert(_index < _tables.size());
				auto &mem = _tables[_index].memory;
				assert(mem.size() >= offset + data.size());
				assert(memcmp(&mem[offset], &data[0], data.size()) == 0);
			};

			// tests ----------------------------------------------------------

			{ // empty
				gen("");
				end();
			}
			{ // constant memory
				gen("3");
				//assert_memory(0, b<4>("\x3\0\0\x1"));

				//nextTable();
				//end();
			}


		}

	}
}