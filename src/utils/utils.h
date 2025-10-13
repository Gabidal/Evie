#ifndef _EVIE_UTILS_H_
#define _EVIE_UTILS_H_

#include <type_traits>
#include <cstdint>
#include <stdexcept>
#include <algorithm>

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

    /**
     * Instances an view to an larger list, for the caller to know how much the callee allocated further from the given view.
     */
    template<
        typename listType,
        typename innerType = typename listType::value_type
    >
    class listView {
    protected:
        innerType* head;            // pointer to the first element of the current view window
        const std::size_t capacity; // total number of elements accessible from head
        std::size_t size;           // negotiable ending index of the view
    public:
    
        /**
         * @brief Creates a view anchored at @p startIndex with an optional initial size.
         * @param realList Source container the view references.
         * @param startIndex Index in @p realList where the view begins.
         * @param viewSize Initial number of elements exposed by the view.
         * @throws std::out_of_range If @p startIndex exceeds @p realList boundaries.
         */
        listView(listType& realList, std::size_t startIndex, std::size_t viewSize = 1) : 
            head(realList.data() + std::min(startIndex, realList.size())), 
            capacity(realList.size() - std::min(startIndex, realList.size())),
            size(viewSize) 
        {
            if (startIndex > realList.size()) throw std::out_of_range("listView: start index out of range");
            if (size > capacity) size = capacity;  // auto clamp, could break
        }

    private:
        listView(innerType* baseHead, std::size_t baseCapacity, std::size_t viewSize) : head(baseHead), capacity(baseCapacity), size(viewSize) {
            if (size > capacity) size = capacity;
        }

    public:
    
        /**
         * @brief Extends the current view by reserving additional elements.
         * @param additionalSize Number of extra slots requested for the view.
         * @return True when the extension fits within the underlying container.
         */
        bool allocate(std::size_t additionalSize) {
            if (size + additionalSize > capacity) return false;

            size += additionalSize;

            return true;
        }

        /**
         * @brief Provides read access to an element in the bounded view.
         * @param index Relative index inside the view window.
         * @return Const reference to the requested element.
         * @throws std::out_of_range If @p index lies outside the whole parent view.
         */
        const innerType& operator[](std::size_t index) const {
            // Since this operator is used to check wether the specific index contains some value we can allow the parent window.
            if (index >= capacity) throw std::out_of_range("listView: index out of range");
            return head[index];
        }

        /**
         * @brief Provides write access to an element in the bounded view.
         * @param index Relative index inside the view window. If this is outside the active view size, tries to allocate to fit.
         * @return Reference to the requested element.
         * @throws std::out_of_range If @p index lies outside the active view and allocatable size.
         */
        innerType& operator[](std::size_t index) {
            if (index >= size && !allocate(index - size + 1)) {
                throw std::out_of_range("listView: index out of range");
            }
            return head[index];
        }

        /**
         * @brief Creates a child view that starts at @p subStart within the current view.
         * @param subStart Offset relative to the current view start.
         * @param subSize Desired size of the child view; auto-clamped if needed.
         * @return A new view referencing the requested sub-range.
         * @throws std::out_of_range If @p subStart exceeds the current view size.
         */
        listView subView(std::size_t subStart, std::size_t subSize = 1) const {
            if (subStart >= size) throw std::out_of_range("listView: subView start index out of range");
            if (subStart + subSize > size) subSize = size - subStart;  // auto clamp, could break
            return listView(head + subStart, capacity - subStart, subSize);
        }

        /**
         * @brief Returns the exclusive end index of the view into the underlying list.
         * @return Exclusive upper bound inside the source container relative to head.
         */
        std::size_t end() const noexcept { return size; }

        /**
         * @brief Returns an iterator to the first element in the view.
         * @return Pointer to the beginning of the view window.
         */
        innerType* begin() noexcept { return head; }

        /**
         * @brief Returns a const iterator to the first element in the view.
         * @return Const pointer to the beginning of the view window.
         */
        const innerType* begin() const noexcept { return head; }

        /**
         * @brief Returns a const iterator to the first element in the view.
         * @return Const pointer to the beginning of the view window.
         */
        const innerType* cbegin() const noexcept { return head; }

        /**
         * @brief Returns an iterator one past the last element in the view.
         * @return Pointer to the element following the view window.
         */
        innerType* endIter() noexcept { return head + size; }

        /**
         * @brief Returns a const iterator one past the last element in the view.
         * @return Const pointer to the element following the view window.
         */
        const innerType* endIter() const noexcept { return head + size; }

        /**
         * @brief Returns a const iterator one past the last element in the view.
         * @return Const pointer to the element following the view window.
         */
        const innerType* cend() const noexcept { return head + size; }
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