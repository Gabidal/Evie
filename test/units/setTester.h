#ifndef _SET_TESTER_H_
#define _SET_TESTER_H_

#include "utils.h"
#include "../../src/utils/utils.h"

#include <vector>
#include <stdexcept>
#include <cstddef>

namespace tester {

	class setTester : public utils::TestSuite {
	public:
		setTester() : utils::TestSuite("set Tester") {
			add_test("Basic Construction", "set constructs correctly from container", test_basic_operations);
			add_test("Set Check", "set check method works correctly", test_set_check);
			add_test("SuperSet Construction", "superSet constructs correctly from sets", test_superset_construction);
			add_test("SuperSet Intersection", "superSet intersection works correctly", test_superset_intersection);
			add_test("SuperSet XOR", "superSet XOR works correctly", test_superset_xor);
			add_test("Range Difference", "range difference works correctly", test_range_difference);
			add_test("Range Subset/Superset", "range subset/superset checks work correctly", test_range_subset_superset);
			add_test("Set Difference", "set difference works correctly", test_set_difference);
			add_test("Set Subset/Superset", "set subset/superset checks work correctly", test_set_subset_superset);
			add_test("SuperSet Difference", "superSet difference works correctly", test_superset_difference);
			add_test("SuperSet Subset/Superset", "superSet subset/superset checks work correctly", test_superset_subset_superset);
		}

	private:
		static void test_basic_operations() {
			using containerType = std::vector<int>;
			containerType container({1, 2, 3, 4, 5, 6, 7, 8, 9, 0});

			::utils::set<containerType> set(container);

			ASSERT_EQ(0, set.getSize().max - set.getSize().min);
			ASSERT_EQ(10, set.getCapacity().max - set.getCapacity().min);

			for (std::size_t i = 0; i < 10; i++) {
				// set | ::utils::range(0, i) is an union operation between the 2D vector an the 2D set, returning an set which is in that range.
				ASSERT_EQ(container[i], (set | ::utils::range(0, i))[i]);
			}
		}

		static void test_set_check() {
			using containerType = std::vector<int>;
			containerType container({1, 2, 3, 4, 5, 6, 7, 8, 9, 0});

			::utils::set<containerType> set(container);

			ASSERT_TRUE(set.check({0, 1}));			// This expands the set in the direction of vec(0, 1)

			::utils::set<containerType> subSet = set & ::utils::range(2, 5); // This intersects the set in the direction of vec(2, 5)

			ASSERT_TRUE(subSet.check({subSet.getSize().min - 1, 0}));		// This expands the set in the direction of vec(-1, 0)
			ASSERT_FALSE(subSet.check({subSet.getSize().min - 3, 0}));		// This expands the set in the direction of vec(-3, 0), which is out of range
		}

		static void test_superset_construction() {
			using containerType = std::vector<int>;

			containerType c1({1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19});
			containerType c2({1, 2, 3, 4, 5});

			::utils::set<containerType> s1(c1, {5, 10});
			::utils::set<containerType> s2(c2, {0, 5});
			::utils::set<containerType> s3(c1, {10, 15});
			::utils::set<containerType> s4(c1, {15, 20});

			::utils::superSet<containerType> ss1(s1 | s2 | s3 | s4);

			ASSERT_EQ((std::int32_t)c1.size(), ss1.getSize().max - ss1.getSize().min);

			for (std::size_t i = 0; i < c1.size(); i++) {
				ASSERT_EQ(c1[i], ss1[i]);
			}
		}

		static void test_superset_intersection() {
			using containerType = std::vector<int>;

			containerType c1({0, 1, 2, 3, 4, 5, 6, 7, 8, 9});

			::utils::set<containerType> s1(c1, {0, 10});
			::utils::set<containerType> s2(c1, {2, 8});
			::utils::set<containerType> s3(c1, {4, 6});

			// Stepwise intersections
			auto i1 = s1 & s2;  // expected [2, 8)
			auto i2 = i1 & s3;  // expected [4, 6)

			ASSERT_EQ(4, i2.getSize().min);
			ASSERT_EQ(6, i2.getSize().max);
			ASSERT_EQ((int32_t)c1.size(), i2.getCapacity().max);

			// Confirm data matches c1 elements within intersection range
			for (std::int32_t i = i2.getSize().min; i < i2.getSize().max; ++i) {
				ASSERT_EQ(c1[i], i2[i]);
			}
		}

