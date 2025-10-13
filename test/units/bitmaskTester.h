#ifndef _BITMASK_TESTER_H_
#define _BITMASK_TESTER_H_

#include "utils.h"
#include "../../src/utils/utils.h"

#include <cstdint>

namespace tester {

    enum class BitFlag : std::uint8_t {
        None    = 0,
        Read    = 1u << 0,
        Write   = 1u << 1,
        Execute = 1u << 2
    };

    using SmallMask = ::utils::bitmask<BitFlag, std::uint8_t>;
    using LargeMask = ::utils::bitmask<BitFlag, std::uint16_t>;

    // Compile-time sanity checks for constexpr friendliness.
    constexpr SmallMask kCompileTimeMask = BitFlag::Read | BitFlag::Write;
    static_assert(kCompileTimeMask.has(BitFlag::Read), "Bitmask constexpr has(Read) should succeed");
    static_assert(!kCompileTimeMask.has(BitFlag::Execute), "Bitmask constexpr has(Execute) should fail");

    class bitmaskTester : public utils::TestSuite {
    public:
        bitmaskTester() : utils::TestSuite("Bitmask Tester") {
            add_test("Default Construction", "bitmask starts cleared", test_default_construction);
            add_test("Single Flag Construction", "bitmask seeded with one flag", test_single_flag_construction);
            add_test("Is Enum", "is(enum) compares exact bits", test_is_enum);
            add_test("Is Mask", "is(bitmask) compares exact mask", test_is_mask);
            add_test("Has Enum", "has(enum) validates contained bits", test_has_enum);
            add_test("Has Mask Smaller Container", "has(bitmask) works across container sizes", test_has_mask_smaller_container);
            add_test("Operator Or Enum", "mask | enum returns union without mutating operand", test_operator_or_enum);
            add_test("Operator Or Mask", "mask | mask unions both operands", test_operator_or_mask);
            add_test("Operator Or Assign Enum", "operator|= with enum expands mask", test_operator_or_assign_enum);
            add_test("Operator Or Assign Mask", "operator|= with mask merges bits", test_operator_or_assign_mask);
            add_test("Get Returns Container", "get exposes underlying storage", test_get_container_value);
            add_test("Free Enum Or", "enum | enum produces bitmask with both flags", test_free_enum_or);
        }

    private:
        static void test_default_construction() {
            constexpr SmallMask mask;
            ASSERT_EQ(static_cast<std::uint8_t>(0), mask.get());
            ASSERT_TRUE(mask.is(BitFlag::None));
        }

        static void test_single_flag_construction() {
            constexpr SmallMask mask(BitFlag::Write);
            ASSERT_EQ(static_cast<std::uint8_t>(BitFlag::Write), mask.get());
            ASSERT_TRUE(mask.is(BitFlag::Write));
            ASSERT_FALSE(mask.has(BitFlag::Read));
        }

        static void test_is_enum() {
            SmallMask mask(BitFlag::Execute);
            ASSERT_TRUE(mask.is(BitFlag::Execute));
            ASSERT_FALSE(mask.is(BitFlag::Write));

            mask |= BitFlag::Read;
            ASSERT_FALSE(mask.is(BitFlag::Execute));
        }

        static void test_is_mask() {
            SmallMask mask(BitFlag::Read);
            mask |= BitFlag::Write;
            SmallMask copy(mask);
            ASSERT_TRUE(mask.is(copy));

            SmallMask different(BitFlag::Write);
            ASSERT_FALSE(mask.is(different));
        }

        static void test_has_enum() {
            SmallMask mask(BitFlag::Read);
            mask |= BitFlag::Write;
            ASSERT_TRUE(mask.has(BitFlag::Read));
            ASSERT_TRUE(mask.has(BitFlag::Write));
            ASSERT_FALSE(mask.has(BitFlag::Execute));
            ASSERT_TRUE(mask.has(BitFlag::None));
        }

        static void test_has_mask_smaller_container() {
            LargeMask large;
            large |= BitFlag::Read;
            large |= BitFlag::Execute;

            ::utils::bitmask<BitFlag, std::uint8_t> small(BitFlag::Read);
            ASSERT_TRUE(large.has(small));

            small |= BitFlag::Write;
            ASSERT_FALSE(large.has(small));
        }

        static void test_operator_or_enum() {
            SmallMask mask(BitFlag::Read);
            const SmallMask combined = mask | BitFlag::Write;
            ASSERT_TRUE(combined.has(BitFlag::Read));
            ASSERT_TRUE(combined.has(BitFlag::Write));
            ASSERT_FALSE(combined.has(BitFlag::Execute));
            ASSERT_TRUE(mask.is(BitFlag::Read));
        }

        static void test_operator_or_mask() {
            SmallMask mask_a(BitFlag::Read);
            SmallMask mask_b(BitFlag::Execute);
            const SmallMask combined = mask_a | mask_b;
            ASSERT_TRUE(combined.has(BitFlag::Read));
            ASSERT_TRUE(combined.has(BitFlag::Execute));
            ASSERT_FALSE(combined.has(BitFlag::Write));
        }

        static void test_operator_or_assign_enum() {
            SmallMask mask(BitFlag::Read);
            mask |= BitFlag::Execute;
            ASSERT_TRUE(mask.has(BitFlag::Read));
            ASSERT_TRUE(mask.has(BitFlag::Execute));
            ASSERT_FALSE(mask.has(BitFlag::Write));
        }

        static void test_operator_or_assign_mask() {
            SmallMask mask(BitFlag::Read);
            SmallMask other(BitFlag::Write);
            mask |= other;
            ASSERT_TRUE(mask.has(BitFlag::Read));
            ASSERT_TRUE(mask.has(BitFlag::Write));
            ASSERT_FALSE(mask.has(BitFlag::Execute));
        }

        static void test_get_container_value() {
            SmallMask mask(BitFlag::Read);
            mask |= BitFlag::Write;
            const std::uint8_t expected = static_cast<std::uint8_t>(BitFlag::Read) | static_cast<std::uint8_t>(BitFlag::Write);
            ASSERT_EQ(expected, mask.get());
        }

        static void test_free_enum_or() {
            const auto combined = BitFlag::Read | BitFlag::Execute;
            ASSERT_TRUE(combined.has(BitFlag::Read));
            ASSERT_TRUE(combined.has(BitFlag::Execute));
            ASSERT_FALSE(combined.has(BitFlag::Write));
        }
    };
}

#endif