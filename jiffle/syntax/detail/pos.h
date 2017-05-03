#pragma once

namespace syntax {
	namespace detail {

		struct pos {
			size_t ch;
			size_t len;
			size_t ln;
			size_t col;
		};

	}
}