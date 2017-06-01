#include "jiffle.h"

#include <stack>

namespace jiffle {

	std::vector<data> evaluate(const std::vector<frame>& frames) {
		// caller's memory (created on demand)
		std::vector<data> output;

		// null if empty
		if (frames.empty()) return output;

		// push entry frame
		std::stack<const frame*> callstack;
		callstack.push(&frames[0]);
		
		// evaluate call stack
		while (!callstack.empty()) {

			// allocate frame memory

			// call start code

			//	-> recursive add to stack, until no more instructions, then exit code and pop
			
			// call exit code

			// free frame memory
		}

		// allocate entry frame memory
		


		return output;
	}

}