		static void test_superset_xor() {
			using containerType = std::vector<int>;

			containerType c({0, 1, 2, 3, 4, 5, 6, 7, 8, 9});

			// Original sets
			::utils::set<containerType> s1(c, {2, 8});  // [2, 8)
			::utils::set<containerType> s2(c, {4, 6});  // [4, 6)

			::utils::superSet<containerType> ss({s1});

			// XOR: remove intersection of s1 and s2
			auto xorSet = ss ^ s2;

			// Expected: two segments: [2, 4) and [6, 8)
			std::vector<::utils::range> expected = {{2, 4}, {6, 8}};

			// Check the ranges and data
			for (std::size_t i = 0; i < xorSet.subsets.size(); ++i) {
				ASSERT_EQ(expected[i].min, xorSet.subsets[i].getSize().min);
				ASSERT_EQ(expected[i].max, xorSet.subsets[i].getSize().max);

				for (std::int32_t j = expected[i].min; j < expected[i].max; ++j) {
					ASSERT_EQ(c[j], xorSet.subsets[i][j]);
				}
			}
		}

		static void test_range_difference() {
			// Test case 1: No overlap
			::utils::range r1(0, 5);
			::utils::range r2(10, 15);
			auto diff1 = r1 - r2;
			ASSERT_EQ(1, (int32_t)diff1.size());
			ASSERT_EQ(0, diff1[0].min);
			ASSERT_EQ(5, diff1[0].max);

			// Test case 2: Complete overlap
			::utils::range r3(5, 10);
			::utils::range r4(5, 10);
			auto diff2 = r3 - r4;
			ASSERT_EQ(0, (int32_t)diff2.size());

			// Test case 3: Partial overlap (left)
			::utils::range r5(0, 10);
			::utils::range r6(5, 15);
			auto diff3 = r5 - r6;
			ASSERT_EQ(1, (int32_t)diff3.size());
			ASSERT_EQ(0, diff3[0].min);
			ASSERT_EQ(5, diff3[0].max);

			// Test case 4: Partial overlap (right)
			::utils::range r7(5, 15);
			::utils::range r8(0, 10);
			auto diff4 = r7 - r8;
			ASSERT_EQ(1, (int32_t)diff4.size());
			ASSERT_EQ(10, diff4[0].min);
			ASSERT_EQ(15, diff4[0].max);

			// Test case 5: r2 is subset of r1
			::utils::range r9(0, 10);
			::utils::range r10(3, 7);
			auto diff5 = r9 - r10;
			ASSERT_EQ(2, (int32_t)diff5.size());
			ASSERT_EQ(0, diff5[0].min);
			ASSERT_EQ(3, diff5[0].max);
			ASSERT_EQ(7, diff5[1].min);
			ASSERT_EQ(10, diff5[1].max);
		}

		static void test_range_subset_superset() {
			::utils::range r1(0, 10);
			::utils::range r2(3, 7);
			::utils::range r3(0, 10);
			::utils::range r4(5, 15);

			// Subset checks
			ASSERT_TRUE(r2 <= r1);    // r2 is subset of r1
			ASSERT_TRUE(r2 < r1);     // r2 is proper subset of r1
			ASSERT_TRUE(r1 <= r1);    // r1 is subset of itself
			ASSERT_FALSE(r1 < r1);    // r1 is not proper subset of itself
			ASSERT_FALSE(r4 <= r1);   // r4 is not subset of r1

			// Superset checks
			ASSERT_TRUE(r1 >= r2);    // r1 is superset of r2
			ASSERT_TRUE(r1 > r2);     // r1 is proper superset of r2
			ASSERT_TRUE(r1 >= r1);    // r1 is superset of itself
			ASSERT_FALSE(r1 > r1);    // r1 is not proper superset of itself
			ASSERT_FALSE(r1 >= r4);   // r1 is not superset of r4
		}

