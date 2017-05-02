#include "read.h"
#include <assert.h>

namespace syntax {
		
	void test() {
		{ // empty
			const char* input = "";
			auto code = syntax::read(input);
			assert(code == nullptr);
		}

		return;
	}

}
