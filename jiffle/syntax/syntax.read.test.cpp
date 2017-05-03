#include "read.h"
#include <assert.h>

namespace syntax {
		
	static expr::ptr simple(const char* input) {
		auto o = read(input);
		// open all wrappers
		while (true) {
			auto c = o->as<construct>();
			if (c == nullptr) break;
			assert(c->items.size() < 2);
			if (c->items.empty())
				return nullptr;
			o = c->items[0];
		}
		return o;
	}

	static void primitive_test() {
		{ // empty
			auto o = simple("");
			assert(o == nullptr);
		}
		{ // comment
			auto o = simple("# hello comment");
			assert(o == nullptr);
		}
		{ // error
			auto input = "`";
			auto o = simple(input);
			auto p = o->as<value_error>();
			assert(p != nullptr && p->type == value_error::Syntax);
			assert(p->pos.ch == 0);
			assert(p->pos.len == strlen(input));
			assert(p->pos.ln == 0);
			assert(p->pos.col == 0);
		}
		{ // void
			auto input = "null";
			auto o = simple(input);
			auto p = o->as<value_null>();
			assert(p != nullptr);
			assert(p->pos.ch == 0);
			assert(p->pos.len == strlen(input));
			assert(p->pos.ln == 0);
			assert(p->pos.col == 0);
		}
		{ // boolean: true
			auto input = "true";
			auto o = simple(input);
			auto p = o->as<value_bool>();
			assert(p != nullptr && p->value == true);
			assert(p->pos.ch == 0);
			assert(p->pos.len == strlen(input));
			assert(p->pos.ln == 0);
			assert(p->pos.col == 0);
		}
		{ // boolean: false
			auto input = "false";
			auto o = simple(input);
			auto p = o->as<value_bool>();
			assert(p != nullptr && p->value == false);
			assert(p->pos.ch == 0);
			assert(p->pos.len == strlen(input));
			assert(p->pos.ln == 0);
			assert(p->pos.col == 0);
		}
		{ // symbol
			auto name = "foo";
			auto o = simple(name);
			auto p = o->as<value_symbol>();
			assert(p != nullptr && p->name == name);
			assert(p->pos.ch == 0);
			assert(p->pos.len == strlen(name));
			assert(p->pos.ln == 0);
			assert(p->pos.col == 0);
		}
		{ // integer
			auto input = "123456";
			auto o = simple(input);
			auto p = o->as<value_integer>();
			assert(p != nullptr && p->value == 123456);
			assert(p->pos.ch == 0);
			assert(p->pos.len == strlen(input));
			assert(p->pos.ln == 0);
			assert(p->pos.col == 0);
		}
		{ // real
			auto input = "123456.0";
			auto o = simple(input);
			auto p = o->as<value_real>();
			assert(p != nullptr && p->value == 123456.0);
			assert(p->pos.ch == 0);
			assert(p->pos.len == strlen(input));
			assert(p->pos.ln == 0);
			assert(p->pos.col == 0);
		}
		{ // string
			auto input = "hello world!\n";
			auto f = std::string("'") + input + std::string("'");
			auto o = simple(f.c_str());
			auto p = o->as<value_string>();
			assert(p != nullptr && p->value == input);
			assert(p->pos.ch == 0);
			assert(p->pos.len == f.length());
			assert(p->pos.ln == 0);
			assert(p->pos.col == 0);
		}
	}

	static void evaluation_test() {
		auto input = "A 1 2";
		auto o = read(input);
		assert(o->pos.ch == 0);
		assert(o->pos.len == strlen(input));
		assert(o->pos.ln == 0);
		assert(o->pos.col == 0);

		auto s = o->as<sequence>();
		assert(s != nullptr);
		assert(s->isExplicit == false);
		assert(s->items.size() == 1);
		assert(s->pos.ch == 0);
		assert(s->pos.len == strlen(input));
		assert(s->pos.ln == 0);
		assert(s->pos.col == 0);

		auto e = s->items[0]->as<evaluation>();
		assert(e != nullptr);

		auto &i = e->items;
		assert(i.size() == 3);
		auto i0 = i[0]->as<value_symbol>();
		auto i1 = i[1]->as<value_integer>();
		auto i2 = i[2]->as<value_integer>();
		assert(i0 != nullptr && i1 != nullptr && i2 != nullptr);

		assert(i0->pos.ch == 0);
		assert(i0->pos.len == 1);
		assert(i0->pos.ln == 0);
		assert(i0->pos.col == 0);
		assert(i0->name == "A");

		assert(i1->pos.ch == 2);
		assert(i1->pos.len == 1);
		assert(i1->pos.ln == 0);
		assert(i1->pos.col == 2);
		assert(i1->value == 1);

		assert(i2->pos.ch == 4);
		assert(i2->pos.len == 1);
		assert(i2->pos.ln == 0);
		assert(i2->pos.col == 4);
		assert(i2->value == 2);
	}

	static void sequence_test() {		
		{ // explicit tuple sequence
			auto input = "A,1,2.0,'hi'";
			auto o = read(input);
			auto s = o->as<sequence>();
			assert(s->type == sequence::Tuple);
			assert(s->isExplicit == true);
			assert(s->items.size() == 4);
			auto i0 = s->items[0]->as<evaluation>()->items[0]->as<value_symbol>();
			auto i1 = s->items[1]->as<evaluation>()->items[0]->as<value_integer>();
			auto i2 = s->items[2]->as<evaluation>()->items[0]->as<value_real>();
			auto i3 = s->items[3]->as<evaluation>()->items[0]->as<value_string>();
			assert(i0 != nullptr && i1 != nullptr && i2 != nullptr && i3 != nullptr);
			assert(i0->pos.ch == 0 && i1->pos.ch == 2 && i2->pos.ch == 4 & i3->pos.ch == 8);
			assert(i0->name == "A" && i1->value == 1 && i2->value == 2.0 && i3->value == "hi");
		}
		{ // implicit tuple sequence
			auto input = "A\n1\n2.0\n'hi'";
			auto o = read(input);
			auto s = o->as<sequence>();
			assert(s->type == sequence::Tuple);
			assert(s->isExplicit == false);
			assert(s->items.size() == 4);
			auto i0 = s->items[0]->as<evaluation>()->items[0]->as<value_symbol>();
			auto i1 = s->items[1]->as<evaluation>()->items[0]->as<value_integer>();
			auto i2 = s->items[2]->as<evaluation>()->items[0]->as<value_real>();
			auto i3 = s->items[3]->as<evaluation>()->items[0]->as<value_string>();
			assert(i0->pos.ch == 0 && i1->pos.ch == 2 && i2->pos.ch == 4 & i3->pos.ch == 8);
			assert(i0->name == "A" && i1->value == 1 && i2->value == 2.0 && i3->value == "hi");
		}
		{ // commented sequence with eval
			auto input = "#comment\n1.0 `err`";
			auto o = read(input);
			auto s = o->as<sequence>();
			assert(s != nullptr);
			assert(s->items.size() == 1);
			auto e = s->items[0]->as<evaluation>();
			assert(e->items.size() == 2);
			auto i0 = e->items[0]->as<value_real>();
			auto i1 = e->items[1]->as<value_error>();
			assert(i0->pos.ch == 9 && i1->pos.ch == 13);
			assert(i0->value == 1.0 && i1->text == "err");
		}
	}

	void read_test() {
		primitive_test();
		evaluation_test();
		sequence_test();
	}

}