		static void test_set_difference() {
			using containerType = std::vector<int>;
			containerType c({0, 1, 2, 3, 4, 5, 6, 7, 8, 9});

			::utils::set<containerType> s1(c, {2, 8});  // [2, 8)
			::utils::range r1(4, 6);                    // [4, 6)

			auto diff = s1 - r1;

			// Expected: two segments: [2, 4) and [6, 8)
			ASSERT_EQ(2, (int32_t)diff.size());
			ASSERT_EQ(2, diff[0].getSize().min);
			ASSERT_EQ(4, diff[0].getSize().max);
			ASSERT_EQ(6, diff[1].getSize().min);
			ASSERT_EQ(8, diff[1].getSize().max);

			// Verify data
			for (std::int32_t i = diff[0].getSize().min; i < diff[0].getSize().max; ++i) {
				ASSERT_EQ(c[i], diff[0][i]);
			}
			for (std::int32_t i = diff[1].getSize().min; i < diff[1].getSize().max; ++i) {
				ASSERT_EQ(c[i], diff[1][i]);
			}
		}

		static void test_set_subset_superset() {
			using containerType = std::vector<int>;
			containerType c({0, 1, 2, 3, 4, 5, 6, 7, 8, 9});

			::utils::set<containerType> s1(c, {2, 8});  // [2, 8)
			::utils::range r1(0, 10);                   // [0, 10)
			::utils::range r2(2, 8);                    // [2, 8)
			::utils::range r3(3, 5);                    // [3, 5)

			// Subset checks
			ASSERT_TRUE(s1 <= r1);    // s1 is subset of r1
			ASSERT_TRUE(s1 < r1);     // s1 is proper subset of r1
			ASSERT_TRUE(s1 <= r2);    // s1 is subset of itself
			ASSERT_FALSE(s1 < r2);    // s1 is not proper subset of itself
			ASSERT_FALSE(s1 <= r3);   // s1 is not subset of r3

			// Superset checks
			ASSERT_TRUE(s1 >= r3);    // s1 is superset of r3
			ASSERT_TRUE(s1 > r3);     // s1 is proper superset of r3
			ASSERT_TRUE(s1 >= r2);    // s1 is superset of itself
			ASSERT_FALSE(s1 > r2);    // s1 is not proper superset of itself
			ASSERT_FALSE(s1 >= r1);   // s1 is not superset of r1
		}

		static void test_superset_difference() {
			using containerType = std::vector<int>;
			containerType c({0, 1, 2, 3, 4, 5, 6, 7, 8, 9});

			::utils::set<containerType> s1(c, {0, 5});   // [0, 5)
			::utils::set<containerType> s2(c, {7, 10});  // [7, 10)
			::utils::set<containerType> s3(c, {3, 8});   // [3, 8)

			::utils::superSet<containerType> ss({s1, s2});

			// Remove s3 from superSet
			auto diffSet = ss - s3;

			// Expected segments: [0, 3) and [8, 10)
			ASSERT_EQ(2, (int32_t)diffSet.subsets.size());
			ASSERT_EQ(0, diffSet.subsets[0].getSize().min);
			ASSERT_EQ(3, diffSet.subsets[0].getSize().max);
			ASSERT_EQ(8, diffSet.subsets[1].getSize().min);
			ASSERT_EQ(10, diffSet.subsets[1].getSize().max);

			// Verify data
			for (std::int32_t i = diffSet.subsets[0].getSize().min; i < diffSet.subsets[0].getSize().max; ++i) {
				ASSERT_EQ(c[i], diffSet.subsets[0][i]);
			}
			for (std::int32_t i = diffSet.subsets[1].getSize().min; i < diffSet.subsets[1].getSize().max; ++i) {
				ASSERT_EQ(c[i], diffSet.subsets[1][i]);
			}
		}

		static void test_superset_subset_superset() {
			using containerType = std::vector<int>;
			containerType c({0, 1, 2, 3, 4, 5, 6, 7, 8, 9});

			::utils::set<containerType> s1(c, {0, 3});   // [0, 3)
			::utils::set<containerType> s2(c, {5, 8});   // [5, 8)
			::utils::set<containerType> s3(c, {0, 10});  // [0, 10)
			::utils::set<containerType> s4(c, {1, 2});   // [1, 2)

			::utils::superSet<containerType> ss({s1, s2});

			// Subset checks
			ASSERT_TRUE(ss <= s3);    // ss is subset of s3
			ASSERT_TRUE(ss < s3);     // ss is proper subset of s3
			ASSERT_FALSE(ss <= s4);   // ss is not subset of s4

			// Superset checks
			ASSERT_TRUE(ss >= s4);    // ss is superset of s4
			ASSERT_TRUE(ss > s4);     // ss is proper superset of s4
			ASSERT_FALSE(ss >= s3);   // ss is not superset of s3
		}

	};

}

#endif