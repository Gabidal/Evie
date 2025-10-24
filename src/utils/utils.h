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

    template<typename, typename>
    class superSet;

    /**
     * Linear space set, giving you the ability to request for sub-sets from this set
     */
    template<typename container, typename value = typename container::value_type>
    class set {
    protected:
        const value* head;               // pointer to the first element of the parent set
        range capacity;                 // parent set size, e.g actual heap limits.
        range size;                     // negotiable and virtual set contains the parent set.

        set(const value* H, range C, range S) : head(H), capacity(C), size(S) {}    // Internal shenanigans
    public:
    
        set(container& realList, range area = {0, 0}) : 
            head(realList.data()),                                  // Absolute pointer to the heap start
            capacity(0, static_cast<int32_t>(realList.size())),     // Full size of the parent list
            size(area)                                              // Initial set size contains the parent set [area.min, area.max]
        {
            if (!capacity.contains(size)) throw std::out_of_range("set: start index out of range");
        }

        const value& operator[](std::size_t index) const {
            if (!size.contains(index)) throw std::out_of_range("set: index out of range");
            return *(head + static_cast<int32_t>(index));
        }

        bool check(range direction) {
            try {
                (void)((*this) | direction);
                return true;
            }
            catch(const std::out_of_range&) {
                return false;
            }
        }

        range getSize() const { return size; }
        range getCapacity() const { return capacity; }

        // Creates an union and returns it if fit
        set operator|(const range area) const {
            range newSize = size | area;
            if (!capacity.contains(newSize)) throw std::out_of_range("set: union out of range");
            return set(head, capacity, newSize);
        }

        // Creates an intersection and returns it if fit
        set operator&(const range area) const {
            range newSize = size & area;
            if (!capacity.contains(newSize)) throw std::out_of_range("set: intersection out of range");
            return set(head, capacity, newSize);
        }

        void operator|=(const range area) {
            range newSize = size | area;
            if (!capacity.contains(newSize)) throw std::out_of_range("set: union out of range");
            size = newSize;
        }

        void operator&=(const range area) {
            range newSize = size & area;
            if (!capacity.contains(newSize)) throw std::out_of_range("set: intersection out of range");
            size = newSize;
        }

        std::vector<set<container, value>> operator^(const range area) const {
            std::vector<set<container, value>> resultSets;

            auto leftovers = size ^ area;

            for (auto& r : leftovers) {
                if (capacity.contains(r)) {
                    resultSets.emplace_back(std::move(set<container, value>(head, capacity, r)));
                }
            }

            return resultSets;
        }

        // Creates a complement/difference and returns it if fit
        std::vector<set<container, value>> operator-(const range area) const {
            std::vector<set<container, value>> resultSets;

            auto leftovers = size - area;

            for (auto& r : leftovers) {
                if (capacity.contains(r)) {
                    resultSets.emplace_back(std::move(set<container, value>(head, capacity, r)));
                }
            }

            return resultSets;
        }

        // Subset check (<=) - this set is subset of or equal to given range
        bool operator<=(const range area) const noexcept {
            return size <= area;
        }

        // Proper subset check (<) - this set is proper subset of given range
        bool operator<(const range area) const noexcept {
            return size < area;
        }

        // Superset check (>=) - this set is superset of or equal to given range
        bool operator>=(const range area) const noexcept {
            return size >= area;
        }

        // Proper superset check (>) - this set is proper superset of given range
        bool operator>(const range area) const noexcept {
            return size > area;
        }

        friend class superSet<container, value>;
    };

    template<typename container, typename value = typename container::value_type>
    class superSet {
        using subset = set<container, value>;   // Less writing yee
        
        range capacity;
        range size;
        
        superSet() {}   // Internal shenanigans
    public:

        std::vector<subset> subsets;
        
        superSet(const std::vector<subset>& sets, range area = {0, 0}) : capacity(area), subsets(sets) {
            rebuildCache();
        }

        superSet(container& realList, range area = {0, 0}) : superSet(std::vector<subset>{ subset(realList, area) }, area) {}

        const value& operator[](std::size_t index) const {
            auto it = std::upper_bound(cache.begin(), cache.end(), index);
            if (it == cache.begin()) throw std::out_of_range("superSet: index too small");
            std::size_t idx = std::distance(cache.begin(), it) - 1;
            return subsets[idx][index];
        }

        void add(subset& s) {
            subsets.push_back(s);
            rebuildCache();
        }

        range getSize() const { return size; }
        range getCapacity() const { return capacity; }

        range begin() const noexcept { return {size.min, size.min}; }
        range end() const noexcept { return {size.max, size.max}; }

        // Creates an union and returns it if fit
        superSet operator|(subset& s) const {
            superSet ss = *this;
            ss.add(s);
            return ss;
        }

        superSet operator|(range r) const {
            return (*this) | subset(nullptr, capacity, r);
        }

        superSet& operator|=(subset& s) {
            add(s);
            return *this;
        }

        superSet operator&(subset& s) {
            superSet ss;

            for (auto& current : subsets) {
                range intersection = current.size & s.size;
                
                // Ensure intersection exists (non-empty range)
                if (intersection.min < intersection.max && current.capacity.contains(intersection)) {
                    ss.subsets.emplace_back(std::move(set<container, value>(current.head, current.capacity, intersection)));
                }
            }

            ss.rebuildCache();
            return ss;
        }

        superSet operator&(range r) const {
            return (*this) & subset(nullptr, capacity, r);
        }

        superSet& operator&=(subset& s) {
            std::vector<subset> newSubsets;

            for (auto& current : subsets) {
                range intersection = current.size & s.size;

                // Ensure intersection exists (non-empty range)
                if (intersection.min < intersection.max && current.capacity.contains(intersection)) {
                    newSubsets.emplace_back(std::move(set<container, value>(current.head, current.capacity, intersection)));
                }
            }

            subsets = std::move(newSubsets);
            rebuildCache();
            return *this;
        }

        superSet operator^(subset& s) {
            superSet ss;

            for (auto& current : subsets) {
                auto intersections = current.getSize() ^ s.getSize(); // vector of non-overlapping ranges

                // If no intersection, keep the whole subset
                if (intersections.empty()) {
                    continue; // everything is overlapped, skip
                }

                // Add all resulting non-overlapping segments
                for (auto& r : intersections) {
                    ss.subsets.emplace_back(std::move(set<container, value>(current.head, current.getCapacity(), r)));
                }
            }

            // Now handle the parts of 's' not overlapping with any subset
            std::vector<range> sParts = { s.getSize() };
            for (auto& current : subsets) {
                std::vector<range> newParts;
                for (auto& part : sParts) {
                    auto leftover = part ^ (current.getSize() & s.getSize());
                    newParts.insert(newParts.end(), leftover.begin(), leftover.end());
                }
                sParts = std::move(newParts);
            }

            for (auto& r : sParts) {
                if (r.min < r.max) {
                    ss.subsets.emplace_back(std::move(set<container, value>(s.head, s.getCapacity(), r)));
                }
            }

            ss.rebuildCache();
            return ss;
        }

        superSet operator^(range r) {
            // Require r in this, otherwise error
            if (!capacity.contains(r)) throw std::out_of_range("superSet: symmetric difference out of range");

            return (*this) ^ subset(nullptr, capacity, r);
        }

        superSet& operator^=(subset& s) {
            *this = (*this) ^ s;
            return *this;
        }

        superSet& operator^=(range r) {
            *this = (*this) ^ r;
            return *this;
        }

        // Complement / Difference
        superSet operator-(subset& s) {
            superSet ss;

            for (auto& current : subsets) {
                auto differences = current.getSize() - s.getSize(); // vector of non-overlapping ranges

                // Add all resulting non-overlapping segments
                for (auto& r : differences) {
                    if (current.capacity.contains(r)) {
                        ss.subsets.emplace_back(std::move(set<container, value>(current.head, current.capacity, r)));
                    }
                }
            }

            ss.rebuildCache();
            return ss;
        }

        superSet& operator-=(subset& s) {
            std::vector<subset> newSubsets;

            for (auto& current : subsets) {
                auto differences = current.getSize() - s.getSize(); // vector of non-overlapping ranges

                // Add all resulting non-overlapping segments
                for (auto& r : differences) {
                    if (current.capacity.contains(r)) {
                        newSubsets.emplace_back(std::move(set<container, value>(current.head, current.capacity, r)));
                    }
                }
            }

            subsets = std::move(newSubsets);
            rebuildCache();
            return *this;
        }

        // Subset check (<=) - all subsets are contained within given subset
        bool operator<=(const subset& s) const noexcept {
            for (const auto& current : subsets) {
                if (!(current.size <= s.size)) {
                    return false;
                }
            }
            return true;
        }

        // Proper subset check (<) - all subsets are properly contained within given subset
        bool operator<(const subset& s) const noexcept {
            return (*this <= s) && (size != s.size);
        }

        // Superset check (>=) - this superset contains the given subset
        bool operator>=(const subset& s) const noexcept {
            return size >= s.size;
        }

        // Proper superset check (>) - this superset properly contains the given subset
        bool operator>(const subset& s) const noexcept {
            return (size > s.size);
        }

        

    private:
        std::vector<std::size_t> cache; // cache[i] = total size of subsets[0..i-1]

        // Each time the super set is changed this needs to be run.
        void rebuildCache() {

            // Respect manual ranges.
            if (capacity == range(0, 0)) {
                capacity = range(0, 0);
                for (auto& c : subsets) capacity |= c.capacity;      // Compute max capacity
            }

            size = range(0, 0);
            for (auto& s : subsets) size |= s.size;                                     // Compute max size

            std::sort(subsets.begin(), subsets.end(), [](const subset& a, const subset& b) { return a.getSize().min < b.getSize().min; });

            cache.clear();

            for (auto& s : subsets) {
                cache.push_back(s.getSize().min); // absolute start of each subset
            }

            if (!capacity.contains(size))
                throw std::out_of_range("superSet: initial size exceeds capacity");
        }
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

template<typename container, typename value = typename container::value_type>
utils::superSet<container, value> operator|(const utils::set<container, value>& lhs, const utils::set<container, value>& rhs) {
    std::vector<utils::set<container, value>> subsets;
    subsets.reserve(2);
    subsets.push_back(lhs);
    subsets.push_back(rhs);

    return utils::superSet<container, value>(subsets);
}

template<typename container, typename value = typename container::value_type>
utils::set<container, value> operator&(const utils::set<container, value>& lhs, const utils::set<container, value>& rhs) {
    utils::range intersection = lhs.getSize() & rhs.getSize();
    if (!lhs.getCapacity().contains(intersection) || !rhs.getCapacity().contains(intersection))
        throw std::out_of_range("superSet &: intersection out of range");

    return utils::set<container, value>(lhs.getCapacity().min == rhs.getCapacity().min ? lhs & intersection : rhs & intersection);
}

template<typename enumType, typename valueType = std::underlying_type_t<enumType>>
constexpr utils::bitmask<enumType, valueType> operator|(enumType a, enumType b) noexcept {
    utils::bitmask<enumType, valueType> bm(a);
    bm |= b;
    return bm;
}

#endif