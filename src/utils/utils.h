#ifndef _EVIE_UTILS_H_
#define _EVIE_UTILS_H_

#include <type_traits>
#include <cstdint>
#include <stdexcept>
#include <algorithm>
#include <vector>

namespace utils {

    template<typename enumType, typename valueType = std::underlying_type_t<enumType>>
    class bitmask {
    protected:
        valueType container = 0;
    public:
        /**
         * @brief Initializes an empty bitmask with no flags set.
         */
        constexpr bitmask() noexcept = default;

        /**
         * @brief Initializes the bitmask with a single enumeration value.
         * @param value Enumeration flag used to seed the mask.
         */
        constexpr bitmask(enumType value) noexcept : container(static_cast<valueType>(value)) {}

        /**
         * @brief Checks whether the mask matches exactly the provided flag.
         * @param value Enumeration flag to compare against.
         * @return True when the stored bits equal @p value.
         */
        constexpr bool is(enumType value) const noexcept {
            return container == static_cast<valueType>(value);
        }

        /**
         * @brief Checks whether this mask matches another mask exactly.
         * @param value Another bitmask instance.
         * @return True when both masks store identical bits.
         */
        constexpr bool is(bitmask value) const noexcept {
            return container == value.container;
        }

        /**
         * @brief Tests whether the mask contains all bits in @p value.
         * @param value Enumeration flag to test.
         * @return True when every bit in @p value is present in the mask.
         */
        constexpr bool has(enumType value) const noexcept {
            return (container & static_cast<valueType>(value)) == static_cast<valueType>(value);
        }

        /**
         * @brief Tests whether this mask contains all bits from another mask with a compatible storage type.
         * @tparam OtherContainer Storage type of the other mask.
         * @param other Bitmask to test against.
         * @return True when every bit in @p other is present in this mask.
         */
        template<typename OtherContainer>
        constexpr bool has(bitmask<enumType, OtherContainer> other) const noexcept {
            static_assert(
                sizeof(OtherContainer) <= sizeof(valueType),
                "bitmask::has() requires the other container to be equal or smaller in size."
            );

            // Truncate own container to the smaller type before comparing.
            const auto truncated = static_cast<OtherContainer>(container);
            return (truncated & other.get()) == other.get();
        }

        /**
         * @brief Combines the current mask with @p value and returns the result.
         * @param value Enumeration flag to merge.
         * @return New bitmask containing the union of flags.
         */
        constexpr bitmask operator|(enumType value) const noexcept {
            bitmask b = *this;
            b.container |= static_cast<valueType>(value);
            return b;
        }

        /**
         * @brief Merges @p value flags into the current mask.
         * @param value Enumeration flag to add.
         * @return Reference to the updated mask.
         */
        constexpr bitmask& operator|=(enumType value) noexcept {
            container |= static_cast<valueType>(value);
            return *this;
        }

        /**
         * @brief Combines two bitmasks into a new mask containing all bits from both.
         * @param value Bitmask whose flags will be union'd.
         * @return New bitmask representing the union.
         */
        constexpr bitmask operator|(bitmask value) const noexcept {
            bitmask b = *this;
            b.container |= value.container;
            return b;
        }

        /**
         * @brief Merges another bitmask into this mask.
         * @param value Bitmask whose flags will be added.
         * @return Reference to the updated mask.
         */
        constexpr bitmask& operator|=(bitmask value) noexcept {
            container |= value.container;
            return *this;
        }

        /**
         * @brief Exposes the underlying storage value representing the set bits.
         * @return Stored container value.
         */
        constexpr valueType get() const noexcept {
            return container;
        }
    };

    class range {
    public:
        int32_t min;
        int32_t max;

        range(int32_t Min = 0, int32_t Max = 0) : min(Min), max(Max) {}

        uint32_t length() const noexcept { return static_cast<uint32_t>(max - min); }

        range operator+(const range& other) const noexcept { return range(min + other.min, max + other.max); }
    
        range& operator+=(const range& other) { min += other.min; max += other.max; return *this; }

        // Union
        range operator|(const range& other) const noexcept { return range(std::min(min, other.min), std::max(max, other.max)); }
        
        // Union
        range& operator|=(const range& other) { min = std::min(min, other.min); max = std::max(max, other.max); return *this; }
        
        // Intersection
        range operator&(const range& other) const noexcept { return range(std::max(min, other.min), std::min(max, other.max)); }
        
        // Intersection
        range& operator&=(const range& other) { min = std::max(min, other.min); max = std::min(max, other.max); return *this; }

        // Symmetric Difference
        std::vector<range> operator^(const range& other) const noexcept {
            std::vector<range> result;

            range inter = *this & other;
            if (inter.min >= inter.max) {
                // No overlap, return both
                result.push_back(*this);
                result.push_back(other);
                return result;
            }

            // Left segment
            if (min < inter.min) result.emplace_back(min, inter.min);

            // Right segment
            if (inter.max < max) result.emplace_back(inter.max, max);

            // Other's left segment
            if (other.min < inter.min) result.emplace_back(other.min, inter.min);

            // Other's right segment
            if (inter.max < other.max) result.emplace_back(inter.max, other.max);

            return result;
        }

        // Complement / Difference
        std::vector<range> operator-(const range& other) const noexcept {
            std::vector<range> result;

            range inter = *this & other;
            if (inter.min >= inter.max) {
                // No overlap, return the whole range
                result.push_back(*this);
                return result;
            }

            // Left segment
            if (min < inter.min) result.emplace_back(min, inter.min);

            // Right segment
            if (inter.max < max) result.emplace_back(inter.max, max);

            return result;
        }

        bool operator==(const range& other) const noexcept { return min == other.min && max == other.max; }

        bool operator!=(const range& other) const noexcept { return !(*this == other); }

        // Subset check (<=) - this range is subset of or equal to other
        bool operator<=(const range& other) const noexcept { return min >= other.min && max <= other.max; }

        // Proper subset check (<) - this range is proper subset of other
        bool operator<(const range& other) const noexcept { return (*this <= other) && (*this != other); }

        // Superset check (>=) - this range is superset of or equal to other
        bool operator>=(const range& other) const noexcept { return min <= other.min && max >= other.max; }

        // Proper superset check (>) - this range is proper superset of other
        bool operator>(const range& other) const noexcept { return (*this >= other) && (*this != other); }

        bool contains(int32_t val) const noexcept {
            return min <= val && max >= val;
        }

        bool contains(const range& val) const noexcept {
            return min <= val.min && max >= val.max;
        }

        // Used by for loops
        void operator++() { ++min; ++max; }
    };
}

// Auto un-namespace locked utilities:

template<typename enumType>
constexpr bool operator<(enumType a, enumType b) noexcept {
    using valueType = std::underlying_type_t<enumType>;
    return static_cast<valueType>(a) < static_cast<valueType>(b);
}

template<typename enumType>
constexpr enumType& operator++(enumType& a) noexcept {
    using valueType = std::underlying_type_t<enumType>;
    a = static_cast<enumType>(static_cast<valueType>(a) + 1);
    return a;
}

template<typename enumType, typename valueType = std::underlying_type_t<enumType>>
constexpr utils::bitmask<enumType, valueType> operator|(enumType a, enumType b) noexcept {
    utils::bitmask<enumType, valueType> bm(a);
    bm |= b;
    return bm;
}

#endif