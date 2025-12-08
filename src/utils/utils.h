#ifndef _EVIE_UTILS_H_
#define _EVIE_UTILS_H_

#include <type_traits>
#include <cstdint>
#include <stdexcept>
#include <algorithm>
#include <vector>

namespace utils {
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

    class link;

    class linkable {
    protected:
        linkable* sibling = nullptr;
    public:

        linkable() = default;
        virtual ~linkable();

        void connect(linkable* other);
        void disconnect();
        
        // Helper to get the partner safely
        linkable* getConnected() const { return sibling; }
    };

    enum class boolToInt {
        FALSE = 0,
        TRUE = 1
    };

    namespace boolToString {
        constexpr const char* FALSE = "false";
        constexpr const char* TRUE  = "true";
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


#endif