#ifndef _LISTVIEW_TESTER_H_
#define _LISTVIEW_TESTER_H_

#include "utils.h"
#include "../../src/utils/utils.h"

#include <vector>
#include <stdexcept>
#include <cstddef>

namespace tester {

	class listViewTester : public utils::TestSuite {
	public:
		listViewTester() : utils::TestSuite("listView Tester") {
			add_test("Construct Basic", "listView anchors at requested start", test_construct_basic);
			add_test("Clamp View Size", "constructor clamps oversized view", test_constructor_clamps_size);
			add_test("Throw On Bad Start", "constructor rejects out of range start", test_constructor_start_out_of_range);
			add_test("Allocate Success", "allocate extends view when capacity allows", test_allocate_success);
			add_test("Allocate Failure", "allocate refuses to exceed capacity", test_allocate_failure);
			add_test("Const Index Access", "const operator[] respects capacity", test_const_index_access);
			add_test("Write Auto Expand", "mutable operator[] grows view within capacity", test_write_auto_expand);
			add_test("Write Out Of Range", "operator[] throws when exceeding capacity", test_write_out_of_range);
			add_test("Subview Basics", "subView reanchors window correctly", test_subview_basics);
			add_test("Subview Out Of Range", "subView rejects start beyond size", test_subview_out_of_range);
			add_test("Iteration Helpers", "begin/endIter span the active view", test_iteration_helpers);
		}

	private:
		using IntList = std::vector<int>;
		using View = ::utils::listView<IntList>;

		static IntList make_sequence(int count) {
			IntList values(count);
			for (int i = 0; i < count; ++i) values[i] = i;
			return values;
		}

		static void test_construct_basic() {
			auto data = make_sequence(5);
			View view(data, 1, 2);

			ASSERT_EQ(static_cast<std::size_t>(2), view.end());
			ASSERT_EQ(&data[1], view.begin());
			ASSERT_EQ(1, view[0]);
			ASSERT_EQ(2, view[1]);
		}

		static void test_constructor_clamps_size() {
			auto data = make_sequence(4);
			View view(data, 2, 10);

			ASSERT_EQ(static_cast<std::size_t>(2), view.end());
			ASSERT_EQ(&data[2], view.begin());
		}

		static void test_constructor_start_out_of_range() {
			auto data = make_sequence(3);
			bool threw = false;
			try {
				View view(data, 4, 1);
				(void)view;
			} catch (const std::out_of_range&) {
				threw = true;
			}
			ASSERT_TRUE(threw);
		}

		static void test_allocate_success() {
			auto data = make_sequence(6);
			View view(data, 0, 2);

			ASSERT_TRUE(view.allocate(3));
			ASSERT_EQ(static_cast<std::size_t>(5), view.end());
		}

		static void test_allocate_failure() {
			auto data = make_sequence(4);
			View view(data, 1, 2);

			ASSERT_FALSE(view.allocate(5));
			ASSERT_EQ(static_cast<std::size_t>(2), view.end());
		}

		static void test_const_index_access() {
			auto data = make_sequence(4);
			const View view(data, 0, 3);

			ASSERT_EQ(0, view[0]);
			ASSERT_EQ(2, view[2]);

			bool threw = false;
			try {
				(void)view[4];
			} catch (const std::out_of_range&) {
				threw = true;
			}
			ASSERT_TRUE(threw);
		}

		static void test_write_auto_expand() {
			auto data = make_sequence(5);
			View view(data, 0, 2);

			view[3] = 99;
			ASSERT_EQ(static_cast<std::size_t>(4), view.end());
			ASSERT_EQ(99, data[3]);
		}

		static void test_write_out_of_range() {
			auto data = make_sequence(4);
			View view(data, 0, 2);

			bool threw = false;
			try {
				view[10] = 7;
			} catch (const std::out_of_range&) {
				threw = true;
			}
			ASSERT_TRUE(threw);
			ASSERT_EQ(static_cast<std::size_t>(2), view.end());
		}

		static void test_subview_basics() {
			auto data = make_sequence(6);
			View parent(data, 1, 4);
			auto child = parent.subView(1, 2);

			ASSERT_EQ(&data[2], child.begin());
			ASSERT_EQ(static_cast<std::size_t>(2), child.end());
			ASSERT_EQ(2, child[0]);
			ASSERT_EQ(3, child[1]);
		}

		static void test_subview_out_of_range() {
			auto data = make_sequence(5);
			View parent(data, 0, 3);

			bool threw = false;
			try {
				(void)parent.subView(4, 1);
			} catch (const std::out_of_range&) {
				threw = true;
			}
			ASSERT_TRUE(threw);
		}

		static void test_iteration_helpers() {
			auto data = make_sequence(5);
			View view(data, 1, 3);

			int sum = 0;
			for (auto* it = view.begin(); it != view.endIter(); ++it) {
				sum += *it;
			}
			ASSERT_EQ(1 + 2 + 3, sum);

			const View& const_view = view;
			int sum_const = 0;
			for (auto* it = const_view.cbegin(); it != const_view.cend(); ++it) {
				sum_const += *it;
			}
			ASSERT_EQ(sum, sum_const);
		}
	};
}

#endif