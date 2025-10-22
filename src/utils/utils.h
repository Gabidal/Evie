#ifndef _EVIE_UTILS_H_
#define _EVIE_UTILS_H_

#include <type_traits>
#include <cstdint>
#include <stdexcept>
#include <algorithm>
#include <vector>

namespace utils {

    template<typename enumType, typename containerSize = std::underlying_type_t<enumType>>
    class bitmask {
    protected:
        containerSize container = 0;
    public:
        /**
         * @brief Initializes an empty bitmask with no flags set.
         */
        constexpr bitmask() noexcept = default;

        /**
         * @brief Initializes the bitmask with a single enumeration value.
         * @param value Enumeration flag used to seed the mask.
         */
        constexpr bitmask(enumType value) noexcept : container(static_cast<containerSize>(value)) {}

        /**
         * @brief Checks whether the mask matches exactly the provided flag.
         * @param value Enumeration flag to compare against.
         * @return True when the stored bits equal @p value.
         */
        constexpr bool is(enumType value) const noexcept {
            return container == static_cast<containerSize>(value);
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
            return (container & static_cast<containerSize>(value)) == static_cast<containerSize>(value);
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
                sizeof(OtherContainer) <= sizeof(containerSize),
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
            b.container |= static_cast<containerSize>(value);
            return b;
        }

        /**
         * @brief Merges @p value flags into the current mask.
         * @param value Enumeration flag to add.
         * @return Reference to the updated mask.
         */
        constexpr bitmask& operator|=(enumType value) noexcept {
            container |= static_cast<containerSize>(value);
            return *this;
        }

        /**
         * @brief Combines two bitmasks into a new mask containing all bits from both.
         * @param value Bitmask whose flags will be unioned.
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
        constexpr containerSize get() const noexcept {
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

        range operator-(const range& other) const noexcept { return range(min - other.min, max - other.max); }

        range& operator-=(const range& other) { min -= other.min; max -= other.max; return *this; }

        // Union
        range operator|(const range& other) const noexcept { return range(std::min(min, other.min), std::max(max, other.max)); }
        
        // Union
        range& operator|=(const range& other) { min = std::min(min, other.min); max = std::max(max, other.max); return *this; }
        
        // Intersection
        range operator&(const range& other) const noexcept { return range(std::max(min, other.min), std::min(max, other.max)); }
        
        // Intersection
        range& operator&=(const range& other) { min = std::max(min, other.min); max = std::min(max, other.max); return *this; }

        bool operator==(const range& other) const noexcept { return min == other.min && max == other.max; }

        bool operator!=(const range& other) const noexcept { return !(*this == other); }

        bool contains(int32_t val) const noexcept {
            return min <= val && max >= val;
        }

        bool inside(const range& val) const noexcept {
            return min <= val.min && max >= val.max;
        }

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
        const range capacity;           // parent set size, e.g actual heap limits.
        range size;                     // negotiable and virtual set inside the parent set.

        set(const value* H, range C, range S) : head(H), capacity(C), size(S) {}    // Internal shenanigans
    public:
    
        set(container& realList, range area = {0, 0}) : 
            head(realList.data()),                                  // Absolute pointer to the heap start
            capacity(0, static_cast<int32_t>(realList.size())),     // Full size of the parent list
            size(area)                                              // Initial set size inside the parent set [area.min, area.max]
        {
            if (!size.inside(capacity)) throw std::out_of_range("set: start index out of range");
        }

        const value& operator[](std::size_t index) const {
            if (!size.contains(index)) throw std::out_of_range("set: index out of range");
            return *(head + size.min + static_cast<int32_t>(index));
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

        // Creates an union and returns it if fit
        set operator|(const range area) const {
            range newSize = size | area;
            if (!newSize.inside(capacity)) throw std::out_of_range("set: union out of range");
            return set(head, capacity, newSize);
        }

        // Creates an intersection and returns it if fit
        set operator&(const range area) const {
            range newSize = size & area;
            if (!newSize.inside(capacity)) throw std::out_of_range("set: intersection out of range");
            return set(head, capacity, newSize);
        }

        void operator|=(const range area) {
            range newSize = size | area;
            if (!newSize.inside(capacity)) throw std::out_of_range("set: union out of range");
            size = newSize;
        }

        void operator&=(const range area) {
            range newSize = size & area;
            if (!newSize.inside(capacity)) throw std::out_of_range("set: intersection out of range");
            size = newSize;
        }

        friend class superSet<container, value>;
    };

    template<typename container, typename value = typename container::value_type>
    class superSet {
        using subset = set<container, value>;   // Less writing yee

        superSet() {}   // Internal shenanigans
    public:
        std::vector<subset> subsets;
        range capacity;
        range size;

        superSet(const std::vector<subset>& sets, range area) : subsets(sets), capacity(area) {
            rebuildCache();
        }

        const value& operator[](std::size_t index) const {
            auto it = std::upper_bound(cache.begin(), cache.end(), index);
            if (it == cache.begin()) throw std::out_of_range("superSet: index too small");
            std::size_t idx = std::distance(cache.begin(), it) - 1;
            return subsets[idx][index - cache[idx]];
        }

        void add(subset& s) {
            subsets.push_back(s);
            rebuildCache();
        }

        // Creates an union and returns it if fit
        superSet operator|(subset& s) const {
            superSet ss = *this;
            ss.add(s);
            return ss;
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
                if (intersection.min < intersection.max && intersection.inside(current.capacity)) {
                    ss.subsets.emplace_back(std::move(set<container, value>(current.head, current.capacity, intersection)));
                }
            }

            ss.rebuildCache();
            return ss;
        }

        superSet& operator&=(subset& s) {
            std::vector<subset> newSubsets;

            for (auto& current : subsets) {
                range intersection = current.size & s.size;

                // Ensure intersection exists (non-empty range)
                if (intersection.min < intersection.max && intersection.inside(current.capacity)) {
                    newSubsets.emplace_back(std::move(set<container, value>(current.head, current.capacity, intersection)));
                }
            }

            subsets = std::move(newSubsets);
            rebuildCache();
            return *this;
        }

    private:
        std::vector<std::size_t> cache; // cache[i] = total size of subsets[0..i-1]

        // Each time the super set is changed this needs to be run.
        void rebuildCache() {
            for (auto& c : subsets) capacity |= c.capacity;      // Compute max capacity
            for (auto& s : subsets) size |= s.size;              // Compute max size

            // Sanity check
            if (!size.inside(capacity))
                throw std::out_of_range("superSet: initial size exceeds capacity");

            cache.clear();
            cache.reserve(subsets.size());
            std::size_t sum = 0;
            for (auto& s : subsets) {
                cache.push_back(sum);
                sum += s.size.max - s.size.min; // effective size
            }
        }
    };
}

// Auto un-namespace locked utilities:

template<typename enumType, typename containerSize = std::underlying_type_t<enumType>>
constexpr utils::bitmask<enumType, containerSize> operator|(enumType a, enumType b) noexcept {
    utils::bitmask<enumType, containerSize> bm(a);
    bm |= b;
    return bm;
}

#endif