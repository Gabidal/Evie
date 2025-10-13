// ============================================================================
//  GGUI - Amalgamated TUI Framework Header
//
//  This file was auto-generated via the ./bin/export.sh
//
//  TABLE OF CONTENTS
//    [1] src/core/utils/superString.h
//    [2] src/core/utils/fastVector.h
//    [3] src/core/utils/constants.h
//    [4] src/core/utils/color.h
//    [5] src/core/utils/types.h
//    [6] src/core/utils/utf.h
//    [7] src/core/utils/style.h
//    [8] src/core/utils/settings.h
//    [9] src/core/utils/utils.h
//    [10] src/core/utils/logger.h
//    [11] src/core/utils/fileStreamer.h
//    [12] src/core/utils/drm.h
//    [13] src/elements/element.h
//    [14] src/elements/listView.h
//    [15] src/elements/textField.h
//    [16] src/elements/canvas.h
//    [17] src/elements/HTML.h
//    [18] src/elements/progressBar.h
//    [19] src/elements/switch.h
//    [20] src/core/renderer.h
//    [21] src/core/addons/addons.h
// ============================================================================

#pragma once

#ifndef _SUPER_STRING_H_
#define _SUPER_STRING_H_

#include <string>
#include <cstring>
#include <array>
#include <vector>
#include <variant>
#include <initializer_list>

namespace GGUI{
    namespace INTERNAL{
        namespace COMPACT_STRING_FLAG{
            constexpr inline unsigned char IS_ASCII          = 1 << 0;
            constexpr inline unsigned char IS_UNICODE        = 1 << 1;
        }

        /**
         * @class Compact_String
         * @brief A lightweight string class optimized for compact storage of ASCII and Unicode strings.
         *
         * The Compact_String class provides an efficient way to store and manipulate short strings,
         * optimizing for the case where the string is a single ASCII character. For longer strings,
         * it stores a pointer to a null-terminated C-style string. The class uses a std::variant to
         * hold either a single character or a pointer to a string, and maintains the size of the string.
         *
         * Key Features:
         * - Stores either a single ASCII character or a pointer to a C-style string.
         * - Provides constructors for ASCII characters, C-style strings, and explicit size/force Unicode.
         * - Offers fast type and content checks for ASCII and Unicode representations.
         * - Supports subscript operator for character access.
         * - Utility methods for getting and setting ASCII/Unicode data.
         * - Designed for use in scenarios where memory efficiency and fast type checks are important.
         *
         * Usage Notes:
         * - The default constructor is intended only for resizing containers and should not be used directly.
         * - The class does not manage the lifetime of external string data; ensure that any pointer passed
         *   to the class remains valid for the lifetime of the Compact_String instance.
         * - The class is constexpr-friendly for compile-time usage where possible.
         */
        class compactString{
        public:
            std::variant<char, const char*> text;
            unsigned int size = 0;

            /**
             * @brief Empty constructor for the Compact_String class. This is only used for resizing a vector of Compact_Strings, and should not be used directly.
             * @warning Do not use this constructor directly, as it will not initialize the Data property.
             * This constructor initializes a Compact_String object with default values.
             */
            constexpr compactString() = default;

            /**
             * @brief Default copy constructor for Compact_String.
             *
             * Creates a new Compact_String object as a copy of an existing one.
             * This constructor performs a member-wise copy of the source object.
             *
             * @param other The Compact_String instance to copy from.
             */
            constexpr compactString(const compactString&) = default;

            /**
             * @brief Move constructor for Compact_String.
             *
             * Constructs a new Compact_String by transferring the resources from another
             * Compact_String instance. The source object is left in a valid but unspecified state.
             *
             * @param other The Compact_String instance to move from.
             */
            constexpr compactString(compactString&&) = default;

            /**
             * @brief Default copy assignment operator for Compact_String.
             *
             * Assigns the contents of another Compact_String to this one.
             * The default implementation performs a member-wise copy of all fields.
             *
             * @param other The Compact_String instance to copy from.
             * @return Reference to this Compact_String after assignment.
             */
            constexpr compactString& operator=(const compactString&) = default;

            /**
             * @brief Default move assignment operator for Compact_String.
             *
             * Allows assigning the contents of another Compact_String to this one using move semantics.
             * The default implementation efficiently transfers resources from the source object,
             * leaving it in a valid but unspecified state.
             *
             * @return Reference to this Compact_String after assignment.
             */
            constexpr compactString& operator=(compactString&&) = default;

            /**
             * @brief Constructs a Compact_String object from a C-style string.
             * 
             * This constructor initializes the Compact_String object by determining the length of the input string.
             * If the length of the string is greater than 1, it stores the string data in Unicode_Data.
             * If the length of the string is 1 or less, it stores the single character in Ascii_Data.
             * 
             * @param data A pointer to a null-terminated C-style string.
             */
            constexpr compactString(const char* data){
                // Store the string as Unicode data if its length is greater than 1.
                // Store the single character as ASCII data.
                size_t length = getLength(data);

                // Check for the most cases
                length > 1 ?    // Transfer larger than single char strings
                    setUnicode(data) : 
                    length == 1 ?   // Handle single character strings and zero length strings
                        setAscii(data[0]) : 
                        setAscii('\0'); // Handle empty string case

            }

            /**
             * @brief Constructs a Compact_String object with a single ASCII character.
             * 
             * This constructor initializes the Compact_String with a single character.
             * The character is stored in the Ascii_Data member of the Data union, and
             * the Size is set to 1.
             * 
             * @param data The ASCII character to initialize the Compact_String with.
             */
            constexpr compactString(const char data) : text(data){
                size = 1;
            }

            /**
             * @brief Constructs a Compact_String object.
             * 
             * This constructor initializes a Compact_String object with the given data and size.
             * It determines the storage format based on the size of the data and the Force_Unicode flag.
             * 
             * @param data A pointer to the character data to be stored.
             * @param size The size of the character data.
             * @param Force_Unicode A boolean flag indicating whether to force the data to be stored as Unicode.
             *                       Defaults to false.
             * 
             * If the size of the data is greater than 1 or if Force_Unicode is true, the data is stored as Unicode.
             * Otherwise, the data is stored as a single ASCII character.
             */
            constexpr compactString(const char* data, const unsigned int Size, const bool forceUnicode = false){
                // Determine data storage based on size and Force_Unicode flag.
                // Store as Unicode data if size is greater than 1 or forced.
                // Store as a single ASCII character.
                (Size > 1 || forceUnicode) ? 
                setUnicode(data) : 
                setAscii(data[0]);

                // If force unicode has been issued, then the size is probably a non-unicode standard size of zero or one, so we need to override the size.
                if (forceUnicode)
                    size = Size;
            }

            /**
             * @brief Checks if a specific UTF flag is set.
             * @param cs_flag The UTF flag to check.
             * @return True if the flag is set, otherwise false.
             */
            constexpr bool is(unsigned char cs_flag) const {
                return (
                    cs_flag == COMPACT_STRING_FLAG::IS_ASCII && std::holds_alternative<char>(text)
                ) || (
                    cs_flag == COMPACT_STRING_FLAG::IS_UNICODE && std::holds_alternative<const char*>(text)
                ) ? true : false;
            }

            // Fast comparison of type and content
            constexpr bool is(const char* other) const {
                return is(COMPACT_STRING_FLAG::IS_UNICODE) ? std::strcmp(std::get<const char*>(text), other) == 0 : false;
            }

            // Fast comparison of type and content
            constexpr bool is(char other) const {
                return is(COMPACT_STRING_FLAG::IS_ASCII) ? std::get<char>(text) == other : false;
            }

            /**
             * @brief Overloaded subscript operator to access character at a given index.
             * 
             * This operator allows access to the character at the specified index.
             * If the size of the string is greater than 1, it returns the character
             * from the Unicode data. If the size is 1, it returns the ASCII data.
             * 
             * @param index The index of the character to access.
             * @return char The character at the specified index.
             */
            constexpr char operator[](int index) const {
                return ((unsigned)index >= size || index < 0) ? 
                    '\0' : // Return null character if index is out of bounds.
                    (is(COMPACT_STRING_FLAG::IS_UNICODE) ? std::get<const char*>(text)[index] : std::get<char>(text));  // Return the character from Unicode or ASCII data.
            }

            constexpr const char* getUnicode(bool force = false) const {
                // If the size is greater than 1, return the Unicode data.
                // Otherwise, return a pointer to the ASCII data.
                return is(COMPACT_STRING_FLAG::IS_UNICODE) || force ? 
                    std::get<const char*>(text) : 
                    nullptr;
            }

            constexpr char getAscii() const {
                // If the size is 1, return the ASCII data.
                // Otherwise, return a null character.
                return is(COMPACT_STRING_FLAG::IS_ASCII) ? 
                    std::get<char>(text) : 
                    '\0';
            }

            constexpr void setUnicode(const char* Text) {
                // Set the Text to the Unicode data.
                text = std::variant<char, const char*>(Text);
                size = getLength(Text); // Update the size based on the new string.
            }

            constexpr void setAscii(const char Text) {
                // Set the Text to the ASCII data.
                text = std::variant<char, const char*>(Text);
                size = 1; // Update the size to 1 since it's a single character.
            }
            
            /**
             * @brief Checks if the UTF object has a default text.
             * @return true if the UTF object has a default text, false otherwise.
             */
            constexpr bool hasDefaultText() const {
                return is(COMPACT_STRING_FLAG::IS_ASCII) ? std::get<char>(text) == ' ' : std::get<const char*>(text)[0] == ' ';
            }

            constexpr bool empty() const {
                // Check if the Compact_String is empty.
                // An empty Compact_String has a size of 0.
                return size == 0;
            }

        protected:
            /**
             * @brief Computes the length of a null-terminated C-string at compile time.
             *
             * This constexpr function iterates through the input string until it encounters
             * the null terminator ('\0'), counting the number of characters.
             *
             * @param str Pointer to the null-terminated C-string.
             * @return The number of characters in the string, excluding the null terminator.
             */
            constexpr size_t getLength(const char* str) {
                size_t length = 0;

                if (!str) {
                    return length; // Return 0 if the input string is null.
                }

                while (str[length] != '\0') ++length;
                return length;
            }
        };

        /**
         * @class Super_String
         * @brief A container class for efficiently managing and concatenating multiple Compact_String objects.
         *
         * The Super_String class provides a fixed-size array of Compact_String objects, allowing for efficient
         * storage, addition, and concatenation of strings and characters. It maintains an internal index to track
         * the current number of stored elements and a liquefied size representing the total number of characters
         * across all stored strings. The class supports various methods for adding strings, characters, and other
         * Super_String instances, as well as clearing its contents and converting the stored data into a single
         * std::string.
         *
         * @tparam maxSize The maximum number of Compact_String objects that can be stored in the Super_String.
         *
         * @note This class is designed for performance and memory efficiency, making it suitable for scenarios
         *       where frequent string concatenation and manipulation are required.
         */
        template<std::size_t maxSize>
        class superString{
        public:
            // Data points either to inlineStorage (default) or an external window provided by fastVector.
            compactString* data = nullptr;
            unsigned int currentIndex = 0;
            unsigned int liquefiedSize = 0;

        protected:
            // Inline storage to avoid dynamic allocations for the common case.
            std::array<compactString, maxSize> inlineStorage{};
        
        public:

            /**
             * @brief Default constexpr constructor for the Super_String class.
             *
             * Initializes a Super_String object with default values at compile time.
             * This constructor does not perform any custom initialization logic.
             */
            constexpr superString() {
                // By default, use inline storage to avoid heap allocations.
                data = inlineStorage.data();
                currentIndex = 0;
                liquefiedSize = 0;
            }

            /**
             * @brief Constructs a Super_String from an initializer list of Compact_String objects.
             *
             * This constructor allows you to initialize a Super_String with a list of Compact_String
             * instances. Each item in the initializer list is added to the Super_String using the Add method.
             *
             * @param data An initializer list containing Compact_String objects to be added to the Super_String.
             */
            constexpr superString(const std::initializer_list<compactString>& Data) : superString() {
                for (const auto& item : Data) {
                    add(item);
                }
            }

            constexpr superString(compactString* preAllocatedWindow) {
                // Use external window memory; caller manages its lifetime.
                data = preAllocatedWindow;
                currentIndex = 0;
                liquefiedSize = 0;
            }

            /**
             * @brief Clears the contents of the Super_String.
             * 
             * This function resets the current index back to the start of the vector,
             * effectively clearing any stored data.
             */
            constexpr void clear(){
                // Set the current index back to the start of the vector.
                currentIndex = 0;
                liquefiedSize = 0;
            }

            /**
             * @brief Adds a new string to the data vector.
             * 
             * This function stores the provided string in the data vector by creating a 
             * Compact_String object from the given data and size, and then adds it to 
             * the Data vector at the current index.
             * 
             * @param data Pointer to the character array containing the string to be added.
             * @param size The size of the string to be added.
             */
            constexpr void add(const char* Data, const int size){
                // Store the string in the Data vector.
                compactString tmp = compactString(Data, size);
                data[currentIndex++] = tmp;
                liquefiedSize += tmp.size; // Update the liquefied size with the size of the new string.
            }

            /**
             * @brief Adds a character to the Super_String.
             * 
             * This function stores the given character in the data vector
             * and increments the current index.
             * 
             * @param data The character to be added to the Super_String.
             */
            constexpr void add(const char Data){
                // Store the character in the data vector.
                data[currentIndex++] = compactString(Data);
                liquefiedSize += 1; // Update the liquefied size with the size of the new character.
            }

            /**
             * @brief Adds the contents of another Super_String to this Super_String.
             *
             * This function appends the contents of the provided Super_String to the current
             * Super_String. If the Expected parameter is false, the function will resize the
             * Data vector to accommodate the additional characters.
             *
             * @param other A pointer to the Super_String to be added.
             * @param Expected A boolean flag indicating whether the reservation size is already
             *                 expected to be sufficient. If false, the Data vector will be resized.
             */
            template<std::size_t OtherMaxSize>
            constexpr void add(const superString<OtherMaxSize>* other){
                // Copy the contents of the other Super_String into the Data vector.
                for (unsigned int i = 0; i < other->currentIndex; i++){
                    data[currentIndex++] = other->data[i];
                }

                liquefiedSize += other->liquefiedSize; // Update the liquefied size with the size of the new string.
            }
            
            /**
             * @brief Add the contents of another Super_String to this one.
             * @param other The Super_String to add.
             * @param Expected If true, the size of the Data vector will not be changed.
             * @details This function is used to concatenate Super_Strings.
             */
            template<std::size_t OtherMaxSize>
            constexpr void add(const superString<OtherMaxSize>& other){
                // Copy the contents of the other Super_String into the Data vector.
                for (unsigned int i = 0; i < other.currentIndex; i++){
                    data[currentIndex++] = other.data[i];
                }
                
                liquefiedSize += other.liquefiedSize; // Update the liquefied size with the size of the new string.
            }

            /**
             * @brief Adds a Compact_String to the data vector.
             * 
             * This function appends the given Compact_String to the current position
             * in the Data vector and then increments the Current_Index.
             * 
             * @param other The Compact_String to add to the data vector.
             */
            constexpr void add(const compactString& other){
                // Store the Compact_String in the data vector.
                data[currentIndex++] = other;
                liquefiedSize += other.size; // Update the liquefied size with the size of the new Compact_String.
            }

            /**
             * @brief Converts the Super_String object to a std::string.
             * 
             * This function goes through the Data vector and calculates the total size of all
             * the strings stored in the vector. It then resizes a std::string to that size and
             * then copies the contents of the Data vector into the std::string.
             * 
             * @return A std::string that contains all the strings stored in the Data vector.
             */
            std::string toString() {
                // Resize a std::string to the total size.
                std::string result;
                result.resize(liquefiedSize);

                // Copy the contents of the Data vector into the std::string.
                int Current_UTF_Insert_Index = 0;
                for(unsigned int i = 0; i < currentIndex; i++){
                    const compactString& Data = data[i];

                    if (Data.size == 0)
                        break;

                    // Size of ones are always already loaded from memory into a char.
                    if (Data.size > 1){
                        // Replace the current contents of the string with the contents of the Unicode Data.
                        result.replace(Current_UTF_Insert_Index, Data.size, Data.getUnicode());

                        Current_UTF_Insert_Index += Data.size;
                    }
                    else{
                        // Add the single character to the string.
                        result[Current_UTF_Insert_Index++] = Data.getAscii();
                    }
                }
                return result;
            }
        };
    }
}

#endif
#ifndef _FAST_VECTOR_H_
#define _FAST_VECTOR_H_

#include <type_traits>
#include <cstring> // std::memcpy


namespace GGUI {
}

#endif
#ifndef _CONSTANTS_H_
#define _CONSTANTS_H_

#include <string>
#include <chrono>
#include <stdexcept>


namespace GGUI{
    namespace TIME{
        constexpr  unsigned int MILLISECOND = 1; 
        constexpr  unsigned int SECOND = MILLISECOND * 1000;
        constexpr  unsigned int MINUTE = SECOND * 60;
        constexpr  unsigned int HOUR = MINUTE * 60;
    }

    namespace constants{
        namespace ANSI{
            // 1 to ESC_CODE
            // 1 to Text_Color | Background_Color
            // 1 to SEPARATE
            // 1 to USE_RGB
            // 1 to SEPARATE
            constexpr unsigned int maximumNeededPreAllocationForOverHead = 1 + 1 + 1 + 1 + 1;

            // 1 to Red
            // 1 to SEPARATE
            // 1 to Green
            // 1 to SEPARATE
            // 1 to Blue
            constexpr unsigned int maximumNeededPreAllocationForColor = 1 + 1 + 1 + 1 + 1;
            
            // 5 to Text_Overhead
            // 5 to Text_Colour
            // 1 to END_COMMAND
            // 5 to Background_Overhead
            // 5 to Background_Colour
            // 1 to END_COMMAND
            constexpr unsigned int maximumNeededPreAllocationForOverhead = 
                maximumNeededPreAllocationForOverHead + maximumNeededPreAllocationForColor + 1 +
                maximumNeededPreAllocationForOverHead + maximumNeededPreAllocationForColor + 1;

            // 1 to RESET_COLOR
            constexpr unsigned int maximumNeededPreAllocationForReset = 1;

            // 5 to Text_Overhead
            // 5 to Text_Colour
            // 1 to END_COMMAND
            // 5 to Background_Overhead
            // 5 to Background_Colour
            // 1 to END_COMMAND
            // 1 to Data
            // 1 to RESET_COLOR
            constexpr unsigned int maximumNeededPreAllocationForEncodedSuperString = maximumNeededPreAllocationForOverhead + 1 + maximumNeededPreAllocationForReset;

            // 1 to Escape code
            // 1 to private SGR telltale '?'
            // 1 to Feature to be disabled or enabled
            // 1 to Enable/Disable feature told above.
            constexpr unsigned int maximumNeededPreAllocationForEnablingOrDisablingPrivateSGRFeature = 1 + 1 + 1 + 1;
        
            // 1 to Escape code
            // 1 to feature to be enabled
            // 1 to END_COMMAND
            constexpr unsigned int maximumNeededPreAllocationForEnablingOrDisablingSGRFeature = 1 + 1 + 1;
            
            // 1 to Escape code
            // 1 to Mouse appearance
            constexpr unsigned int maximumNeededPreAllocationForSettingCursorShape = 1 + 1;

            // CSI (Control Sequence Introducer) sequences.
            constexpr INTERNAL::compactString ESC_CODE = "\x1B[";      // Also known as \e[ or \o33
            constexpr INTERNAL::compactString SEPARATE = ';';
            constexpr INTERNAL::compactString USE_RGB = '2';
            constexpr INTERNAL::compactString END_COMMAND = 'm';
            constexpr INTERNAL::compactString RESET_COLOR = "\x1B[0m";  // Basically same as RESET_SGR but baked the end command into it for Super_String
            constexpr INTERNAL::compactString CLEAR_SCREEN = "\x1B[2J";
            constexpr INTERNAL::compactString CLEAR_SCROLLBACK = "\x1B[3J";
            constexpr INTERNAL::compactString SET_CURSOR_TO_START = "\x1B[H";
            constexpr INTERNAL::compactString RESET_CONSOLE = "\x1B[c";
            // Cursor shape (DECSCUSR) fragments (value + space + 'q').
            constexpr INTERNAL::compactString CURSOR_SHAPE_BLINKING_BLOCK = "0 q";     // Also default
            constexpr INTERNAL::compactString CURSOR_SHAPE_STEADY_BLOCK = "2 q";
            constexpr INTERNAL::compactString CURSOR_SHAPE_BLINKING_UNDERLINE = "3 q";
            constexpr INTERNAL::compactString CURSOR_SHAPE_STEADY_UNDERLINE = "4 q";
            constexpr INTERNAL::compactString CURSOR_SHAPE_BLINKING_BAR = "5 q";
            constexpr INTERNAL::compactString CURSOR_SHAPE_STEADY_BAR = "6 q";

            /**
             * @brief Enable or disable a private SGR feature
             * 
             * @param command The SGR command to enable or disable
             * @param Enable If true, enable the feature. Otherwise, disable it.
             * @return A Super_String object with the correct escape sequence to enable or disable the feature
             */
            constexpr INTERNAL::superString<maximumNeededPreAllocationForEnablingOrDisablingPrivateSGRFeature> Enable_Private_SGR_Feature(const INTERNAL::compactString& command, bool Enable = true) { 
                INTERNAL::superString<maximumNeededPreAllocationForEnablingOrDisablingPrivateSGRFeature> Result;

                // Add the escape code
                Result.add(ESC_CODE);

                // Add the private SGR telltale '?'
                Result.add('?');

                // Add the command to enable or disable
                Result.add(command);

                // Finally, add the enable or disable code
                if (Enable)
                    Result.add('h');
                else
                    Result.add('l');

                return Result;
            }


            // SGR (Select Graphic Rendition)
            
            /**
             * @brief Enable a SGR feature
             * 
             * @param command The command to enable
             * @return A Super_String object with the correct escape sequence to enable the feature
             */
            constexpr INTERNAL::superString<maximumNeededPreAllocationForEnablingOrDisablingSGRFeature> Enable_SGR_Feature(const INTERNAL::compactString& command) {
                INTERNAL::superString<maximumNeededPreAllocationForEnablingOrDisablingSGRFeature> Result;

                // Add the escape code
                Result.add(ESC_CODE);

                // Add the command to enable
                Result.add(command);

                // Add the end command
                Result.add(END_COMMAND);

                return Result;
            }

            // Build a cursor shape control sequence: ESC[ <Ps> q where fragment already provides "<digit> q".
            constexpr INTERNAL::superString<maximumNeededPreAllocationForSettingCursorShape> Set_Cursor_Shape(const INTERNAL::compactString& fragment){
                INTERNAL::superString<maximumNeededPreAllocationForSettingCursorShape> Result;
                Result.add(ESC_CODE);
                Result.add(fragment);
                return Result;
            }

            // SGR constants
            constexpr INTERNAL::compactString RESET_SGR = "0";                                  // Removes all SGR features. 
            constexpr INTERNAL::compactString BOLD = "1";                                       // Not widely supported!
            constexpr INTERNAL::compactString FAINT = "2";                                      // Not widely supported!
            constexpr INTERNAL::compactString ITALIC = "3";                                     // Not widely supported! (Can also be same as blink)
            constexpr INTERNAL::compactString UNDERLINE = "4";              
            constexpr INTERNAL::compactString SLOW_BLINK = "5";                                 // ~150 BPM
            constexpr INTERNAL::compactString RAPID_BLINK = "6";                                // Not widely supported!
            constexpr INTERNAL::compactString INVERT_FOREGROUND_WITH_BACKGROUND = "7";          // Not widely supported!
            constexpr INTERNAL::compactString CONCEAL = "8";                                    // Not widely supported!
            constexpr INTERNAL::compactString CROSSED_OUT = "9";                                // Not widely supported!
            constexpr INTERNAL::compactString PRIMARY_FONT = "10";                              // Sets the default font.
            constexpr INTERNAL::compactString ALTERNATIVE_FONT_1 = "11";                        // Custom font slot.
            constexpr INTERNAL::compactString ALTERNATIVE_FONT_2 = "12";                        // Custom font slot.
            constexpr INTERNAL::compactString ALTERNATIVE_FONT_3 = "13";                        // Custom font slot.
            constexpr INTERNAL::compactString ALTERNATIVE_FONT_4 = "14";                        // Custom font slot.
            constexpr INTERNAL::compactString ALTERNATIVE_FONT_5 = "15";                        // Custom font slot.
            constexpr INTERNAL::compactString ALTERNATIVE_FONT_6 = "16";                        // Custom font slot.
            constexpr INTERNAL::compactString ALTERNATIVE_FONT_7 = "17";                        // Custom font slot.
            constexpr INTERNAL::compactString ALTERNATIVE_FONT_8 = "18";                        // Custom font slot.
            constexpr INTERNAL::compactString ALTERNATIVE_FONT_9 = "19";                        // Custom font slot.
            constexpr INTERNAL::compactString FRAKTUR = "20";                                   // Not widely supported! (But cool font)
            constexpr INTERNAL::compactString NOT_BOLD = "21";                                  // Removes the BOLD feature
            constexpr INTERNAL::compactString NORMAL_INTENSITY = "22";                          // Removes BOLD and ITALIC and other affixes.
            constexpr INTERNAL::compactString NOT_UNDERLINE = "23";                             // Removes UNDERLINE.
            constexpr INTERNAL::compactString NOT_BLINK = "24";                                 // Removes BLINK.
            constexpr INTERNAL::compactString INVERT_INVERT_FOREGROUND_WITH_BACKGROUND = "27";  // Inverts the INVERT_FOREGROUND_WITH_BACKGROUND.
            constexpr INTERNAL::compactString TEXT_COLOR = "38";                               // Sets the foreground color.
            constexpr INTERNAL::compactString DEFAULT_TEXT_COLOR = "39";                       // Sets the default color.
            constexpr INTERNAL::compactString BACKGROUND_COLOR = "48";                         // Sets the background color.
            constexpr INTERNAL::compactString DEFAULT_BACKGROUND_COLOR = "49";                 // Sets the default color.

            // Private SGR codes
            constexpr INTERNAL::compactString REPORT_MOUSE_HIGHLIGHTS = "1000";
            constexpr INTERNAL::compactString REPORT_MOUSE_BUTTON_WHILE_MOVING = "1002";
            constexpr INTERNAL::compactString REPORT_MOUSE_ALL_EVENTS = "1003";

            constexpr INTERNAL::compactString MOUSE_CURSOR = "25";
            constexpr INTERNAL::compactString SCREEN_CAPTURE = "47"; // 47l = restores screen, 47h = saves screen
            constexpr INTERNAL::compactString ALTERNATIVE_SCREEN_BUFFER = "1049"; // 1049l = disables alternative buffer, 1049h = enables alternative buffer
            // End of enable settings for ANSI

            // ACC (ASCII Control Characters)
            constexpr char NONE = 0;
            constexpr char START_OF_HEADING = 1;
            constexpr char START_OF_TEXT = 2;
            constexpr char END_OF_TEXT = 3;
            constexpr char END_OF_TRANSMISSION = 4;
            constexpr char ENQUIRY = 5;
            constexpr char ACKNOWLEDGE = 6;
            constexpr char BELL = 7;
            constexpr char BACKSPACE = 8;
            constexpr char HORIZONTAL_TAB = 9;
            constexpr char LINE_FEED = 10;             // Also known as newline
            constexpr char VERTICAL_TAB = 11;
            constexpr char FORM_FEED = 12;
            constexpr char CARRIAGE_RETURN = 13;
            constexpr char SHIFT_OUT = 14;
            constexpr char SHIFT_IN = 15;
            constexpr char DATA_LINK_ESCAPE = 16;
            constexpr char DEVICE_CONTROL_1 = 17;
            constexpr char DEVICE_CONTROL_2 = 18;
            constexpr char DEVICE_CONTROL_3 = 19;
            constexpr char DEVICE_CONTROL_4 = 20;
            constexpr char NEGATIVE_ACKNOWLEDGE = 21;
            constexpr char SYNCHRONOUS_IDLE = 22;
            constexpr char END_OF_TRANSMISSION_BLOCK = 23;
            constexpr char CANCEL = 24;
            constexpr char END_OF_MEDIUM = 25;
            constexpr char SUBSTITUTE = 26;
            constexpr char ESCAPE = 27;
            constexpr char FILE_SEPARATOR = 28;
            constexpr char GROUP_SEPARATOR = 29;
            constexpr char RECORD_SEPARATOR = 30;
            constexpr char UNIT_SEPARATOR = 31;
            constexpr char DEL = 127;

            constexpr const INTERNAL::compactString toCompactTable[256] = {
                INTERNAL::compactString("0", 1), INTERNAL::compactString("1", 1), INTERNAL::compactString("2", 1), INTERNAL::compactString("3", 1), INTERNAL::compactString("4", 1), INTERNAL::compactString("5", 1), INTERNAL::compactString("6", 1), INTERNAL::compactString("7", 1), INTERNAL::compactString("8", 1), INTERNAL::compactString("9", 1),
                INTERNAL::compactString("10", 2), INTERNAL::compactString("11", 2), INTERNAL::compactString("12", 2), INTERNAL::compactString("13", 2), INTERNAL::compactString("14", 2), INTERNAL::compactString("15", 2), INTERNAL::compactString("16", 2), INTERNAL::compactString("17", 2), INTERNAL::compactString("18", 2), INTERNAL::compactString("19", 2),
                INTERNAL::compactString("20", 2), INTERNAL::compactString("21", 2), INTERNAL::compactString("22", 2), INTERNAL::compactString("23", 2), INTERNAL::compactString("24", 2), INTERNAL::compactString("25", 2), INTERNAL::compactString("26", 2), INTERNAL::compactString("27", 2), INTERNAL::compactString("28", 2), INTERNAL::compactString("29", 2),
                INTERNAL::compactString("30", 2), INTERNAL::compactString("31", 2), INTERNAL::compactString("32", 2), INTERNAL::compactString("33", 2), INTERNAL::compactString("34", 2), INTERNAL::compactString("35", 2), INTERNAL::compactString("36", 2), INTERNAL::compactString("37", 2), INTERNAL::compactString("38", 2), INTERNAL::compactString("39", 2),
                INTERNAL::compactString("40", 2), INTERNAL::compactString("41", 2), INTERNAL::compactString("42", 2), INTERNAL::compactString("43", 2), INTERNAL::compactString("44", 2), INTERNAL::compactString("45", 2), INTERNAL::compactString("46", 2), INTERNAL::compactString("47", 2), INTERNAL::compactString("48", 2), INTERNAL::compactString("49", 2),
                INTERNAL::compactString("50", 2), INTERNAL::compactString("51", 2), INTERNAL::compactString("52", 2), INTERNAL::compactString("53", 2), INTERNAL::compactString("54", 2), INTERNAL::compactString("55", 2), INTERNAL::compactString("56", 2), INTERNAL::compactString("57", 2), INTERNAL::compactString("58", 2), INTERNAL::compactString("59", 2),
                INTERNAL::compactString("60", 2), INTERNAL::compactString("61", 2), INTERNAL::compactString("62", 2), INTERNAL::compactString("63", 2), INTERNAL::compactString("64", 2), INTERNAL::compactString("65", 2), INTERNAL::compactString("66", 2), INTERNAL::compactString("67", 2), INTERNAL::compactString("68", 2), INTERNAL::compactString("69", 2),
                INTERNAL::compactString("70", 2), INTERNAL::compactString("71", 2), INTERNAL::compactString("72", 2), INTERNAL::compactString("73", 2), INTERNAL::compactString("74", 2), INTERNAL::compactString("75", 2), INTERNAL::compactString("76", 2), INTERNAL::compactString("77", 2), INTERNAL::compactString("78", 2), INTERNAL::compactString("79", 2),
                INTERNAL::compactString("80", 2), INTERNAL::compactString("81", 2), INTERNAL::compactString("82", 2), INTERNAL::compactString("83", 2), INTERNAL::compactString("84", 2), INTERNAL::compactString("85", 2), INTERNAL::compactString("86", 2), INTERNAL::compactString("87", 2), INTERNAL::compactString("88", 2), INTERNAL::compactString("89", 2),
                INTERNAL::compactString("90", 2), INTERNAL::compactString("91", 2), INTERNAL::compactString("92", 2), INTERNAL::compactString("93", 2), INTERNAL::compactString("94", 2), INTERNAL::compactString("95", 2), INTERNAL::compactString("96", 2), INTERNAL::compactString("97", 2), INTERNAL::compactString("98", 2), INTERNAL::compactString("99", 2),
                INTERNAL::compactString("100", 3), INTERNAL::compactString("101", 3), INTERNAL::compactString("102", 3), INTERNAL::compactString("103", 3), INTERNAL::compactString("104", 3), INTERNAL::compactString("105", 3), INTERNAL::compactString("106", 3), INTERNAL::compactString("107", 3), INTERNAL::compactString("108", 3), INTERNAL::compactString("109", 3),
                INTERNAL::compactString("110", 3), INTERNAL::compactString("111", 3), INTERNAL::compactString("112", 3), INTERNAL::compactString("113", 3), INTERNAL::compactString("114", 3), INTERNAL::compactString("115", 3), INTERNAL::compactString("116", 3), INTERNAL::compactString("117", 3), INTERNAL::compactString("118", 3), INTERNAL::compactString("119", 3),
                INTERNAL::compactString("120", 3), INTERNAL::compactString("121", 3), INTERNAL::compactString("122", 3), INTERNAL::compactString("123", 3), INTERNAL::compactString("124", 3), INTERNAL::compactString("125", 3), INTERNAL::compactString("126", 3), INTERNAL::compactString("127", 3), INTERNAL::compactString("128", 3), INTERNAL::compactString("129", 3),
                INTERNAL::compactString("130", 3), INTERNAL::compactString("131", 3), INTERNAL::compactString("132", 3), INTERNAL::compactString("133", 3), INTERNAL::compactString("134", 3), INTERNAL::compactString("135", 3), INTERNAL::compactString("136", 3), INTERNAL::compactString("137", 3), INTERNAL::compactString("138", 3), INTERNAL::compactString("139", 3),
                INTERNAL::compactString("140", 3), INTERNAL::compactString("141", 3), INTERNAL::compactString("142", 3), INTERNAL::compactString("143", 3), INTERNAL::compactString("144", 3), INTERNAL::compactString("145", 3), INTERNAL::compactString("146", 3), INTERNAL::compactString("147", 3), INTERNAL::compactString("148", 3), INTERNAL::compactString("149", 3),
                INTERNAL::compactString("150", 3), INTERNAL::compactString("151", 3), INTERNAL::compactString("152", 3), INTERNAL::compactString("153", 3), INTERNAL::compactString("154", 3), INTERNAL::compactString("155", 3), INTERNAL::compactString("156", 3), INTERNAL::compactString("157", 3), INTERNAL::compactString("158", 3), INTERNAL::compactString("159", 3),
                INTERNAL::compactString("160", 3), INTERNAL::compactString("161", 3), INTERNAL::compactString("162", 3), INTERNAL::compactString("163", 3), INTERNAL::compactString("164", 3), INTERNAL::compactString("165", 3), INTERNAL::compactString("166", 3), INTERNAL::compactString("167", 3), INTERNAL::compactString("168", 3), INTERNAL::compactString("169", 3),
                INTERNAL::compactString("170", 3), INTERNAL::compactString("171", 3), INTERNAL::compactString("172", 3), INTERNAL::compactString("173", 3), INTERNAL::compactString("174", 3), INTERNAL::compactString("175", 3), INTERNAL::compactString("176", 3), INTERNAL::compactString("177", 3), INTERNAL::compactString("178", 3), INTERNAL::compactString("179", 3),
                INTERNAL::compactString("180", 3), INTERNAL::compactString("181", 3), INTERNAL::compactString("182", 3), INTERNAL::compactString("183", 3), INTERNAL::compactString("184", 3), INTERNAL::compactString("185", 3), INTERNAL::compactString("186", 3), INTERNAL::compactString("187", 3), INTERNAL::compactString("188", 3), INTERNAL::compactString("189", 3),
                INTERNAL::compactString("190", 3), INTERNAL::compactString("191", 3), INTERNAL::compactString("192", 3), INTERNAL::compactString("193", 3), INTERNAL::compactString("194", 3), INTERNAL::compactString("195", 3), INTERNAL::compactString("196", 3), INTERNAL::compactString("197", 3), INTERNAL::compactString("198", 3), INTERNAL::compactString("199", 3),
                INTERNAL::compactString("200", 3), INTERNAL::compactString("201", 3), INTERNAL::compactString("202", 3), INTERNAL::compactString("203", 3), INTERNAL::compactString("204", 3), INTERNAL::compactString("205", 3), INTERNAL::compactString("206", 3), INTERNAL::compactString("207", 3), INTERNAL::compactString("208", 3), INTERNAL::compactString("209", 3),
                INTERNAL::compactString("210", 3), INTERNAL::compactString("211", 3), INTERNAL::compactString("212", 3), INTERNAL::compactString("213", 3), INTERNAL::compactString("214", 3), INTERNAL::compactString("215", 3), INTERNAL::compactString("216", 3), INTERNAL::compactString("217", 3), INTERNAL::compactString("218", 3), INTERNAL::compactString("219", 3),
                INTERNAL::compactString("220", 3), INTERNAL::compactString("221", 3), INTERNAL::compactString("222", 3), INTERNAL::compactString("223", 3), INTERNAL::compactString("224", 3), INTERNAL::compactString("225", 3), INTERNAL::compactString("226", 3), INTERNAL::compactString("227", 3), INTERNAL::compactString("228", 3), INTERNAL::compactString("229", 3),
                INTERNAL::compactString("230", 3), INTERNAL::compactString("231", 3), INTERNAL::compactString("232", 3), INTERNAL::compactString("233", 3), INTERNAL::compactString("234", 3), INTERNAL::compactString("235", 3), INTERNAL::compactString("236", 3), INTERNAL::compactString("237", 3), INTERNAL::compactString("238", 3), INTERNAL::compactString("239", 3),
                INTERNAL::compactString("240", 3), INTERNAL::compactString("241", 3), INTERNAL::compactString("242", 3), INTERNAL::compactString("243", 3), INTERNAL::compactString("244", 3), INTERNAL::compactString("245", 3), INTERNAL::compactString("246", 3), INTERNAL::compactString("247", 3), INTERNAL::compactString("248", 3), INTERNAL::compactString("249", 3),
                INTERNAL::compactString("250", 3), INTERNAL::compactString("251", 3), INTERNAL::compactString("252", 3), INTERNAL::compactString("253", 3), INTERNAL::compactString("254", 3), INTERNAL::compactString("255", 3)
            };
        }

        constexpr unsigned long long NONE = (unsigned long long)1 << 0;
        constexpr unsigned long long ENTER = (unsigned long long)1 << 1;
        constexpr unsigned long long ESCAPE = (unsigned long long)1 << 2;
        constexpr unsigned long long BACKSPACE = (unsigned long long)1 << 3;
        constexpr unsigned long long TAB = (unsigned long long)1 << 4;
        constexpr unsigned long long UP = (unsigned long long)1 << 5;
        constexpr unsigned long long DOWN = (unsigned long long)1 << 6;
        constexpr unsigned long long LEFT = (unsigned long long)1 << 7;
        constexpr unsigned long long RIGHT = (unsigned long long)1 << 8;
        constexpr unsigned long long SPACE = (unsigned long long)1 << 9;
        constexpr unsigned long long SHIFT = (unsigned long long)1 << 10;
        constexpr unsigned long long ALT = (unsigned long long)1 << 11;
        constexpr unsigned long long CONTROL = (unsigned long long)1 << 12;
        constexpr unsigned long long SUPER = (unsigned long long)1 << 13;
        constexpr unsigned long long HOME = (unsigned long long)1 << 14;
        constexpr unsigned long long INSERT = (unsigned long long)1 << 15;
        constexpr unsigned long long DEL = (unsigned long long)1 << 16;
        constexpr unsigned long long END = (unsigned long long)1 << 17;
        constexpr unsigned long long PAGE_UP = (unsigned long long)1 << 18;
        constexpr unsigned long long PAGE_DOWN = (unsigned long long)1 << 19;
        constexpr unsigned long long F0 = (unsigned long long)1 << 20;
        constexpr unsigned long long F1 = (unsigned long long)1 << 21;
        constexpr unsigned long long F2 = (unsigned long long)1 << 22;
        constexpr unsigned long long F3 = (unsigned long long)1 << 23;
        constexpr unsigned long long F4 = (unsigned long long)1 << 24;
        constexpr unsigned long long F5 = (unsigned long long)1 << 25;
        constexpr unsigned long long F6 = (unsigned long long)1 << 26;
        constexpr unsigned long long F7 = (unsigned long long)1 << 27;
        constexpr unsigned long long F8 = (unsigned long long)1 << 28;
        constexpr unsigned long long F9 = (unsigned long long)1 << 29;
        constexpr unsigned long long F10 = (unsigned long long)1 << 30;
        constexpr unsigned long long F11 = (unsigned long long)1 << 31;
        constexpr unsigned long long F12 = (unsigned long long)1 << 32;
        constexpr unsigned long long F13 = (unsigned long long)1 << 33;
        constexpr unsigned long long F14 = (unsigned long long)1 << 34;
        constexpr unsigned long long F15 = (unsigned long long)1 << 35;
        constexpr unsigned long long F16 = (unsigned long long)1 << 36;

        // Should not fucking exist bro!
        //constexpr unsigned long long SHIFT_TAB = (unsigned long long)1 << 37;


        //key_Press includes [a-z, A-Z] & [0-9]
        constexpr unsigned long long KEY_PRESS = (unsigned long long)1 << 38;

        // EASY MOUSE API
        constexpr unsigned long long MOUSE_LEFT_CLICKED = (unsigned long long)1 << 39;
        constexpr unsigned long long MOUSE_MIDDLE_CLICKED = (unsigned long long)1 << 40;
        constexpr unsigned long long MOUSE_RIGHT_CLICKED = (unsigned long long)1 << 41;

        // NOTE: These will be spammed until it is not pressed anymore!
        constexpr unsigned long long MOUSE_LEFT_PRESSED = (unsigned long long)1 << 42;
        constexpr unsigned long long MOUSE_MIDDLE_PRESSED = (unsigned long long)1 << 43;
        constexpr unsigned long long MOUSE_RIGHT_PRESSED = (unsigned long long)1 << 44;

        constexpr unsigned long long MOUSE_MIDDLE_SCROLL_UP = (unsigned long long)1 << 45;
        constexpr unsigned long long MOUSE_MIDDLE_SCROLL_DOWN = (unsigned long long)1 << 46;
    
    }

    namespace KEYBOARD_BUTTONS{
        static constexpr std::string_view ESC = "ECS";
        static constexpr std::string_view F1 = "F1";
        static constexpr std::string_view F2 = "F2";
        static constexpr std::string_view F3 = "F3";
        static constexpr std::string_view F4 = "F4";
        static constexpr std::string_view F5 = "F5";
        static constexpr std::string_view F6 = "F6";
        static constexpr std::string_view F7 = "F7";
        static constexpr std::string_view F8 = "F8";
        static constexpr std::string_view F9 = "F9";
        static constexpr std::string_view F10 = "F10";
        static constexpr std::string_view F11 = "F11";
        static constexpr std::string_view F12 = "F12";
        static constexpr std::string_view PRTSC = "PRTSC";
        static constexpr std::string_view SCROLL_LOCK = "SCROLL_LOCK";
        static constexpr std::string_view PAUSE = "PAUSE";
        static constexpr std::string_view SECTION = "SECTION";
        static constexpr std::string_view BACKSPACE = "BACKSPACE";
        static constexpr std::string_view TAB = "TAB";
        static constexpr std::string_view ENTER = "ENTER";
        static constexpr std::string_view CAPS = "CAPS";
        static constexpr std::string_view SHIFT = "SHIFT";
        static constexpr std::string_view CONTROL = "CTRL";
        static constexpr std::string_view SUPER = "SUPER";
        static constexpr std::string_view ALT = "ALT";
        static constexpr std::string_view SPACE = "SPACE";
        static constexpr std::string_view ALTGR = "ALTGR";
        static constexpr std::string_view FN = "FN";
        static constexpr std::string_view INS = "INS";
        static constexpr std::string_view HOME = "HOME";
        static constexpr std::string_view PAGE_UP = "PAGE_UP";
        static constexpr std::string_view DEL = "DELETE";
        static constexpr std::string_view INSERT = "INSERT";
        static constexpr std::string_view END = "END";
        static constexpr std::string_view PAGE_DOWN = "PAGE_DOWN";

        static constexpr std::string_view UP = "UP";
        static constexpr std::string_view DOWN = "DOWN";
        static constexpr std::string_view LEFT = "LEFT";
        static constexpr std::string_view RIGHT = "RIGHT";

        static constexpr std::string_view MOUSE_LEFT = "MOUSE_LEFT";
        static constexpr std::string_view MOUSE_MIDDLE = "MOUSE_MIDDLE";
        static constexpr std::string_view MOUSE_RIGHT = "MOUSE_RIGHT";
        static constexpr std::string_view MOUSE_SCROLL_UP = "MOUSE_SCROLL_UP";
        static constexpr std::string_view MOUSE_SCROLL_DOWN = "MOUSE_SCROLL_DOWN";
    }

    
    namespace SYMBOLS{
        constexpr INTERNAL::compactString TOP_LEFT_CORNER = "┌";//"\e(0\x6c\e(B";
        constexpr INTERNAL::compactString BOTTOM_LEFT_CORNER = "└";//"\e(0\x6d\e(B";
        constexpr INTERNAL::compactString TOP_RIGHT_CORNER = "┐";//"\e(0\x6b\e(B";
        constexpr INTERNAL::compactString BOTTOM_RIGHT_CORNER = "┘";//"\e(0\x6a\e(B";
        constexpr INTERNAL::compactString VERTICAL_LINE = "│";//"\e(0\x78\e(B";
        constexpr INTERNAL::compactString HORIZONTAL_LINE = "─";//"\e(0\x71\e(B";
        constexpr INTERNAL::compactString VERTICAL_RIGHT_CONNECTOR = "├";//"\e(0\x74\e(B";
        constexpr INTERNAL::compactString VERTICAL_LEFT_CONNECTOR = "┤";//"\e(0\x75\e(B";
        constexpr INTERNAL::compactString HORIZONTAL_BOTTOM_CONNECTOR = "┬";//"\e(0\x76\e(B";
        constexpr INTERNAL::compactString HORIZONTAL_TOP_CONNECTOR = "┴";//"\e(0\x77\e(B";
        constexpr INTERNAL::compactString CROSS_CONNECTOR = "┼";//"\e(0\x6e\e(B";

        constexpr INTERNAL::compactString CENTERED_HORIZONTAL_LINE = "━";//"\e(0\x2501\e(B";
        constexpr INTERNAL::compactString FULL_BLOCK = "█";//"\e(0\x2588\e(B";

        constexpr INTERNAL::compactString RADIOBUTTON_OFF = "○";
        constexpr INTERNAL::compactString RADIOBUTTON_ON = "◉";

        constexpr INTERNAL::compactString EMPTY_CHECK_BOX = "☐";
        constexpr INTERNAL::compactString CHECKED_CHECK_BOX = "☒";
    }
}

#endif
#ifndef _COLOR_H_
#define _COLOR_H_

#include <string>
#include <limits>
#include <math.h>
#include <stdint.h>
#include <ostream>


namespace GGUI{
    class RGB{
    public:
        unsigned char Red = 0;
        unsigned char Green = 0;
        unsigned char Blue = 0;

        constexpr RGB(unsigned char r, unsigned char g, unsigned char b) : Red(r), Green(g), Blue(b) {}

        constexpr RGB() = default;

        /**
         * @brief Converts the RGB colour to a string.
         *
         * @param Result The result string.
         */
        constexpr void getColourAsSuperString(INTERNAL::superString<constants::ANSI::maximumNeededPreAllocationForColor>* Result) const {
            // Add the red value to the string
            Result->add(constants::ANSI::toCompactTable[Red]);
            
            // Add the separator to the string
            Result->add(constants::ANSI::SEPARATE);
            
            // Add the green value to the string
            Result->add(constants::ANSI::toCompactTable[Green]);
            
            // Add the separator to the string
            Result->add(constants::ANSI::SEPARATE);
            
            // Add the blue value to the string
            Result->add(constants::ANSI::toCompactTable[Blue]);
        }
    
        /**
         * @brief Populates a Super_String with ANSI escape codes for text or background color.
         * 
         * This function adds the escape codes for either text or background color to the provided Super_String.
         * Ensure that the Result has been initialized with at least Maximum_Needed_Pre_Allocation_For_Over_Head.
         * 
         * @param Result A pointer to the Super_String to populate with escape codes.
         * @param Is_Text_Color A boolean to determine if the codes are for text color (true) or background color (false).
         */
        constexpr void getOverHeadAsSuperString(INTERNAL::superString<GGUI::constants::ANSI::maximumNeededPreAllocationForOverHead>* Result, const bool Is_Text_Color = true) const {
            Result->add(constants::ANSI::ESC_CODE);
            
            Is_Text_Color ? 
                Result->add(constants::ANSI::TEXT_COLOR) :
                Result->add(constants::ANSI::BACKGROUND_COLOR);

            Result->add(constants::ANSI::SEPARATE);
            Result->add(constants::ANSI::USE_RGB);
            Result->add(constants::ANSI::SEPARATE);
        }

        constexpr INTERNAL::superString<GGUI::constants::ANSI::maximumNeededPreAllocationForOverHead> getOverHeadAsSuperString(const bool Is_Text_Color = true) const {
            INTERNAL::superString<GGUI::constants::ANSI::maximumNeededPreAllocationForOverHead> Result;

            Result.add(constants::ANSI::ESC_CODE);
            
            Is_Text_Color ? 
                Result.add(constants::ANSI::TEXT_COLOR) :
                Result.add(constants::ANSI::BACKGROUND_COLOR);

            Result.add(constants::ANSI::SEPARATE);
            Result.add(constants::ANSI::USE_RGB);
            Result.add(constants::ANSI::SEPARATE);

            return Result;
        }
    
        constexpr bool operator==(const RGB& Other) const{
            // only take the bits from the first 3 unsigned chars
            return Red == Other.Red && Green == Other.Green && Blue == Other.Blue;
        }

        constexpr bool operator!=(const RGB& Other) const{
            return !(*this == Other);
        }

        constexpr RGB operator+(const RGB& Other) const{
            return RGB(Red + Other.Red, Green + Other.Green, Blue + Other.Blue);
        }

        constexpr RGB operator*(const float Scalar) const{
            return RGB((unsigned char)((float)Red * Scalar), (unsigned char)((float)Green * Scalar), (unsigned char)((float)Blue * Scalar));
        }

        constexpr RGB operator!() const{
            return RGB(UINT8_MAX - Red, UINT8_MAX - Green, UINT8_MAX - Blue);
        }

        constexpr void add(const RGB& other, float opacity){
            // Calculate the reverse alpha
            float Reverse_Alpha = 1.0f - opacity;

            // Blend the colors based on the opacity
            Red = (unsigned char)((float)Red * Reverse_Alpha + (float)other.Red * opacity);
            Green = (unsigned char)((float)Green * Reverse_Alpha + (float)other.Green * opacity);
            Blue = (unsigned char)((float)Blue * Reverse_Alpha + (float)other.Blue * opacity);
        }
    };

    namespace COLOR{
        // Basic colors
        static constexpr RGB WHITE = RGB(255, 255, 255);
        static constexpr RGB BLACK = RGB(0, 0, 0);
        static constexpr RGB RED = RGB(255, 0, 0);
        static constexpr RGB GREEN = RGB(0, 255, 0);
        static constexpr RGB BLUE = RGB(0, 0, 255);
        static constexpr RGB YELLOW = RGB(255, 255, 0);
        static constexpr RGB ORANGE = RGB(255, 128, 0);
        static constexpr RGB CYAN = RGB(0, 255, 255);
        static constexpr RGB MAGENTA = RGB(255, 0, 255);
        
        // Gray scale variations
        static constexpr RGB GRAY = RGB(128, 128, 128);
        static constexpr RGB LIGHT_GRAY = RGB(211, 211, 211);
        static constexpr RGB DARK_GRAY = RGB(64, 64, 64);
        static constexpr RGB SILVER = RGB(192, 192, 192);
        static constexpr RGB DIM_GRAY = RGB(105, 105, 105);
        static constexpr RGB SLATE_GRAY = RGB(112, 128, 144);
        static constexpr RGB LIGHT_SLATE_GRAY = RGB(119, 136, 153);
        static constexpr RGB DARK_SLATE_GRAY = RGB(47, 79, 79);
        
        // Light color variations
        static constexpr RGB LIGHT_RED = RGB(255, 128, 128);
        static constexpr RGB LIGHT_GREEN = RGB(128, 255, 128);
        static constexpr RGB LIGHT_BLUE = RGB(173, 216, 230);
        static constexpr RGB LIGHT_YELLOW = RGB(255, 255, 128);
        static constexpr RGB LIGHT_CYAN = RGB(128, 255, 255);
        static constexpr RGB LIGHT_MAGENTA = RGB(255, 128, 255);
        
        // Dark color variations
        static constexpr RGB DARK_RED = RGB(128, 0, 0);
        static constexpr RGB DARK_GREEN = RGB(0, 128, 0);
        static constexpr RGB DARK_BLUE = RGB(0, 0, 128);
        static constexpr RGB DARK_YELLOW = RGB(128, 128, 0);
        static constexpr RGB DARK_CYAN = RGB(0, 128, 128);
        static constexpr RGB DARK_MAGENTA = RGB(128, 0, 128);
        
        // Extended basic colors
        static constexpr RGB TEAL = DARK_CYAN;
        static constexpr RGB NAVY = DARK_BLUE;
        static constexpr RGB MAROON = DARK_RED;
        static constexpr RGB OLIVE = DARK_YELLOW;
        static constexpr RGB LIME = GREEN;
        static constexpr RGB AQUA = CYAN;
        static constexpr RGB FUCHSIA = MAGENTA;
        
        // Purple shades
        static constexpr RGB PURPLE = DARK_MAGENTA;
        static constexpr RGB VIOLET = RGB(238, 130, 238);
        static constexpr RGB INDIGO = RGB(75, 0, 130);
        static constexpr RGB PLUM = RGB(221, 160, 221);
        static constexpr RGB ORCHID = RGB(218, 112, 214);
        static constexpr RGB MEDIUM_PURPLE = RGB(147, 112, 219);
        static constexpr RGB DARK_VIOLET = RGB(148, 0, 211);
        static constexpr RGB BLUE_VIOLET = RGB(138, 43, 226);
        
        // Pink shades
        static constexpr RGB PINK = RGB(255, 192, 203);
        static constexpr RGB LIGHT_PINK = RGB(255, 182, 193);
        static constexpr RGB HOT_PINK = RGB(255, 105, 180);
        static constexpr RGB DEEP_PINK = RGB(255, 20, 147);
        static constexpr RGB MEDIUM_VIOLET_RED = RGB(199, 21, 133);
        static constexpr RGB PALE_VIOLET_RED = RGB(219, 112, 147);
        
        // Orange shades
        static constexpr RGB ORANGE_RED = RGB(255, 69, 0);
        static constexpr RGB DARK_ORANGE = RGB(255, 140, 0);
        static constexpr RGB CORAL = RGB(255, 127, 80);
        static constexpr RGB LIGHT_SALMON = RGB(255, 160, 122);
        static constexpr RGB SALMON = RGB(250, 128, 114);
        static constexpr RGB DARK_SALMON = RGB(233, 150, 122);
        static constexpr RGB TOMATO = RGB(255, 99, 71);
        
        // Brown shades
        static constexpr RGB BROWN = RGB(165, 42, 42);
        static constexpr RGB SADDLE_BROWN = RGB(139, 69, 19);
        static constexpr RGB SIENNA = RGB(160, 82, 45);
        static constexpr RGB CHOCOLATE = RGB(210, 105, 30);
        static constexpr RGB DARK_GOLDENROD = RGB(184, 134, 11);
        static constexpr RGB PERU = RGB(205, 133, 63);
        static constexpr RGB ROSY_BROWN = RGB(188, 143, 143);
        static constexpr RGB GOLDENROD = RGB(218, 165, 32);
        static constexpr RGB SANDY_BROWN = RGB(244, 164, 96);
        static constexpr RGB TAN = RGB(210, 180, 140);
        static constexpr RGB BURLYWOOD = RGB(222, 184, 135);
        static constexpr RGB WHEAT = RGB(245, 222, 179);
        
        // Green shades
        static constexpr RGB FOREST_GREEN = RGB(34, 139, 34);
        static constexpr RGB SEA_GREEN = RGB(46, 139, 87);
        static constexpr RGB MEDIUM_SEA_GREEN = RGB(60, 179, 113);
        static constexpr RGB SPRING_GREEN = RGB(0, 255, 127);
        static constexpr RGB MEDIUM_SPRING_GREEN = RGB(0, 250, 154);
        static constexpr RGB DARK_SEA_GREEN = RGB(143, 188, 143);
        static constexpr RGB LIGHT_SEA_GREEN = RGB(32, 178, 170);
        static constexpr RGB PALE_GREEN = RGB(152, 251, 152);
        static constexpr RGB LAWN_GREEN = RGB(124, 252, 0);
        static constexpr RGB CHARTREUSE = RGB(127, 255, 0);
        static constexpr RGB GREEN_YELLOW = RGB(173, 255, 47);
        static constexpr RGB YELLOW_GREEN = RGB(154, 205, 50);
        static constexpr RGB OLIVE_DRAB = RGB(107, 142, 35);
        
        // Blue shades
        static constexpr RGB LIGHT_STEEL_BLUE = RGB(176, 196, 222);
        static constexpr RGB POWDER_BLUE = RGB(176, 224, 230);
        static constexpr RGB SKY_BLUE = RGB(135, 206, 235);
        static constexpr RGB LIGHT_SKY_BLUE = RGB(135, 206, 250);
        static constexpr RGB DEEP_SKY_BLUE = RGB(0, 191, 255);
        static constexpr RGB DODGER_BLUE = RGB(30, 144, 255);
        static constexpr RGB CORNFLOWER_BLUE = RGB(100, 149, 237);
        static constexpr RGB STEEL_BLUE = RGB(70, 130, 180);
        static constexpr RGB ROYAL_BLUE = RGB(65, 105, 225);
        static constexpr RGB MEDIUM_BLUE = RGB(0, 0, 205);
        static constexpr RGB MIDNIGHT_BLUE = RGB(25, 25, 112);
        
        // Cyan/Turquoise shades
        static constexpr RGB DARK_TURQUOISE = RGB(0, 206, 209);
        static constexpr RGB MEDIUM_TURQUOISE = RGB(72, 209, 204);
        static constexpr RGB TURQUOISE = RGB(64, 224, 208);
        static constexpr RGB PALE_TURQUOISE = RGB(175, 238, 238);
        static constexpr RGB AQUAMARINE = RGB(127, 255, 212);
        static constexpr RGB MEDIUM_AQUAMARINE = RGB(102, 205, 170);
        
        // Material Design inspired colors
        namespace MATERIAL {
            static constexpr RGB RED = RGB(244, 67, 54);
            static constexpr RGB PINK = RGB(233, 30, 99);
            static constexpr RGB PURPLE = RGB(156, 39, 176);
            static constexpr RGB DEEP_PURPLE = RGB(103, 58, 183);
            static constexpr RGB INDIGO = RGB(63, 81, 181);
            static constexpr RGB BLUE = RGB(33, 150, 243);
            static constexpr RGB LIGHT_BLUE = RGB(3, 169, 244);
            static constexpr RGB CYAN = RGB(0, 188, 212);
            static constexpr RGB TEAL = RGB(0, 150, 136);
            static constexpr RGB GREEN = RGB(76, 175, 80);
            static constexpr RGB LIGHT_GREEN = RGB(139, 195, 74);
            static constexpr RGB LIME = RGB(205, 220, 57);
            static constexpr RGB YELLOW = RGB(255, 235, 59);
            static constexpr RGB AMBER = RGB(255, 193, 7);
            static constexpr RGB ORANGE = RGB(255, 152, 0);
            static constexpr RGB DEEP_ORANGE = RGB(255, 87, 34);
            static constexpr RGB BROWN = RGB(121, 85, 72);
            static constexpr RGB GRAY = RGB(158, 158, 158);
            static constexpr RGB BLUE_GRAY = RGB(96, 125, 139);
        }
        
        // UI/Theme colors
        static constexpr RGB SUCCESS = RGB(40, 167, 69);
        static constexpr RGB WARNING = RGB(255, 193, 7);
        static constexpr RGB DANGER = RGB(220, 53, 69);
        static constexpr RGB INFO = RGB(23, 162, 184);
        static constexpr RGB PRIMARY = RGB(0, 123, 255);
        static constexpr RGB SECONDARY = RGB(108, 117, 125);
        
        // Vintage/Retro colors
        static constexpr RGB ANTIQUE_WHITE = RGB(250, 235, 215);
        static constexpr RGB BEIGE = RGB(245, 245, 220);
        static constexpr RGB BISQUE = RGB(255, 228, 196);
        static constexpr RGB BLANCHED_ALMOND = RGB(255, 235, 205);
        static constexpr RGB CORNSILK = RGB(255, 248, 220);
        static constexpr RGB EGGSHELL = RGB(240, 234, 214);
        static constexpr RGB FLORAL_WHITE = RGB(255, 250, 240);
        static constexpr RGB GAINSBORO = RGB(220, 220, 220);
        static constexpr RGB GHOST_WHITE = RGB(248, 248, 255);
        static constexpr RGB HONEYDEW = RGB(240, 255, 240);
        static constexpr RGB IVORY = RGB(255, 255, 240);
        static constexpr RGB LAVENDER = RGB(230, 230, 250);
        static constexpr RGB LAVENDER_BLUSH = RGB(255, 240, 245);
        static constexpr RGB LEMON_CHIFFON = RGB(255, 250, 205);
        static constexpr RGB LINEN = RGB(250, 240, 230);
        static constexpr RGB MINT_CREAM = RGB(245, 255, 250);
        static constexpr RGB MISTY_ROSE = RGB(255, 228, 225);
        static constexpr RGB MOCCASIN = RGB(255, 228, 181);
        static constexpr RGB NAVAJO_WHITE = RGB(255, 222, 173);
        static constexpr RGB OLD_LACE = RGB(253, 245, 230);
        static constexpr RGB PAPAYA_WHIP = RGB(255, 239, 213);
        static constexpr RGB PEACH_PUFF = RGB(255, 218, 185);
        static constexpr RGB SEASHELL = RGB(255, 245, 238);
        static constexpr RGB SNOW = RGB(255, 250, 250);
        static constexpr RGB THISTLE = RGB(216, 191, 216);
        static constexpr RGB WHITE_SMOKE = RGB(245, 245, 245);
    }

    inline std::ostream& operator<<(std::ostream& os, const GGUI::RGB& color) {
        os << "RGB(" << static_cast<int>(color.Red) << ", "
           << static_cast<int>(color.Green) << ", "
           << static_cast<int>(color.Blue) << ")";
        return os;
    }
}

#endif
#ifndef _TYPES_H_
#define _TYPES_H_

#include <string>
#include <functional>
#include <chrono>
#include <mutex>
#include <memory>


namespace GGUI{
    
    // Literal type
    class FVector2{
    public:
        float X = 0;
        float Y = 0;

        /**
         * @brief Default constructor
         *
         * Initializes the FVector2 with the given x and y values.
         *
         * @param x The x-coordinate. Default is 0.0f.
         * @param y The y-coordinate. Default is 0.0f.
         */
        constexpr FVector2(float x = 0.0f, float y = 0.0f) noexcept
            : X(x), Y(y) {}

        /**
         * @brief Copy constructor
         *
         * Initializes the FVector2 by copying another FVector2.
         *
         * @param other The FVector2 to copy.
         */
        constexpr FVector2(const FVector2& other) noexcept = default;

        /**
         * @brief Move constructor
         *
         * Initializes the FVector2 by moving another FVector2.
         *
         * @param other The FVector2 to move.
         */
        constexpr FVector2(FVector2&& other) noexcept = default;

        /**
         * @brief Copy assignment operator
         *
         * Assigns another FVector2 to this one by copying its values.
         *
         * @param other The FVector2 to copy.
         * @return A reference to this FVector2.
         */
        constexpr FVector2& operator=(const FVector2& other) noexcept = default;

        /**
         * @brief Move assignment operator
         *
         * Moves the values from another FVector2 to this one.
         *
         * @param other The FVector2 to move.
         * @return A reference to this FVector2.
         */
        constexpr FVector2& operator=(FVector2&& other) noexcept = default;

        /**
         * @brief + operator with a float
         *
         * Adds a float to FVector2, creating a new FVector2.
         *
         * @param num The float to add.
         * @return A new FVector2 with the added float.
         */
        constexpr FVector2 operator+(float num) const noexcept {
            return FVector2(X + num, Y + num);
        }

        /**
         * @brief - operator with a float
         *
         * Subtracts a float from FVector2, creating a new FVector2.
         *
         * @param num The float to subtract.
         * @return A new FVector2 with the subtracted float.
         */
        constexpr FVector2 operator-(float num) const noexcept {
            return FVector2(X - num, Y - num);
        }


        /**
         * @brief * operator with a float
         *
         * Multiplies the FVector2 by a float, creating a new FVector2.
         *
         * @param num The float to multiply.
         * @return A new FVector2 with the multiplied float.
         */
        constexpr FVector2 operator*(float num) const noexcept {
            return FVector2(X * num, Y * num);
        }
    };
    
    // Literal type
    class FVector3 : public FVector2 {
    public:
        float Z = 0;

        /**
         * @brief Default constructor
         *
         * Initializes the FVector3 with the given x, y, and z values.
         *
         * @param x The x-coordinate. Default is 0.0f.
         * @param y The y-coordinate. Default is 0.0f.
         * @param z The z-coordinate. Default is 0.0f.
         */
        constexpr FVector3(float x = 0.0f, float y = 0.0f, float z = 0.0f) noexcept
            : FVector2(x, y), Z(z) {}

        /**
         * @brief Copy constructor
         *
         * Initializes the FVector3 by copying another FVector3.
         *
         * @param other The FVector3 to copy.
         */
        constexpr FVector3(const FVector3& other) noexcept = default;

        /**
         * @brief Move constructor
         *
         * Initializes the FVector3 by moving another FVector3.
         *
         * @param other The FVector3 to move.
         */
        constexpr FVector3(FVector3&& other) noexcept = default;

        /**
         * @brief Copy assignment operator
         *
         * Assigns another FVector3 to this one by copying its values.
         *
         * @param other The FVector3 to copy.
         * @return A reference to this FVector3.
         */
        constexpr FVector3& operator=(const FVector3& other) noexcept = default;

        /**
         * @brief Move assignment operator
         *
         * Assigns another FVector3 to this one by moving its values.
         *
         * @param other The FVector3 to move.
         * @return A reference to this FVector3.
         */
        constexpr FVector3& operator=(FVector3&& other) noexcept = default;

        /**
         * @brief + operator with a float
         *
         * Adds a float to FVector3, creating a new FVector3.
         *
         * @param num The float to add.
         * @return A new FVector3 with the added float.
         */
        constexpr FVector3 operator+(float num) const noexcept {
            return FVector3(X + num, Y + num, Z + num);
        }


        /**
         * @brief - operator with a float
         *
         * Subtracts a float from FVector3, creating a new FVector3.
         *
         * @param num The float to subtract.
         * @return A new FVector3 with the subtracted float.
         */
        constexpr FVector3 operator-(float num) const noexcept {
            return FVector3(X - num, Y - num, Z - num);
        }

        /**
         * @brief * operator with a float
         *
         * Multiplies the FVector3 by a float, creating a new FVector3.
         *
         * @param num The float to multiply.
         * @return A new FVector3 with the multiplied float.
         */
        constexpr FVector3 operator*(float num) const noexcept {
            return FVector3(X * num, Y * num, Z * num);
        }

        /**
         * @brief + operator with another FVector3
         *
         * Adds another FVector3 to this one, creating a new FVector3.
         *
         * @param other The FVector3 to add.
         * @return A new FVector3 with the added values.
         */
        constexpr FVector3 operator+(const FVector3& other) const noexcept {
            return FVector3(X + other.X, Y + other.Y, Z + other.Z);
        }

        /**
         * @brief - operator with another FVector3
         *
         * Subtracts another FVector3 from this one, creating a new FVector3.
         *
         * @param other The FVector3 to subtract.
         * @return A new FVector3 with the subtracted values.
         */
        constexpr FVector3 operator-(const FVector3& other) const noexcept {
            return FVector3(X - other.X, Y - other.Y, Z - other.Z);
        }

        /**
         * @brief * operator with another FVector3 (component-wise multiplication)
         *
         * Performs component-wise multiplication with another FVector3, creating a new FVector3.
         *
         * @param other The FVector3 to multiply.
         * @return A new FVector3 with the component-wise multiplied values.
         */
        constexpr FVector3 operator*(const FVector3& other) const noexcept {
            return FVector3(X * other.X, Y * other.Y, Z * other.Z);
        }
    };

    class IVector2{
    public:
        short X = 0;  //Horizontal
        short Y = 0;  //Vertical

        /**
         * @brief Default constructor
         *
         * Initializes the IVector2 with the given x and y values.
         *
         * @param x The x-coordinate. Default is 0.
         * @param y The y-coordinate. Default is 0.
         */
        constexpr IVector2(short x = 0, short y = 0) noexcept
            : X(x), Y(y) {}

        /**
         * @brief Copy constructor
         * 
         * Initializes the IVector2 by copying another IVector2.
         * @param other The IVector2 to copy.
         */
        constexpr IVector2(const IVector2& other) noexcept = default;

        /**
         * @brief Move constructor
         * 
         * Initializes the IVector2 by moving another IVector2.
         * @param other The IVector2 to move.
         */
        constexpr IVector2(IVector2&& other) noexcept = default;

        /**
         * @brief Copy assignment operator
         *
         * Assigns another IVector2 to this one by copying its values.
         *
         * @param other The IVector2 to copy.
         * @return A reference to this IVector2.
         */
        constexpr IVector2& operator=(const IVector2& other) noexcept = default;

        /**
         * @brief Move assignment operator
         *
         * Moves the values from another IVector2 to this one.
         *
         * @param other The IVector2 to move.
         * @return A reference to this IVector2.
         */
        constexpr IVector2& operator=(IVector2&& other) noexcept = default;

        /**
         * @brief += operator with a pointer to an IVector2
         *
         * Adds the values of the IVector2 pointed to by the pointer to this IVector2.
         *
         * @param other The pointer to the IVector2 to add.
         */
        constexpr void operator+=(IVector2* other) noexcept {
            X += other->X;
            Y += other->Y;
        }

        /**
         * @brief += operator with an FVector2
         *
         * Adds the values of the FVector2 to this IVector2.
         *
         * @param other The FVector2 to add.
         */
        constexpr void operator+=(FVector2 other) noexcept {
            X += static_cast<short>(other.X);
            Y += static_cast<short>(other.Y);
        }

        /**
         * @brief += operator with another IVector2
         *
         * Adds the values of another IVector2 to this one.
         *
         * @param other The IVector2 to add.
         */
        constexpr void operator+=(IVector2 other) noexcept {
            X += other.X;  // Add the x-coordinate
            Y += other.Y;  // Add the y-coordinate
        }

        /**
         * @brief + operator with another IVector2
         *
         * Creates a new IVector2 with the added values of this IVector2 and the other IVector2.
         *
         * @param other The IVector2 to add.
         * @return A new IVector2 with the added values.
         */
        constexpr IVector2 operator+(const IVector2& other) const noexcept {
            return IVector2(X + other.X, Y + other.Y);
        }

        /**
         * @brief - operator with another IVector2
         *
         * Creates a new IVector2 with the subtracted values of this IVector2 and the other IVector2.
         *
         * @param other The IVector2 to subtract.
         * @return A new IVector2 with the subtracted values.
         */
        constexpr IVector2 operator-(const IVector2& other) const noexcept {
            return IVector2(X - other.X, Y - other.Y);
        }

        /**
         * @brief * operator with a float
         *
         * Multiplies the IVector2 by a float, creating a new IVector2.
         *
         * @param num The float to multiply.
         * @return A new IVector2 with the multiplied float.
         */
        constexpr IVector2 operator*(float num) const noexcept {
            return IVector2(static_cast<short>(X * num), static_cast<short>(Y * num)); // Multiply each coordinate by num
        }

        /**
         * @brief == operator with another IVector2
         * 
         * Compares the IVector2 with another IVector2.
         * 
         * @param other The IVector2 to compare with.
         * @return True if the IVector2s are equal, otherwise false.
         */
        constexpr bool operator==(const IVector2& other) const noexcept {
            return X == other.X && Y == other.Y; // Check if the coordinates are equal
        }

        /**
         * @brief != operator with another IVector2
         * 
         * Compares the IVector2 with another IVector2.
         * 
         * @param other The IVector2 to compare with.
         * @return False if the IVector2s are equal, otherwise true.
         */
        constexpr bool operator!=(const IVector2& other) const noexcept {
            return X != other.X || Y != other.Y; // Check if the coordinates are not equal
        }

        /**
         * @brief Converts the IVector2 to a string
         *
         * Converts the IVector2 to a string representation.
         *
         * @return A string representation of the IVector2.
         */
        std::string To_String() const {
            return std::to_string(X) + ", " + std::to_string(Y);
        }
    };

    class IVector3 : public IVector2{
    public:
        short Z = 0;  //priority (the higher the more likely it will be at top).

        /**
         * @brief Default constructor
         *
         * Initializes the IVector3 with the given x, y and z values.
         *
         * @param x The x-coordinate. Default is 0.
         * @param y The y-coordinate. Default is 0.
         * @param z The z-coordinate. Default is 0.
         */
        constexpr IVector3(short x = 0, short y = 0, short z = 0) noexcept
            : IVector2(x, y), Z(z) {}

        /**
         * @brief Copy constructor
         *
         * Initializes the IVector3 by copying another IVector3.
         *
         * @param other The IVector3 to copy.
         */
        constexpr IVector3(const IVector3& other) noexcept = default;

        /**
         * @brief Move constructor
         *
         * Initializes the IVector3 by moving another IVector3.
         *
         * @param other The IVector3 to move.
         */
        constexpr IVector3(IVector3&& other) noexcept = default;

        /**
         * @brief Copy assignment operator
         *
         * Assigns another IVector3 to this one by copying its values.
         *
         * @param other The IVector3 to copy.
         * @return A reference to this IVector3.
         */
        constexpr IVector3& operator=(const IVector3& other) noexcept = default;

        /**
         * @brief Move assignment operator
         *
         * Moves the values from another IVector3 to this one.
         *
         * @param other The IVector3 to move.
         * @return A reference to this IVector3.
         */
        constexpr IVector3& operator=(IVector3&& other) noexcept = default;

        /**
         * @brief += operator with a pointer to an IVector3
         *
         * Adds the values of the IVector3 pointed to by the pointer to this IVector3.
         *
         * @param other The pointer to the IVector3 to add.
         */
        constexpr void operator+=(IVector3* other) noexcept {
            X += other->X;
            Y += other->Y;
            Z += other->Z;
        }

        /**
         * @brief += operator with another IVector3
         *
         * Adds the values of another IVector3 to this one.
         *
         * @param other The IVector3 to add.
         */
        constexpr void operator+=(IVector3 other) noexcept {
            X += other.X;  // Add the x-coordinate
            Y += other.Y;  // Add the y-coordinate
            Z += other.Z;  // Add the z-coordinate
        }

        /**
         * @brief + operator with another IVector3
         *
         * Creates a new IVector3 with the added values of this IVector3 and the other IVector3.
         *
         * @param other The IVector3 to add.
         * @return A new IVector3 with the added values.
         */
        constexpr IVector3 operator+(const IVector3& other) const noexcept {
            return IVector3(X + other.X, Y + other.Y, Z + other.Z);
        }

        constexpr IVector3 operator-(const IVector3& other) const noexcept {
            return IVector3(X - other.X, Y - other.Y, Z - other.Z);
        }

        constexpr IVector3 operator+(int constant) const noexcept {
            return IVector3(X + constant, Y + constant, Z + constant); // Add the constant to each coordinate
        }

        constexpr IVector3 operator-(int constant) const noexcept {
            return IVector3(X - constant, Y - constant, Z - constant); // Subtract the constant from each coordinate
        }


        /**
         * @brief * operator with a float
         *
         * Multiplies the IVector3 by a float, creating a new IVector3.
         *
         * @param num The float to multiply.
         * @return A new IVector3 with the multiplied float.
         */
        constexpr IVector3 operator*(float num) const noexcept {
            return IVector3(X * num, Y * num, Z * num); // Multiply each coordinate by num
        }

        /**
         * @brief == operator with another IVector3
         * 
         * Compares the IVector3 with another IVector3.
         * 
         * @param other The IVector3 to compare with.
         * @return True if the IVector3s are equal, otherwise false.
         */
        constexpr bool operator==(const IVector3& other) const noexcept {
            return X == other.X && Y == other.Y && Z == other.Z; // Check if the coordinates are equal
        }

        /**
         * @brief != operator with another IVector3
         * 
         * Compares the IVector3 with another IVector3.
         * 
         * @param other The IVector3 to compare with.
         * @return False if the IVector3s are equal, otherwise true.
         */
        constexpr bool operator!=(const IVector3& other) const noexcept {
            return X != other.X || Y != other.Y || Z != other.Z; // Check if the coordinates are not equal
        }

        /**
         * @brief Converts the IVector3 to a string.
         * 
         * This function returns a string in the format "X, Y, Z" where X, Y, and Z are the coordinates of the IVector3.
         * The output string is designed to be human-readable, and is not designed to be efficient for serialization or other purposes.
         * 
         * @return A string representation of the IVector3.
         */
        std::string To_String(){
            return std::to_string(X) + ", " + std::to_string(Y) + ", " + std::to_string(Z);
        }
    
        /**
         * @brief Converts the IVector3 to a string.
         * 
         * This function returns a string in the format "X, Y, Z" where X, Y, and Z are the coordinates of the IVector3.
         * The output string is designed to be human-readable, and is not designed to be efficient for serialization or other purposes.
         * 
         * @return A string representation of the IVector3.
         */
        std::string To_String() const {
            return std::to_string(X) + ", " + std::to_string(Y) + ", " + std::to_string(Z);
        }
    };


    class event{
    public:
        unsigned long long criteria;
    };

    class input : public event{
    public:
        unsigned short X = 0;
        unsigned short Y = 0;
        char data = 0;

        // The input information like the character written.
        input(char d, unsigned long long t){
            data = d;
            criteria = t;
        }

        input(IVector3 c, unsigned long long t){
            X = (unsigned short )c.X;
            Y = (unsigned short )c.Y;
            criteria = t;
        }
    };

    class action : public event{
    public:
        class element* host = nullptr;

        std::function<bool(GGUI::event*)> Job;
        
        std::string ID; 
    
        action() = default;
        action(unsigned long long Criteria, std::function<bool(GGUI::event*)> job, std::string id){
            criteria = Criteria;
            Job = job;
            host = nullptr;
            ID = id;
        }

        action(unsigned long long Criteria, std::function<bool(GGUI::event*)> job, class element* Host, std::string id){
            criteria = Criteria;
            Job = job;
            host = Host;
            ID = id;
        }
    };

    namespace MEMORY_FLAGS{
        inline unsigned char PROLONG_MEMORY     = 1 << 0;
        inline unsigned char RETRIGGER          = 1 << 1;
    }

    class memory : public action{
    public:
        std::chrono::high_resolution_clock::time_point startTime;
        size_t endTime = 0;

        // By default all memories automatically will not prolong each other similar memories.
        unsigned char flags = 0x0;

        // When the job starts, job, prolong previous similar job by this time.
        memory(size_t end, std::function<bool(GGUI::event*)>job, unsigned char Flags = 0x0, std::string id = ""){
            startTime = std::chrono::high_resolution_clock::now();
            endTime = end;
            Job = job;
            flags = Flags;
            ID = id;
        }

        bool is(const unsigned char f) const{
            return (flags & f) > 0;
        }

        void set(const unsigned char f){
            flags |= f;
        }
    };

    namespace INTERNAL{
        struct fittingArea{
            IVector2 negativeOffset;
            IVector2 start;
            IVector2 end;
        };

        enum class borderConnection{
            NONE    = 0 << 0,
            UP      = 1 << 0,
            DOWN    = 1 << 1,
            LEFT    = 1 << 2,
            RIGHT   = 1 << 3
        };

        constexpr bool operator==(const borderConnection lhs, const borderConnection rhs) {
            return static_cast<int>(lhs) == static_cast<int>(rhs);
        }

        constexpr borderConnection operator|(const borderConnection lhs, const borderConnection rhs) {
            return static_cast<borderConnection>(static_cast<int>(lhs) | static_cast<int>(rhs));
        }

        constexpr void operator|=(borderConnection& lhs, const borderConnection rhs) {
            lhs = static_cast<borderConnection>(static_cast<int>(lhs) | static_cast<int>(rhs));
        }

        enum class STAIN_TYPE{
            CLEAN = 0,              // No change
            COLOR = 1 << 0,         // BG and other color related changes
            EDGE = 1 << 1,          // Title and border changes.
            DEEP = 1 << 2,          // Children changes. Deep because the childs are connected via AST.
            STRETCH = 1 << 3,       // Width and or height changes.
            STATE = 1 << 4,         // This is for Switches that based on their state display one symbol differently.
            MOVE = 1 << 5,          // Enabled, to signal absolute position caching.
            FINALIZE = 1 << 6,      // This is used to signal that the element is finalized and the stylings are successfully been embedded.
            RESET = 1 << 7,         // This is to remove redundant STRETCH flagging.
        };

        /**
         * @brief Performs bitwise OR operation on two STAIN_TYPE values.
         * @details This operator allows combining two STAIN_TYPE values using a bitwise OR operation.
         *          It returns the result as an unsigned integer.
         *
         * @param a The first STAIN_TYPE value.
         * @param b The second STAIN_TYPE value.
         * @return The result of the bitwise OR operation as an unsigned integer.
         */
        constexpr unsigned int operator|(const STAIN_TYPE a, const STAIN_TYPE b) {
            // Cast both STAIN_TYPE values to unsigned integers and perform the bitwise OR operation.
            return static_cast<unsigned int>(a) | static_cast<unsigned int>(b);
        }

        /**
         * @brief Performs bitwise OR operation on a STAIN_TYPE value and an unsigned integer.
         * @details This operator allows combining a STAIN_TYPE value with an unsigned integer using a bitwise OR operation.
         *          It returns the result as an unsigned integer.
         *
         * @param a The STAIN_TYPE value.
         * @param b The unsigned integer.
         * @return The result of the bitwise OR operation as an unsigned integer.
         */
        constexpr unsigned int operator|(const STAIN_TYPE a, const unsigned int b){
            return static_cast<unsigned int>(a) | b;
        }

        /**
         * @brief Performs bitwise OR operation on an unsigned integer and a STAIN_TYPE value.
         * @details This operator allows combining an unsigned integer with a STAIN_TYPE value using a bitwise OR operation.
         *          It returns the result as an unsigned integer.
         *
         * @param a The unsigned integer.
         * @param b The STAIN_TYPE value.
         * @return The result of the bitwise OR operation as an unsigned integer.
         */
        constexpr unsigned int operator|(const unsigned int a, const STAIN_TYPE b){
            return a | static_cast<unsigned int>(b);
        }

        class STAIN{
        public:
            STAIN_TYPE Type = STAIN_TYPE::CLEAN;

            /**
             * @brief Checks if the specified STAIN_TYPE is set in the current STAIN object.
             * @details This function checks if a given STAIN_TYPE flag is set in the current
             *          STAIN object. For the CLEAN flag, it checks if the type is less than
             *          or equal to CLEAN. For other flags, it performs a bitwise AND operation.
             *
             * @param f The STAIN_TYPE flag to check.
             * @return true if the specified flag is set; false otherwise.
             */
            constexpr bool is(const STAIN_TYPE f) const {
                // Special handling for the CLEAN flag
                if (f == STAIN_TYPE::CLEAN) {
                    return Type <= f;
                }
                // Check if the specified flag is set using bitwise AND
                return (static_cast<unsigned int>(Type) & static_cast<unsigned int>(f)) == static_cast<unsigned int>(f);
            }

            constexpr bool has(const unsigned int f) const {
                return (static_cast<unsigned int>(Type) & static_cast<unsigned int>(f)) != 0;
            }

            /**
             * @brief Clears a STAIN_TYPE flag from the current STAIN object.
             * @details This function clears a given STAIN_TYPE flag from the current
             *          STAIN object. It performs a bitwise AND operation with the
             *          bitwise compliment of the specified flag.
             *
             * @param f The STAIN_TYPE flag to clear.
             */
            constexpr void Clean(const STAIN_TYPE f){
                Type = (STAIN_TYPE)(static_cast<unsigned int>(Type) & ~static_cast<unsigned int>(f));
            }

            /**
             * @brief Clears a STAIN_TYPE flag from the current STAIN object.
             * @details This function clears a given STAIN_TYPE flag from the current
             *          STAIN object. It performs a bitwise AND operation with the
             *          bitwise compliment of the specified flag.
             *
             * @param f The STAIN_TYPE flag to clear.
             */
            constexpr void Clean(const unsigned int f){
                Type = (STAIN_TYPE)(static_cast<unsigned int>(Type) & ~f);
            }

            /**
             * @brief Marks the specified STAIN_TYPE flag as dirty.
             * @details This function sets a given STAIN_TYPE flag on the current
             *          STAIN object, indicating that the element needs to be reprocessed
             *          for the specified attributes.
             *
             * @param f The STAIN_TYPE flag to set.
             */
            constexpr void Dirty(const STAIN_TYPE f) {
                // Set the specified flag using bitwise OR
                Type = (STAIN_TYPE)(static_cast<unsigned int>(Type) | static_cast<unsigned int>(f));
            }

            /**
             * @brief Marks the specified STAIN_TYPE flag as dirty.
             * @details This function sets a given STAIN_TYPE flag on the current
             *          STAIN object, indicating that the element needs to be reprocessed
             *          for the specified attributes.
             *
             * @param f The STAIN_TYPE flag to set.
             */
            constexpr void Dirty(const unsigned int f){
                // Set the specified flag using bitwise OR
                Type = (STAIN_TYPE)(static_cast<unsigned int>(Type) | f);
            }

        };

        enum class ENCODING_FLAG{
            NONE        = 0 << 0,
            START       = 1 << 0,
            END         = 1 << 1
        };

        constexpr bool operator== (ENCODING_FLAG& a, const ENCODING_FLAG& b) {
            return static_cast<unsigned char>(a) == static_cast<unsigned char>(b);
        }

        constexpr void operator|= (ENCODING_FLAG& a, const ENCODING_FLAG& b) {
            a = static_cast<ENCODING_FLAG>(static_cast<unsigned char>(a) | static_cast<unsigned char>(b));
        }

        constexpr ENCODING_FLAG operator&(const ENCODING_FLAG& a, const ENCODING_FLAG& b) {
            return static_cast<ENCODING_FLAG>(static_cast<unsigned char>(a) & static_cast<unsigned char>(b));
        }

        constexpr ENCODING_FLAG operator|(const ENCODING_FLAG& a, const ENCODING_FLAG& b) {
            return static_cast<ENCODING_FLAG>(static_cast<unsigned char>(a) | static_cast<unsigned char>(b));
        }

        enum class STATE{
            UNKNOWN,

            INIT,
            DESTROYED,
            HIDDEN,
            SHOWN

        };

        enum class ALLOCATION_TYPE{
            UNKNOWN         = 0 << 0,
            STACK           = 1 << 0,
            HEAP            = 1 << 1,
            DATA            = 1 << 2
        };

        namespace LOGGER{
            extern void Log(std::string Text);
        }

        namespace atomic{
            template<typename T>
            class guard {
            public:
                std::mutex shared; // Mutex to guard shared data
                std::unique_ptr<T> data;

                /**
                 * @brief Constructs a Guard object and initializes its Data member.
                 * 
                 * This constructor creates a unique pointer to an instance of type T
                 * and assigns it to the Data member of the Guard object.
                 */
                guard() : data(std::make_unique<T>()) {}

                /**
                 * @brief Functor to execute a job with thread safety.
                 * 
                 * This operator() function takes a std::function that operates on a reference to a T object.
                 * It ensures that the job is executed with mutual exclusion by using a std::lock_guard to lock
                 * the mutex. If the job throws an exception, it catches it and reports the failure.
                 * 
                 * @param job A std::function that takes a reference to a T object and performs some operation.
                 * 
                 * @throws Any exception thrown by the job function will be caught and reported.
                 */
                void operator()(std::function<void(T&)> job) {
                    std::lock_guard<std::mutex> lock(shared); // Automatically manages mutex locking and unlocking
                    try {
                        job(*data);
                    } catch (...) {
                        INTERNAL::LOGGER::Log("Failed to execute the function!");
                    }
                }

                /**
                 * @brief Reads the data in a thread-safe manner.
                 * 
                 * This function acquires a lock on the shared mutex to ensure that the data
                 * is read in a thread-safe manner. It returns a copy of the data.
                 * 
                 * @return T A copy of the data.
                 */
                T read() {
                    std::lock_guard<std::mutex> lock(shared);
                    return *data;
                }

                /**
                 * @brief Destructor for the Guard class.
                 *
                 * This destructor ensures that the Data object is properly destroyed
                 * by acquiring a lock on the Shared mutex before resetting the Data.
                 * The use of std::lock_guard ensures that the mutex is automatically
                 * released when the destructor exits, preventing potential deadlocks.
                 */
                ~guard() {
                    std::lock_guard<std::mutex> lock(shared);
                    data.reset(); // Ensures proper destruction
                }
            };   
        }
    
        // This class contains carry flags from previous cycle cross-thread, if another thread had some un-finished things when another thread was already running.
        class Carry{
        public:
            bool Resize = false;
            bool Terminate = false;     // Signals the shutdown of subthreads.

            ~Carry() = default;
        };
    }
}

#endif
#ifndef _UTF_H_
#define _UTF_H_

#include <string>
#include <variant>
#include <utility>
#include <cstdint>


namespace GGUI{
    class UTF : public INTERNAL::compactString {
    public:
        INTERNAL::ENCODING_FLAG flags = INTERNAL::ENCODING_FLAG::NONE;

        RGB foreground;
        RGB background;

        constexpr UTF() {}

        /**
         * @brief Copy constructor for the UTF class.
         *
         * This constructor initializes a new UTF object as a copy of another UTF object.
         *
         * @param other The UTF object to copy.
         */
        constexpr UTF(const GGUI::UTF& other)
            : INTERNAL::compactString(other),
              foreground(other.foreground),
              background(other.background) {}

        /**
         * @brief Constructs a new UTF object from a single character and a pair of foreground and background colors.
         * @param data The character to store in the UTF object.
         * @param color A pair of RGB objects representing the foreground and background colors. If not provided, defaults to {{}, {}}.
         */
        constexpr UTF(const char data, const std::pair<RGB, RGB> color = {{}, {}}) : compactString(data) {
            foreground = {color.first};
            background = {color.second};
        }

        /**
         * @brief Constructs a new UTF object from a C-style string and a pair of foreground and background colors.
         * @param data The C-style string to store in the UTF object.
         * @param color A pair of RGB objects representing the foreground and background colors. If not provided, defaults to {{}, {}}.
         */
        constexpr UTF(const char* data, const std::pair<RGB, RGB> color = {{}, {}}) : compactString(data) {
            foreground = {color.first};
            background = {color.second};
        }

        /**
         * @brief Constructs a new UTF object from a Compact_String and a pair of foreground and background colors.
         * @param CS The Compact_String to store in the UTF object.
         * @param color A pair of RGB objects representing the foreground and background colors. If not provided, defaults to {{}, {}}.
         */
        constexpr UTF(const compactString CS, const std::pair<RGB, RGB> color = {{}, {}}) : compactString(CS) {
            foreground = {color.first};
            background = {color.second};
        }

        /**
         * @brief Checks if a specific UTF flag is set.
         * @param utf_flag The UTF flag to check.
         * @return True if the flag is set, otherwise false.
         */
        constexpr bool is(const INTERNAL::ENCODING_FLAG flag) const {
            return (flags & flag) == flag;
        }

        /**
         * @brief Checks if a specific UTF flag is set.
         * @param cs_flag The UTF flag to check.
         * @return True if the flag is set, otherwise false.
         */
        constexpr bool is(unsigned char cs_flag) const {
            return compactString::is(cs_flag);
        }

        // Fast comparison of type and content
        constexpr bool is(const char* other) const {
            return compactString::is(other);
        }

        // Fast comparison of type and content
        constexpr bool is(char other) const {
            return compactString::is(other);
        }

        /**
         * @brief Sets a specific UTF flag.
         * @param utf_flag The UTF flag to set.
         */
        constexpr void setFlag(const INTERNAL::ENCODING_FLAG flag) {
            flags |= flag;
        }

        /**
         * @brief Sets the foreground color of the UTF element.
         * @param color The RGB color to set as the foreground color.
         */
        constexpr void setForeground(const RGB color) {
            foreground = color;
        }

        /**
         * @brief Sets the background color of the UTF element.
         * @param color The RGB color to set as the background color.
         */
        constexpr void setBackground(const RGB color) {
            background = color;
        }

        /**
         * @brief Sets the foreground and background color of the UTF element.
         * @param primals A pair of RGB colors. The first element is the foreground color; the second is the background color.
         */
        constexpr void setColor(const std::pair<RGB, RGB>& primals) {
            foreground = primals.first;
            background = primals.second;
        }

        /**
         * @brief Sets the text of the UTF element to a single character.
         * @param data The character to set as the text.
         */
        constexpr void setText(const char data) {
            compactString::setAscii(data);
        }

        /**
         * @brief Sets the text of the UTF element to a null-terminated string.
         * @param data The null-terminated string to set as the text.
         */
        constexpr void setText(const char* data) {
            compactString::setUnicode(data);
        }

        /**
         * @brief Sets the text of the UTF element to that of another UTF element.
         * @param other The other UTF element to copy the text from.
         */
        constexpr void setText(const UTF& other) {
            compactString::operator=(other);
        }


        /**
         * @brief Assign a character to the UTF object.
         * @param text The character to assign.
         */
        constexpr void operator=(const char Text) {
            setText(Text);
        }

        constexpr void operator=(const char* Text) {
            setText(Text);
        }

        /**
         * @brief Assigns a UTF object to another UTF object.
         * @param other The UTF object to assign.
         * @return The assigned UTF object.
         */
        constexpr UTF& operator=(const UTF& other) {
            compactString::operator=(other);
            foreground = other.foreground;
            background = other.background;
            return *this;
        }
    };

    namespace SYMBOLS{
        inline const UTF EMPTY_UTF(' ', {COLOR::WHITE, COLOR::BLACK});
    }

}

#endif
#ifndef _STYLE_H_
#define _STYLE_H_


#include <variant>
#include <array>
#include <string>
#include <cassert>
#include <cstdarg>
#include <algorithm>

namespace GGUI{
    // Externies
    class element;
    class textField;
    class styling;
    extern void EXIT(int signum);
    namespace INTERNAL{
        enum class STAIN_TYPE;
        extern void reportStack(const std::string& problemDescription);

    
        enum class EMBED_ORDER{
            INSTANT,
            DELAYED
        };

        enum class EVALUATION_TYPE{
            DEFAULT,        // no further evaluation needed, just return the value
            PERCENTAGE     // the value is a percentage of the parent attribute
        };
    }

    enum class ANCHOR{
        UP,
        DOWN,
        LEFT,
        RIGHT,
        CENTER
    };

    enum class VALUE_STATE{
        UNINITIALIZED,
        INITIALIZED,
        VALUE
    };

    enum class DIRECTION{
        ROW,
        COLUMN
    };

    // This namespace is an wrapper for the user not to see these !!
    namespace STYLING_INTERNAL{

        template<typename P>
        constexpr bool isNonDiscriminantScalar(const P value, const float scalar);

        template<typename P>
        constexpr std::string toString(const P value);

        template<typename T>
        class value {
        protected:
            T data;
            float percentage;   // This will be changed later on into an std::variant holding different scaling types.
            INTERNAL::EVALUATION_TYPE evaluationType = INTERNAL::EVALUATION_TYPE::DEFAULT;

        public:
            /**
             * Constructor for value class
             * @param value The value to be stored in the variant
             * @param type The type of the value
             * @param use_constexpr Whether to use constexpr or not
             */
            constexpr value(const T d, const INTERNAL::EVALUATION_TYPE type = INTERNAL::EVALUATION_TYPE::DEFAULT)
                : data(d), percentage(0.0f), evaluationType(type) {}

            /**
             * Constructor for value class
             * @param value The value to be stored in the variant
             * @param type The type of the value
             * @param use_constexpr Whether to use constexpr or not
             *
             * This constructor is used when the value is a float and the type is
             * specified. The constexpr parameter is used to determine whether to
             * use a constexpr constructor or not.
             */
            constexpr value(const float f, const INTERNAL::EVALUATION_TYPE type = INTERNAL::EVALUATION_TYPE::PERCENTAGE)
                : data{}, percentage(f), evaluationType(type) {}

            /**
             * Copy constructor
             * @param other The object to be copied from
             *
             * This constructor is used to create a copy of the value object.
             * The data and the evaluation type are copied from the other object.
             */
            constexpr value(const value<T>& other)
                : data(other.data), percentage(other.percentage), evaluationType(other.evaluationType) {}


            /**
             * Assignment operator
             * @param other The object to be assigned from
             * @return The object itself, for chaining
             *
             * This operator is used to assign the data and the evaluation type
             * from another object of the same type.
             */
            constexpr value& operator=(const value& other) {
                // Copy the data and the evaluation type from the other object
                data = other.data;
                percentage = other.percentage;
                evaluationType = other.evaluationType;
                // Return the object itself, for chaining
                return *this;
            }

            /**
             * Assignment operator
             * @param initialization_data The value to be assigned to the data member
             * @return The object itself, for chaining
             *
             * This operator is used to assign a value of type T to the data member.
             * The type of the value is set to DEFAULT.
             */
            constexpr value& operator=(const T initialization_data) {
                // Set the data to the value passed in
                data = initialization_data;
                // Set the evaluation type to DEFAULT
                evaluationType = INTERNAL::EVALUATION_TYPE::DEFAULT;
                // Return the object itself, for chaining
                return *this;
            }

            /**
             * Assignment operator for float type
             * @param initialization_data The float value to be assigned to the data member
             * @return The object itself, for chaining
             *
             * This operator is used to assign a float value to the data member.
             * The type of the value is set to PERCENTAGE.
             */
            constexpr value& operator=(const float initialization_data) {
                // Set the data to the float value passed in
                percentage = initialization_data;
                // Set the evaluation type to PERCENTAGE
                evaluationType = INTERNAL::EVALUATION_TYPE::PERCENTAGE;
                // Return the object itself, for chaining
                return *this;
            }

            constexpr bool operator==(const value<T>& other) const {
                // if (evaluationType != other.evaluationType){
                //     INTERNAL::reportStack("Cannot compare two different eval type values!");
                //     EXIT(1);
                //     return false;   // for warnings.
                // }
                // else{
                //     switch (evaluationType)
                //     {
                //     case INTERNAL::EVALUATION_TYPE::DEFAULT:
                        return data == other.data;
                //     case INTERNAL::EVALUATION_TYPE::PERCENTAGE:
                //         return percentage == other.percentage;
                //     default:
                //         INTERNAL::reportStack("Evaluation type: " + std::to_string((int)evaluationType) + " not supported!");
                //         EXIT(1);
                //         return false;   // for warnings.
                //     }
                // }
            }

            constexpr value<T> operator+(const value<T>& other){
                if (evaluationType != other.evaluationType){
                    INTERNAL::reportStack("Cannot add two different eval type values!");
                    EXIT(1);
                    return false;   // for warnings.
                }
                else{
                    switch (evaluationType)
                    {
                    case INTERNAL::EVALUATION_TYPE::DEFAULT:
                        return value<T>(data + other.data);
                    case INTERNAL::EVALUATION_TYPE::PERCENTAGE:
                        return value<T>(percentage + other.percentage);
                    default:
                        INTERNAL::reportStack("Evaluation type: " + std::to_string((int)evaluationType) + " not supported!");
                        EXIT(1);
                        return value<T>(0);
                    }
                }
            }

            constexpr value<T> operator-(const value<T>& other){
                if (evaluationType != other.evaluationType){
                    // TODO: add capability to call reportStack in Styles.h
                    INTERNAL::LOGGER::Log("Cannot substract two different eval type values!");
                    EXIT(1);
                    return false;   // for warnings.
                }
                else{
                    switch (evaluationType)
                    {
                    case INTERNAL::EVALUATION_TYPE::DEFAULT:
                        return value<T>(data - other.data);
                    case INTERNAL::EVALUATION_TYPE::PERCENTAGE:
                        return value<T>(percentage - other.percentage);
                    default:
                        INTERNAL::LOGGER::Log("Evaluation type: " + std::to_string((int)evaluationType) + " not supported!");
                        EXIT(1);
                        return value<T>(0);
                    }
                }
            }

            /**
             * Evaluate function
             * @param parental_value The value to be multiplied by. Only used if the evaluation type is PERCENTAGE.
             * 
             * This function is used to evaluate the value of the variant based on the evaluation type.
             * If the evaluation type is DEFAULT, the data is returned without any modification.
             * If the evaluation type is PERCENTAGE, the parental value is multiplied by the data and the result is returned.
             * If the evaluation type is not supported, an error message is printed and the data is returned without any modification.
             */
            constexpr void Evaluate(const T parental_value) {
                switch (evaluationType) {
                    case INTERNAL::EVALUATION_TYPE::DEFAULT:
                        // If the evaluation type is DEFAULT then just return the data without any modification
                        return;
                    case INTERNAL::EVALUATION_TYPE::PERCENTAGE:
                        // If the evaluation type is PERCENTAGE then multiply the parental value by the data and return the result
                        data = static_cast<T>(static_cast<T>(parental_value) * percentage);

                        #if GGUI_DEBUG
                        if (isNonDiscriminantScalar<T>(parental_value, percentage)){
                            INTERNAL::LOGGER::Log("Percentage value of: '" + std::to_string(percentage) + "' causes non-discriminant results with: '" + toString(parental_value) + "'.");
                        }
                        #endif

                        return;
                    default:
                        INTERNAL::LOGGER::Log("Evaluation type not supported!");
                        // If the evaluation type is not supported then just return the data without any modification
                        return;
                }
            }

            /**
             * Get the value of the variant.
             * @tparam P The type to cast the variant to.
             * @return The value of the variant as the requested type.
             * @throws std::bad_variant_access If the requested type doesn't match the type of the data.
             */
            template<typename P>
            constexpr P get() const {
                return data;
            }

            /**
             * Getter methods
             * @tparam P The type to cast the variant to.
             * @return The value of the variant as the requested type.
             * @throws std::bad_variant_access If the requested type doesn't match the type of the data.
             */
            template<typename P>
            constexpr P get() {                
                return data;
            }

            /**
             * Get the evaluation type of the variant.
             * @return The evaluation type of the variant.
             */
            constexpr INTERNAL::EVALUATION_TYPE Get_Type() { return evaluationType; }

            /**
             * Get the evaluation type of the variant.
             * @return The evaluation type of the variant.
             */
            constexpr INTERNAL::EVALUATION_TYPE Get_Type() const { return evaluationType; }

            /**
             * @brief Direct access to the underlying data of the variant.
             *        This is an exclusive Dev function, with serious repercussions for misuse.
             * @tparam P The type to cast the variant to.
             * @return A reference to the underlying data of the variant.
             * @throws std::bad_variant_access If the requested type doesn't match the type of the data.
             */
            template<typename P>
            constexpr P& Direct() { 
                return data;
            }

            /**
             * @brief Set the value of the variant.
             * @param value The value to set the variant to.
             * @details This sets the value of the variant to the provided value.
             *          The evaluation type is set to INTERNAL::EVALUATION_TYPE::DEFAULT.
             */
            constexpr void Set(const T d) {
                data = d;
                evaluationType = INTERNAL::EVALUATION_TYPE::DEFAULT;
            }

            /**
             * @brief Set the value of the variant to a percentage.
             * @param value The value to set the variant to.
             * @details This sets the value of the variant to the provided value, and sets the evaluation type to INTERNAL::EVALUATION_TYPE::PERCENTAGE.
             */
            constexpr void Set(const float f){
                percentage = f;
                evaluationType = INTERNAL::EVALUATION_TYPE::PERCENTAGE;
            }
        };

        class styleBase{
        public:
            // This is used to prevent accidental overwrites.
            VALUE_STATE status;

            // Represents when the value is embedded.
            INTERNAL::EMBED_ORDER order;

            // This is used to store all appended style_bases through the operator|.
            styleBase* next;

            /**
             * @brief Construct a new constexpr style_base object.
             * @param status The status to initialize the style_base with.
             * @param use_constexpr A flag indicating whether to use constexpr. This parameter is not used.
             */
            constexpr styleBase(VALUE_STATE Status = VALUE_STATE::UNINITIALIZED, INTERNAL::EMBED_ORDER Order = INTERNAL::EMBED_ORDER::INSTANT) : status(Status), order(Order), next(nullptr) {}

            /**
             * @brief Destructor of the style_base class.
             *
             * This destructor takes care of deleting the "Other" pointer if it is a heap-allocated object.
             * It does this by calling the Is_Deletable() function to check if the pointer is likely to be
             * a heap-allocated object. If it is, it deletes the object using the delete keyword.
             */
            virtual ~styleBase();

            virtual styleBase* copy() const = 0;

            /**
             * @brief Overload the | operator to allow for appending of style_bases.
             * @param other The style_base to append.
             * @return A pointer to the style_base that was appended to.
             * @note This bind the two objects indefinefly!
             */
            constexpr styleBase* operator|(styleBase* other){
                other->next = this;

                return other;
            }

            /**
             * @brief Overload the | operator to allow for appending of style_bases.
             * @param other The style_base to append.
             * @return A reference to the style_base that was appended to.
             * @note This bind the two objects indefinefly!
             */
            constexpr styleBase& operator|(styleBase& other){
                other.next = this;

                return other;
            }

            /**
             * @brief Overload the | operator to allow for appending of style_bases using move semantics.
             * @param other The style_base to append.
             * @return A reference to the style_base that was appended to.
             * @details This function is used to append a style_base to another style_base using move semantics.
             *          It sets the Other pointer of the moved object to the current object. Then it returns
             *          a reference to the moved object to ensure proper chaining of operations.
             * @note Since operators for "a | b | c" will re-write the previous "a | b" when at c with "ab | c",
             *       it will produce "ac" and not "abc". Thus, we return the secondary object so that: 
             *       "a | b | c" -> "b(a) | c" -> "c(b(a))".
             */
            constexpr styleBase&& operator|(styleBase&& other){
                other.next = this;

                return std::move(other);
            }

            /**
             * @brief Evaluates the style value based on a given Styling object.
             * @param host The Styling object to evaluate the style value with.
             * @details This function is used to evaluate the style value based on a given Styling object.
             *          It is called by the Styling class when the style value needs to be evaluated.
             *          The function is responsible for setting the Status variable to the evaluated status.
             *          The function is also responsible for setting the Value variable to the evaluated value.
             *          The function should be implemented by the derived classes to perform the evaluation.
             */
            virtual void evaluate([[maybe_unused]] const styling* self, [[maybe_unused]] const styling* host) {};

            /**
             * @brief Imprints the style's identity into the Styling object.
             * @param host The Styling object to imprint the style into.
             * @param owner The Element that owns the style.
             * @return The type of stain this style will leave.
             * @details This function allows single style classes to incorporate their unique characteristics into a Styling object. 
             *          It should be implemented by derived classes to define how the style affects the Styling and Element objects.
             */
            virtual INTERNAL::STAIN_TYPE embedValue([[maybe_unused]] styling* host, [[maybe_unused]] element* owner) { return INTERNAL::STAIN_TYPE::CLEAN; };
        };

        class RGBValue : public styleBase{
        public:
            value<RGB> color = RGB(0, 0, 0);

            /**
             * @brief Construct a new RGB_VALUE object using constexpr.
             * @param value The RGB value to set.
             * @param Default The default value state.
             * @param use_constexpr Flag indicating whether to use constexpr.
             * @details This constructor initializes an RGB_VALUE object with the given parameters,
             *          using constexpr for compile-time evaluation.
             */
            constexpr RGBValue(const GGUI::RGB Value, const VALUE_STATE Default = VALUE_STATE::VALUE) 
                : styleBase(Default), color(Value, INTERNAL::EVALUATION_TYPE::DEFAULT) {}

            /**
             * @brief Construct a new RGB_VALUE object using constexpr.
             * @param value The relative percentage of inherited from parent.
             * @param Default The default value state.
             * @param use_constexpr Flag indicating whether to use constexpr.
             * @details This constructor initializes an RGB_VALUE object with the given parameters,
             *          using constexpr for compile-time evaluation.
             */
            constexpr RGBValue(const float Value, const VALUE_STATE Default = VALUE_STATE::VALUE) 
                : styleBase(Default), color(Value, INTERNAL::EVALUATION_TYPE::PERCENTAGE) {}
            
            constexpr RGBValue() = default;

            /**
             * @brief Destructor for the RGB_VALUE class.
             * @details This destructor is necessary to ensure that the base class destructor is called.
             */
            inline ~RGBValue() override { styleBase::~styleBase(); }

            inline styleBase* copy() const override {
                return new RGBValue(*this);
            }

            /**
             * @brief Overload the assignment operator for RGB_VALUE.
             * @param other The other RGB_VALUE object to assign from.
             * @return A reference to this RGB_VALUE object.
             * @details This function assigns the value and status of the other RGB_VALUE object to this one.
             *          It only copies the information if the other object is enabled.
             */
            constexpr RGBValue& operator=(const RGBValue& other){
                // Only copy the information if the other is enabled.
                if (other.status >= status){
                    color = other.color;

                    status = other.status;
                }
                return *this;
            }

            /**
             * @brief Overload the assignment operator for RGB_VALUE.
             * @param other The other RGB_VALUE object to assign from.
             * @return A reference to this RGB_VALUE object.
             * @details This function assigns the RGB value of the other RGB_VALUE object to this one.
             *          It sets the status of the object to VALUE_STATE::VALUE after the assignment.
             */
            constexpr RGBValue& operator=(const GGUI::RGB other){
                color = other;
                status = VALUE_STATE::VALUE;
                return *this;
            }
        
            /**
             * @brief Overload the compare operator for RGB_VALUE.
             * @param other The other RGB_VALUE object to compare against.
             * @return A boolean indicating whether the two RGB_VALUE objects are equal.
             * @details This function compares the value and status of the two RGB_VALUE objects.
             */
            constexpr bool operator==(const RGBValue& other) const{
                return color.get<RGB>() == other.color.get<RGB>();
            }

            /**
             * @brief Overload the compare operator for RGB_VALUE.
             * @param other The other RGB_VALUE object to compare against.
             * @return A boolean indicating whether the two RGB_VALUE objects are not equal.
             * @details This function compares the value and status of the two RGB_VALUE objects.
             */
            constexpr bool operator!=(const RGBValue& other) const{
                return !(color.get<RGB>() == other.color.get<RGB>());
            }

            /**
             * @brief Copy constructor for RGB_VALUE.
             * @param other The RGB_VALUE object to copy from.
             * @details This constructor creates a new RGB_VALUE object that is a copy of the other one.
             *          It copies the value and status of the other object, and sets the status to VALUE_STATE::VALUE.
             */
            constexpr RGBValue(const GGUI::STYLING_INTERNAL::RGBValue& other) : styleBase(other.status), color(other.color){}

            /**
             * @brief Embeds the value of an RGB_VALUE object into a Styling object.
             * @param host The Styling object to embed the value into.
             * @param owner The Element that owns the Styling object.
             * @return A INTERNAL::STAIN_TYPE indicating the type of stain that was embedded.
             * @details This function does not actually embed any values and simply returns INTERNAL::STAIN_TYPE::CLEAN.
             */
            INTERNAL::STAIN_TYPE embedValue([[maybe_unused]] styling* host, element* owner) override;

            /**
             * @brief Evaluate the RGB_VALUE.
             * @param owner The styling owner to evaluate against.
             * @details This is a pure virtual function that subclasses must implement to define how the RGB value is evaluated.
             */
            inline void evaluate([[maybe_unused]] const styling* self, [[maybe_unused]] const styling* owner) override {};
        };

        class boolValue : public styleBase{
        public:
            bool value = false;

            /**
             * @brief Construct a new constexpr BOOL_VALUE object.
             * @param value The boolean value to set.
             * @param Default The default value state to use.
             * @param use_constexpr A flag indicating whether to use constexpr. This parameter is not used.
             */
            constexpr boolValue(const bool Value, const VALUE_STATE Default = VALUE_STATE::VALUE) 
                : styleBase(Default), value(Value) {}
            
            constexpr boolValue() = default;

            /**
             * @brief Destructor for the BOOL_VALUE class.
             * @details This destructor is responsible for properly deallocating all the memory
             * allocated by the BOOL_VALUE object, including its parent class resources.
             */
            inline ~boolValue() override { styleBase::~styleBase(); }

            inline styleBase* copy() const override {
                return new boolValue(*this);
            }

            /**
             * @brief Overload the assignment operator for BOOL_VALUE.
             * @param other The other BOOL_VALUE object to assign from.
             * @return A reference to this BOOL_VALUE object.
             * @details This function assigns the value and status of the other BOOL_VALUE object to this one.
             *          It only copies the information if the other object is enabled.
             */
            constexpr boolValue& operator=(const boolValue& other){
                // Only copy the information if the other is enabled.
                if (other.status >= status){
                    value = other.value; // Copy the value from the other object
                    
                    status = other.status; // Update the status to the other object's status
                }
                return *this; // Return a reference to this object
            }

            /**
             * @brief Overload the assignment operator for BOOL_VALUE.
             * @param other The boolean value to assign to this BOOL_VALUE.
             * @return A reference to this BOOL_VALUE object.
             * @details This function assigns the boolean value to the BOOL_VALUE object,
             *          setting the status to VALUE_STATE::VALUE.
             */
            constexpr boolValue& operator=(const bool other){
                value = other; // Assign the boolean value to the Value member
                status = VALUE_STATE::VALUE; // Set the status to indicate a valid value
                return *this; // Return a reference to this object
            }
        
            /**
             * @brief Copy constructor for BOOL_VALUE.
             * @param other The BOOL_VALUE object to copy from.
             * @details This constructor creates a new BOOL_VALUE object that is a copy of the other one.
             *          It copies the value and status of the other object.
             */
            constexpr boolValue(const GGUI::STYLING_INTERNAL::boolValue& other) 
                : styleBase(other.status), value(other.value) {}
            
            /**
             * @brief Evaluate the BOOL_VALUE.
             * @param owner The styling owner to evaluate against.
             * @details This function is a no-op for BOOL_VALUE, as it does not have any dynamically computable values.
             */
            inline void evaluate([[maybe_unused]] const styling* self, [[maybe_unused]] const styling* owner) override {};
            
            /**
             * @brief Embeds the value of a BOOL_VALUE object into a Styling object.
             * @param host The Styling object to embed the value into.
             * @param owner The Element that owns the Styling object.
             * @return A INTERNAL::STAIN_TYPE indicating the type of stain that was embedded.
             * @details This function does not actually embed any values and simply returns INTERNAL::STAIN_TYPE::CLEAN.
             */
            INTERNAL::STAIN_TYPE embedValue([[maybe_unused]] styling* host,  element* owner) override;
        };
        
        class numberValue : public styleBase{
        public:
            value<int> number = 0;

            /**
             * @brief Construct a new NUMBER_VALUE object from a float.
             * @param value The floating point value to initialize the NUMBER_VALUE with.
             * @param Default The default value state of the NUMBER_VALUE.
             * @details This constructor initializes the NUMBER_VALUE with the provided float value and default state.
             *          The value is converted to a percentage (multiplying by 0.01) and stored as a float in the Value member.
             */
            constexpr numberValue(float Value, VALUE_STATE Default = VALUE_STATE::VALUE) : styleBase(Default), number(Value, INTERNAL::EVALUATION_TYPE::PERCENTAGE){}

            /**
             * @brief Construct a new NUMBER_VALUE object from an integer using constexpr.
             * @param value The integer value to initialize the NUMBER_VALUE with.
             * @param Default The default value state of the NUMBER_VALUE.
             * @param use_constexpr A flag indicating whether to use constexpr. This parameter is not used.
             * @details This constructor initializes a NUMBER_VALUE object with the provided integer value and default state,
             *          using constexpr for compile-time evaluation.
             */
            constexpr numberValue(int Value, VALUE_STATE Default = VALUE_STATE::VALUE) : styleBase(Default), number(Value, INTERNAL::EVALUATION_TYPE::DEFAULT){}
            
            constexpr numberValue(unsigned int Value, VALUE_STATE Default = VALUE_STATE::VALUE) : styleBase(Default), number((signed int)Value, INTERNAL::EVALUATION_TYPE::DEFAULT){}

            constexpr numberValue() = default;

            /**
             * @brief Destructor for NUMBER_VALUE.
             * @details This destructor is responsible for cleaning up the resources allocated by the NUMBER_VALUE object.
             *          It is marked as `override` to ensure that it is called when the object is destroyed.
             */
            inline ~numberValue() override { styleBase::~styleBase(); }

            inline styleBase* copy() const override {
                return new numberValue(*this);
            }

            /**
             * @brief Overload the assignment operator for NUMBER_VALUE.
             * @param other The other NUMBER_VALUE object to assign from.
             * @return A reference to this NUMBER_VALUE object.
             * @details This function assigns the value and status of the other NUMBER_VALUE object to this one.
             *          It only copies the information if the other object is enabled.
             */
            constexpr numberValue& operator=(const numberValue& other){
                // Only copy the information if the other is enabled.
                if (other.status >= status){
                    number = other.number;

                    status = other.status;
                }
                return *this;
            }

            /**
             * @brief Overload the assignment operator for NUMBER_VALUE.
             * @param other The other number to assign from.
             * @return A reference to this NUMBER_VALUE object.
             * @details This function assigns the value of the other number to this NUMBER_VALUE object.
             *          It sets the status to VALUE_STATE::VALUE and returns the modified object.
             */
            constexpr numberValue& operator=(const int other){
                number = other;
                status = VALUE_STATE::VALUE;
                return *this;
            }

            /**
             * @brief Overload the compare operator for the NUMBER_VALUE class.
             * @param other The other NUMBER_VALUE object to compare against.
             * @return true if the two objects are equal; false otherwise.
             * @details This function compares the value and status of the two NUMBER_VALUE objects.
             */
            constexpr bool operator==(const numberValue& other) const{
                return number.get<int>() == other.number.get<int>();
            }

            /**
             * @brief Overload the compare operator for the NUMBER_VALUE class.
             * @param other The other NUMBER_VALUE object to compare against.
             * @return true if the two objects are not equal; false otherwise.
             * @details This function compares the value and status of the two NUMBER_VALUE objects.
             */
            constexpr bool operator!=(const numberValue& other) const{
                return number.get<int>() != other.number.get<int>();
            }
        
            /**
             * @brief Construct a new NUMBER_VALUE object from another NUMBER_VALUE object using constexpr.
             * @param other The other NUMBER_VALUE object to construct from.
             * @details This constructor initializes a new NUMBER_VALUE object with the same value and status as the given object.
             */
            constexpr numberValue(const GGUI::STYLING_INTERNAL::numberValue& other) : styleBase(other.status), number(other.number){}
            
            /**
             * @brief Embeds the value of a NUMBER_VALUE object into a Styling object.
             * @param host The Styling object to embed the value into.
             * @param owner The Element that owns the Styling object.
             * @return A INTERNAL::STAIN_TYPE indicating the type of stain that was embedded.
             * @details This function does not actually embed any values and simply returns INTERNAL::STAIN_TYPE::CLEAN.
             */
            INTERNAL::STAIN_TYPE embedValue(styling* host, element* owner) override;
            
            /**
             * @brief Evaluate the RGB_VALUE.
             * @param owner The styling owner to evaluate against.
             * @details This is a pure virtual function that subclasses must implement to define how the RGB value is evaluated.
             *          When called, the function should evaluate the RGB value based on the owner object and set the Value property accordingly.
             */
            inline void evaluate([[maybe_unused]] const styling* self, [[maybe_unused]] const styling* owner) override {};

            /**
             * @brief Directly access the value of this NUMBER_VALUE object.
             * @return A reference to the value of this NUMBER_VALUE object.
             * @details This function returns a reference to the value of this NUMBER_VALUE object, allowing it to be directly accessed and modified.
             */
            constexpr int& direct() { return number.Direct<int>(); }
        };

        template<typename T>
        class enumValue : public styleBase{
        public:
            T value;

            /**
             * @brief Construct a new ENUM_VALUE object using constexpr.
             * @param value The enum value to set.
             * @param Default The default value state.
             * @details This constructor initializes an ENUM_VALUE object with the given enum value and default state,
             *          using constexpr for compile-time evaluation.
             */
            constexpr enumValue(const T Value, const VALUE_STATE Default = VALUE_STATE::VALUE) : styleBase(Default), value(Value){}

            constexpr enumValue() = default;

            /**
             * @brief Destructor for ENUM_VALUE.
             * @details This destructor is responsible for cleaning up the resources allocated by the ENUM_VALUE object.
             *          It is marked as `override` to ensure that it is called when the object is destroyed.
             */
            inline ~enumValue() override { styleBase::~styleBase(); }

            inline styleBase* copy() const override {
                return new enumValue(*this);
            }

            /**
             * @brief Overload the assignment operator for ENUM_VALUE.
             * @param other The other ENUM_VALUE object to assign from.
             * @return A reference to this ENUM_VALUE object.
             * @details This function assigns the value and status of the other ENUM_VALUE object to this one.
             *          It only copies the information if the other object is enabled.
             */
            constexpr enumValue& operator=(const enumValue& other){
                // Only copy the information if the other is enabled.
                if (other.status >= status){
                    value = other.value;

                    status = other.status;
                }
                return *this;
            }

            /**
             * @brief Overload the assignment operator for ENUM_VALUE.
             * @param other The other value to assign from.
             * @return A reference to this ENUM_VALUE object.
             * @details This function assigns the value and status of the other ENUM_VALUE object to this one.
             *          It only copies the information if the other object is enabled.
             */
            constexpr enumValue& operator=(const T other){
                value = other;
                status = VALUE_STATE::VALUE;
                return *this;
            }
        
            /**
             * @brief Copy constructor for ENUM_VALUE.
             * @param other The ENUM_VALUE object to copy from.
             * @details This constructor creates a new ENUM_VALUE object that is a copy of the other one.
             *          It copies the value and status of the other object.
             */
            constexpr enumValue(const GGUI::STYLING_INTERNAL::enumValue<T>& other) 
                : styleBase(other.status), value(other.value) {}
                        
            /**
             * @brief Evaluate the style.
             * @param owner The Styling object that owns this style.
             * @details This function is used to evaluate the style with the given Styling object.
             *          It is used to support dynamic values like percentage depended values.
             *          The function does not do anything as of now.
             */
            inline void evaluate([[maybe_unused]] const styling* self, [[maybe_unused]] const styling* owner) override {};

            /**
             * @brief Embed the value of this style into the given Styling object.
             * @param host The Styling object to embed the value into.
             * @param owner The Element that owns the Styling object.
             * @return The type of stain that this style embeds.
             * @details This function embeds the value of this style into the given Styling object.
             *          The value is not evaluated or modified in any way.
             *          It is used to support dynamic values like percentage depended values.
             *          The function does not do anything as of now.
             */
            inline INTERNAL::STAIN_TYPE embedValue([[maybe_unused]] styling* host, [[maybe_unused]] element* owner) override { return INTERNAL::STAIN_TYPE::CLEAN; };
        };
        
        class vectorValue : public styleBase{
        public:
            value<int> X = 0;
            value<int> Y = 0;
            value<int> Z = 0;

            /**
             * @brief Construct a new Vector object using constexpr.
             * @param value The value to set.
             * @param Default The default value state.
             * @param use_constexpr Flag indicating whether to use constexpr.
             * @details This constructor initializes a Vector object with the given parameters,
             *          using constexpr for compile-time evaluation.
             */
            constexpr vectorValue(const GGUI::IVector3 value, const VALUE_STATE Default = VALUE_STATE::VALUE) : styleBase(Default), 
                X(value.X, INTERNAL::EVALUATION_TYPE::DEFAULT), Y(value.Y, INTERNAL::EVALUATION_TYPE::DEFAULT), Z(value.Z, INTERNAL::EVALUATION_TYPE::DEFAULT){}
            
            constexpr vectorValue(const value<int> x, const value<int> y, const value<int> z = 0, const VALUE_STATE Default = VALUE_STATE::VALUE) : styleBase(Default), X(x), Y(y), Z(z){}

            constexpr vectorValue() = default;

            /**
             * @brief Destructor for Vector.
             * @details This destructor is responsible for cleaning up all resources allocated by the Vector object.
             *          It calls the base class destructor (style_base::~style_base()) to ensure all parent class resources
             *          are properly cleaned up.
             */
            inline ~vectorValue() override { styleBase::~styleBase(); }

            inline styleBase* copy() const override {
                return new vectorValue(*this);
            }

            /**
             * @brief Overload the assignment operator for Vector.
             * @param other The other Vector object to assign from.
             * @return A reference to this Vector object.
             * @details This function assigns the value and status of the other Vector object to this one.
             *          It only copies the information if the other object is enabled.
             */
            constexpr vectorValue& operator=(const vectorValue& other){
                // Only copy the information if the other is enabled.
                if (other.status >= status){
                    X = other.X;
                    Y = other.Y;
                    Z = other.Z;

                    status = other.status;
                }
                return *this;
            }

            /**
             * @brief Overload the assignment operator for Vector.
             * @param other The IVector3 object to assign from.
             * @return A reference to this Vector object.
             * @details This function assigns the value and status of the other IVector3 object to this one.
             *          It sets the status to VALUE_STATE::VALUE and the value to the given IVector3 object.
             */
            constexpr vectorValue& operator=(const GGUI::IVector3 other){
                X = other.X;
                Y = other.Y;
                Z = other.Z;
                status = VALUE_STATE::VALUE;
                return *this;
            }

            /** 
             * @brief Overload the equals to operator for Vector.
             * @param other The other Vector object to compare with.
             * @return A boolean indicating whether the two Vector objects are equal.
             */
            constexpr bool operator==(const vectorValue& other) const {
                return X == other.X && Y == other.Y && Z == other.Z;
            }

            /** 
             * @brief Overload the not equals to operator for Vector.
             * @param other The other Vector object to compare with.
             * @return A boolean indicating whether the two Vector objects are not equal.
             */
            constexpr bool operator!=(const vectorValue& other) const {
                return !(*this == other);
            }

            /**
             * @brief Construct a new constexpr Vector object from another Vector.
             * @param other The Vector to copy from.
             * @details This constructor initializes a Vector object using the status and value from another Vector object,
             *          utilizing constexpr for compile-time evaluation.
             */
            constexpr vectorValue(const GGUI::STYLING_INTERNAL::vectorValue& other) 
                : styleBase(other.status), X(other.X), Y(other.Y), Z(other.Z) {}
            
            /**
             * @brief Get the current value of the Vector.
             * @return The current IVector3 value.
             * @details Packs the values into an IVector3; Returns it.
             */
            constexpr IVector3 get() { 
                return IVector3(X.get<int>(), Y.get<int>(), Z.get<int>());
            }

            /**
             * @brief Get the current value of the Vector.
             * @return The current IVector3 value.
             * @details This function returns the current value stored in the Vector.
             *          This value can be used to get the current value of the Vector as an IVector3 object.
             */
            constexpr IVector3 get() const { return IVector3(X.get<int>(), Y.get<int>(), Z.get<int>()); }

            /**
             * @brief Set the current value of the Vector.
             * @param value The new value to set the Vector to.
             * @details This function sets the current value of the Vector to the given IVector3 object.
             *          It also sets the Status of the Vector to VALUE_STATE::VALUE.
             */
            constexpr void set(const IVector3 value){
                X = value.X;
                Y = value.Y;
                Z = value.Z;
                status = VALUE_STATE::VALUE;
            }

            inline vectorValue operator+(const vectorValue& other) {
                // check if debug mode is on
                #ifdef GGUI_DEBUG
                assert(X.Get_Type() == other.X.Get_Type() && "two different evaluation types for X");
                assert(Y.Get_Type() == other.Y.Get_Type() && "two different evaluation types for Y");
                assert(Z.Get_Type() == other.Z.Get_Type() && "two different evaluation types for Z");
                #endif

                return vectorValue(X + other.X, Y + other.Y, Z + other.Z);
            }

            inline vectorValue operator-(const vectorValue& other){
                // check if debug mode is on
                #ifdef GGUI_DEBUG
                assert(X.Get_Type() != other.X.Get_Type() && "two different evaluation types for X");
                assert(Y.Get_Type() != other.Y.Get_Type() && "two different evaluation types for Y");
                assert(Z.Get_Type() != other.Z.Get_Type() && "two different evaluation types for Z");
                #endif

                return vectorValue(X - other.X, Y - other.Y, Z - other.Z);
            }
        
            inline void operator+=(const IVector3 v){
                // check if debug mode is on
                #ifdef GGUI_DEBUG
                assert(X.Get_Type() == INTERNAL::EVALUATION_TYPE::DEFAULT && "X is not a default evaluation type");
                assert(Y.Get_Type() == INTERNAL::EVALUATION_TYPE::DEFAULT && "Y is not a default evaluation type");
                assert(Z.Get_Type() == INTERNAL::EVALUATION_TYPE::DEFAULT && "Z is not a default evaluation type");
                #endif

                X.Direct<int>() += v.X;
                Y.Direct<int>() += v.Y;
                Z.Direct<int>() += v.Z;
            }

            /**
             * @brief Evaluate the Vector value.
             * @param owner The Styling object that the Vector is a part of.
             * @details This function evaluates the Vector value.
             *          For dynamically computable values like percentage depended this function is overridden.
             *          Currently it covers:
             *          - screen space
             */
            inline void evaluate([[maybe_unused]] const styling* self, [[maybe_unused]] const styling* owner) override {};
            
            /**
             * @brief Embeds the value of a Vector object into a Styling object.
             * @param host The Styling object to embed the value into.
             * @param owner The Element that owns the Styling object.
             * @return A INTERNAL::STAIN_TYPE indicating the type of stain that was embedded.
             * @details This function does not actually embed any values and simply returns INTERNAL::STAIN_TYPE::CLEAN.
             */
            INTERNAL::STAIN_TYPE embedValue([[maybe_unused]] styling* host,  element* owner) override;
        };
    
        /**
         * @brief Checks if there is a leftover when multiplying two values.
         *
         * This function takes two parameters of type P and float, respectively, and checks if there is any leftover
         * when multiplying them. It does this by comparing the product of the integer parts of the parameters with
         * the product of the original parameters.
         *
         * @tparam P The type of the first parameter.
         * @param A The first parameter.
         * @param B The second parameter.
         * @return true if there is a leftover, false otherwise.
         */
        template<typename P>
        constexpr bool hasLeftover(const P A, const float B){
            return (static_cast<int>(A) * static_cast<int>(B)) - (static_cast<float>(A) * B) != 0;
        }

        /**
         * @brief Checks if a given value is a non-discriminant scalar.
         * @tparam P The type of the value to be checked.
         * @param value The value to be checked.
         * @param scalar The scalar value to compare against.
         * @return true if the value is a non-discriminant scalar, false otherwise.
         * 
         * @note Supported types: float | int | unsigned char | unsigned int | RGB | RGBA | FVector2 | FVector3 | IVector3 | GGUI::STYLING_INTERNAL::Vector | RGB_VALUE | NUMBER_VALUE
         */
        template<typename P>
        constexpr bool isNonDiscriminantScalar(const P value, const float scalar){
            // Skip checking for redundant scalars.
            if (scalar == 1.0f || scalar == 0.0f){
                return false;
            }

            // For types of int and float, we can use basic std::fmod
            if constexpr (std::is_same_v<P, float> || std::is_same_v<P, int> || std::is_same_v<P, unsigned char> || std::is_same_v<P, unsigned int>){
                return hasLeftover<P>(value, scalar);
            }
            else if constexpr (std::is_same_v<P, RGB>){
                return isNonDiscriminantScalar<unsigned char>(value.Red, scalar) && isNonDiscriminantScalar<unsigned char>(value.Green, scalar) && isNonDiscriminantScalar<unsigned char>(value.Blue, scalar);
            }
            else if constexpr (std::is_same_v<P, FVector2>){
                return isNonDiscriminantScalar<float>(value.X, scalar) && isNonDiscriminantScalar<float>(value.Y, scalar);
            }
            else if constexpr (std::is_same_v<P, FVector3>){
                return isNonDiscriminantScalar<float>(value.Z, scalar) && isNonDiscriminantScalar<FVector2>(value, scalar);
            }
            else if constexpr (std::is_same_v<P, IVector3>){
                return isNonDiscriminantScalar<int>(value.X, scalar) && isNonDiscriminantScalar<int>(value.Y, scalar) && isNonDiscriminantScalar<int>(value.Z, scalar);
            }
            else if constexpr (std::is_same_v<P, GGUI::STYLING_INTERNAL::vectorValue>){
                return isNonDiscriminantScalar<IVector3>(value.Get(), scalar);
            }
            else if constexpr (std::is_same_v<P, RGBValue>){
                return isNonDiscriminantScalar<RGB>(static_cast<RGBValue>(value).color.get<RGB>(), scalar);
            }
            else if constexpr (std::is_same_v<P, numberValue>){
                return isNonDiscriminantScalar<int>(static_cast<numberValue>(value).number.get<int>(), scalar);
            }
            else {
                static_assert(!std::is_same_v<P, P>, "Unsupported type!");
            }
        }

        template<typename P>
        constexpr std::string toString(const P value){
            if constexpr (std::is_same_v<P, std::string> || std::is_same_v<P, const char*> || std::is_same_v<P, char*>){
                // These are already strings
                return value;
            }
            else if constexpr (std::is_same_v<P, float> || std::is_same_v<P, int> || std::is_same_v<P, unsigned char> || std::is_same_v<P, unsigned int>){
                return std::to_string(value);
            }
            else if constexpr (std::is_same_v<P, RGB>){
                return toString(value.Red) + ", " + toString(value.Green) + ", " + toString(value.Blue);
            }
            else if constexpr (std::is_same_v<P, FVector2>){
                return toString(value.X) + ", " + toString(value.Y);
            }
            else if constexpr (std::is_same_v<P, FVector3>){
                return toString(static_cast<FVector2>(value)) + ", " + toString(value.Z);
            }
            else if constexpr (std::is_same_v<P, IVector3>){
                return toString(value.X) + ", " + toString(value.Y) + ", " + toString(value.Z);
            }
            else if constexpr (std::is_same_v<P, GGUI::STYLING_INTERNAL::vectorValue>){
                return toString(value.Get());
            }
            else if constexpr (std::is_same_v<P, RGBValue>){
                return toString(static_cast<RGBValue>(value).color.get<RGB>());
            }
            else if constexpr (std::is_same_v<P, numberValue>){
                return toString(static_cast<numberValue>(value).number.get<int>());
            }
            else {
                static_assert(!std::is_same_v<P, P>, "Unsupported type!");
            }
        }
    
        class empty : public styleBase{
        public:
            constexpr empty() = default;

            inline ~empty() override { styleBase::~styleBase(); }

            inline styleBase* copy() const override {
                return new empty(*this);
            }
        };
    }

    class position : public STYLING_INTERNAL::vectorValue{
    public:
        constexpr position(const IVector3 value, const VALUE_STATE Default = VALUE_STATE::VALUE) : vectorValue(value, Default){}

        constexpr position(const FVector3 value, const VALUE_STATE Default = VALUE_STATE::VALUE) : vectorValue(value.X, value.Y, value.Z, Default){
            transformCenterToTopLeftOrigin();
        }

        constexpr position(const vectorValue&& value, const VALUE_STATE Default = VALUE_STATE::VALUE) : vectorValue(value.X, value.Y, value.Z, Default){
            transformCenterToTopLeftOrigin();
        }

        constexpr position(const vectorValue& value, const VALUE_STATE Default = VALUE_STATE::VALUE) : vectorValue(value.X, value.Y, value.Z, Default){
            transformCenterToTopLeftOrigin();
        }

        constexpr position(const STYLING_INTERNAL::value<int> x, const STYLING_INTERNAL::value<int> y, const STYLING_INTERNAL::value<int> z = 0, const VALUE_STATE Default = VALUE_STATE::VALUE) : vectorValue(x, y, z, Default){
            transformCenterToTopLeftOrigin();
        }

        inline ~position() override { vectorValue::~vectorValue(); }

        inline styleBase* copy() const override {
            return new position(*this);
        }

        constexpr position(const GGUI::position& other) : vectorValue(other){}

        constexpr position& operator=(const position& other) = default;

        // for dynamically computable values like percentage depended
        // currently covers:
        // - screen space
        void evaluate(const styling* self, const styling* owner) override;

        INTERNAL::STAIN_TYPE embedValue(styling* host, element* owner) override;
    protected:
        /**
         * @brief Transforms the position from center origin to top-left origin.
         *
         * This function adjusts the X and Y coordinates of the position by adding a 0.5f offset
         * if their evaluation type is PERCENTAGE. This transformation is used to convert a 
         * position that is originally centered to one that is based on the top-left corner.
         *
         * @note The Z coordinate is not affected by this transformation.
         */
        constexpr void transformCenterToTopLeftOrigin(){
            // Add 0.5f offset to the vectors
            if (X.Get_Type() == INTERNAL::EVALUATION_TYPE::PERCENTAGE)
                X = X + 0.5f;
            if (Y.Get_Type() == INTERNAL::EVALUATION_TYPE::PERCENTAGE)
                Y = Y + 0.5f;
            // no need to affect Z.
        }
    };

    class width : public STYLING_INTERNAL::numberValue{
    public:
        constexpr width(const int Value, const VALUE_STATE Default = VALUE_STATE::VALUE) : numberValue(Value, Default){}
        constexpr width(const unsigned int Value, const VALUE_STATE Default = VALUE_STATE::VALUE) : numberValue(Value, Default){}

        // When this is used we should automatically disable AllowDynamicSize
        constexpr width(const float Value, const VALUE_STATE Default = VALUE_STATE::VALUE) : numberValue(Value, Default){}

        inline ~width() override { numberValue::~numberValue(); }

        inline styleBase* copy() const override {
            return new width(*this);
        }

        constexpr width(const GGUI::width& other) : numberValue(other){}

        constexpr width& operator=(const width& other) = default;

        void evaluate(const styling* self, const styling* owner) override;

        INTERNAL::STAIN_TYPE embedValue(styling* host, element* owner) override;

        constexpr int get() const { return number.get<int>(); }

        constexpr void Set(const int Value){
            number = Value;
            status = VALUE_STATE::VALUE;
        }
    };

    class height : public STYLING_INTERNAL::numberValue{
    public:
        constexpr height(const int Value, const VALUE_STATE Default = VALUE_STATE::VALUE) : numberValue(Value, Default){}
        constexpr height(const unsigned int Value, const VALUE_STATE Default = VALUE_STATE::VALUE) : numberValue(Value, Default){}

        // When this is used we should automatically disable AllowDynamicSize
        constexpr height(const float Value, const VALUE_STATE Default = VALUE_STATE::VALUE) : numberValue(Value, Default){}

        inline ~height() override { numberValue::~numberValue(); }

        inline styleBase* copy() const override {
            return new height(*this);
        }

        constexpr height(const GGUI::height& other) : numberValue(other){}

        constexpr height& operator=(const height& other) = default;
        
        // for dynamically computable values like percentage depended
        // currently covers:
        // - screen space
        void evaluate(const styling* self, const styling* owner) override;
        
        INTERNAL::STAIN_TYPE embedValue(styling* host, element* owner) override;

        constexpr int get() const { return number.get<int>(); }

        constexpr void Set(const int Value){
            number = Value;
            status = VALUE_STATE::VALUE;
        }
    };

    class enableBorder : public STYLING_INTERNAL::boolValue{
    public:
        constexpr enableBorder(const bool Value, const VALUE_STATE Default = VALUE_STATE::VALUE) : boolValue(Value, Default){}

        constexpr enableBorder(const GGUI::enableBorder& other) : boolValue(other.value, other.status){}

        inline ~enableBorder() override { boolValue::~boolValue(); }

        inline styleBase* copy() const override {
            return new enableBorder(*this);
        }

        constexpr enableBorder& operator=(const enableBorder& other) = default;

        constexpr bool operator==(const enableBorder& other) const{
            return value == other.value;
        }

        constexpr bool operator!=(const enableBorder& other) const{
            return value != other.value;
        }

        // for dynamically computable values like percentage depended
        // currently covers:
        // - screen space
        inline void evaluate([[maybe_unused]] const styling* self, [[maybe_unused]] const styling* owner) override {};
        
        INTERNAL::STAIN_TYPE embedValue(styling* host, element* owner) override;
    };

    class textColor : public STYLING_INTERNAL::RGBValue{
    public:
        constexpr textColor(const RGB Color, const VALUE_STATE Default = VALUE_STATE::VALUE) : RGBValue(Color, Default){}

        constexpr textColor(const float relative_percentage, const VALUE_STATE Default = VALUE_STATE::VALUE) : RGBValue(relative_percentage, Default){}

        inline ~textColor() override { RGBValue::~RGBValue(); }

        inline styleBase* copy() const override {
            return new textColor(*this);
        }

        constexpr textColor(const GGUI::textColor& other) : RGBValue(other){}

        constexpr textColor& operator=(const textColor& other) = default;

        // for dynamically computable values like percentage depended
        // currently covers:
        // - screen space
        void evaluate(const styling* self, const styling* owner) override;
        
        INTERNAL::STAIN_TYPE embedValue(styling* host, element* owner) override;
    };

    class backgroundColor : public STYLING_INTERNAL::RGBValue{
    public:
        constexpr backgroundColor(const RGB Color, const VALUE_STATE Default = VALUE_STATE::VALUE) : RGBValue(Color, Default){}

        constexpr backgroundColor(const float relative_percentage, const VALUE_STATE Default = VALUE_STATE::VALUE) : RGBValue(relative_percentage, Default){}

        inline ~backgroundColor() override { RGBValue::~RGBValue(); }

        inline styleBase* copy() const override {
            return new backgroundColor(*this);
        }

        constexpr backgroundColor(const GGUI::backgroundColor& other) : RGBValue(other){}

        constexpr backgroundColor& operator=(const backgroundColor& other) = default;

        // for dynamically computable values like percentage depended
        // currently covers:
        // - screen space
        void evaluate(const styling* self, const styling* owner) override;
        
        INTERNAL::STAIN_TYPE embedValue(styling* host, element* owner) override;
    };

    class borderColor : public STYLING_INTERNAL::RGBValue{
    public:
        constexpr borderColor(const RGB Color, const VALUE_STATE Default = VALUE_STATE::VALUE) : RGBValue(Color, Default){}

        constexpr borderColor(const float relative_percentage, const VALUE_STATE Default = VALUE_STATE::VALUE) : RGBValue(relative_percentage, Default){}

        inline ~borderColor() override { RGBValue::~RGBValue(); }

        inline styleBase* copy() const override {
            return new borderColor(*this);
        }

        constexpr borderColor(const GGUI::borderColor& other) : RGBValue(other){}

        constexpr borderColor& operator=(const borderColor& other) = default;

        // for dynamically computable values like percentage depended
        // currently covers:
        // - screen space
        void evaluate(const styling* self, const styling* owner) override;
        
        INTERNAL::STAIN_TYPE embedValue(styling* host, element* owner) override;
    };

    class borderBackgroundColor : public STYLING_INTERNAL::RGBValue{
    public:
        constexpr borderBackgroundColor(const RGB Color, const VALUE_STATE Default = VALUE_STATE::VALUE) : RGBValue(Color, Default){}

        constexpr borderBackgroundColor(const float relative_percentage, const VALUE_STATE Default = VALUE_STATE::VALUE) : RGBValue(relative_percentage, Default){}

        inline ~borderBackgroundColor() override { RGBValue::~RGBValue(); }

        inline styleBase* copy() const override {
            return new borderBackgroundColor(*this);
        }

        constexpr borderBackgroundColor(const GGUI::borderBackgroundColor& other) : RGBValue(other){}

        constexpr borderBackgroundColor& operator=(const borderBackgroundColor& other) = default;
        
        // for dynamically computable values like percentage depended
        // currently covers:
        // - screen space
        void evaluate(const styling* self, const styling* owner) override;
        
        INTERNAL::STAIN_TYPE embedValue(styling* host, element* owner) override;
    };

    class hoverBorderColor : public STYLING_INTERNAL::RGBValue{
    public:
        constexpr hoverBorderColor(const RGB Color, const VALUE_STATE Default = VALUE_STATE::VALUE) : RGBValue(Color, Default){}

        constexpr hoverBorderColor(const float relative_percentage, const VALUE_STATE Default = VALUE_STATE::VALUE) : RGBValue(relative_percentage, Default){}

        inline ~hoverBorderColor() override { RGBValue::~RGBValue(); }

        inline styleBase* copy() const override {
            return new hoverBorderColor(*this);
        }

        constexpr hoverBorderColor(const GGUI::hoverBorderColor& other) : RGBValue(other){}

        constexpr hoverBorderColor& operator=(const hoverBorderColor& other) = default;
        
        // for dynamically computable values like percentage depended
        // currently covers:
        // - screen space
        void evaluate(const styling* self, const styling* owner) override;
        
        INTERNAL::STAIN_TYPE embedValue(styling* host, element* owner) override;
    };

    class hoverTextColor : public STYLING_INTERNAL::RGBValue{
    public:
        constexpr hoverTextColor(const RGB Color, const VALUE_STATE Default = VALUE_STATE::VALUE) : RGBValue(Color, Default){}

        constexpr hoverTextColor(const float  relative_percentage, const VALUE_STATE Default = VALUE_STATE::VALUE) : RGBValue(relative_percentage, Default){}

        inline ~hoverTextColor() override { RGBValue::~RGBValue(); }

        inline styleBase* copy() const override {
            return new hoverTextColor(*this);
        }

        constexpr hoverTextColor(const GGUI::hoverTextColor& other) : RGBValue(other){}

        constexpr hoverTextColor& operator=(const hoverTextColor& other) = default;
        
        // for dynamically computable values like percentage depended
        // currently covers:
        // - screen space
        void evaluate(const styling* self, const styling* owner) override;
        
        INTERNAL::STAIN_TYPE embedValue(styling* host, element* owner) override;
    };

    class hoverBackgroundColor : public STYLING_INTERNAL::RGBValue{
    public:
        constexpr hoverBackgroundColor(const RGB Color, const VALUE_STATE Default = VALUE_STATE::VALUE) : RGBValue(Color, Default){}

        constexpr hoverBackgroundColor(const float relative_percentage, const VALUE_STATE Default = VALUE_STATE::VALUE) : RGBValue(relative_percentage, Default){}

        inline ~hoverBackgroundColor() override { RGBValue::~RGBValue(); }

        inline styleBase* copy() const override {
            return new hoverBackgroundColor(*this);
        }

        constexpr hoverBackgroundColor(const GGUI::hoverBackgroundColor& other) : RGBValue(other){}

        constexpr hoverBackgroundColor& operator=(const hoverBackgroundColor& other) = default;
        
        // for dynamically computable values like percentage depended
        // currently covers:
        // - screen space
        void evaluate(const styling* self, const styling* owner) override;
        
        INTERNAL::STAIN_TYPE embedValue(styling* host, element* owner) override;
    };

    class hoverBorderBackgroundColor : public STYLING_INTERNAL::RGBValue{
    public:
        constexpr hoverBorderBackgroundColor(const RGB Color, const VALUE_STATE Default = VALUE_STATE::VALUE) : RGBValue(Color, Default){}

        constexpr hoverBorderBackgroundColor(const float relative_percentage, const VALUE_STATE Default = VALUE_STATE::VALUE) : RGBValue(relative_percentage, Default){}

        inline ~hoverBorderBackgroundColor() override { RGBValue::~RGBValue(); }

        inline styleBase* copy() const override {
            return new hoverBorderBackgroundColor(*this);
        }

        constexpr hoverBorderBackgroundColor(const GGUI::hoverBorderBackgroundColor& other) : RGBValue(other){}

        constexpr hoverBorderBackgroundColor& operator=(const hoverBorderBackgroundColor& other) = default;
        
        // for dynamically computable values like percentage depended
        // currently covers:
        // - screen space
        void evaluate(const styling* self, const styling* owner) override;
        
        INTERNAL::STAIN_TYPE embedValue(styling* host, element* owner) override;
    };

    class focusBorderColor : public STYLING_INTERNAL::RGBValue{
    public:
        constexpr focusBorderColor(const RGB Color, const VALUE_STATE Default = VALUE_STATE::VALUE) : RGBValue(Color, Default){}

        constexpr focusBorderColor(const float relative_percentage, const VALUE_STATE Default = VALUE_STATE::VALUE) : RGBValue(relative_percentage, Default){}

        inline ~focusBorderColor() override { RGBValue::~RGBValue(); }

        inline styleBase* copy() const override {
            return new focusBorderColor(*this);
        }

        constexpr focusBorderColor(const GGUI::focusBorderColor& other) : RGBValue(other){}

        constexpr focusBorderColor& operator=(const focusBorderColor& other) = default;
        
        // for dynamically computable values like percentage depended
        // currently covers:
        // - screen space
        void evaluate(const styling* self, const styling* owner) override;
        
        INTERNAL::STAIN_TYPE embedValue(styling* host, element* owner) override;
    };

    class focusTextColor : public STYLING_INTERNAL::RGBValue{
    public:
        constexpr focusTextColor(const RGB Color, const VALUE_STATE Default = VALUE_STATE::VALUE) : RGBValue(Color, Default){}

        constexpr focusTextColor(const float relative_percentage, const VALUE_STATE Default = VALUE_STATE::VALUE) : RGBValue(relative_percentage, Default){}

        inline ~focusTextColor() override { RGBValue::~RGBValue(); }

        inline styleBase* copy() const override {
            return new focusTextColor(*this);
        }

        constexpr focusTextColor(const GGUI::focusTextColor& other) : RGBValue(other){}

        constexpr focusTextColor& operator=(const focusTextColor& other) = default;
        
        // for dynamically computable values like percentage depended
        // currently covers:
        // - screen space
        void evaluate(const styling* self, const styling* owner) override;
        
        INTERNAL::STAIN_TYPE embedValue(styling* host, element* owner) override;
    };

    class focusBackgroundColor : public STYLING_INTERNAL::RGBValue{
    public:
        constexpr focusBackgroundColor(const RGB Color, const VALUE_STATE Default = VALUE_STATE::VALUE) : RGBValue(Color, Default){}

        constexpr focusBackgroundColor(const float relative_percentage, const VALUE_STATE Default = VALUE_STATE::VALUE) : RGBValue(relative_percentage, Default){}

        inline ~focusBackgroundColor() override { RGBValue::~RGBValue(); }

        inline styleBase* copy() const override {
            return new focusBackgroundColor(*this);
        }

        constexpr focusBackgroundColor(const GGUI::focusBackgroundColor& other) : RGBValue(other){}

        constexpr focusBackgroundColor& operator=(const focusBackgroundColor& other) = default;
        
        // for dynamically computable values like percentage depended
        // currently covers:
        // - screen space
        void evaluate(const styling* self, const styling* owner) override;
        
        INTERNAL::STAIN_TYPE embedValue(styling* host, element* owner) override;
    };

    class focusBorderBackgroundColor : public STYLING_INTERNAL::RGBValue{
    public:
        constexpr focusBorderBackgroundColor(const RGB Color, const VALUE_STATE Default = VALUE_STATE::VALUE) : RGBValue(Color, Default){}

        constexpr focusBorderBackgroundColor(const float relative_percentage, const VALUE_STATE Default = VALUE_STATE::VALUE) : RGBValue(relative_percentage, Default){}

        inline ~focusBorderBackgroundColor() override { RGBValue::~RGBValue(); }

        inline styleBase* copy() const override {
            return new focusBorderBackgroundColor(*this);
        }

        constexpr focusBorderBackgroundColor(const GGUI::focusBorderBackgroundColor& other) : RGBValue(other){}

        constexpr focusBorderBackgroundColor& operator=(const focusBorderBackgroundColor& other) = default;
        
        // for dynamically computable values like percentage depended
        // currently covers:
        // - screen space
        void evaluate(const styling* self, const styling* owner) override;
        
        INTERNAL::STAIN_TYPE embedValue(styling* host, element* owner) override;
    };

    class styledBorder : public STYLING_INTERNAL::styleBase{
    public:
        const char* topLeftCorner             = "┌";//"\e(0\x6c\e(B";
        const char* bottomLeftCorner          = "└";//"\e(0\x6d\e(B";
        const char* topRightCorner            = "┐";//"\e(0\x6b\e(B";
        const char* bottomRightCorner         = "┘";//"\e(0\x6a\e(B";
        const char* verticalLine               = "│";//"\e(0\x78\e(B";
        const char* horizontalLine             = "─";//"\e(0\x71\e(B";
        const char* verticalRightConnector    = "├";//"\e(0\x74\e(B";
        const char* verticalLeftConnector     = "┤";//"\e(0\x75\e(B";
        const char* horizontalBottomConnector = "┬";//"\e(0\x76\e(B";
        const char* horizontalTopConnector    = "┴";//"\e(0\x77\e(B";
        const char* crossConnector             = "┼";//"\e(0\x6e\e(B";
    
        /**
         * @brief A structure to hold the border style of a widget.
         *
         * The style is represented as a vector of strings, each string being a character
         * that will be used to draw the border of the widget. The vector must have the
         * following size and order:
         * - 0: Top left corner
         * - 1: Bottom left corner
         * - 2: Top right corner
         * - 3: Bottom right corner
         * - 4: Vertical line
         * - 5: Horizontal line
         * - 6: Vertical right connector
         * - 7: Vertical left connector
         * - 8: Horizontal bottom connector
         * - 9: Horizontal top connector
         * - 10: Cross connector
         */
        constexpr styledBorder(const std::array<const char*, 11> values, const VALUE_STATE Default = VALUE_STATE::VALUE) : styleBase(Default){
            topLeftCorner = values[0];
            bottomLeftCorner = values[1];
            topRightCorner = values[2];
            bottomRightCorner = values[3];
            verticalLine = values[4];
            horizontalLine = values[5];
            verticalRightConnector = values[6];
            verticalLeftConnector = values[7];
            horizontalBottomConnector = values[8];
            horizontalTopConnector = values[9];
            crossConnector = values[10];
        }

        constexpr styledBorder() = default;

        inline ~styledBorder() override { styleBase::~styleBase(); }

        inline styleBase* copy() const override {
            return new styledBorder(*this);
        }

        constexpr styledBorder& operator=(const styledBorder& other){
            if (other.status >= status){
                topLeftCorner = other.topLeftCorner;
                bottomLeftCorner = other.bottomLeftCorner;
                topRightCorner = other.topRightCorner;
                bottomRightCorner = other.bottomRightCorner;
                verticalLine = other.verticalLine;
                horizontalLine = other.horizontalLine;
                verticalRightConnector = other.verticalRightConnector;
                verticalLeftConnector = other.verticalLeftConnector;
                horizontalBottomConnector = other.horizontalBottomConnector;
                horizontalTopConnector = other.horizontalTopConnector;
                crossConnector = other.crossConnector;

                status = other.status;
            }
            return *this;
        }
    
        constexpr styledBorder(const GGUI::styledBorder& other) : styleBase(other.status){
            topLeftCorner = other.topLeftCorner;
            bottomLeftCorner = other.bottomLeftCorner;
            topRightCorner = other.topRightCorner;
            bottomRightCorner = other.bottomRightCorner;
            verticalLine = other.verticalLine;
            horizontalLine = other.horizontalLine;
            verticalRightConnector = other.verticalRightConnector;
            verticalLeftConnector = other.verticalLeftConnector;
            horizontalBottomConnector = other.horizontalBottomConnector;
            horizontalTopConnector = other.horizontalTopConnector;
            crossConnector = other.crossConnector;
        }

        inline void evaluate([[maybe_unused]] const styling* self, [[maybe_unused]] const styling* owner) override {};
        
        INTERNAL::STAIN_TYPE embedValue(styling* host, element* owner) override;

        const char* getBorder(const INTERNAL::borderConnection flags);

        INTERNAL::borderConnection getBorderType(const char* border);
    };

    class flowPriority : public STYLING_INTERNAL::enumValue<DIRECTION>{
    public:
        constexpr flowPriority(const DIRECTION Value, const VALUE_STATE Default = VALUE_STATE::VALUE) : enumValue(Value, Default){}

        inline ~flowPriority() override { enumValue<DIRECTION>::~enumValue<DIRECTION>(); }

        inline styleBase* copy() const override {
            return new flowPriority(*this);
        }

        constexpr flowPriority(const GGUI::flowPriority& other) : enumValue(other.value, other.status){}

        constexpr flowPriority& operator=(const flowPriority& other) = default;
        
        // for dynamically computable values like percentage depended
        // currently covers:
        // - screen space
        inline void evaluate([[maybe_unused]] const styling* self, [[maybe_unused]] const styling* owner) override {};

        INTERNAL::STAIN_TYPE embedValue(styling* host, element* owner) override;
    };

    class wrap : public STYLING_INTERNAL::boolValue{
    public:
        constexpr wrap(const bool Value, const VALUE_STATE Default = VALUE_STATE::VALUE) : boolValue(Value, Default){}

        inline ~wrap() override { boolValue::~boolValue(); }

        inline styleBase* copy() const override {
            return new wrap(*this);
        }

        constexpr wrap(const GGUI::wrap& other) : boolValue(other.value, other.status){}

        constexpr wrap& operator=(const wrap& other) = default;
        
        // for dynamically computable values like percentage depended
        // currently covers:
        // - screen space
        inline void evaluate([[maybe_unused]] const styling* self, [[maybe_unused]] const styling* owner) override {};
        
        INTERNAL::STAIN_TYPE embedValue(styling* host, element* owner) override;
    };

    class allowOverflow : public STYLING_INTERNAL::boolValue{
    public:
        constexpr allowOverflow(const bool Value, const VALUE_STATE Default = VALUE_STATE::VALUE) : boolValue(Value, Default){}

        inline ~allowOverflow() override { boolValue::~boolValue(); }

        inline styleBase* copy() const override {
            return new allowOverflow(*this);
        }

        constexpr allowOverflow(const GGUI::allowOverflow& other) : boolValue(other.value, other.status){}

        constexpr allowOverflow& operator=(const allowOverflow& other) = default;
        
        // for dynamically computable values like percentage depended
        // currently covers:
        // - screen space
        inline void evaluate([[maybe_unused]] const styling* self, [[maybe_unused]] const styling* owner) override {};
        
        INTERNAL::STAIN_TYPE embedValue(styling* host, element* owner) override;
    };

    class allowDynamicSize : public STYLING_INTERNAL::boolValue{
    public:
        constexpr allowDynamicSize(const bool Value, const VALUE_STATE Default = VALUE_STATE::VALUE) : boolValue(Value, Default){}

        inline ~allowDynamicSize() override { boolValue::~boolValue(); }

        inline styleBase* copy() const override {
            return new allowDynamicSize(*this);
        }

        constexpr allowDynamicSize(const GGUI::allowDynamicSize& other) : boolValue(other.value, other.status){}

        constexpr allowDynamicSize& operator=(const allowDynamicSize& other) = default;
        
        // for dynamically computable values like percentage depended
        // currently covers:
        // - screen space
        inline void evaluate([[maybe_unused]] const styling* self, [[maybe_unused]] const styling* owner) override {};
        
        INTERNAL::STAIN_TYPE embedValue(styling* host, element* owner) override;
    };

    class margin : public STYLING_INTERNAL::styleBase{
    public:
        STYLING_INTERNAL::value<unsigned int> Top = (unsigned)0;
        STYLING_INTERNAL::value<unsigned int> Bottom = (unsigned)0;
        STYLING_INTERNAL::value<unsigned int> Left = (unsigned)0;
        STYLING_INTERNAL::value<unsigned int> Right = (unsigned)0;

        constexpr margin(const unsigned int top, const unsigned int bottom, const unsigned int left, const unsigned int right, const VALUE_STATE Default = VALUE_STATE::VALUE) : styleBase(Default), Top(top), Bottom(bottom), Left(left), Right(right){}

        constexpr margin() = default;

        inline ~margin() override { styleBase::~styleBase(); }

        inline styleBase* copy() const override {
            return new margin(*this);
        }

        // operator overload for copy operator
        constexpr margin& operator=(const margin& other){
            // Only copy the information if the other is enabled.
            if (other.status >= status){
                Top = other.Top;
                Bottom = other.Bottom;
                Left = other.Left;
                Right = other.Right;

                status = other.status;
            }
            return *this;
        }

        constexpr margin(const GGUI::margin& other) : styleBase(other.status), Top(other.Top), Bottom(other.Bottom), Left(other.Left), Right(other.Right){}
        
        // for dynamically computable values like percentage depended
        // currently covers:
        // - screen space
        void evaluate(const styling* self, const styling* owner) override;
        
        INTERNAL::STAIN_TYPE embedValue(styling* host, element* owner) override;
    };

    class opacity : public STYLING_INTERNAL::styleBase{
    protected:
        float Value = 1.0f;
    public:
        constexpr opacity(const float value, const VALUE_STATE state = VALUE_STATE::VALUE) : styleBase(state), Value(value){}

        inline ~opacity() override { styleBase::~styleBase(); }

        inline styleBase* copy() const override {
            return new opacity(*this);
        }

        constexpr opacity& operator=(const opacity& other){
            // Only copy the information if the other is enabled.
            if (other.status >= status){
                Value = other.Value;

                status = other.status;
            }
            return *this;
        }

        constexpr opacity(const GGUI::opacity& other) : styleBase(other.status), Value(other.Value){}

        // Since opacity always represents an percentile of its self being displayed on top of its parent.
        inline void evaluate([[maybe_unused]] const styling* self, [[maybe_unused]] const styling* owner) override {};
        
        INTERNAL::STAIN_TYPE embedValue(styling* host, element* owner) override;

        constexpr float Get() const { return Value; }

        constexpr void Set(const float value){
            Value = value;
            status = VALUE_STATE::VALUE;
        }
    };

    class allowScrolling : public STYLING_INTERNAL::boolValue{
    public:
        constexpr allowScrolling(const bool Value, const VALUE_STATE Default = VALUE_STATE::VALUE) : boolValue(Value, Default){}

        inline ~allowScrolling() override { boolValue::~boolValue(); }

        inline styleBase* copy() const override {
            return new allowScrolling(*this);
        }

        constexpr allowScrolling(const GGUI::allowScrolling& other) : boolValue(other.value, other.status){}

        constexpr allowScrolling& operator=(const allowScrolling& other) = default;
        
        // for dynamically computable values like percentage depended
        // currently covers:
        // - screen space
        inline void evaluate([[maybe_unused]] const styling* self, [[maybe_unused]] const styling* owner) override {};
        
        INTERNAL::STAIN_TYPE embedValue(styling* host, element* owner) override;
    };

    class anchor : public STYLING_INTERNAL::enumValue<ANCHOR>{
    public:
        constexpr anchor(const ANCHOR Value, const VALUE_STATE Default = VALUE_STATE::VALUE) : enumValue(Value, Default){}

        inline ~anchor() override { enumValue<ANCHOR>::~enumValue<ANCHOR>(); }

        inline styleBase* copy() const override {
            return new anchor(*this);
        }

        constexpr anchor(const GGUI::anchor& other) : enumValue(other.value, other.status){}

        constexpr anchor& operator=(const anchor& other) = default;
        
        // for dynamically computable values like percentage depended
        // currently covers:
        // - screen space
        inline void evaluate([[maybe_unused]] const styling* self, [[maybe_unused]] const styling* owner) override {};
        
        INTERNAL::STAIN_TYPE embedValue(styling* host, element* owner) override;
    };

    class node : public STYLING_INTERNAL::styleBase{
    public:
        element* Value;

        constexpr node(element* value = nullptr, const VALUE_STATE Default = VALUE_STATE::VALUE) : styleBase(Default, INTERNAL::EMBED_ORDER::DELAYED), Value(value){}
        
        constexpr node(const GGUI::node& other) : styleBase(other.status, INTERNAL::EMBED_ORDER::DELAYED), Value(other.Value){}

        inline ~node() override { styleBase::~styleBase(); }

        inline styleBase* copy() const override;

        constexpr node& operator=(const node& other){
            // Only copy the information if the other is enabled.
            if (other.status >= status){
                Value = other.Value;

                status = other.status;

                order = other.order;
            }
            return *this;
        }

        inline void evaluate([[maybe_unused]] const styling* self, [[maybe_unused]] const styling* owner) override {};

        INTERNAL::STAIN_TYPE embedValue(styling* host, element* owner) override;
    };

    class childs : public STYLING_INTERNAL::styleBase{
    protected:
        std::array<element*, INT8_MAX> Value;
    public:
        constexpr childs(const std::initializer_list<element*> value, const VALUE_STATE Default = VALUE_STATE::VALUE) : styleBase(Default, INTERNAL::EMBED_ORDER::DELAYED), Value{}{
            assert(value.size() <= INT8_MAX);
            for (size_t i = 0; i < value.size(); i++){
                Value[i] = *(value.begin() + i);
            }
        }

        constexpr childs(const GGUI::childs& other) : styleBase(other.status, INTERNAL::EMBED_ORDER::DELAYED), Value(other.Value){}

        inline ~childs() override { styleBase::~styleBase(); }

        inline styleBase* copy() const override;

        constexpr childs& operator=(const childs& other){
            // Only copy the information if the other is enabled.
            if (other.status >= status){
                Value = other.Value;

                status = other.status;

                order = other.order;
            }
            return *this;
        }

        inline void evaluate([[maybe_unused]] const styling* self, [[maybe_unused]] const styling* owner) override {};

        INTERNAL::STAIN_TYPE embedValue(styling* host, element* owner) override;

        // -----< UTILS >-----

        // iterator fetcher to skip the nullptr tail of the Value
        inline std::array<element*, INT8_MAX>::const_iterator begin() const {
            return Value.cbegin();
        }

        // iterator fetcher to skip the nullptr tail of the Value
        inline std::array<element*, INT8_MAX>::const_iterator end() const {
            return std::find(Value.cbegin(), Value.cend(), nullptr);
        }

        inline int length() const {
            return std::distance(begin(), end());
        }
    };

    class onInit : public STYLING_INTERNAL::styleBase{
    public:
        void (*Value)(element* self);

        constexpr onInit(void (*value)(element* self), const VALUE_STATE Default = VALUE_STATE::VALUE) : styleBase(Default), Value(value){}

        constexpr onInit(const GGUI::onInit& other) : styleBase(other.status), Value(other.Value){}

        inline ~onInit() override { styleBase::~styleBase(); }

        inline styleBase* copy() const override {
            return new onInit(*this);
        }

        constexpr onInit& operator=(const onInit& other){
            // Only copy the information if the other is enabled.
            if (other.status >= status){
                Value = other.Value;

                status = other.status;
            }
            return *this;
        }

        inline void evaluate([[maybe_unused]] const styling* self, [[maybe_unused]] const styling* owner) override {};

        INTERNAL::STAIN_TYPE embedValue(styling* host, element* owner) override;
    };

    class onDestroy : public STYLING_INTERNAL::styleBase{
    public:
        void (*Value)(element* self);

        constexpr onDestroy(void (*value)(element* self), const VALUE_STATE Default = VALUE_STATE::VALUE) : styleBase(Default), Value(value){}

        constexpr onDestroy(const GGUI::onDestroy& other) : styleBase(other.status), Value(other.Value){}

        inline ~onDestroy() override { styleBase::~styleBase(); }

        inline styleBase* copy() const override {
            return new onDestroy(*this);
        }

        constexpr onDestroy& operator=(const onDestroy& other){
            // Only copy the information if the other is enabled.
            if (other.status >= status){
                Value = other.Value;

                status = other.status;
            }
            return *this;
        }

        inline void evaluate([[maybe_unused]] const styling* self, [[maybe_unused]] const styling* owner) override {};

        INTERNAL::STAIN_TYPE embedValue(styling* host, element* owner) override;
    };

    class onHide : public STYLING_INTERNAL::styleBase{
    public:
        void (*Value)(element* self);

        constexpr onHide(void (*value)(element* self), const VALUE_STATE Default = VALUE_STATE::VALUE) : styleBase(Default), Value(value){}

        constexpr onHide(const GGUI::onHide& other) : styleBase(other.status), Value(other.Value){}

        inline ~onHide() override { styleBase::~styleBase(); }

        inline styleBase* copy() const override {
            return new onHide(*this);
        }

        constexpr onHide& operator=(const onHide& other){
            // Only copy the information if the other is enabled.
            if (other.status >= status){
                Value = other.Value;

                status = other.status;
            }
            return *this;
        }

        inline void evaluate([[maybe_unused]] const styling* self, [[maybe_unused]] const styling* owner) override {};

        INTERNAL::STAIN_TYPE embedValue(styling* host, element* owner) override;
    };

    class onShow : public STYLING_INTERNAL::styleBase{
    public:
        void (*Value)(element* self);

        constexpr onShow(void (*value)(element* self), const VALUE_STATE Default = VALUE_STATE::VALUE) : styleBase(Default), Value(value){}

        constexpr onShow(const GGUI::onShow& other) : styleBase(other.status), Value(other.Value){}

        inline ~onShow() override { styleBase::~styleBase(); }

        inline styleBase* copy() const override {
            return new onShow(*this);
        }

        constexpr onShow& operator=(const onShow& other){
            // Only copy the information if the other is enabled.
            if (other.status >= status){
                Value = other.Value;

                status = other.status;
            }
            return *this;
        }

        inline void evaluate([[maybe_unused]] const styling* self, [[maybe_unused]] const styling* owner) override {};

        INTERNAL::STAIN_TYPE embedValue(styling* host, element* owner) override;
    };

    class name : public STYLING_INTERNAL::styleBase{
    public:
        INTERNAL::compactString Value;

        constexpr name(INTERNAL::compactString value, const VALUE_STATE Default = VALUE_STATE::VALUE) : styleBase(Default), Value(value){}

        constexpr name(const GGUI::name& other) : styleBase(other.status), Value(other.Value){}

        inline ~name() override { styleBase::~styleBase(); }

        inline styleBase* copy() const override {
            return new name(*this);
        }

        constexpr name& operator=(const name& other){
            // Only copy the information if the other is enabled.
            if (other.status >= status){
                Value = other.Value;

                status = other.status;
            }
            return *this;
        }

        inline void evaluate([[maybe_unused]] const styling* self, [[maybe_unused]] const styling* owner) override {};

        INTERNAL::STAIN_TYPE embedValue(styling* host, element* owner) override;
    };

    class title : public name{
    public:
        constexpr title(const INTERNAL::compactString&& value, const VALUE_STATE Default = VALUE_STATE::VALUE) : name(value, Default){}

        inline ~title() override { styleBase::~styleBase(); }

        inline styleBase* copy() const override {
            return new title(*this);
        }

        constexpr title(const GGUI::title& other) : name(other.Value, other.status){}

        constexpr title& operator=(const title& other){
            // Only copy the information if the other is enabled.
            if (other.status >= status){
                Value = other.Value;

                status = other.status;
            }
            return *this;
        }

        INTERNAL::STAIN_TYPE embedValue(styling* host, element* owner) override;

        constexpr bool empty(){
            return Value.empty();
        }
    };

    class display : public STYLING_INTERNAL::boolValue{
    public:
        constexpr display(const bool Value, const VALUE_STATE Default = VALUE_STATE::VALUE) : boolValue(Value, Default){}

        inline ~display() override { boolValue::~boolValue(); }

        inline styleBase* copy() const override {
            return new display(*this);
        }

        constexpr display(const GGUI::display& other) : boolValue(other.value, other.status){}

        constexpr display& operator=(const display& other){
            // Only copy the information if the other is enabled.
            if (other.status >= status){
                value = other.value;

                status = other.status;
            }
            return *this;
        }

        inline void evaluate([[maybe_unused]] const styling* self, [[maybe_unused]] const styling* owner) override {};

        INTERNAL::STAIN_TYPE embedValue(styling* host, element* owner) override;
    };

    class sprite;
    class onDraw : public STYLING_INTERNAL::styleBase{
    public:
        GGUI::sprite (*Value)(unsigned int x, unsigned int y);

        constexpr onDraw(GGUI::sprite (*value)(unsigned int x, unsigned int y), const VALUE_STATE Default = VALUE_STATE::VALUE) : styleBase(Default), Value(value){}
        
        constexpr onDraw(const GGUI::onDraw& other) : styleBase(other.status), Value(other.Value){}

        inline ~onDraw() override { styleBase::~styleBase(); }

        inline styleBase* copy() const override {
            return new onDraw(*this);
        }

        constexpr onDraw& operator=(const onDraw& other){
            // Only copy the information if the other is enabled.
            if (other.status >= status){
                Value = other.Value;

                status = other.status;
            }
            return *this;
        }

        inline void evaluate([[maybe_unused]] const styling* self, [[maybe_unused]] const styling* owner) override {};

        INTERNAL::STAIN_TYPE embedValue(styling* host, element* owner) override;
    };

    class text : public STYLING_INTERNAL::styleBase{
    public:
        const char* Value;

        constexpr text(const char* value, const VALUE_STATE Default = VALUE_STATE::VALUE) : styleBase(Default), Value(value){}

        constexpr text(const GGUI::text& other) : styleBase(other.status), Value(other.Value){}

        inline ~text() override { styleBase::~styleBase(); }

        inline styleBase* copy() const override {
            return new text(*this);
        }

        constexpr text& operator=(const text& other){
            // Only copy the information if the other is enabled.
            if (other.status >= status){
                Value = other.Value;

                status = other.status;
            }
            return *this;
        }

        inline void evaluate([[maybe_unused]] const styling* self, [[maybe_unused]] const styling* owner) override {};

        INTERNAL::STAIN_TYPE embedValue(styling* host, element* owner) override;
    };

    class onClick : public STYLING_INTERNAL::styleBase{
    public:
        bool (*Value)(element* self);

        constexpr onClick(bool (*value)(element* self), const VALUE_STATE Default = VALUE_STATE::VALUE) : styleBase(Default, INTERNAL::EMBED_ORDER::DELAYED), Value(value){}

        constexpr onClick(const GGUI::onClick& other) : styleBase(other.status, INTERNAL::EMBED_ORDER::DELAYED), Value(other.Value){}

        inline ~onClick() override { styleBase::~styleBase(); }

        inline styleBase* copy() const override {
            return new onClick(*this);
        }

        constexpr onClick& operator=(const onClick& other){
            // Only copy the information if the other is enabled.
            if (other.status >= status){
                Value = other.Value;

                status = other.status;
                
                order = other.order;
            }
            return *this;
        }

        inline void evaluate([[maybe_unused]] const styling* self, [[maybe_unused]] const styling* owner) override {};

        INTERNAL::STAIN_TYPE embedValue(styling* host, element* owner) override;
    };

    class onInput : public STYLING_INTERNAL::styleBase{
    public:
        void (*Value)(textField* self, char c);

        constexpr onInput(void (*value)(textField* self, char c), const VALUE_STATE Default = VALUE_STATE::VALUE) : styleBase(Default), Value(value){}

        constexpr onInput(const GGUI::onInput& other) : styleBase(other.status), Value(other.Value){}

        inline ~onInput() override { styleBase::~styleBase(); }

        inline styleBase* copy() const override {
            return new onInput(*this);
        }

        constexpr onInput& operator=(const onInput& other){
            // Only copy the information if the other is enabled.
            if (other.status >= status){
                Value = other.Value;

                status = other.status;
            }
            return *this;
        }

        inline void evaluate([[maybe_unused]] const styling* self, [[maybe_unused]] const styling* owner) override {};

        INTERNAL::STAIN_TYPE embedValue(styling* host, element* owner) override;
    };

    class styling{
    public:
        position                      Position                        = position(IVector3(0, 0, 0), VALUE_STATE::INITIALIZED);

        width                         Width                           = width(1, VALUE_STATE::INITIALIZED);
        height                        Height                          = height(1, VALUE_STATE::INITIALIZED);

        title                         Title                           = title(INTERNAL::compactString(nullptr, 0, true), VALUE_STATE::INITIALIZED);

        enableBorder                  Border_Enabled                  = enableBorder(false, VALUE_STATE::INITIALIZED);
        textColor                     Text_Color                      = textColor(COLOR::WHITE, VALUE_STATE::INITIALIZED);
        backgroundColor               Background_Color                = backgroundColor(COLOR::BLACK, VALUE_STATE::INITIALIZED);
        borderColor                   Border_Color                    = borderColor(COLOR::WHITE, VALUE_STATE::INITIALIZED);
        borderBackgroundColor         Border_Background_Color         = borderBackgroundColor(COLOR::BLACK, VALUE_STATE::INITIALIZED);
        
        hoverBorderColor              Hover_Border_Color              = hoverBorderColor(COLOR::WHITE, VALUE_STATE::INITIALIZED);
        hoverTextColor                Hover_Text_Color                = hoverTextColor(COLOR::WHITE, VALUE_STATE::INITIALIZED);
        hoverBackgroundColor          Hover_Background_Color          = hoverBackgroundColor(COLOR::DARK_GRAY, VALUE_STATE::INITIALIZED);
        hoverBorderBackgroundColor    Hover_Border_Background_Color   = hoverBorderBackgroundColor(COLOR::BLACK, VALUE_STATE::INITIALIZED);

        focusBorderColor              Focus_Border_Color              = focusBorderColor(COLOR::WHITE, VALUE_STATE::INITIALIZED);
        focusTextColor                Focus_Text_Color                = focusTextColor(COLOR::BLACK, VALUE_STATE::INITIALIZED);
        focusBackgroundColor          Focus_Background_Color          = focusBackgroundColor(COLOR::WHITE, VALUE_STATE::INITIALIZED);
        focusBorderBackgroundColor    Focus_Border_Background_Color   = focusBorderBackgroundColor(COLOR::BLACK, VALUE_STATE::INITIALIZED);

        styledBorder                  Border_Style;
        
        flowPriority                  Flow_Priority                   = flowPriority(DIRECTION::ROW, VALUE_STATE::INITIALIZED);
        wrap                          Wrap                            = wrap(false, VALUE_STATE::INITIALIZED);

        allowOverflow                 Allow_Overflow                  = allowOverflow(false, VALUE_STATE::INITIALIZED);
        allowDynamicSize              Allow_Dynamic_Size              = allowDynamicSize(false, VALUE_STATE::INITIALIZED);
        margin                        Margin;

        opacity                       Opacity                         = opacity(1.0f, VALUE_STATE::INITIALIZED);  // 100%

        allowScrolling                Allow_Scrolling                 = allowScrolling(false, VALUE_STATE::INITIALIZED);

        anchor                        Align                           = anchor(ANCHOR::LEFT, VALUE_STATE::INITIALIZED);

        std::vector<element*>         Childs;

        /**
         * @brief Default constructor for the Styling class.
         * 
         * This constructor initializes a new instance of the Styling class with default values.
         */
        inline styling() = default;

        /**
         * @brief Constructs a Styling object with the given style attributes.
         * 
         * @param attributes A pointer to a style_base object containing the style attributes.
         */
        styling(STYLING_INTERNAL::styleBase* attributes){
            unParsedStyles = attributes;
        }

        /**
         * @brief Constructs a Styling object with the given style attributes.
         * 
         * @param attributes A reference to a style_base object containing the style attributes.
         */
        styling(STYLING_INTERNAL::styleBase& attributes){
            unParsedStyles = &attributes;
        }

        /**
         * @brief Constructs a Styling object with the given style attributes.
         * 
         * @param attributes A style_base object containing the style attributes.
         */
        styling(STYLING_INTERNAL::styleBase&& attributes){
            unParsedStyles = &attributes;
        }

        /**
         * @brief Associates the given style attributes with the current object.
         *
         * This function sets the internal pointer to the provided style_base object,
         * allowing the object to use or reference the specified styling attributes.
         *
         * @param attributes Reference to a style_base object containing the style attributes to be added.
         */
        void add(STYLING_INTERNAL::styleBase& attributes){
            unParsedStyles = &attributes;
        }

        /**
         * @brief Adds a new style by moving the given style attributes.
         *
         * Stores a pointer to the provided style attributes, which are passed as an rvalue reference.
         * Note: The lifetime of the passed attributes must outlive the usage of un_parsed_styles.
         *
         * @param attributes Rvalue reference to a style_base object containing style attributes.
         */
        void add(STYLING_INTERNAL::styleBase&& attributes){
            unParsedStyles = &attributes;
        }

        /**
         * Embeds the styles of the current styling object into the element.
         * 
         * This function is used to embed the styles of the current styling object into the element.
         * It takes the element as a parameter and embeds the styles into it.
         * The styles are embedded by looping through the un_parsed_styles list and calling the Embed_Value function on each attribute.
         * The Embed_Value function is responsible for embedding the attribute into the element.
         * The changes to the element are recorded in the changes variable, which is of type STAIN.
         * The type of the changes is then added to the element's stains list.
         * The function returns nothing.
         * @param owner The element to which the styles will be embedded.
         */
        void embedStyles(element* owner);

        /**
         * @brief Copies the values of the given Styling object to the current object.
         *
         * This will copy all the values of the given Styling object to the current object.
         *
         * @param other The Styling object to copy from.
         */
        void copy(const styling& other);

        /**
         * @brief Copies the styling information from another Styling object.
         * 
         * This function copies the styling information from the provided 
         * Styling object pointer. It internally calls the overloaded Copy 
         * function that takes a reference to a Styling object.
         * 
         * @param other A pointer to the Styling object from which to copy the styling information.
         */
        void copy(const styling* other){
            // use the reference one
            copy(*other);
        }

        void copyUnParsedStyles();
        
        void copyUnParsedStyles(const styling* other);

        // Returns the pointer of whom this evaluation is to reference.
        styling* getReference(element* owner);

        /**
         * @brief Evaluates all dynamic attribute values for the owner element.
         *
         * This function evaluates the dynamic attribute values of the styling associated
         * with the specified element. It determines the point of interest, which is
         * either the element's parent or the element itself if no parent exists,
         * and uses its style as a reference for evaluation.
         *
         * @param owner The element whose dynamic attributes are to be evaluated.
         * @return True if there wae changes in the attributes evaluated, false otherwise.
         */
        bool evaluateDynamicAttributeValues(element* owner);

        bool evaluateDynamicPosition(element* owner, styling* reference = nullptr);

        bool evaluateDynamicDimensions(element* owner, styling* reference = nullptr);

        bool evaluateDynamicBorder(element* owner, styling* reference = nullptr);
    
        bool evaluateDynamicColors(element* owner, styling* reference = nullptr);
    protected:
    
        // The construction time given styles are first put here, before embedding them into this class.
        STYLING_INTERNAL::styleBase* unParsedStyles = nullptr;
    };

    namespace STYLES{
        namespace BORDER{
            inline styledBorder Double = std::array<const char*, 11>{
                "╔", "╚", "╗", "╝", "║", "═", "╠", "╣", "╦", "╩", "╬"
            };

            inline styledBorder Round = std::array<const char*, 11>{
                "╭", "╰", "╮", "╯", "│", "─", "├", "┤", "┬", "┴", "┼"
            };

            inline styledBorder Single = std::array<const char*, 11>{
                "┌", "└", "┐", "┘", "│", "─", "├", "┤", "┬", "┴", "┼"
            };

            inline styledBorder Bold = std::array<const char*, 11>{
                "▛", "▙", "▜", "▟", "█", "▅", "▉", "▉", "▉", "▉", "▉"
            };

            inline styledBorder Modern = std::array<const char*, 11>{
                "/", "\\", "\\", "/", "|", "-", "|", "|", "-", "-", "+"
            };
        }

        namespace CONSTANTS{
            // inline styling Default;
            inline STYLING_INTERNAL::empty Default;
            inline styling defaultStyling;
        }

        // CAUTION!: These anchoring vector presets, are made to work where the origin is at the center (0, 0).
        inline GGUI::STYLING_INTERNAL::vectorValue left = GGUI::STYLING_INTERNAL::vectorValue(-0.5f, 0.0f);
        // CAUTION!: These anchoring vector presets, are made to work where the origin is at the center (0, 0).
        inline GGUI::STYLING_INTERNAL::vectorValue top = GGUI::STYLING_INTERNAL::vectorValue(0.0f, -0.5f);
        // CAUTION!: These anchoring vector presets, are made to work where the origin is at the center (0, 0).
        inline GGUI::STYLING_INTERNAL::vectorValue right = GGUI::STYLING_INTERNAL::vectorValue(0.5f, 0.0f);
        // CAUTION!: These anchoring vector presets, are made to work where the origin is at the center (0, 0).
        inline GGUI::STYLING_INTERNAL::vectorValue bottom = GGUI::STYLING_INTERNAL::vectorValue(0.0f, 0.5f);
        // CAUTION!: These anchoring vector presets, are made to work where the origin is at the center (0, 0).
        inline GGUI::STYLING_INTERNAL::vectorValue center = GGUI::STYLING_INTERNAL::vectorValue(0.0f, 0.0f);
        // CAUTION!: These anchoring vector presets, are made to work where the origin is at the center (0, 0).
        inline GGUI::STYLING_INTERNAL::vectorValue prioritize = GGUI::STYLING_INTERNAL::vectorValue(0.0f, 0.0f, std::numeric_limits<short>::max());
    }

}

#endif
#ifndef _SETTINGS_H_
#define _SETTINGS_H_

#include <chrono>
#include <string>
#include <vector>
#include <functional>

namespace GGUI {
    namespace INTERNAL {
        extern std::string constructLoggerFileName();
    }

    namespace SETTINGS{
        /**
         * @brief Enumeration of supported argument types for command line parsing.
         */
        enum class ArgumentType {
            FLAG,           ///< Boolean flag argument (no value expected)
            STRING,         ///< String value argument
            INTEGER,        ///< Integer value argument
            UNSIGNED_LONG   ///< Unsigned long value argument
        };

        /**
         * @brief Descriptor class for command line arguments.
         * 
         * This class encapsulates information about a command line argument including
         * its name, type, description, and a callback function to handle the parsed value.
         */
        class ArgumentDescriptor {
        public:
            std::string name;                                    ///< Argument name (without dashes)
            ArgumentType type;                                   ///< Type of argument (flag, string, integer, etc.)
            std::string description;                             ///< Human-readable description for help text
            std::function<void(const std::string&)> handler;    ///< Callback function to handle parsed value

            /**
             * @brief Constructs an ArgumentDescriptor.
             * 
             * @param argName The name of the argument (without leading dashes)
             * @param argType The type of the argument
             * @param argDescription Description of the argument for help text
             * @param argHandler Callback function to handle the parsed value
             */
            ArgumentDescriptor(const std::string& argName, 
                             ArgumentType argType, 
                             const std::string& argDescription,
                             std::function<void(const std::string&)> argHandler)
                : name(argName), type(argType), description(argDescription), handler(argHandler) {}

            /**
             * @brief Checks if this argument requires a value.
             * 
             * @return true if the argument type requires a value, false for flags
             */
            bool requiresValue() const {
                return type != ArgumentType::FLAG;
            }

            /**
             * @brief Gets the type name as a string for help text.
             * 
             * @return String representation of the argument type
             */
            std::string getTypeName() const {
                switch (type) {
                    case ArgumentType::FLAG: return "flag";
                    case ArgumentType::STRING: return "string";
                    case ArgumentType::INTEGER: return "integer";
                    case ArgumentType::UNSIGNED_LONG: return "unsigned long";
                    default: return "unknown";
                }
            }
        };

        // Given as --mousePressCooldown = 123
        extern unsigned long long Mouse_Press_Down_Cooldown;  // Milliseconds

        // Given as --enableWordWrapping
        extern bool Word_Wrapping;

        // Given as --enableGammaCorrection
        extern bool ENABLE_GAMMA_CORRECTION;

        namespace LOGGER{
            // Given as --loggerFileName = "GGUI.log"
            extern std::string File_Name;
        }

        // Given as --enableDRM
        extern bool enableDRM;

        extern void parseCommandLineArguments(int argc, char** argv);

        /**
         * @brief Initializes the settings for the application.
         *
         * This function sets up the necessary configurations for the application
         * by initializing the logger file name using the internal logger file name
         * construction method.
         */
        extern void initSettings();
    }
}

#endif
#ifndef _UTILS_H_
#define _UTILS_H_
/**
 * This is an Utils file made for the Renderer.cpp to use internally, these are just removed to clean up the source code.
 */


#include <math.h>
#include <cstring>
#include <algorithm> // std::clamp
#include <cmath>     // std::pow, std::lround

namespace GGUI{
    class element;
    class UTF;
    class RGB;


    extern std::string toString(UTF coloredText, bool transparentBackground = false);
}

#endif
#ifndef _LOGGER_H_
#define _LOGGER_H_

#include <queue>


namespace GGUI{

    class fileStream;

    
    /**
     * @brief Reports an error to the user.
     * @param Problem The error message to display.
     * @note If the main window is not created yet, the error will be printed to the console.
     * @note This function is thread safe.
     */
    extern void report(const std::string& problem);
}

#endif
#ifndef _FILE_STREAMER_H_
#define _FILE_STREAMER_H_


#include <fstream>
#include <functional>
#include <unordered_map>
#include <memory>
#include <stdio.h>
#include <deque>
#include <fcntl.h>

#if _WIN32
    #include <io.h>
#else
    #include <unistd.h>
#endif

namespace GGUI{

    /*
        Utilities to manage file streams.
    */

    class fileStream;

    // Managed file stream handles. Ownership is now RAII via unique_ptr.
    extern std::unordered_map<std::string, std::unique_ptr<fileStream>> File_Streamer_Handles;

    /**
     * @brief Returns the current working directory of the program.
     * @return The current working directory as a string.
     *
     * This function returns the current working directory of the program
     * as a string. This is useful for finding the location of configuration
     * files, data files, and other resources that need to be accessed from
     * the program.
     */
    extern std::string getCurrentLocation();
 
    /**
     * @brief Pulls the current contents of STDIN as a string.
     * @return The contents of STDIN as a string.
     *
     * This function pulls the current contents of STDIN as a string. This is
     * useful for applications that need to access the output of a previous
     * process. Note that this function will only work if the application was
     * started as a non TTY enabled application. If the application was started
     * as a TTY enabled application, this function will fail.
     */
    extern std::string pullSTDIN();
    
    /**
     * @brief Checks if the application was started from a TTY
     * @return True if the application was started from a TTY, false otherwise
     *
     * This function checks if the application was started from a TTY
     * by checking if STDIN is a TTY.
     */
    extern bool hasStartedAsTTY();

    namespace INTERNAL {
        class bufferCapture;
    }


    enum class FILE_STREAM_TYPE{
        UN_INITIALIZED  = 0 << 0,
        READ            = 1 << 0,
        WRITE           = 1 << 1,
        STD_CAPTURE     = 1 << 2
    };

    class fileStream{
    private:
        INTERNAL::bufferCapture* Buffer_Capture = nullptr;
        std::fstream Handle;
        std::vector<std::function<void()>> On_Change = {};
        std::string Previous_Content = "";
        unsigned long long Previous_Hash = 0;
        FILE_STREAM_TYPE Type = FILE_STREAM_TYPE::UN_INITIALIZED;
    public:
        std::string Name = "";

        /**
         * @brief Constructor of the FILE_STREAM class.
         * @param file_name The name of the file to open.
         * @param on_change The event handler to be called when the file is changed.
         * @param type describes the pipe method, READ/WRITE/STD_CAPTURE
         * @param atomic If set will not add the file stream to the amassed file stream handle service.
         *
         * If read_from_std_cout is true, a new file is created where the std::cout is piped into.
         * If there is already a file handle for this file name, the event handler is added to the list of event
         * handlers for that file. If not, a new file handle is created and the event handler is added to the list
         * of event handlers for the new file.
         */
        fileStream(std::string file_name, std::function<void()> on_change = [](){}, FILE_STREAM_TYPE type = FILE_STREAM_TYPE::READ);

        /**
         * @brief Intended for Logger Atomic::Guard, do not use as User!
         */
        fileStream() = default;

        fileStream(const fileStream&) = delete;
        fileStream& operator=(const fileStream&) = delete;

        fileStream(fileStream&&) = default;  // Allow move
        fileStream& operator=(fileStream&&) = default;

        /**
         * @brief Destructor for the FILE_STREAM class.
         *
         * This destructor ensures that resources are properly released
         * when a FILE_STREAM object is destroyed. It closes the associated
         * BUFFER_CAPTURE if it exists and also closes the file handle.
         */
        ~fileStream();

        /**
         * @brief Read the content of the file.
         * @return The content of the file as a string.
         * 
         * This function will read the content of the file and return it as a string.
         * If the file is empty, this function will return an empty string.
         * If the file does not exist, or any other error occurs, this function will return an empty string and print an error message to the console.
         * If read_from_std_cout is true, this function will read the content from the buffer capture instead of the file.
         */
        std::string Read();

        /**
         * @brief overwrites the given buffer into the file, clearing the previous content of said file.
         * @param Buffer The buffer to write into the file.
         */
        void Write(std::string Buffer);

        /**
         * @brief Append line of text to the end of the file.
         * @param Line The line of text to append to the file.
         */
        void Append(std::string Line);
    
        /**
         * @brief Read the content of the file quickly without checking if the file has changed.
         * @return The content of the file as a string.
         *
         * This function is faster than the normal Read() function, but it does not check if the file has changed.
         * If the file has changed since the last call to Read(), this function will return the old content.
         * If the file does not exist, or any other error occurs, this function will return an empty string and print an error message to the console.
         * If read_from_std_cout is true, this function will read the content from the buffer capture instead of the file.
         */
        std::string Fast_Read() { return Previous_Content; }

        /**
         * @brief Checks if the file has changed and notifies the event handlers if so.
         *
         * This function is called by the FILE_STREAM class when it wants to check if the file has changed.
         * It reads the new content from the file, calculates the hash of the new content, and compares it with the previous hash.
         * If the hash has changed, it notifies all the event handlers by calling them.
         */
        void Changed();

        /**
         * @brief Adds a new event handler to the list of event handlers for this file.
         * @param on_change The function to be called when the file changes.
         * 
         * This function adds a new event handler to the list of event handlers for this file.
         * If read_from_std_cout is true, the event handler is added to the list of event handlers of the Buffer_Capture object.
         * Otherwise, the event handler is added to the list of event handlers of this FILE_STREAM object.
         */
        void Add_On_Change_Handler(std::function<void()> on_change);

        /**
         * @brief Checks if the file stream is a std::cout stream.
         * @return True if the file stream is a std::cout stream, false otherwise.
         *
         * This function checks if the file stream is a std::cout stream, i.e. if it is a stream that is
         * associated with the BUFFER_CAPTURE class. If it is, it returns true, otherwise it returns false.
         */
        bool Is_Cout_Stream(){
            return Buffer_Capture != nullptr;
        }

        FILE_STREAM_TYPE Get_type(){
            return Type;
        }
    };

    class filePosition{
    public:
        std::string File_Name = "";     // Originated.
        unsigned int Line_Number = 0;   // Y
        unsigned int Character = 0;     // X

        /**
         * @brief Constructor for the FILE_POSITION class.
         * @param File_Name The name of the file that the position is in.
         * @param Line_Number The line number of the position.
         * @param Character The character number of the position.
         * 
         * This constructor creates a new FILE_POSITION object with the given file name, line number and character number.
         */
        filePosition(std::string file_name, unsigned int line_number, unsigned int character){
            this->File_Name = file_name;
            this->Line_Number = line_number;
            this->Character = character;
        }

        /**
         * @brief Default constructor for the FILE_POSITION class.
         * @details This constructor creates a new FILE_POSITION object with default values for the file name, line number and character number.
         */
        filePosition() = default;

        /**
         * @brief Converts the FILE_POSITION object to a string.
         * @return A string that represents the FILE_POSITION object in the format "File_Name:Line_Number:Character".
         *
         * This function converts the FILE_POSITION object to a string by concatenating the file name, line number and character number with a colon in between.
         * The resulting string can be used to log or display the position of the file stream.
         */
        std::string To_String(){
            return File_Name + ":" + std::to_string(Line_Number) + ":" + std::to_string(Character);
        }
    };
    
    /**
     * @brief Adds an event handler that is called when the file is changed.
     * @param File_Name The name of the file to add the event handler for.
     * @param Handle The event handler to be called when the file is changed.
     *
     * If there is already a file handle for this file name, the event handler is
     * added to the list of event handlers for that file. If not, a new file
     * handle is created and the event handler is added to the list of event
     * handlers for that file.
     */
    extern void addFileStreamHandle(std::string File_Handle, std::function<void()> Handle, FILE_STREAM_TYPE type);

    /**
     * @brief Returns the file stream handle associated with the given file name.
     * @param File_Name The name of the file to retrieve the handle for.
     * @return The file stream handle associated with the given file name, or nullptr if no handle exists.
     */
    extern fileStream* getFileStreamHandle(std::string File_Name);

    #if _WIN32
        class CMD{
        private:
            void* In;
            void* Out;
        public:
            /**
             * @brief Constructor for the CMD class
             * @details Initializes a pipe for inter-process communication.
             *          The pipe is created with security attributes that allow
             *          the handles to be inherited by child processes.
             */
            CMD();

            /**
             * @brief Destructor for the CMD class
             * @details Closes the handles created by the constructor and releases
             *          any resources associated with the pipe.
             */
            ~CMD() = default;

            /**
             * @brief Runs a command in a shell and returns its output as a string
             * @param Command The command to execute
             * @return The output of the command as a string
             *
             * This function runs a command in a shell and captures its output. It
             * then returns the output as a string.
             */
            std::string Run(std::string command);
        };
    #else
        class CMD{  // Unix implementation:
        private:
            union __INTERNAL_CMD_FILE_DESCRIPTOR__ {
                struct __INTERNAL_CMD_WAY__ {
                    int In;
                    int Out;
                } Way;
                int FDS[2];
            } File_Descriptor;
        public:

            /**
             * @brief Constructor for the CMD class
             *
             * This function creates a pipe and stores the file descriptors in the
             * File_Descriptor structure. The constructor is used to create a new
             * object of the CMD class.
             */
            CMD();
            
            /**
             * @brief Destructor for the CMD class
             * @details Closes the handles created by the constructor and releases
             *          any resources associated with the pipe.
             */
            ~CMD() = default;

            /**
             * @brief Run a command using the system shell and capture its output.
             * @param Command The command to be executed.
             * @return The output of the command as a string.
             * 
             * This function forks a new process to execute the specified command
             * using the system shell. The output of the command is redirected to
             * a pipe and captured by the parent process. The function returns the
             * captured output as a string.
             */
            std::string Run(std::string command);
        };
    #endif
}

#endif
#ifndef _DRM_H_
#define _DRM_H_


#if _WIN32

#else
#include <unistd.h>
#include <cstdint>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include <stdexcept>
#include <string>
#endif

namespace GGUI {
    namespace INTERNAL {
        namespace DRM {
            extern const char* handshakePortLocation;

            struct cell {
                char utf[4];    // Not null terminal, since we already know its size, leftovers are nulled out.

                RGB foregroundColor;
                RGB backgroundColor;
            };

            extern void packAbstractBuffer(char* destinationBuffer, std::vector<UTF>& abstractBuffer);

            constexpr int failRetryWaitTime = TIME::SECOND * 5;

            namespace packet {
                enum class type {
                    UNKNOWN,
                    DRAW_BUFFER,    // For sending/receiving cells
                    INPUT,          // Fer sending/receiving input data
                    NOTIFY,         // Contains an notify flag sending like empty buffers, for optimized polling.
                    RESIZE,         // For sending/receiving GGUI resize
                };

                class base {
                public:
                    type packetType = type::UNKNOWN;

                    base(type t) : packetType(t) {}
                };

                namespace notify {
                    enum class type {
                        UNKNOWN         = 0 << 0,
                        EMPTY_BUFFER    = 1 << 0,
                        CLOSED          = 1 << 1,   // When GGUI client has shutdown
                    };

                    class base : public packet::base {
                    public:
                        type notifyType = type::UNKNOWN;

                        base(type t) : packet::base(packet::type::NOTIFY), notifyType(t) {}
                    };
                }

                namespace input {
                    enum class controlKey {
                        UNKNOWN         = 0 << 0,
                        SHIFT           = 1 << 0,
                        CTRL            = 1 << 1,
                        SUPER           = 1 << 2,
                        ALT             = 1 << 3,
                        ALTGR           = 1 << 4,
                        FN              = 1 << 5,
                        PRESSED_DOWN    = 1 << 6,   // Always on/off to indicate if the key is being pressed down or not.
                    };

                    // Bitwise operators for controlKey enum class
                    inline controlKey operator&(controlKey lhs, controlKey rhs) {
                        return static_cast<controlKey>(static_cast<int>(lhs) & static_cast<int>(rhs));
                    }
                    
                    inline controlKey operator|(controlKey lhs, controlKey rhs) {
                        return static_cast<controlKey>(static_cast<int>(lhs) | static_cast<int>(rhs));
                    }
                    
                    inline controlKey operator^(controlKey lhs, controlKey rhs) {
                        return static_cast<controlKey>(static_cast<int>(lhs) ^ static_cast<int>(rhs));
                    }
                    
                    inline controlKey operator~(controlKey key) {
                        return static_cast<controlKey>(~static_cast<int>(key));
                    }

                    enum class additionalKey {
                        UNKNOWN,
                        F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
                        ARROW_UP, ARROW_DOWN, ARROW_LEFT, ARROW_RIGHT,
                        HOME, END, PAGE_UP, PAGE_DOWN,
                        INSERT, DEL,
                        LEFT_CLICK, MIDDLE_CLICK, RIGHT_CLICK, SCROLL_UP, SCROLL_DOWN,
                    };

                    class base : public packet::base {
                    public:
                        IVector2 mouse;             // Mouse position in the terminal   
                        controlKey modifiers;       // Control keys pressed
                        additionalKey additional;   // Additional keys pressed, which are not declared in ASCII
                        unsigned char key;          // ASCII key pressed, if any

                        base() : packet::base(packet::type::INPUT), mouse(), modifiers(controlKey::UNKNOWN), additional(additionalKey::UNKNOWN), key(0) {}
                    };
                
                    extern void translatePacketInputToGGUIInput(input::base* packetInput);
                }

                namespace resize {
                    class base : public packet::base {
                    public:
                        IVector2 size;

                        base(int width, int height) : packet::base(packet::type::RESIZE), size(width, height) {}
                    };
                }

                union maxSizetype {
                    notify::base n;
                    input::base i;
                    resize::base r;
                };

                // Computes at compile time the maximum needed buffer length for a packet.
                constexpr int size = sizeof(maxSizetype);
            }

            #if _WIN32
            extern void connectDRMBackend();
            
            extern void sendBuffer(std::vector<UTF>&);

            extern void pollInputs();

            extern void translateInputs();

            extern void retryDRMConnect();
            #else

            namespace tcp {
                constexpr size_t pollingRate = TIME::MILLISECOND * 32;

                /**
                 * @brief Represents a TCP connection for sending and receiving data.
                 * 
                 * This class wraps a socket file descriptor and provides methods for
                 * sending and receiving typed data over a TCP connection.
                 */
                class connection {
                    int handle;
                public:
                    /**
                     * @brief Constructs a connection from an existing socket file descriptor.
                     * 
                     * @param socketFd The socket file descriptor to wrap. Must be a valid TCP socket.
                     * @throws std::invalid_argument if socketFd is negative (invalid).
                     */
                    explicit connection(int socketFd) : handle(socketFd) {
                        if (socketFd < 0) {
                            GGUI::INTERNAL::LOGGER::Log("Invalid socket file descriptor provided to connection constructor");
                        }
                    }

                    explicit connection() : handle(-1) {}  // Initialize to invalid state

                    // destructors are called via the handle manager and thus not needed to be called here

                    /**
                     * @brief Destructor that closes the connection if still open.
                     */
                    ~connection() {
                        close();
                    }

                    // Disable copy constructor and assignment operator to prevent double-close
                    connection(const connection&) = delete;
                    connection& operator=(const connection&) = delete;

                    // Enable move constructor and assignment operator
                    connection(connection&& other) noexcept : handle(other.handle) {
                        other.handle = -1;
                    }

                    connection& operator=(connection&& other) noexcept {
                        if (this != &other) {
                            close();
                            handle = other.handle;
                            other.handle = -1;
                        }
                        return *this;
                    }

                    /**
                     * @brief Sends typed data over the TCP connection.
                     * 
                     * This template method sends count elements of type T over the connection.
                     * It ensures all data is sent by checking the return value against the expected size.
                     * 
                     * @tparam T The type of data to send
                     * @param data Pointer to the data buffer to send
                     * @param count Number of elements of type T to send
                     * @return true if all data was successfully sent, false otherwise
                     * @throws std::runtime_error if the socket is invalid or closed
                     */
                    template<typename T>
                    bool Send(const T* data, size_t count = 1) {
                        if (handle < 0) {
                            GGUI::INTERNAL::LOGGER::Log("Cannot send on closed socket");
                        }
                        if (!data && count > 0) {
                            return false;
                        }
                        
                        size_t totalBytes = count * sizeof(T);
                        ssize_t sent = send(handle, data, totalBytes, 0);
                        
                        if (sent < 0) {
                            // Send failed due to error
                            return false;
                        }
                        
                        return sent == static_cast<ssize_t>(totalBytes);
                    }

                    /**
                     * @brief Receives typed data from the TCP connection.
                     * 
                     * This template method receives count elements of type T from the connection.
                     * It ensures all expected data is received by checking the return value.
                     * 
                     * @tparam T The type of data to receive
                     * @param data Pointer to the buffer where received data will be stored
                     * @param count Number of elements of type T to receive
                     * @return true if all expected data was successfully received, false otherwise
                     * @throws std::runtime_error if the socket is invalid or closed
                     */
                    template<typename T>
                    bool Receive(T* data, size_t count = 1) {
                        if (handle < 0) {
                            GGUI::INTERNAL::LOGGER::Log("Cannot receive on closed socket");
                        }
                        if (!data && count > 0) {
                            return false;
                        }
                        
                        size_t totalBytes = count * sizeof(T);
                        ssize_t recvd = recv(handle, data, totalBytes, 0);
                        
                        if (recvd < 0) {
                            // Receive failed due to error
                            return false;
                        }
                        if (recvd == 0) {
                            // Connection closed by peer
                            return false;
                        }
                        
                        return recvd == static_cast<ssize_t>(totalBytes);
                    }

                    /**
                     * @brief Gets the underlying socket file descriptor.
                     * 
                     * @return The socket file descriptor, or -1 if the connection is closed
                     */
                    int getHandle() const { return handle; }

                    /**
                     * @brief Closes the TCP connection.
                     * 
                     * This method closes the underlying socket and marks the connection as invalid.
                     * It's safe to call this method multiple times.
                     */
                    void close() {
                        if (handle >= 0) {
                            ::close(handle);
                            handle = -1; // Mark as closed
                        }
                    }
                };

                /**
                 * @brief TCP listener for accepting incoming connections.
                 * 
                 * This class creates a TCP server socket that can listen for and accept
                 * incoming connections on a specified port.
                 */
                class listener {
                    int handle;
                public:
                    /**
                     * @brief Default constructor that creates an uninitialized listener.
                     * 
                     * Creates a listener with an invalid handle. Must be assigned or moved
                     * from a properly constructed listener before use.
                     */
                    listener() : handle(-1) {}

                    /**
                     * @brief Constructs a TCP listener on the specified port.
                     * 
                     * Creates a TCP socket, binds it to the specified port (or any available port if 0),
                     * and starts listening for incoming connections.
                     * 
                     * @param port The port number to listen on. Use 0 to let the system assign an available port.
                     * @throws std::runtime_error if socket creation, binding, or listening fails
                     */
                    explicit listener(uint16_t port) : handle(-1) {
                        // Create socket
                        handle = socket(AF_INET, SOCK_STREAM, 0);
                        if (handle < 0) {
                            GGUI::INTERNAL::LOGGER::Log("Failed to create socket: " + std::string(strerror(errno)));
                        }

                        // Set socket option to reuse address
                        int opt = 1;
                        if (setsockopt(handle, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
                            ::close(handle);
                            GGUI::INTERNAL::LOGGER::Log("Failed to set socket option SO_REUSEADDR: " + std::string(strerror(errno)));
                        }

                        // Bind socket to address
                        sockaddr_in addr{};
                        addr.sin_family = AF_INET;
                        addr.sin_addr.s_addr = INADDR_ANY;
                        addr.sin_port = htons(port);
                        
                        if (bind(handle, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
                            ::close(handle);
                            GGUI::INTERNAL::LOGGER::Log("Failed to bind socket to port " + std::to_string(port) + ": " + std::string(strerror(errno)));
                        }

                        // Start listening
                        if (listen(handle, 5) < 0) {
                            ::close(handle);
                            GGUI::INTERNAL::LOGGER::Log("Failed to start listening on socket: " + std::string(strerror(errno)));
                        }
                    }

                    /**
                     * @brief Destructor that properly closes the listener socket.
                     */
                    ~listener() {
                        if (handle >= 0) {
                            ::close(handle);
                        }
                    }

                    // Disable copy constructor and assignment operator to prevent double-close
                    listener(const listener&) = delete;
                    listener& operator=(const listener&) = delete;

                    // Enable move constructor and assignment operator
                    listener(listener&& other) noexcept : handle(other.handle) {
                        other.handle = -1;
                    }

                    listener& operator=(listener&& other) noexcept {
                        if (this != &other) {
                            if (handle >= 0) {
                                ::close(handle);
                            }
                            handle = other.handle;
                            other.handle = -1;
                        }
                        return *this;
                    }

                    /**
                     * @brief Accepts an incoming connection.
                     * 
                     * This method blocks until a client connects to the listener.
                     * 
                     * @return A connection object representing the accepted client connection
                     * @throws std::runtime_error if the listener socket is closed or accept fails
                     */
                    connection Accept() {
                        if (handle < 0) {
                            GGUI::INTERNAL::LOGGER::Log("Cannot accept on closed listener");
                        }

                        int connFd = accept(handle, nullptr, nullptr);
                        if (connFd < 0) {
                            GGUI::INTERNAL::LOGGER::Log("Failed to accept connection: " + std::string(strerror(errno)));
                        }
                        
                        return connection(connFd);
                    }

                    /**
                     * @brief Gets the port number this listener is bound to.
                     * 
                     * This is particularly useful when the listener was created with port 0,
                     * as it returns the actual port assigned by the system.
                     * 
                     * @return The port number this listener is bound to
                     * @throws std::runtime_error if getting the socket name fails
                     */
                    uint16_t getPort() {
                        if (handle < 0) {
                            GGUI::INTERNAL::LOGGER::Log("Cannot get port of closed listener");
                        }

                        sockaddr_in actual{};
                        socklen_t len = sizeof(actual);
                        
                        if (getsockname(handle, reinterpret_cast<sockaddr*>(&actual), &len) < 0) {
                            GGUI::INTERNAL::LOGGER::Log("Failed to get socket name: " + std::string(strerror(errno)));
                        }

                        return ntohs(actual.sin_port);
                    }
                };

                /**
                 * @brief Utility class for creating outgoing TCP connections.
                 * 
                 * This class provides static methods to establish TCP connections to remote hosts.
                 */
                class sender {
                public:
                    /**
                     * @brief Creates a TCP connection to the specified host and port.
                     * 
                     * Establishes a TCP connection to the given host and port. This method
                     * creates a socket, resolves the host address, and connects to the remote endpoint.
                     * 
                     * @param port The port number to connect to
                     * @param host The hostname or IP address to connect to (defaults to localhost)
                     * @return A connection object representing the established connection
                     * @throws std::runtime_error if socket creation, address resolution, or connection fails
                     */
                    static connection getConnection(uint16_t port, const char* host = "127.0.0.1") {
                        if (!host) {
                            throw std::invalid_argument("Host cannot be null");
                        }

                        // Create socket
                        int sockFd = socket(AF_INET, SOCK_STREAM, 0);
                        if (sockFd < 0) {
                            GGUI::INTERNAL::LOGGER::Log("Failed to create socket: " + std::string(strerror(errno)));
                        }

                        // Prepare address structure
                        sockaddr_in addr{};
                        addr.sin_family = AF_INET;
                        addr.sin_port = htons(port);
                        
                        // Convert IP address from text to binary form
                        int result = inet_pton(AF_INET, host, &addr.sin_addr);
                        if (result <= 0) {
                            ::close(sockFd);
                            if (result == 0) {
                                GGUI::INTERNAL::LOGGER::Log("Invalid IP address format: " + std::string(host));
                            } else {
                                GGUI::INTERNAL::LOGGER::Log("inet_pton failed: " + std::string(strerror(errno)));
                            }
                        }

                        // Connect to the remote host
                        if (connect(sockFd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
                            ::close(sockFd);
                            GGUI::INTERNAL::LOGGER::Log("Failed to connect to " + std::string(host) + ":" + std::to_string(port) + " - " + std::string(strerror(errno)));

                            return connection();    // Return an empty connection with -1 as fd
                        }
                        
                        return connection(sockFd);
                    }
                };
            
            }

            // This will contain the open connection between DRM and this client.
            extern tcp::connection DRMConnection;

            extern void connectDRMBackend();
            
            extern void sendBuffer(std::vector<UTF>& abstractBuffer);

            extern void retryDRMConnect();

            extern void close();

            // Called via the input thread.
            extern void pollInputs();

            extern void translateInputs();

            #endif

        }
    }
}

#endif
#ifndef _ELEMENT_H_
#define _ELEMENT_H_

#include <string>
#include <sstream>
#include <cstring>
#include <vector>
#include <functional>
#include <chrono>
#include <atomic>
#include <limits>

#include <iostream>


namespace GGUI{
    namespace STYLING_INTERNAL {
        class styleBase;
    }
    namespace INTERNAL {
        extern void renderer();
    }

    class element{
    protected:
        // Only fetch one parent UP, and own position +, then child repeat in Render pipeline.
        IVector3 absolutePositionCache;

        class element* Parent = nullptr;

        // Determines if the element is rendered or not.
        bool Show = true;
        
        std::vector<UTF> renderBuffer;

        // State machine for render pipeline only focus on changed aspects.
        INTERNAL::STAIN Dirty;

        bool Focused = false;
        bool Hovered = false;

        // Human readable ID.
        std::string Name = "";

        // For long term support made this a pointer to avoid size mismatch.
        styling* Style = nullptr;

        void (*On_Init)(element*) = nullptr;
        void (*On_Destroy)(element*) = nullptr;
        void (*On_Hide)(element*) = nullptr;
        void (*On_Show)(element*) = nullptr;
    public:

        /**
         * @brief Constructor for the GGUI::element class.
         * 
         * This constructor initializes an element with a given style and optionally embeds styles during construction.
         * 
         * @param style A reference to a STYLING_INTERNAL::styleBase object that defines the base styling for the element.
         * @param Embed_Styles_On_Construct A boolean flag indicating whether styles should be embedded during construction.
         *        - If true, styles are embedded immediately, and the element's initialization state is finalized.
         *        - If false, styles are not embedded, and a deep copy of unparsed styles is created for later use.
         * 
         * @note If styles are embedded during construction, the element's `On_Init` method will not be called again by 
         *       the main embedding process.
         * 
         * @todo Implement the deep copy functionality for unparsed styles in `Style->copyUnParsedStyles()`.
         */
        element(STYLING_INTERNAL::styleBase& style = STYLES::CONSTANTS::Default, bool Embed_Styles_On_Construct = false);

        /**
         * @brief Constructs an element with the given style and optional embedding of styles.
         * 
         * @param style A rvalue reference to a `STYLING_INTERNAL::styleBase` object representing the style to be applied to the element.
         * @param Embed_Styles_On_Construct A boolean flag indicating whether to embed styles during construction. Defaults to `false`.
         */
        element(STYLING_INTERNAL::styleBase&& style, bool Embed_Styles_On_Construct = false) : element(style, Embed_Styles_On_Construct) {}

        /**
         * @brief For correctly copying data between elements, try the Copy() function.
         * Copying is removed, so that Slicing doesn't happen for the VTable
         */
        element(const element&) = delete;
        element& operator=(const GGUI::element&) = delete;

        element& operator=(element&&) = default;
        element(element&&) = default;

        /**
         * @brief The destructor for the Element class.
         *
         * This destructor is responsible for properly deallocating all the memory
         * allocated by the Element object.
         *
         * @note This destructor is also responsible for cleaning up the parent
         * element's vector of child elements and the event handlers list.
         */
        virtual ~element();

        /**
         * @brief Creates a deep copy of this Element, including all its children.
         * 
         * @return A new Element object that is a copy of this one.
         */
        element* copy() const;

        /**
         * @brief Applies the given styling to the element.
         *
         * This function copies the provided styling object into the element's current style,
         * updating its appearance accordingly.
         *
         * @param s Reference to a styling object containing the new style properties to apply.
         */
        void addStyling(styling& s){
            Style->copy(s);
        }

        /**
         * @brief Adds a style to the current element's style collection.
         *
         * This function takes a reference to a style object and adds it to the element's
         * internal style handler. It allows dynamic modification or extension of the element's
         * appearance at runtime.
         *
         * @param s Reference to a style_base object containing the styling information to add.
         */
        void addStyling(STYLING_INTERNAL::styleBase& s){
            Style->add(s);
            Style->embedStyles(this);
        }
        
        /**
         * @brief Adds a new style to the element's styling collection.
         *
         * This function takes ownership of the provided style object and adds it to the
         * element's internal style manager. The style is passed as an rvalue reference,
         * allowing for efficient move semantics.
         *
         * @param s An rvalue reference to a style_base object representing the style to add.
         */
        void addStyling(STYLING_INTERNAL::styleBase&& s){
            addStyling(s);
        }

        /**
         * @brief Returns the Dirty object for the Element.
         * @details This function returns the Dirty object, which is a bitfield
         *          that keeps track of what needs to be reprocessed on the
         *          Element when it is asked to render.
         * @return A reference to the Dirty object.
         */
        constexpr INTERNAL::STAIN& getDirty(){
            return Dirty;
        }


        /**
         * @brief Returns true if the element is currently focused.
         * @return A boolean indicating whether the element is focused.
         */
        constexpr bool isFocused() const {
            return Focused;
        }

        /**
         * @brief Sets the focus state of the element.
         * @details Sets the focus state of the element to the given value.
         *          If the focus state changes, the element will be dirtied and the frame will be updated.
         * @param f The new focus state.
         */
        void setFocus(bool f);

        /**
         * @brief Returns true if the element is currently hovered.
         * @return A boolean indicating whether the element is hovered.
         */
        constexpr bool isHovered() const {
            return Hovered;
        }

        /**
         * @brief Sets the hover state of the element.
         * @details Sets the hover state of the element to the given value.
         *          If the hover state changes, the element will be dirtied and the frame will be updated.
         * @param h The new hover state.
         */
        void setHoverState(bool h);

        /**
         * @brief Executes the handler function associated with a given state.
         * @details This function checks if there is a registered handler for the specified state.
         *          If a handler exists, it invokes the handler function.
         * @param s The state for which the handler should be executed.
         */
        constexpr void check(INTERNAL::STATE s){
            if (s == INTERNAL::STATE::INIT && On_Init){
                // Since the rendering hasn't yet started and the function here may be reliant on some relative information, we need to evaluate the the dynamic values.
                Style->evaluateDynamicAttributeValues(this);

                On_Init(this);
            }
            else if (s == INTERNAL::STATE::DESTROYED && On_Destroy)
                On_Destroy(this);
            else if (s == INTERNAL::STATE::HIDDEN && On_Hide)
                On_Hide(this);
            else if (s == INTERNAL::STATE::SHOWN && On_Show)
                On_Show(this);
        }

        /**
         * @brief Retrieves the styling information of the element.
         * @details This function returns the current styling object associated with the element.
         *          The styling object contains various style attributes such as colors, borders, etc.
         * @return The styling object of the element.
         */
        styling getStyle() const;

        /**
         * @brief Sets the styling information of the element.
         * @details This function sets the styling information of the element to the given value.
         *          If the element already has a styling object associated with it, the function will
         *          copy the given styling information to the existing object. Otherwise, the function
         *          will create a new styling object and associate it with the element.
         * @param css The new styling information to associate with the element.
         */
        void setStyle(styling css);

        /**
         * @brief Sets the opacity of the element.
         * @details This function takes a float value between 0.0f and 1.0f and sets the
         * opacity of the element to that value. If the value is greater than 1.0f, the
         * function will report an error and do nothing.
         * @param[in] Opacity The opacity value to set.
         */
        void setOpacity(float Opacity);
        
        /**
         * @brief Sets the opacity of the element using an integer percentage.
         * @details This function takes an unsigned integer value between 0 and 100, representing the opacity percentage,
         * and sets the element's opacity. If the value is greater than 100, it will report an error and do nothing.
         * @param[in] Opacity The opacity percentage to set.
         */
        void setOpacity(unsigned int Opacity);

        /**
         * @brief Gets the current border style of the element.
         * @details This function returns the current border style of the element.
         *          The border style is a structure that contains the strings for
         *          the top left corner, top right corner, bottom left corner, bottom
         *          right corner, horizontal line, vertical line, vertical right connector,
         *          vertical left connector, horizontal bottom connector, horizontal top
         *          connector, and cross connector.
         * @return The current border style of the element.
         */
        styledBorder getBorderStyle() const {
            return Style->Border_Style;
        }

        /**
         * @brief Gets the opacity of the element.
         * @details This function returns the current opacity of the element as a float value.
         *          The opacity is a value between 0.0 and 1.0, where 0.0 is fully transparent
         *          and 1.0 is fully opaque.
         * @return The current opacity of the element.
         */
        float getOpacity() const; 

        /**
         * @brief Checks if the element is transparent.
         * @details This function determines whether the element is transparent by checking
         *          if the element's opacity is not equal to 1.0f. An opacity less than 1.0f
         *          indicates that the element is partially or fully transparent.
         * @return True if the element is transparent; otherwise, false.
         */
        bool isTransparent() const;
        
        /**
         * @brief Retrieves the parent element.
         * @details This function returns the parent element of the current element.
         *          If the element has no parent, it will return nullptr.
         * @return A pointer to the parent element.
         */
        constexpr element* getParent() const noexcept {
            return Parent;
        }

        /**
         * @brief Sets the parent of this element.
         * @details This function sets the parent of this element to the given element.
         *          If the given element is nullptr, it will clear the parent of this
         *          element.
         * @param parent The parent element to set.
         */
        void setParent(element* parent);

        /**
         * @brief Sets the border visibility of the element.
         * @details This function takes a boolean as a parameter and sets the border visibility of the element accordingly.
         *          If the new state is different from the current state, the element will be marked as dirty with the EDGE stain.
         * @param b The new state of the border visibility.
         */
        virtual void showBorder(bool b);

        /**
         * @brief Sets the border visibility of the element.
         * @details This function toggles the border visibility based on the provided state.
         *          If the state has changed, it updates the border visibility, marks the 
         *          element as dirty with the EDGE stain, and updates the frame.
         * @param b The desired state of the border visibility.
         * @param Previous_State The current state of the border visibility.
         */
        virtual void showBorder(bool b, bool Previous_state);

        /**
         * @brief Checks if the element has a border.
         * @details This function checks if the element has a border.
         *          It returns true if the element has a border, false otherwise.
         * @return True if the element has a border, false otherwise.
         */
        bool hasBorder();

        /**
         * @brief Displays or hides the element and all its children.
         * @details This function changes the display status of the element and all its children.
         *          If the element is displayed, all its children are also displayed. If the element is hidden,
         *          all its children are also hidden.
         * @param f A boolean indicating whether to display (true) or hide (false) the element and its children.
         */
        void display(bool f);

        /**
         * @brief Returns whether the element is currently displayed.
         * @details This function is used to check whether the element is currently displayed or hidden.
         *          It returns true if the element is displayed and false if the element is hidden.
         * @return A boolean indicating whether the element is displayed (true) or hidden (false).
         */
        bool isDisplayed();

        /**
         * @brief Adds a child element to the element.
         * @details This function adds a child element to the element. If the element has a border, it will
         *          be taken into account when calculating the size of the parent element. If the child element
         *          exceeds the size of the parent element, the parent element will be resized to fit the child
         *          element. If the parent element is not allowed to resize, the child element will be resized to
         *          fit the parent element.
         * @param Child The child element to add.
         */
        virtual void addChild(element* Child);

        /**
         * @brief Adds a vector of child elements to the current element.
         * @param childs The vector of child elements to add.
         *
         * This function adds all the child elements to the current element by calling the Add_Child function for each element in the vector.
         * It also marks the current element as dirty with the DEEP stain after adding all the elements.
         */
        virtual void setChilds(std::vector<element*> childs);

        /**
         * @brief Retrieves the list of child elements.
         * @details This function returns a reference to the vector containing all child elements
         *          associated with the current element's style.
         * @return A reference to the vector of child elements.
         */
        virtual std::vector<element*>& getChilds();

        /**
         * @brief Removes a child element from the current element.
         * @param handle The pointer to the child element to be removed.
         * @return true if the element was successfully removed, false if not.
         *
         * This function iterates through the vector of child elements and checks
         * if the element at the current index is equal to the handle passed as an argument.
         * If it is, the element is deleted and the parent element is marked as dirty with the DEEP and COLOR stains.
         * If the currently focused element is the one being removed, the mouse position is set to the parent element's position.
         */
        virtual bool remove(element* handle);

        /**
         * @brief Removes the element at a given index from the list of child elements.
         * @details This function checks if the index is valid (i.e. if the index is within the bounds of the vector of child elements).
         *          If the index is valid, it removes the element at the specified index from the vector of child elements and deletes the element.
         *          If the index is invalid, the function returns false.
         * @param index The index of the element to remove.
         * @return True if the element was successfully removed, false otherwise.
         */
        virtual bool remove(unsigned int index);

        /**
         * @brief Set the width and height of the element.
         * @details This function sets the width and height of the element to the specified values.
         *          If the width or height is different from the current width or height, then the element will be resized and the STRETCH stain is set.
         *          The Update_Frame() function is also called to update the frame.
         * @param width The new width of the element.
         * @param height The new height of the element.
         */
        void setDimensions(unsigned int width, unsigned int height);

        /**
         * @brief Get the width of the element.
         * @details This function returns the width of the element.
         * @return The width of the element.
         */
        constexpr unsigned int getWidth(){ return Style->Width.get(); }

        /**
         * @brief Get the height of the element.
         * @details This function returns the height of the element.
         * @return The height of the element.
         */
        constexpr unsigned int getHeight() { return Style->Height.get(); }

        /**
         * @brief Set the width of the element.
         * @details This function sets the width of the element to the specified value.
         *          If the width is different from the current width, then the element will be resized and the STRETCH stain is set.
         *          The Update_Frame() function is also called to update the frame.
         * @param width The new width of the element.
         */
        void setWidth(unsigned int width);

        /**
         * @brief Set the height of the element.
         * @details This function sets the height of the element to the specified value.
         *          If the height is different from the current height, then the element will be resized and the STRETCH stain is set.
         *          The Update_Frame() function is also called to update the frame.
         * @param height The new height of the element.
         */
        void setHeight(unsigned int height);

        /**
         * @brief Retrieves the evaluation type of the width property.
         * 
         * This function returns the evaluation type of the width property
         * from the style's width value.
         * 
         * @return EVALUATION_TYPE The evaluation type of the width property.
         */
        INTERNAL::EVALUATION_TYPE getWidthType() { return Style->Width.number.Get_Type(); }

        /**
         * @brief Retrieves the evaluation type of the height value.
         * 
         * This function returns the evaluation type of the height value from the style's height property.
         * 
         * @return EVALUATION_TYPE The evaluation type of the height value.
         */
        INTERNAL::EVALUATION_TYPE getHeightType() { return Style->Height.number.Get_Type(); }

        /**
         * @brief Set the position of the element.
         * @details This function sets the position of the element to the specified coordinates.
         *          If the position changes, the element will be marked as dirty for movement
         *          and the frame will be updated.
         * @param c The new position of the element.
         */
        void setPosition(IVector3 c);
       
        /**
         * @brief Set the position of the element.
         * @details This function sets the position of the element to the specified coordinates.
         *          If the position changes, the element will be marked as dirty for movement
         *          and the frame will be updated.
         * @param c The new position of the element.
         */
        void setPosition(IVector3* c);

        /**
         * @brief Updates the position of the element by adding the given vector.
         *
         * This function increments the current position of the element by the specified vector `v`.
         *
         * @param v The vector to add to the element's current position.
         */
        void updatePosition(IVector3 v);

        /**
         * @brief Get the position of the element.
         * @details This function retrieves the position of the element from its style.
         * @return The position of the element as an IVector3 object.
         */
        constexpr IVector3 getPosition() { return Style->Position.get(); }

        /**
         * @brief Get the absolute position of the element.
         * @details This function returns the cached absolute position of the element.
         *          The absolute position is the position of the element in the context of the entire document or window.
         * @return The absolute position of the element as an IVector3 object.
         */
        constexpr IVector3 getAbsolutePosition() { return absolutePositionCache; }

        /**
         * @brief Sets the title of the window and updates border visibility and colors accordingly.
         * 
         * This function sets the window's title and ensures that the border is shown if the title is not empty.
         * If the window previously had hidden borders, it updates the border colors based on the background color.
         * 
         * @param t The new title for the window.
         */
        void setTitle(INTERNAL::compactString t);

        /**
         * @brief Returns the title of the window.
         * 
         * @return The title of the window as a string.
         */
        INTERNAL::compactString getTitle();

        /**
         * @brief Set the margin of the element.
         * @details This function sets the margin of the element to the specified margin values.
         *          The margin is stored in the element's style.
         * @param margin The new margin values for the element.
         */
        void setMargin(margin margin);

        /**
         * @brief Get the margin of the element.
         * @details This function retrieves the margin of the element from its style.
         * @return The margin of the element as a GGUI::margin object.
         */
        margin getMargin() { return Style->Margin; }
        
        /**
         * @brief Sets the background color of the element.
         * 
         * This function sets the background color of the element to the specified RGB value. 
         * If the border background color is the same as the current background color, 
         * it updates the border background color as well. Marks the element as dirty for 
         * color updates and triggers a frame update.
         * 
         * @param color The RGB color to set as the background color.
         */
        void setBackgroundColor(RGB color);

        /**
         * @brief Retrieves the background color of the element.
         * 
         * This function returns the RGB value of the background color 
         * from the element's style.
         * 
         * @return The RGB color of the element's background.
         */
        constexpr RGB getBackgroundColor() { return Style->Background_Color.color.get<RGB>(); }
        
        /**
         * @brief Sets the border color of the element.
         * 
         * This function sets the border color of the element to the specified RGB value. Marks the element as dirty for color updates and triggers a frame update.
         * 
         * @param color The RGB color to set as the border color.
         */
        void setBorderColor(RGB color);
        
        /**
         * @brief Retrieves the border color of the element.
         * 
         * This function returns the RGB value of the border color 
         * from the element's style.
         * 
         * @return The RGB color of the element's border.
         */
        constexpr RGB getBorderColor(){ return Style->Border_Color.color.get<RGB>(); }

        /**
         * @brief Sets the border background color of the element.
         * 
         * This function sets the border background color of the element to the specified RGB value.
         * It marks the element as dirty for color updates and triggers a frame update.
         * 
         * @param color The RGB color to set as the border background color.
         */
        void setBorderBackgroundColor(RGB color);
        
        /**
         * @brief Retrieves the border background color of the element.
         * 
         * This function returns the RGB value of the border background color
         * from the element's style.
         * 
         * @return The RGB color of the element's border background.
         */
        constexpr RGB getBorderBackgroundColor(){ return Style->Border_Background_Color.color.get<RGB>(); }
        
        /**
         * @brief Sets the text color of the element.
         * 
         * This function sets the text color of the element to the specified RGB value. 
         * It marks the element as dirty for color updates and triggers a frame update.
         * 
         * @param color The RGB color to set as the text color.
         */
        void setTextColor(RGB color);

        /**
         * @brief Retrieves the text color of the element.
         * 
         * This function returns the RGB value of the text color
         * from the element's style.
         * 
         * @return The RGB color of the element's text.
         */
        constexpr RGB getTextColor(){ return Style->Text_Color.color.get<RGB>(); }

        /**
         * @brief Sets the hover border color of the element.
         * 
         * This function sets the border color of the element when the mouse hovers over it
         * to the specified RGB value. Marks the element as dirty for color updates and
         * triggers a frame update.
         * 
         * @param color The RGB color to set as the hover border color.
         */
        void setHoverBorderColor(RGB color);

        /**
         * @brief Retrieves the hover border color of the element.
         * 
         * This function returns the RGB value of the border color when the mouse hovers over the element
         * from the element's style.
         * 
         * @return The RGB color of the element's hover border.
         */
        constexpr RGB getHoverBorderColor(){ return Style->Hover_Border_Color.color.get<RGB>(); }

        /**
         * @brief Sets the hover background color of the element.
         * 
         * This function sets the background color of the element when the mouse hovers over it
         * to the specified RGB value. Marks the element as dirty for color updates and triggers
         * a frame update.
         * 
         * @param color The RGB color to set as the hover background color.
         */
        void setHoverBackgroundColor(RGB color);

        /**
         * @brief Retrieves the hover background color of the element.
         * 
         * This function returns the RGB value of the background color when the mouse hovers over the element
         * from the element's style.
         * 
         * @return The RGB color of the element's hover background.
         */
        constexpr RGB getHoverBackgroundColor(){ return Style->Hover_Background_Color.color.get<RGB>(); }

        /**
         * @brief Sets the hover text color of the element.
         * 
         * This function sets the text color of the element when the mouse hovers over it
         * to the specified RGB value. Marks the element as dirty for color updates and triggers
         * a frame update.
         * 
         * @param color The RGB color to set as the hover text color.
         */
        void setHoverTextColor(RGB color);

        /**
         * @brief Retrieves the hover text color of the element.
         * 
         * This function returns the RGB value of the text color when the mouse hovers over the element
         * from the element's style.
         * 
         * @return The RGB color of the element's hover text.
         */
        constexpr RGB getHoverTextColor(){ return Style->Hover_Text_Color.color.get<RGB>(); }

        /**
         * @brief Sets the hover border background color of the element.
         * 
         * This function sets the background color of the element's border 
         * when the mouse hovers over it to the specified RGB value. It marks 
         * the element as dirty for color updates and triggers a frame update.
         * 
         * @param color The RGB color to set as the hover border background color.
         */
        void setHoverBorderBackgroundColor(RGB color);

        /**
         * @brief Retrieves the hover border background color of the element.
         * 
         * This function returns the RGB value of the background color of the element's border
         * when the mouse hovers over it from the element's style.
         * 
         * @return The RGB color of the element's hover border background.
         */
        constexpr RGB getHoverBorderBackgroundColor(){ return Style->Hover_Border_Background_Color.color.get<RGB>(); }

        /**
         * @brief Sets the focus border color of the element.
         * 
         * This function sets the color of the element's border when it is focused to the specified RGB value. It marks the element as dirty for color updates and triggers a frame update.
         * 
         * @param color The RGB color to set as the focus border color.
         */
        void setFocusBorderColor(RGB color);

        /**
         * @brief Retrieves the focus border color of the element.
         * 
         * This function returns the RGB value of the border color when the element is focused
         * from the element's style.
         * 
         * @return The RGB color of the element's focus border.
         */
        constexpr RGB getFocusBorderColor(){ return Style->Focus_Border_Color.color.get<RGB>(); }

        /**
         * @brief Sets the focus background color of the element.
         * 
         * This function sets the background color of the element when it is focused to the specified RGB value. It marks the element as dirty for color updates and triggers a frame update.
         * 
         * @param color The RGB color to set as the focus background color.
         */
        void setFocusBackgroundColor(RGB color);

        /**
         * @brief Retrieves the focus background color of the element.
         * 
         * This function returns the RGB value of the background color when the element is focused
         * from the element's style.
         * 
         * @return The RGB color of the element's focus background.
         */
        constexpr RGB getFocusBackgroundColor(){ return Style->Focus_Background_Color.color.get<RGB>(); }

        /**
         * @brief Sets the focus text color of the element.
         * 
         * This function sets the text color of the element when it is focused to the specified RGB value. It marks the element as dirty for color updates and triggers a frame update.
         * 
         * @param color The RGB color to set as the focus text color.
         */
        void setFocusTextColor(RGB color);

        /**
         * @brief Retrieves the focus text color of the element.
         * 
         * This function returns the RGB value of the text color when the element is focused
         * from the element's style.
         * 
         * @return The RGB color of the element's focus text.
         */
        constexpr RGB getFocusTextColor(){ return Style->Focus_Text_Color.color.get<RGB>(); }

        /**
         * @brief Sets the focus border background color of the element.
         * 
         * This function sets the focus border background color of the element to the specified RGB value.
         * It marks the element as dirty for color updates and triggers a frame update.
         * 
         * @param color The RGB color to set as the focus border background color.
         */
        void setFocusBorderBackgroundColor(RGB color);

        /**
         * @brief Retrieves the focus border background color of the element.
         * 
         * This function returns the RGB value of the focus border background color
         * from the element's style.
         * 
         * @return The RGB color of the element's focus border background.
         */
        constexpr RGB getFocusBorderBackgroundColor(){ return Style->Focus_Border_Background_Color.color.get<RGB>(); }

        /**
         * @brief Sets the alignment of the element.
         * 
         * This function sets the alignment of the element to the specified ALIGN value.
         * 
         * @param Align The alignment value to set for the element.
         */
        void setAnchor(ANCHOR Anchor);

        /**
         * @brief Sets the alignment of the element.
         * 
         * This function sets the alignment of the element to the specified ALIGN value.
         * 
         * @param Align The alignment value to set for the element.
         */
        constexpr ANCHOR getAlign(){ return Style->Align.value; }

        /**
         * @brief Sets the flow priority of the element.
         * 
         * This function sets the flow priority of the element to the specified DIRECTION value.
         * The flow priority determines how the element will be aligned in its parent when the parent is a flow layout.
         * 
         * @param Priority The flow priority value to set for the element.
         */
        void setFlowPriority(DIRECTION d);

        /**
         * @brief Retrieves the flow priority of the element.
         * 
         * This function returns the DIRECTION value that was previously set with Set_Flow_Priority.
         * The flow priority determines how the element will be aligned in its parent when the parent is a flow layout.
         * 
         * @return The flow priority value of the element.
         */
        constexpr DIRECTION getFlowPriority(){ return Style->Flow_Priority.value; }

        /**
         * @brief Sets whether the element will wrap its contents to the next line when it hits the edge of the screen.
         * 
         * This function sets whether the element will wrap its contents to the next line when it hits the edge of the screen.
         * If true, the element will wrap its contents to the next line when it hits the edge of the screen.
         * If false, the element will not wrap its contents to the next line when it hits the edge of the screen.
         * 
         * @param Wrap The value to set for whether the element will wrap its contents to the next line.
         */
        void setWrap(bool w);

        /**
         * @brief Retrieves the wrap setting of the element.
         * 
         * This function returns whether the element will wrap its contents to the next line
         * when it reaches the edge of the screen.
         * 
         * @return True if the element will wrap its contents, false otherwise.
         */
        constexpr bool getWrap(){ return Style->Wrap.value; }

        /**
         * @brief Sets whether the element is allowed to dynamically resize.
         * 
         * This function enables or disables the ability of the element to 
         * adjust its size based on its content.
         * 
         * @param True A boolean indicating whether dynamic resizing is allowed.
         */
        void allowDynamicSize(bool True);

        /**
         * @brief Checks whether the element is allowed to dynamically resize.
         * 
         * This function checks the Allow_Dynamic_Size property in the element's style
         * and returns its value.
         * 
         * @return True if the element is allowed to dynamically resize, false otherwise.
         */
        constexpr bool isDynamicSizeAllowed(){ return Style->Allow_Dynamic_Size.value; }

        /**
         * @brief Sets whether the element allows overflow.
         * 
         * This function enables or disables the overflow property of the element,
         * allowing child elements to exceed the parent's dimensions without resizing it.
         * 
         * @param True A boolean indicating whether overflow is allowed.
         */ 
        void allowOverflow(bool True);

        /**
         * @brief Checks whether the element allows overflow.
         * 
         * This function checks the Allow_Overflow property in the element's style
         * and returns its value.
         * 
         * @return True if the element allows overflow, false otherwise.
         */
        constexpr bool isOverflowAllowed(){ return Style->Allow_Overflow.value; }
        
        /**
         * @brief Recursively computes the size of the element based on its children.
         * 
         * This function will go through all the elements that are being displayed and
         * compute their size based on the size of their children. If the element has
         * children and the children have changed, then the element will be resized
         * to fit the children. If the element does not have children, then the function
         * will not do anything.
         * 
         * @note This function is called automatically by the framework when the
         *       elements are being rendered. It is not necessary to call this function
         *       manually.
         */
        void computeDynamicSize();

        /**
         * @brief Updates the parent element of the current element.
         * @details This function is called when the current element is added, removed, or moved
         *          to a different parent element. It marks the parent element as dirty and
         *          requests a render update.
         * @param New_Element The new parent element.
         *
         * @note If the parent element does not have a valid render buffer (i.e., its
         *       `Is_Displayed()` function returns false), this function marks the parent
         *       element as dirty with the `INTERNAL::STAIN_TYPE::DEEP` and `INTERNAL::STAIN_TYPE::COLOR` stains.
         *       This ensures that the parent element is re-rendered from scratch when the
         *       rendering thread is updated.
         */
        void updateParent(element* New_Element);

        /**
         * @brief Resizes the element to fit the size of its parent element.
         * @details This function is called when the parent element is resized and the
         *          current element is a child of the parent element. It resizes the
         *          current element to fit the size of its parent element. If the parent
         *          element does not have a valid render buffer (i.e., its
         *          `Is_Displayed()` function returns false), this function does
         *          nothing.
         * @param parent The parent element to resize to.
         * @return true if the resize was successful, false otherwise.
         */
        virtual bool resizeTo([[maybe_unused]] element* parent){
            return false;
        }

        /**
         * @brief Sets the custom border style for the element.
         * @details This function sets the custom border style for the element, marks the element's edges as dirty, and ensures that the border is visible.
         * @param style The custom border style to set.
         */
        void setCustomBorderStyle(GGUI::styledBorder style);

        /**
         * @brief Gets the custom border style of the element.
         * @return The custom border style of the element.
         */
        GGUI::styledBorder getCustomBorderStyle(){ return Style->Border_Style; }

        /**
         * @brief Composes the RGB values of the text color and background color of the element.
         * 
         * This function will return a pair of RGB values, where the first element is the
         * color of the text and the second element is the color of the background.
         * 
         * If the element is focused, the function will return the RGB values of the focused
         * text color and background color. If the element is hovered, the function will
         * return the RGB values of the hovered text color and background color. Otherwise,
         * the function will return the RGB values of the normal text color and background
         * color.
         * 
         * @return A pair of RGB values representing the text color and background color of the element.
         */
        constexpr std::pair<RGB, RGB>  composeAllTextRGBValues(){
            if (Focused){
                return {Style->Focus_Text_Color.color.get<RGB>(), Style->Focus_Background_Color.color.get<RGB>()};
            }
            else if (Hovered){
                return {Style->Hover_Text_Color.color.get<RGB>(), Style->Hover_Background_Color.color.get<RGB>()};
            }
            else{
                return {Style->Text_Color.color.get<RGB>(), Style->Background_Color.color.get<RGB>()};
            }
        }

        /**
         * @brief Composes the RGB values of the border color and background color of the element.
         * @details This function will return the RGB values of the border color and background color of the element.
         * If the element is focused, the function will return the RGB values of the focused border color and background color.
         * If the element is hovered, the function will return the RGB values of the hovered border color and background color.
         * Otherwise, the function will return the RGB values of the normal border color and background color.
         * @return A pair of RGB values representing the border color and background color of the element.
         */
        constexpr std::pair<RGB, RGB> composeAllBorderRGBValues(){
            if (Focused){
                return {Style->Focus_Border_Color.color.get<RGB>(), Style->Focus_Border_Background_Color.color.get<RGB>()};
            }
            else if (Hovered){
                return {Style->Hover_Border_Color.color.get<RGB>(), Style->Hover_Border_Background_Color.color.get<RGB>()};
            }
            else{
                return {Style->Border_Color.color.get<RGB>(), Style->Border_Background_Color.color.get<RGB>()};
            }
        }

        /**
         * @brief Returns the name of the element.
         * @details This function returns a string that represents the name of the element.
         *          The name is constructed by concatenating the name of the element with the 
         *          class name of the element, separated by a "<" and a ">".
         * @return The name of the element.
         */
        virtual std::string getName() const {
            return "element<" + getNameAsRaw() + ">";
        }

        /**
         * @brief Retrieves the name of the element as a raw string.
         * 
         * If the element's name is not set (i.e., the Name string is empty),
         * this function returns the memory address of the element as a string.
         * Otherwise, it returns the Name string.
         * 
         * @return A std::string containing either the element's name or its memory address.
         */
        std::string getNameAsRaw() const;

        /**
         * @brief Checks if the element's name is empty.
         * 
         * @return true if the name of the element has no characters, false otherwise.
         */
        bool hasEmptyName() const;

        /**
         * @brief Set the name of the element.
         * @details This function sets the name of the element and stores it in the global Element_Names map.
         * @param name The name of the element.
         */
        void setName(std::string name);

        /**
         * @brief Removes the element from the parent element.
         * @details This function first checks if the element has a parent.
         *          If the element has a parent, it calls the parent's Remove() function to remove the element from the parent.
         *          If the element does not have a parent, it prints an error message to the console.
         *          The function does not update the frame, so it is the caller's responsibility to update the frame after calling this function.
         */
        void remove();

        /**
         * @brief A function that registers a lambda to be executed when the element is clicked.
         * @details The lambda is given a pointer to the Event object that triggered the call.
         *          The lambda is expected to return true if it was successful and false if it failed.
         * @param action The lambda to be called when the element is clicked.
         */
        void onClick(std::function<bool(GGUI::event*)> action);

        /**
         * @brief A function that registers a lambda to be executed when the element is interacted with in any way.
         * @details The lambda is given a pointer to the Event object that triggered the call.
         *          The lambda is expected to return true if it was successful and false if it failed.
         * @param criteria The criteria to check for when deciding whether to execute the lambda.
         * @param action The lambda to be called when the element is interacted with.
         * @param GLOBAL Whether the lambda should be executed even if the element is not under the mouse.
         */
        void on(unsigned long long criteria, std::function<bool(GGUI::event*)> action, bool GLOBAL = false);

        /**
         * @brief Retrieves an element by its name.
         * 
         * This function searches through the child elements of the current element
         * to find an element with the specified name. It performs a recursive search
         * through all descendants.
         * 
         * @param name The name of the element to search for.
         * @return A pointer to the element with the specified name, or nullptr if no such element is found.
         */
        element* getElement(std::string name);

        // TEMPLATES
        //-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_

        /**
         * @brief Retrieves all child elements that have the same type as the given template.
         * @details This function takes a template argument representing the type of the elements to retrieve.
         *          It goes through the child AST, and checks if the element in question is the same type as the template T.
         *          If the element in question is of the same type as the template T, it adds it to the result vector.
         *          The function then recursively goes through the child AST, and checks if any of the child elements are of the same type as the template T.
         *          If any of the child elements are of the same type as the template T, it adds them to the result vector.
         *          The function then returns the result vector, which contains all child elements that have the same type as the given template.
         * @param T The type of the elements to retrieve.
         * @return A vector of pointers to the elements that have the same type as the given template.
         */
        template<typename T>
        std::vector<T*> getElements(){
            std::vector<T*> result;

            // Check if the element in question is of the same type as the template T.
            if (dynamic_cast<T*>(this)){
                result.push_back((T*)this);
            }

            // Go through the child AST, and check if any of the child elements are of the same type as the template T.
            for (auto e : Style->Childs){
                // Recursively go through the child AST, and check if any of the child elements are of the same type as the template T.
                std::vector<T*> child_result = e->getElements<T>();

                // Add the results of the recursive call to the result vector.
                result.insert(result.end(), child_result.begin(), child_result.end());
            }

            // Return the result vector, which contains all child elements that have the same type as the given template.
            return result;
        }

        /**
         * @brief Retrieves all nested elements, including this element.
         * @details This function collects all nested elements recursively, starting from this element.
         *          If 'Show_Hidden' is false, hidden elements are excluded from the result.
         * @param Show_Hidden Flag to determine whether to include hidden elements in the result.
         * @return A vector of pointers to all nested elements.
         */
        std::vector<element*> getAllNestedElements(bool Show_Hidden = false);

        //-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_

        /**
         * @brief Default virtual function for scrolling up.
         * @details By default, elements do not have inherent scrolling abilities.
         *          This function is used as a base for other elements to implement their own scrolling.
         */
        virtual void scrollUp() {}

        /**
         * @brief Default virtual function for scrolling down.
         * @details By default, elements do not have inherent scrolling abilities.
         *          This function is used as a base for other elements to implement their own scrolling.
         */
        virtual void scrollDown() {}

        /**
         * @brief Reorders child elements based on their z-position.
         * @details This function sorts the child elements of the current element by their z-coordinate
         *          in ascending order, so that elements with a higher z-coordinate appear later in the list.
         */
        void reOrderChilds();

        /**
         * @brief Focuses the element.
         * @details This function updates the global focus information by setting the mouse position to the element's position and updating the focused element.
         */
        void focus();

        /**
         * @brief Adds a handler function to the state handlers map.
         * @details This function takes a state and a handler function as arguments.
         *          The handler function is stored in the State_Handlers map with the given state as the key.
         * @param s The state for which the handler should be executed.
         * @param job The handler function to be executed
         */
        void onState(INTERNAL::STATE s, void (*job)(element* self));

        // Customization helper function
        //-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_
        
        /**
         * @brief Adds a stain to the element.
         * @details This function adds the given stain to the element's stains list.
         *          The stains list is used to keep track of which properties of the element need to be re-evaluated.
         *          The function takes a STAIN_TYPE as a parameter and adds it to the list of stains.
         * @param s The stain to be added.
         */
        void addStain(INTERNAL::STAIN_TYPE s){
            Dirty.Dirty(s);
        }

        /**
         * @brief
         * This function determines if the given element is a direct child of this element (in the DOM tree),
         * and if it is visible on the screen (does not go out of bounds of the parent element).
         * @param other Child element to check
         * @return True if the child element is visible within the bounds of the parent.
         */
        bool childIsShown(element* other);

        /**
         * @brief Sets the callback function to be executed during the initialization of the element.
         * 
         * @param func A pointer to a function that takes a pointer to the current element (`element* self`) 
         *             as its parameter and returns void. This function will be called during the 
         *             initialization phase of the element.
         */
        inline void setOnInit(void (*func)(element* self)){
            On_Init = func;
        }

        /**
         * @brief Sets a callback function to be executed when the element is destroyed.
         * 
         * @param func A pointer to a function that takes a pointer to the current element 
         *             (`element* self`) as its parameter. This function will be called 
         *             during the destruction process of the element.
         */
        inline void setOnDestroy(void (*func)(element* self)){
            On_Destroy = func;
        }

        /**
         * @brief Sets the callback function to be executed when the element is hidden.
         * 
         * @param func A pointer to a function that takes a pointer to the current 
         *             element (`element* self`) as its parameter. This function will 
         *             be called when the element is hidden.
         */
        inline void setOnHide(void (*func)(element* self)){
            On_Hide = func;
        }

        /**
         * @brief Sets the callback function to be executed when the element is shown.
         * 
         * @param func A pointer to a function that takes a pointer to the current 
         *             element (`element* self`) as its parameter. This function 
         *             will be called when the element is shown.
         */
        inline void setOnShow(void (*func)(element* self)){
            On_Show = func;
        }

        /**
         * @brief Forces the evaluation of dynamic style attributes for the current element.
         * 
         * This method checks if the element has an associated style object. If a style object
         * exists, it triggers the evaluation of dynamic attribute values for the element
         * by calling the `evaluateDynamicAttributeValues` method on the style object.
         * 
         * @note This function is typically used to ensure that the element's style is
         * up-to-date, especially when dynamic attributes are involved.
         */
        inline void forceStyleEvaluation(){
            if (Style)
                Style->evaluateDynamicAttributeValues(this);
        }
        
    protected:
    
        /**
         * @brief Retrieves the direct styling associated with this element.
         * 
         * This function returns a pointer to the styling object (`GGUI::styling`) 
         * that is directly associated with the current element. The returned 
         * styling object can be used to access or modify the visual properties 
         * of the element.
         * 
         * @return GGUI::styling* Pointer to the direct styling object of the element.
         */
        styling* getDirectStyle();

        /**
         * @brief Add the border of the window to the rendered string.
         *
         * @param Result The string to add the border to.
         */
        void renderBorders(std::vector<UTF>& Result);

        /**
         * @brief Renders the title of the element into the provided result buffer.
         * 
         * This function writes the title text of the element into the `Result` vector,
         * taking into account the available width and applying an ellipsis ("...") if
         * the title is too long to fit within the allocated space. The title is styled
         * using the element's text RGB values.
         * 
         * @param Result A vector of UTF objects where the rendered title will be stored.
         *               The vector should have sufficient size to accommodate the rendered text.
         * 
         * @details
         * - If the title is empty, the function returns immediately without modifying `Result`.
         * - The title's length is determined by the `Style->Title.Value.size`.
         * - The function calculates the writable length based on the element's width,
         *   taking into account borders and the space required for the ellipsis.
         * - If the title exceeds the writable length, an ellipsis is appended to indicate truncation.
         * - The function ensures that no characters are written beyond the available width.
         * 
         * @note The function assumes that the `Result` vector is pre-allocated and large enough
         *       to hold the rendered title and ellipsis.
         */
        void renderTitle(std::vector<UTF>& Result);

        /**
         * @brief Apply the color system to the rendered string.
         *
         * This function applies the color system set by the style to the rendered string.
         * It is called after the element has been rendered and the result is stored in the
         * Result vector.
         *
         * @param Result The vector containing the rendered string.
         */
        void applyColors(std::vector<UTF>& Result);
        
        /**
         * @brief Posts a process that handles the intersection of borders between two elements and their parent.
         * @details This function posts a process that handles the intersection of borders between two elements and their parent.
         *          The process calculates the intersection points of the borders and then constructs a bit mask that portraits the connections the middle point has.
         *          With the calculated bit mask it can fetch from the 'SYMBOLS::Border_Identifiers' the right border string.
         * @param A The first element.
         * @param B The second element.
         * @param Parent_Buffer The buffer of the parent element.
         */
        void postProcessBorders(element* A, element* B, std::vector<UTF>& Parent_Buffer);

        /**
         * @brief Update the absolute position cache of the element.
         * @details This function updates the cached absolute position of the element by adding the position of the element to the position of its parent.
         */
        void updateAbsolutePositionCache();
        
        /**
         * @brief Check if any children have changed.
         * @details This function will check if any of the children have changed, this is used to determine if the element needs to be re-drawn.
         * @return true if any children have changed, false otherwise.
         */
        bool childrenChanged();
        
        /**
         * @brief Check if there are any transparent children.
         * @details This function determines if the current element or any of its children
         *          are transparent and require redrawing.
         * @return True if any child is transparent and not clean; otherwise, false.
         */
        bool hasTransparentChildren();    
        
        /**
         * @brief Retrieves the final size limit of the element.
         *
         * This function calculates the final size limit of the element based on its
         * properties and its parent's constraints. If overflow is allowed, the size
         * limit is set to the maximum possible value. Otherwise, the size is determined
         * by the element's dimensions or inherited from the parent if dynamic sizing
         * is allowed.
         *
         * @return GGUI::IVector3 The final size limit of the element.
         */
        IVector3 getFinalLimit();
        
        /**
         * @brief Get the fitting dimensions for the given child element.
         *
         * This function takes a child element and calculates the fitting dimensions for it.
         * The fitting dimensions are the width and height of the child element that does not exceed the
         * bounds of the parent element. If the child element is colliding with another child element
         * then the fitting dimensions are reduced to the point where the collision is resolved.
         *
         * @param child The child element for which the fitting dimensions are calculated.
         * @return A pair containing the width and height of the fitting dimensions.
         */
        std::pair<unsigned int, unsigned int> getFittingDimensions(element* child);

        /**
         * @brief Calculates the hitboxes of all child elements of the element.
         * @details This function calculates the hitboxes of all child elements of the element.
         *          The hitbox of a child element is the area of the element that is actually visible
         *          on the screen. The function takes the starting offset into the child array as an
         *          argument. If no argument is provided, the function starts at the beginning of the
         *          child array.
         * @param Starting_Offset The starting offset into the child array. If no argument is provided,
         *                         the function starts at the beginning of the child array.
         */
        virtual void calculateChildsHitboxes([[maybe_unused]] unsigned int Starting_Offset = 0) {}

        /** 
         * @brief Marks the Element as fully dirty by setting all stain types.
         * 
         * This function sets each stain type on the Dirty object, indicating
         * that the Element needs to be reprocessed for all attributes.
         */
        constexpr void fullyStain(){
            // Mark the element as dirty for all possible stain types to ensure
            // complete re-evaluation and rendering.
            this->Dirty.Dirty(
                INTERNAL::STAIN_TYPE::STRETCH | 
                INTERNAL::STAIN_TYPE::COLOR | INTERNAL::STAIN_TYPE::DEEP | 
                INTERNAL::STAIN_TYPE::EDGE | INTERNAL::STAIN_TYPE::MOVE
                // INTERNAL::STAIN_TYPE::FINALIZE // <- only constructors have the right to set this flag!
            );
        }
        
        /**
         * @brief Embeds styles into the current element and its child elements.
         * 
         * This function calls the Embed_Styles method on the current element's style,
         * passing the current element as a parameter. It then recursively calls the 
         * Embed_Styles method on each child element's style.
         */
        void embedStyles();
        
        /**
         * @brief Creates a new instance of the element class.
         * 
         * This virtual function is intended to be overridden by derived classes
         * to provide a mechanism for creating instances of the respective class.
         * By default, it creates and returns a new instance of the base `element` class.
         * 
         * @return A pointer to a newly created instance of the `element` class.
         */
        virtual element* createInstance() const {
            return new element();
        }
        
        /**
         * @brief Renders the element and its children into the Render_Buffer nested buffer of the window.
         * @details This function processes the element to generate a vector of UTF objects representing the current state.
         * It handles different stains such as CLASS, STRETCH, COLOR, and EDGE to ensure the element is rendered correctly.
         * @return A vector of UTF objects representing the rendered element and its children.
         */
        virtual std::vector<GGUI::UTF>& render();

        // Give thread::renderer() access to our private render method.
        friend void INTERNAL::renderer();
        
        // Give styling class access to some private methods.
        friend class styling;
        friend class STYLING_INTERNAL::styleBase;

        // Since protected methods can be accessed via the derived class only if it is as "this" pointer, so we need to give it access.
        friend class listView;
    
    // Some of the above mentioned functions be of help
    public:
        
        /**
         * @brief Provides access to embedStyles().
         *
         * This is a temporary workaround for managing RTTI during construction time.
         * The base class element performs style embedding if forced, which means the
         * virtual table (VT) has not yet been set to the derived class that is calling
         * the base class constructor.
         */
        void compile() { embedStyles(); }
        
    };
}

#endif
#ifndef _LIST_VIEW_H_
#define _LIST_VIEW_H_



namespace GGUI{
    namespace INTERNAL {
        extern void renderLogger(const std::string& problem);
    }

    class listView : public element{
    protected:
        //We can always assume that the list starts from the upper left corner, right?
        element* Last_Child = new element(position(0, 0) | width(0) | height(0));
    public:

        /**
         * @brief Default constructor for List_View.
         * 
         * This constructor calls the default constructor of Element and sets the Allow_Dynamic_Size property to true.
         */
        listView() : element(){ allowDynamicSize(true); }

        /**
         * @brief Constructor for List_View.
         * 
         * This constructor calls the Element constructor with the given style and then sets the Allow_Dynamic_Size property to true.
         * This is so that the list view can automatically resize itself based on the elements it contains.
         * 
         * @param s The style for the list view.
         */
        listView(STYLING_INTERNAL::styleBase& s, bool Embed_Styles_On_Construct = false) : element(s, Embed_Styles_On_Construct){ allowDynamicSize(true); }
        listView(STYLING_INTERNAL::styleBase&& s, bool Embed_Styles_On_Construct = false) : listView(s, Embed_Styles_On_Construct){}

        /**
         * @brief Destructor for the List_View class.
         *
         * This destructor is responsible for properly deallocating all the memory
         * allocated by the List_View object, including its child elements.
         */
        ~listView() override {
            // Delete all child elements to avoid memory leaks.
            for (element* e : Style->Childs) {
                delete e;
            }
        }

        //End of user constructors.

        /**
         * @brief Adds a child element to the list view.
         * @details This function adds a child element to the list view and manages the positioning and sizing
         *          of the child element within the list view. It takes into account the list's flow direction,
         *          border offsets, and dynamic sizing capabilities.
         * @param e The child element to be added.
         */
        void addChild(element* e) override;
        
        /**
         * @brief Gets the name of the list view.
         * @details This function returns the name of the list view in the format "List_View<Name>".
         * @return The name of the list view.
         */
        std::string getName() const override;

        /**
         * @brief Removes a child element from the list view.
         * @param remove The child element to be removed.
         * @return true if the element was successfully removed, false if not.
         *
         * This function removes a child element from the list view and updates the position of all elements following the removed element.
         * It also recalculates the width and height of the list view and updates the dimensions of the list view if it is dynamically sized.
         */
        bool remove(element* e) override;

        /**
         * @brief Sets the flow direction of the list view.
         * @details This function sets the flow priority of the list view to the specified direction.
         *          The flow direction determines how the child elements are arranged within the list view.
         * @param gd The direction to set as the flow priority.
         */
        void setFlowDirection(DIRECTION gd){
            Style->Flow_Priority = gd;
        }

        /**
         * @brief Gets the current flow direction of the list view.
         * @details This function returns the current flow direction of the list view.
         * @return The flow direction of the list view.
         */
        DIRECTION getFlowDirection(){
            return Style->Flow_Priority.value;
        }

        /**
         * @brief Gets a child element from the list view by its index.
         * @details This function returns a pointer to the child element at the specified index.
         *          The index is checked to be within the range of the child array.
         * @param index The index of the child element to retrieve.
         * @return The child element at the specified index, or nullptr if the index is out of range.
         */
        template<typename  T>
        T* get(int index){
            if (index > (signed)Style->Childs.size() - 1)
                return nullptr;

            if (index < 0)
                index = (signed)Style->Childs.size() + index - 1;

            return (T*)this->Style->Childs[index];
        }


    protected:
        /**
         * @brief Creates a deep copy of the List_View object.
         * @details This function creates a new List_View object and copies all the data from the current List_View object to the new one.
         * @return A pointer to the new List_View object.
         */
        element* createInstance() const override {
            return new listView();
        }

        /**
         * @brief Calculates the hitboxes of all child elements of the list view.
         * @details This function is similar to the Remove(Element* c) like behaviour.
         *          It takes into account the border offsets of both the current and the next element as well as their positions.
         *          For an horizontal list, it checks if the next element's width is greater than the current element's width.
         *          For a vertical list, it checks if the next element's height is greater than the current element's height.
         *          If the next element is greater in size than the current element, it sets the maximum width/height to the next element's width/height.
         *          It finally sets the dimensions of the list view to the maximum width and height if the list view is dynamically sized and the maximum width/height is greater than the current width/height.
         * @param Starting_Offset The starting offset into the child array.
         */
        void calculateChildsHitboxes(unsigned int Starting_Offset = 0) override;

        /**
         * @brief Retrieves the dimension limits of the list view.
         *
         * This function calculates and returns the dimension limits of the list view
         * based on its configuration. The behavior depends on whether overflow or 
         * dynamic sizing is allowed.
         *
         * @return GGUI::IVector3 The dimension limits of the list view:
         * - If overflow is allowed, the dimensions are unbounded and set to {INT16_MAX, INT16_MAX}.
         * - If dynamic sizing is allowed, the dimensions are calculated as the difference
         *   between the position of the last child element and the final limit.
         * - Otherwise, the dimensions are determined by the width and height of the list view.
         */
        IVector3 getDimensionLimit();

        // Give access to Last_Child
        friend class scrollView;
    };

    class scrollView : public element{
    protected:
        int Scroll_Index = 0;  // Render based on the offset of the scroll_index by flow direction.
    public:

        /**
         * @brief Default constructor for the scrollView class.
         * 
         * This constructor initializes a scrollView object by calling the base class
         * element's default constructor.
         */
        scrollView() : element() { allowOverflow(true); }

        /**
         * @brief Constructor for the Scroll_View class.
         * @details This constructor initializes a Scroll_View object with the specified styling.
         * @param s The styling to be applied to the Scroll_View.
         * @param Embed_Styles_On_Construct If true, the styling will be embedded into the Scroll_View's style. Only use if you know what you're doing!!!
         */
        scrollView(STYLING_INTERNAL::styleBase& s, bool Embed_Styles_On_Construct = false) : element(s, Embed_Styles_On_Construct) { allowOverflow(true); }
        scrollView(STYLING_INTERNAL::styleBase&& s, bool Embed_Styles_On_Construct = false) : scrollView(s, Embed_Styles_On_Construct){}

        /**
         * @brief Constructor for the Scroll_View class.
         * @details This constructor initializes a Scroll_View object with a reference to a List_View object.
         * @param container The List_View object to be used as the container for the Scroll_View.
         */
        scrollView(listView& container);

        /**
         * @brief Adds a child element to the Scroll_View.
         * @details This function adds a child element to the Scroll_View and marks the Scroll_View as dirty with the DEEP stain.
         * @param e The child element to be added.
         */
        void addChild(element* e) override;

        /**
         * @brief Enables or disables scrolling for the Scroll_View.
         * @details This function updates the scrolling capability of the Scroll_View.
         *          If scrolling is enabled, it ensures that scrolling events are registered.
         * @param allow A boolean indicating whether to enable or disable scrolling.
         */
        void allowScrolling(bool allow);
    
        /**
         * @brief Checks if the scrolling is enabled for the Scroll_View.
         * @details This function checks the value of the Allow_Scrolling property of the Scroll_View's styling.
         * @return A boolean indicating whether the scrolling is enabled for the Scroll_View.
         */
        bool isScrollingEnabled(){
            return Style->Allow_Scrolling.value;
        }

        /**
         * @brief Scrolls the view up by one index.
         * @details Decreases the scroll index if it is greater than zero and updates the container's position based on the growth direction.
         * Marks the view as dirty for a deep update.
         */
        void scrollUp() override;

        /**
         * @brief Scrolls the view down by one index.
         * @details Increases the scroll index by one and updates the container's position based on the growth direction.
         * Marks the view as dirty for a deep update.
         */
        void scrollDown() override;

        /**
         * @brief Removes a child element from the scroll view.
         * @details This function forwards the request to the Remove(Element* remove) function of the container.
         * @param remove The element to be removed.
         * @return true if the element was successfully removed, false if not.
         */
        bool remove(element* e) override;

        /**
         * @brief Gets the name of the scroll view.
         * @details This function returns the name of the scroll view.
         * @return The name of the scroll view.
         */
        std::string getName() const override;

        /**
         * @brief Sets the growth direction of the scroll view.
         * @details This function forwards the request to the Set_Flow_Direction(DIRECTION gd) function of the container.
         * @param gd The direction value to set as the growth direction.
         */
        void setGrowthDirection(DIRECTION gd){
            ((listView*)Style->Childs[0])->setFlowDirection(gd);
        }

        /**
         * @brief Gets the current growth direction of the scroll view.
         * @details This function retrieves the current growth direction of the scroll view.
         * @return The current growth direction of the scroll view.
         */
        DIRECTION getGrowthDirection(){
            return ((listView*)Style->Childs[0])->getFlowDirection();
        }

        /**
         * @brief Gets a child element from the scroll view by its index.
         * @details This function forwards the request to the Get(int index) function of the container.
         * @param index The index of the child element to retrieve.
         * @return The child element at the specified index, or nullptr if the index is out of range.
         */
        template<typename  T>
        T* get(int index){
            return ((listView*)Style->Childs[0])->get<T>(index);
        }

        /**
         * @brief Retrieves the child elements of the current element.
         * 
         * This function overrides the base class method to return a reference
         * to the vector of child elements managed by the container associated
         * with this element.
         * 
         * @return A reference to a vector of pointers to the child elements.
         */
        std::vector<element*>& getChilds() override {
            return getContainer()->getChilds();
        }

    protected:
        /**
         * @brief Safely moves the current element to a new scrollView element.
         * 
         * This function overrides the createInstance method from the base class and 
         * creates a new instance of the scrollView element.
         * 
         * @return A pointer to the newly created scrollView element.
         */
        element* createInstance() const override {
            return new scrollView();
        }

        /**
         * @brief Gets the container of the scroll view.
         * @details This function retrieves the container of the scroll view, which is a List_View.
         * @return The container of the scroll view.
         */
        listView* getContainer(){
            // If the container has not been yet initialized, do so.
            if (element::getChilds().size() == 0){
                allowOverflow(true);
                element::addChild(new listView(
                    name((getName() + "::container").c_str()) | 
                    flowPriority(element::getFlowPriority())
                ));
            }

            return (listView*)Style->Childs[0];
        }

        // This is only needed until auto scrolling has been implemented.
        friend void INTERNAL::renderLogger(const std::string&);
    };
}

#endif
#ifndef _TEXT_FIELD_H_
#define _TEXT_FIELD_H_



namespace GGUI{
    class textField : public element{
    protected:
        std::string Text = "";

        // This will hold the text by lines, and does not re-allocate memory for whole text, only for indicies.
        std::vector<INTERNAL::compactString> Text_Cache; 

        /**
         * @brief Updates the text cache list by newlines, and if no found then set the Text as the zeroth index.
         * @details This function will also determine the longest line length and store it in the class.
         * @note This function will also check if the lines can be appended to the previous line or not.
         */
        void updateTextCache();
    public:

        /**
         * @brief Constructor for the Text_Field class.
         * @details This constructor takes an optional Styling parameter, and an optional string parameter.
         *          If the string parameter is not given, it defaults to an empty string.
         *          If the Styling parameter is not given, it defaults to a Styling object with
         *          the default values for the Height and Width properties.
         *          The Styling parameter is used to set the style of the Text_Field object.
         * @param text The text to be displayed in the Text_Field object.
         * @param s The Styling object to use for the Text_Field object.
         * @param Embed_Styles_On_Construct If true, the styling will be embedded into the Text_Field's style. Only use if you know what you're doing!!!
         */
        textField(STYLING_INTERNAL::styleBase& s = STYLES::CONSTANTS::Default, bool Embed_Styles_On_Construct = false);
        
        /**
         * @brief Constructs a textField object with the specified style and optional embedding of styles.
         * 
         * @param s A rvalue reference to a styleBase object that defines the styling for the textField.
         * @param Embed_Styles_On_Construct A boolean flag indicating whether styles should be embedded during construction. 
         *        Defaults to false.
         */
        textField(STYLING_INTERNAL::styleBase&& s, bool Embed_Styles_On_Construct = false) : textField(s, Embed_Styles_On_Construct){}

        ~textField() override = default;

        /**
         * @brief Sets the text of the text field.
         * @details This function first stops the GGUI engine, then sets the text with a space character added to the beginning, and finally updates the text field's dimensions to fit the new text. The text is then reset in the Render_Buffer nested buffer of the window.
         * @param text The new text for the text field.
         */
        void setText(std::string text);

        /**
         * @brief Gets the text of the text field.
         * @details This function returns the string containing the text of the text field.
         * @return The text of the text field as a string.
         */
        std::string getText(){
            return Text;
        }

        /**
         * @brief Aligns text to the left within the text field.
         * @param Result A vector of UTF objects to store the aligned text.
         * @details This function iterates over each line in the Text_Cache and aligns them to the left side 
         *          of the text field. The function respects the maximum height and width of the text field 
         *          and handles overflow according to the Style settings.
         */
        void alignTextLeft(std::vector<UTF>& Result);
        
        /**
         * @brief Aligns text to the right within the text field.
         * @param Result A vector of UTF objects to store the aligned text.
         * @details This function iterates over each line in the Text_Cache and aligns them to the right side
         *          of the text field. The function respects the maximum height and width of the text field
         *          and handles overflow according to the Style settings.
         */
        void alignTextRight(std::vector<UTF>& Result);
        
        /**
         * @brief Aligns text to the center within the text field.
         * @param Result A vector of UTF objects to store the aligned text.
         * @details This function iterates over each line in the Text_Cache and aligns them to the center of the text field. The function respects the maximum height and width of the text field
         *          and handles overflow according to the Style settings.
         */
        void alignTextCenter(std::vector<UTF>& Result);

        /**
         * @brief Listens for input and calls a function when user presses any key.
         * @param Then A function that takes a character as input and does something with it.
         * @details This function creates three actions (for key press, enter, and backspace) that listen for input when the text field is focused. If the event is a key press or enter, it
         *          calls the Then function with the character as input. If the event is a backspace, it removes the last character from the text field. In all cases, it marks the text field as
         *          dirty and updates the frame.
         */
        void input(std::function<void(textField*, char)> Then);


    protected:
        /**
         * @brief Renders the text field into the Render_Buffer.
         * @details This function processes the text field to generate a vector of UTF objects representing the current state.
         * It handles different stains such as CLASS, STRETCH, COLOR, EDGE, and DEEP to ensure the text field is rendered correctly.
         * @return A vector of UTF objects representing the rendered text field.
         */
        std::vector<GGUI::UTF>& render() override;
        
        /**
         * @brief Creates a new instance of the textField element.
         * 
         * This method overrides the base class implementation to return
         * a dynamically allocated instance of the textField class.
         * 
         * @return A pointer to a newly created textField instance.
         */
        element* createInstance() const override {
            return new textField();
        }

        // switchBox uses textField::render() so lets give it some access.
        friend class switchBox;
    };
}

#endif
#ifndef _CANVAS_H_
#define _CANVAS_H_



#include <vector>

namespace GGUI{
    class sprite{
    protected:
        std::vector<GGUI::UTF> Frames;
        
        int Offset = 0;     // This is for more beautiful mass animation systems
        int Speed = 1;      // Using decimals too slow hmmm...
        
        int Frame_Distance = 1;
        
        bool Is_Power_Of_Two = false;
    public:

        /**
         * @brief Constructor for Sprite class.
         * @details This constructor initializes a Sprite object with a vector of UTF objects representing the frames,
         * an offset to determine when to start playing the animation, and a speed to control the animation playback.
         * @param frames A vector of UTF objects representing the frames of the animation.
         * @param offset The number of frames to skip before playing the animation.
         * @param speed The speed of the animation playback.
         */
        sprite(std::vector<GGUI::UTF> frames, int offset = 0, int speed = 1);

        /**
         * @brief Constructs a Sprite object with a single frame.
         * @details This constructor initializes the Sprite with a single UTF frame, setting the offset and speed for animation.
         * @param frame A UTF object representing the single frame of the sprite.
         * @param offset The number of frames to skip before playing the animation. Default is 0.
         * @param speed The speed of the animation playback. Default is 1.
         */
        sprite(GGUI::UTF frame){
            // Add the provided frame to the Frames vector.
            Frames.push_back(frame);
        }

        /**
         * @brief Constructs a Sprite object with default values.
         * @details This constructor sets the Sprite to have a single UTF frame, which is a space character, and sets the offset and speed for animation.
         */
        sprite() : Frame_Distance(1){
            // Set the default frame to a space character.
            Frames.push_back(GGUI::UTF(' '));
            
            // Set the default offset and speed values.
            Offset = 0;
            Speed = 1;
            
            // Set the Is_Power_Of_Two flag to false, indicating the sprite does not have a power of two size.
            Is_Power_Of_Two = false;
        }

        void setAnimationSpeed(int speed){
            Speed = speed;
        }

        void setOffset(int offset){
            Offset = offset;
        }

    protected:
        /**
         * @brief Renders a UTF character based on the sprite's current frame and speed.
         * @param Current_Frame The current frame of the animation.
         * @return The rendered UTF character.
         */
        UTF render(unsigned char Current_Time);

        friend class canvas;
    };

    class canvas : public element{
    private:
        // DONT GIVE THIS TO USER!!!
        canvas(){}
    protected:
        std::vector<sprite> Buffer;

        unsigned char Current_Animation_Frame = 0;

        // For speeding up sprite sets, to avoid redundant checks in unordered_maps.
        bool Multi_Frame = false;

        GGUI::sprite (*On_Draw)(unsigned int x, unsigned int y) = 0;
    public:
        /**
         * @brief Constructs a canvas element with the specified style and optional embedding of styles.
         * 
         * @param s A reference to a `STYLING_INTERNAL::styleBase` object that defines the style for the canvas.
         * @param Embed_Styles_On_Construct A boolean flag indicating whether styles should be embedded during construction. 
         *        Defaults to `false`.
         */
        canvas(STYLING_INTERNAL::styleBase& s, bool Embed_Styles_On_Construct = false) : element(s, Embed_Styles_On_Construct){}

        /**
         * @brief Constructs a canvas object with the specified style and optional embedding of styles.
         * 
         * @param s A rvalue reference to a `STYLING_INTERNAL::styleBase` object that defines the style for the canvas.
         * @param Embed_Styles_On_Construct A boolean flag indicating whether to embed styles during construction. 
         *        Defaults to false.
         */
        canvas(STYLING_INTERNAL::styleBase&& s, bool Embed_Styles_On_Construct = false) : canvas(s, Embed_Styles_On_Construct){}
        
        /**
         * @brief Destructor for the Terminal_Canvas class.
         * @details This destructor checks if the current Terminal_Canvas instance is part of the multi-frame list.
         *          If it is, it removes the instance from the list to properly manage resources.
         *          It then calls the base class destructor to ensure all parent class resources are cleaned up.
         */
        ~canvas() override;

        /**
         * @brief Advances the animation to the next frame.
         * 
         * This function increments the current animation frame counter,
         * allowing the animation to progress to the next frame in the sequence.
         */
        void setNextAnimationFrame() { Current_Animation_Frame++; }

        /**
         * @brief Set the sprite at the specified location on the terminal canvas.
         * @details This function places a sprite on the canvas at the given (x, y) coordinates.
         * The function also handles buffer resizing and multi-frame management.
         * @param x The x coordinate of the location.
         * @param y The y coordinate of the location.
         * @param sprite The sprite to be placed.
         * @param Flush Whether or not to call Update_Frame() after setting the sprite.
         */
        void set(unsigned int x, unsigned int y, sprite& sprite, bool Flush = true);

        /**
         * @brief Set the sprite at the specified location on the terminal canvas.
         * @details This function places a sprite on the canvas at the given (x, y) coordinates.
         * The function also handles buffer resizing and multi-frame management.
         * @param x The x coordinate of the location.
         * @param y The y coordinate of the location.
         * @param sprite The sprite to be placed.
         * @param Flush Whether or not to call Update_Frame() after setting the sprite.
         */
        void set(unsigned int x, unsigned int y, sprite&& sprite, bool Flush = true);

        /**
         * @brief Set the UTF sprite at the specified location on the terminal canvas.
         * @details This function places a UTF sprite on the canvas at the given (x, y) coordinates.
         * It also handles buffer resizing when necessary.
         * @param x The x coordinate of the location.
         * @param y The y coordinate of the location.
         * @param sprite The UTF sprite to be placed.
         * @param Flush Whether or not to call Update_Frame() after setting the sprite.
         */
        void set(unsigned int x, unsigned int y, UTF& sprite, bool Flush = true);
        
        /**
         * @brief Flush the canvas.
         * @details This function can be used to manually flush the canvas.
         * If Force_Flush is true, the canvas will be marked as dirty for color updates.
         * @param Force_Flush Whether or not to mark the canvas as dirty for color updates.
         */
        void flush(bool Force_Flush = false);

        /**
         * @brief Checks if the canvas is in a multi-frame state.
         * 
         * @return true if the canvas is in a multi-frame state, false otherwise.
         */
        bool isMultiFrame(){ return Multi_Frame; }

        /**
         * @brief Returns the name of the Terminal_Canvas as a string.
         * @details The returned string is a combination of the class name and the Name property.
         * @return A string representing the name of the Terminal_Canvas.
         */
        std::string getName() const override {
            // Concatenate class name and Name property to form the full name.
            return "canvas<" + Name + ">";
        }
    
        /**
         * @brief Embeds a vector of points into the canvas.
         * @param pixels A vector of points where the x and y coordinates are embedded in the vector by row major order.
         * @param border_style The style of border to use for constructing the bit masks.
         * @param flush Whether to flush the buffer after embedding the vector.
         * @return void
         * @details
         * This function takes a vector of points and embeds them into the canvas. The points are expected to be in row major order
         * and the vector should have a size equal to the usable area of the canvas. The function will then construct the bit masks
         * by analyzing the ways the points connect to each other. The bit masks are then used to construct the symbols on the canvas.
         * The symbols are looked up in the custom_border map based on the bit mask. If the symbol is not found in the map, the point is skipped.
         * The function will then set the points in the canvas to the corresponding symbol. If flush is true, the buffer is flushed after
         * the points are set.
         */
        void embedPoints(std::vector<bool> pixels, styledBorder border_style = GGUI::STYLES::BORDER::Single, bool Flush = true);

        /**
         * @brief Sets the callback function to be invoked during the drawing process.
         * 
         * This function allows you to specify a custom callback that will be called
         * for each pixel or coordinate during the drawing operation. The callback
         * should return a GGUI::sprite object based on the provided x and y coordinates.
         * 
         * @param on_draw A pointer to a function that takes two unsigned integers 
         *                (x and y coordinates) as input and returns a GGUI::sprite object.
         */
        void setOnDraw(GGUI::sprite (*on_draw)(unsigned int x, unsigned int y)){
            this->On_Draw = on_draw;
        }

    protected:
        /**
         * @brief Creates a deep copy of the Terminal_Canvas and returns it as a movable Element.
         * @return A deep copy of the Terminal_Canvas as a movable Element.
         */
        element* createInstance() const override {
            return new canvas();
        }
        
        /**
         * @brief Render the canvas.
         * @details This function renders the canvas, applying all the necessary transformations and color changes.
         *          It also handles the multi-frame list and sprite animations.
         * @return A vector of UTF objects representing the rendered canvas.
         */
        std::vector<GGUI::UTF>&  render() override;
    };

    namespace DRAW{

        /**
         * @brief Draws a line on the canvas.
         * @param x1 The x-coordinate of the first point.
         * @param y1 The y-coordinate of the first point.
         * @param x2 The x-coordinate of the second point.
         * @param y2 The y-coordinate of the second point.
         * @param pixels The vector of pixels of the canvas.
         * @param width The width of the canvas.
         * @details
         * This function draws a line on the canvas by setting the pixels to true.
         * It uses the Bresenham line drawing algorithm to determine which pixels to set.
         */
        void line(int x1, int y1, int x2, int y2, std::vector<bool>& pixels, int width);

        /**
         * @brief Helper function for the above, creates a line on a given buffer.
         * @param Start The starting point of the line.
         * @param End The ending point of the line.
         * @param Buffer_Width The width of the buffer.
         * @return A vector of booleans representing the line on the buffer.
         * @details
         * This function creates a line on a given buffer by setting the pixels to true.
         * It uses the Bresenham line drawing algorithm to determine which pixels to set.
         */
        std::vector<bool> line(FVector2 Start, FVector2 End, int Buffer_Width);

        /**
         * @brief Symmetrical circle draw helper.
         * @param x_center The x position of the center of the circle.
         * @param y_center The y position of the center of the circle.
         * @param x The current x position of the circle.
         * @param y The current y position of the circle.
         * @param pixels The vector of pixels of the canvas.
         * @param width The width of the canvas.
         * @details
         * This function is a helper function for drawing a circle on the canvas.
         * It fills in the circle symmetrically by setting the pixels to true.
         */
        void symmetryFillerForCircle(int x_center, int y_center, int x, int y, std::vector<bool>& pixels, int width);

        /**
         * @brief Fills a circle in a given buffer with true values.
         * @param x_center The x position of the center of the circle.
         * @param y_center The y position of the center of the circle.
         * @param r The radius of the circle.
         * @param pixels The buffer to fill.
         * @param width The width of the buffer.
         * @details
         * This function fills a circle in a given buffer with true values by
         * using the Bresenham circle drawing algorithm to determine which pixels
         * to set.
         */
        void circle(int x_center, int y_center, int r, std::vector<bool>& pixels, int width);

        /**
         * @brief Fills a circle in a given buffer with true values.
         * @param Center The center of the circle.
         * @param Radius The radius of the circle.
         * @param Buffer_Width The width of the buffer.
         * @return A boolean vector representing the circle.
         * @details
         * This function fills a circle in a given buffer with true values by
         * using the Bresenham circle drawing algorithm to determine which pixels
         * to set.
         */
        std::vector<bool> circle(FVector2 Center, int Radius, int Buffer_Width);

        /**
         * @brief Draws a cubic Bezier curve in a given buffer with true values.
         * @param P0 The first control point of the curve.
         * @param P1 The second control point of the curve.
         * @param P2 The third control point of the curve.
         * @param P3 The fourth control point of the curve.
         * @param Buffer_Width The width of the buffer.
         * @param pixels The boolean vector representing the buffer.
         * @details
         * This function draws a cubic Bezier curve in a given buffer with true values by
         * using the parametric equation of the Bezier curve to determine which pixels
         * to set.
         */
        void cubicBezierCurve(FVector2 P0, FVector2 P1, FVector2 P2, FVector2 P3, std::vector<bool>& pixels, int width);
        
        /**
         * @brief Draws a cubic Bezier curve in a given buffer with true values.
         * @param P0 The first control point of the curve.
         * @param P1 The second control point of the curve.
         * @param P2 The third control point of the curve.
         * @param P3 The fourth control point of the curve.
         * @param Buffer_Width The width of the buffer.
         * @return A boolean vector representing the buffer with true values where the curve is drawn.
         * @details
         * This function draws a cubic Bezier curve in a given buffer with true values by
         * using the parametric equation of the Bezier curve to determine which pixels
         * to set.
         */
        std::vector<bool> cubicBezierCurve(FVector2 P0, FVector2 P1, FVector2 P2, FVector2 P3, int Buffer_Width);

    }

    namespace FONT{
        // Based on: https://learn.microsoft.com/en-us/typography/opentype/spec/otff
        class fontHeader{
        public:
        };

        fontHeader parseFontFile(std::string File_Name);
    }

}

#endif
#ifndef _HTML_H_
#define _HTML_H_



#include <unordered_map>

namespace GGUI{

    class HTML : public element{
    private:
        // DONT GIVE TO USER !!!
        HTML(){}
    private:
        fileStream* Handle = nullptr;
    public:
    
        /**
         * @brief Constructor of the HTML class.
         * @param File_Name The name of the file to open.
         * 
         * This constructor will pause the GGUI renderer and create a new file stream
         * that will read the file and parse the HTML when it is changed.
         * The parsed HTML will be set as the child of this HTML object.
         */
        HTML(std::string File_Name);

        /**
         * @brief Destructor of the HTML class.
         * 
         * This destructor is responsible for properly deallocating all the memory
         * allocated by the HTML object.
         * It will also close the file stream associated with the HTML object.
         */
        ~HTML() override{
            if (Handle != nullptr){
                delete Handle;
            }
        }

        /**
         * @brief Creates a deep copy of the HTML object.
         * @return A pointer to the new HTML object.
         * 
         * This function will create a new HTML object and copy all the data from the current
         * HTML object to the new one. This is useful for creating a new HTML object that is
         * a modified version of the current one.
         */
        element* createInstance() const override {
            return new HTML();
        }

        /**
         * @brief Gets the name of the HTML object.
         * @return The name of the HTML object.
         * 
         * This function will return the name of the HTML object, which is the name
         * of the file that was opened using the HTML constructor.
         */
        std::string getName() const override{
            return "HTML<" + Name + ">";
        }
    };

    namespace INTERNAL{
        enum class HTML_GROUP_TYPES{
            UNKNOWN,
            TEXT,
            NUMBER,
            OPERATOR,   // =, 
            WRAPPER,    // <>, [], {}, (), "", ''
            SPACING,    // newline, ' ', '\t'
            ATTRIBUTE,  // Contains attributes as an wrapper extension. id="123"
        };

        enum class PARSE_BY{
            NONE                    = 0,
            TOKEN_WRAPPER           = 1 << 0,
            DYNAMIC_WRAPPER         = 1 << 1, 
            OPERATOR_PARSER         = 1 << 2,
            NUMBER_POSTFIX_PARSER   = 1 << 3,
        };

        enum class HTML_POSITION_TYPE{
            STATIC,     // Default positioning, like in GGUI.
            RELATIVE,   // Relative to parent.
            ABSOLUTE,   // Relative to screen.
            FIXED,      // Relative to screen, but does not move with scrolling.
            STICKY,     // Relative to screen until crosses given threshold.
        };

        /**
         * @brief Bitwise OR operator for PARSE_BY.
         * @details This function takes two PARSE_BY values and returns a new PARSE_BY value that is the result of the bitwise OR operation on the two input values.
         * @param first The first PARSE_BY value.
         * @param second The second PARSE_BY value.
         * @return The result of the bitwise OR operation on the two input values.
         */
        extern PARSE_BY operator|(PARSE_BY first, PARSE_BY second);

        /**
         * @brief Bitwise AND operator for PARSE_BY.
         * @details This function takes two PARSE_BY values and returns a new PARSE_BY value that is the result of the bitwise AND operation on the two input values.
         * @param first The first PARSE_BY value.
         * @param second The second PARSE_BY value.
         * @return The result of the bitwise AND operation on the two input values.
         */
        extern PARSE_BY operator&(PARSE_BY first, PARSE_BY second);

        /**
         * @brief Bitwise OR operator for PARSE_BY.
         * @details This function takes a PARSE_BY value and a PARSE_BY value and sets the first value to the result of the bitwise OR operation on the two input values.
         * @param first The first PARSE_BY value.
         * @param second The second PARSE_BY value.
         */
        extern void operator|=(PARSE_BY& first, PARSE_BY second);

        class HTMLToken{
        public:
            HTML_GROUP_TYPES Type = HTML_GROUP_TYPES::UNKNOWN;
            std::string Data = "";
            std::vector<HTMLToken*> Childs;    // also contains attributes!
            filePosition Position;

            PARSE_BY Parsed_By = PARSE_BY::NONE;

            /**
             * @brief Constructor for an HTML_Token.
             * @param Type The type of the token.
             * @param Data The data of the token.
             */
            HTMLToken(HTML_GROUP_TYPES type, std::string data) {
                this->Type = type;
                this->Data = data;
            }

            /**
             * @brief Constructor for an HTML_Token.
             * @param Type The type of the token.
             * @param Data The data of the token.
             * @param position The position of the token in the source file.
             */
            HTMLToken(HTML_GROUP_TYPES type, char data, filePosition position){
                this->Type = type;
                this->Data.push_back(data);
                this->Position = position;
            }

            /**
             * @brief Checks if the Parsed_By contains specific bit mask.
             * @details This function takes a PARSE_BY value and checks if the bit mask of that value is set in the Parsed_By of the current token.
             * @param f The PARSE_BY value to check.
             * @return True if the bit mask of the value is set, otherwise false.
             */
            bool is(PARSE_BY f){
                return (Parsed_By & f) == f;
            }

            /**
             * @brief Checks if the Parsed_By contains specific bit mask.
             * @details This function takes a PARSE_BY value and checks if the bit mask of that value is set in the Parsed_By of the current token.
             * @param f The PARSE_BY value to check.
             * @return True if the bit mask of the value is set, otherwise false.
             */
            bool has(PARSE_BY f) {
                return (f & Parsed_By) > PARSE_BY::NONE;
            }

            /**
             * @brief Default constructor for an HTML_Token.
             * @details This constructor does not initialize any values and should be used with caution.
             */
            HTMLToken() = default;
        };

        class HTMLGroup{
        public:
            HTML_GROUP_TYPES Type = HTML_GROUP_TYPES::UNKNOWN;
            char Start = 0;
            char End = 0;
            bool Is_Sticky = true;

            /**
             * @brief Constructor for an HTML_Group.
             * @details This constructor sets the type, start and end of the group.
             * @param Type The type of the group.
             * @param Start The start of the group.
             * @param End The end of the group.
             * @param Is_Sticky Is the group sticky?
             */
            HTMLGroup(HTML_GROUP_TYPES type, char start, char end, bool is_sticky = true){
                this->Type = type;
                this->Start = start;
                this->End = end;
                this->Is_Sticky = is_sticky;
            }
        };

        class HTMLNode{
        public:
            std::string Tag_Name = "";  // DIV, HREF, etc...
            
            std::vector<HTMLNode*> Childs;
            HTMLNode* parent = nullptr;

            filePosition Position;

            HTMLToken* RAW = nullptr;
            HTML_GROUP_TYPES Type = HTML_GROUP_TYPES::UNKNOWN;

            // Postfixes are in child[0] for numbers.
            // Decimals are also number typed.
            // Operators left is Child[0] and Right at Child[1].
            // Attributes cannot be computed, before some contextual data on AST level is constructed, since the postfix operands depend on these kind of information from parent.
            std::unordered_map<std::string, GGUI::INTERNAL::HTMLToken*> Attributes;    // contains ID, Name, Class, Color, BG_Color, etc...
        };

        /**
         * @brief Parses the HTML tokens.
         * @param Input The vector of tokens to parse.
         * 
         * This function parses the HTML tokens by combining wrappers like: <, >, (, ), etc...
         * It also captures decimals, parses operators in Reverse PEMDAS order, and combines dynamic wrappers like: <html>, </html>
         */
        extern void parse(std::vector<HTMLToken*>& Input);

        /**
         * @brief Parses raw HTML buffer into elements.
         * @param Raw_Buffer The raw HTML buffer to parse.
         * @param parent The parent element to set for top-level nodes.
         * @return A vector of parsed HTML elements.
         */
        extern std::vector<element*> parseHTML(std::string Raw_Buffer, element* parent);
        
        /**
         * @brief Parses the HTML tokens.
         * @param Input The vector of tokens to parse.
         * @return The parsed vector of HTML tokens.
         * 
         * This function parses the HTML tokens by combining wrappers like: <, >, (, ), etc...
         * It also captures decimals, parses operators in Reverse PEMDAS order, and combines dynamic wrappers like: <html>, </html>
         */
        extern std::vector<HTMLToken*>& parseHTML(std::vector<HTMLToken*>& Input);

        extern void parseEmbeddedBytes(int& i, std::vector<HTMLToken*>& Input);

        /**
         * @brief Parses all wrappers in the given vector of tokens.
         * @param i The index to start from.
         * @param Input The vector of tokens to parse.
         * 
         * This function parses all wrappers in the input, such as:
         * - <>
         * - []
         * - {}
         * - ()
         * - ""
         * - '
         */
        extern void parseAllWrappers(int& i, std::vector<HTMLToken*>& Input);

        /**
         * @brief Parses all wrappers in the given vector of tokens.
         * @param i The index to start from.
         * @param Input The vector of tokens to parse.
         * @param word The word which is used to identify the dynamic wrapper.
         * 
         * This function parses all wrappers in the input, such as:
         * - <>
         * - []
         * - {}
         * - ()
         * - ""
         * - '
         */
        extern void parseDynamicWrappers(int& i, std::vector<HTMLToken*>& Input, std::string word);

        /**
         * @brief Parses a wrapper token and all of its child tokens.
         * @param start_pattern The pattern that starts the wrapper.
         * @param end_pattern The pattern that ends the wrapper.
         * @param i The index of the start pattern in the input vector.
         * @param Input The input vector of tokens.
         * @details This function starts from the given index and every time it finds a start pattern it starts a new loop from the start pattern index until the end pattern count hits 0 and puts all of the tokens between the start and end pattern into the childs.
         *          The start pattern is not included in the child tokens.
         *          The end pattern is not included in the child tokens.
         *          The parsed by flag is set to the start pattern token.
         *          The function returns nothing, but modifies the input vector by deleting the tokens between the start and end pattern.
         *          If the start pattern is not found in the input vector, the function does nothing.
         *          If the nested count is still above 0 even after looping through all the tokens, just ignore lolw.
         */
        extern void parseWrapper(std::string start_pattern, std::string end_pattern, int& i, std::vector<HTMLToken*>& Input);

        extern const std::vector<HTMLGroup> Groups;

        /**
         * @brief Lexes the raw HTML string into a vector of HTML tokens.
         * @param Raw_Buffer The input HTML string to be tokenized.
         * @return A vector of pointers to HTML_Token objects.
         *
         * This function slices the given HTML text into tokens based on character groups.
         * It identifies different types of tokens such as text, numbers, operators, etc.
         * and returns a vector containing these tokens.
         */
        extern std::vector<HTMLToken*> lexHTML(std::string Raw_Buffer);

        /**
         * @brief Parses a vector of HTML tokens into HTML nodes.
         * @param Input A vector of pointers to HTML_Token objects to be parsed.
         * @return A vector of pointers to HTML_Node objects.
         *
         * This function processes each HTML token in the input vector, 
         * converts them into HTML nodes using the Factory function, 
         * and returns a vector containing these nodes. 
         * Only non-null nodes are added to the result vector.
         */
        extern std::vector<HTMLNode*> parseLexedTokens(std::vector<HTMLToken*> Input);

        extern std::unordered_map<std::string, std::function<GGUI::element* (HTMLNode*)>>* HTMLTranslators;

        extern std::unordered_map<std::string, double> POSTFIX_COEFFICIENT;

        extern std::unordered_map<std::string, void*> RELATIVE_COEFFICIENT;

        /**
         * @brief A simple hash function for strings.
         * @param str The string to be hashed.
         * @param h The current hash value.
         * @return The hashed string.
         * @details This function uses the djb2 hash algorithm to hash the given string.
         *          The hash value is computed in a way that is independent of the order of the characters in the string.
         *          The hash value is a 32-bit unsigned integer.
         */
        constexpr unsigned int hash(const char* str, int h = 0)
        {
            return !str[h] ? 5381 : (hash(str, h+1) * 33) ^ str[h];
        }

        /**
         * @brief Parses a vector of HTML nodes into elements.
         * @param Input A vector of pointers to HTML_Node objects to be parsed.
         * @return A vector of pointers to Element objects.
         *
         * This function processes each HTML node in the input vector, 
         * tries to find a translator fitting for the node's tag name, 
         * and if found, runs it. If the translator returns a non-null element, 
         * the element is added to the result vector. The translator is run in a 
         * paused GGUI state, meaning all events and updates are paused. After the 
         * translator has been run, the processed node is removed from the input vector.
         */
        extern std::vector<element*> parseTranslators(std::vector<HTMLNode*>& Input);

        /**
         * @brief Converts an HTML token into an HTML node.
         * @param Input The input HTML token to be converted.
         * @return A pointer to an HTML node.
         *
         * This function processes the given HTML token and converts it into an HTML node.
         * It checks if the child is just a text and if so sets the type of the node to be text.
         * Then it iterates over each child of the token and tries to find attributes.
         * If an attribute is found, it's added to the node's attribute list.
         * If the child is not an attribute, it's converted into an HTML node using this function recursively.
         * Finally, the function returns the HTML node.
         */
        extern HTMLNode* factory(HTMLToken* Input);

        /**
         * @brief Parses a postfix for a numeric token.
         * @param i The current index in the input vector.
         * @param Input The input vector of HTML tokens to be parsed.
         * 
         * This function processes the token at the given index in the input vector.
         * It checks if the token is a number, and if so, it checks if the next token is a postfix.
         * If the next token is a postfix, it adds it to the current token as a child and marks the current token as having a postfix.
         * Then it removes the postfix token from the input vector.
         */
        extern void parseNumericPostfix(int& i, std::vector<HTMLToken*>& Input);

        /**
         * @brief Parses a decimal number token.
         * @param i The current index in the input vector.
         * @param Input The input vector of HTML tokens to be parsed.
         * 
         * This function processes the token at the given index in the input vector.
         * If the token is a decimal number, it checks if the left and right side of the decimal token are numbers.
         * If they are, it checks if the decimal number is valid by trying to convert it to a double.
         * If the decimal number is valid, it creates a new token with the decimal value and replaces the current token with it.
         * If the decimal number is invalid, it reports an error.
         */
        extern void parseDecimal(int& i, std::vector<HTMLToken*>& Input);

        /**
         * @brief Parses an operator token and its surrounding tokens.
         * @param i The current index in the input vector.
         * @param Input The input vector of HTML tokens to be parsed.
         * @param operator_type The character representing the operator.
         *
         * This function processes the token at the given index in the input vector.
         * It checks if the token is an operator and not already parsed by the operator parser.
         * If the current token matches the operator type and is not already parsed, it adds the
         * left and right tokens as children of the operator token and marks it as parsed by the operator parser.
         * The function also sets the type of the token to ATTRIBUTE and removes the left and right tokens
         * from the input vector. The index is updated accordingly.
         */
        extern void parseOperator(int& i, std::vector<HTMLToken*>& Input, char operator_type);

        /**
         * @brief Reports an error to the user.
         * @param problem The error message to display.
         * @param location The location of the error in the file.
         *
         * This function appends the location of the error to the error message and
         * calls the GGUI::Report function to display the error to the user.
         */
        extern void report(std::string problem, filePosition location);

        /**
         * @brief Converts an Element to an HTML node.
         * @param e The element to convert.
         * @return A pointer to an HTML node.
         *
         * This function creates an HTML node from the given element.
         * It sets the tag name of the node to "div" and the type to wrapper.
         * Then it adds the attributes of the element to the node.
         * The width and height of the element are added as number type attributes.
         */
        extern HTMLNode* elementToNode(element* e);

        /**
         * @brief Computes the value of a given token.
         * @param val The token to compute the value of.
         * @param parent The parent node of the token.
         * @param attr_name The name of the attribute to compute the value of.
         * @return The computed value of the token.
         *
         * This function is called by the translator to compute the value of a token.
         * It checks if the token is an operator and if so, it calls the Compute_Operator
         * function to compute the value of the operator. If the token is not an operator, it
         * checks if the token has a postfix and if so, it calls the Compute_Post_Fix_As_Coefficient
         * function to compute the coefficient of the postfix. The computed value is then returned.
         */
        extern double computeVal(HTMLToken* val, HTMLNode* parent, std::string attr_name);

        /**
         * @brief Computes the result of an operator token.
         * 
         * This function evaluates an operator token by computing the values
         * of its left and right child nodes and applying the operator to
         * those values. Supported operators are +, -, *, /, and =.
         * 
         * @param op The operator token to evaluate.
         * @param parent The parent HTML node of the token.
         * @param attr_name The name of the attribute associated with the token.
         * @return The computed result of the operator.
         */
        extern double computeOperator(HTMLToken* op, HTMLNode* parent, std::string attr_name);

        /**
         * @brief Computes the result of a postfix token as a coefficient.
         * @details This function evaluates a postfix token as a coefficient by
         *          multiplying the coefficient value with the specified attribute
         *          value of the parent node or the parent's parent node.
         *          Supported postfixes are vw, vh, %, vmin, and vmax.
         * @param postfix The postfix token to evaluate.
         * @param parent The parent HTML node of the token.
         * @param attr_name The name of the attribute associated with the token.
         * @return The computed coefficient.
         */
        extern double computePostFixAsCoefficient(std::string postfix, HTMLNode* parent, std::string attr_name);

        /**
         * @brief Translates an HTML node's attributes to an Element.
         * @param e The Element to translate the attributes to.
         * @param input The HTML node to translate the attributes from.
         *
         * This function translates the attributes of the given HTML node to the given Element.
         * It sets the Element's width and height attributes using the Compute_Val function.
         * It also sets the Element's flexbox properties if the flex-direction attribute is set.
         */
        extern void translateAttributesToElement(element* e, HTMLNode* input);

        /**
         * @brief Translates the child nodes of an HTML node to an Element.
         * @param e The Element to which the child nodes are to be translated.
         * @param input The HTML node containing the child nodes.
         * @param Set_Text_To A pointer to a string where the resulting text will be set.
         *
         * This function processes each child node of the given HTML node.
         * If a child node is of type TEXT, its tag name is added to the Raw_Text vector.
         * If a child node has a tag name "br", a newline character is added to the Raw_Text vector.
         * If a translator is available for a child node's tag name, the child node is translated 
         * to an Element, and the Element is added as a child to the parent Element.
         * Finally, the concatenated text from the Raw_Text vector is set to the provided string pointer.
         */
        extern void translateChildsToElement(element* e, HTMLNode* input, std::string* Set_Text_To);
    }
}

#endif
#ifndef _PROGRESS_BAR_H_
#define _PROGRESS_BAR_H_



namespace GGUI{

    namespace progress{
        enum class partType{
            EMPTY,
            HEAD,
            BODY,
            TAIL
        };

        class part : public STYLING_INTERNAL::styleBase{
        public:
            INTERNAL::compactString character = INTERNAL::compactString(' ');
            RGB color = COLOR::GRAY;
            partType type = partType::EMPTY;

            constexpr part(partType t, RGB fillColor = COLOR::GREEN, INTERNAL::compactString cs = INTERNAL::compactString(' '), const VALUE_STATE Default = VALUE_STATE::VALUE) : styleBase(Default) { type = t; color = fillColor; character = cs; }

            constexpr part() = default;

            inline ~part() override { styleBase::~styleBase(); }

            inline styleBase* copy() const override {
                return new part(*this);
            }
            
            constexpr part& operator=(const part& other){
                // Only copy the information if the other is enabled.
                if (other.status >= status){
                    character = other.character;
                    color = other.color;
                    type = other.type;

                    status = other.status;
                }
                return *this;
            }

            constexpr part(const part& other) : styleBase(other.status), character(other.character), color(other.color), type(other.type) {}

            INTERNAL::STAIN_TYPE embedValue([[maybe_unused]] styling* host, element* owner) override;

            inline void evaluate([[maybe_unused]] const styling* self, [[maybe_unused]] const styling* owner) override {};
        };


        class Bar : public element{
        protected:
            float Progress = 0; // 0.0 - 1.0

            INTERNAL::compactString Head = INTERNAL::compactString('>');
            INTERNAL::compactString Body = INTERNAL::compactString('-');
            INTERNAL::compactString Tail = INTERNAL::compactString('|');
            INTERNAL::compactString Empty = INTERNAL::compactString(' ');

            RGB Head_Color = GGUI::COLOR::LIGHT_GRAY;
            RGB Body_Color = GGUI::COLOR::GRAY;
            RGB Tail_Color = GGUI::COLOR::GRAY;
            RGB Empty_Color = GGUI::COLOR::DARK_GRAY;

            std::vector<UTF> Content;
        public:

            /**
             * @brief Constructor for Progress_Bar.
             *
             * This constructor calls the Element constructor with the given style and
             * initializes the Progress_Bar object with default values.
             *
             * @param s The style for the Progress_Bar.
             * @param Embed_Styles_On_Construct If true, the styling will be embedded into the Progress_Bar's style. Only use if you know what you're doing!!!
             */
            Bar(STYLING_INTERNAL::styleBase& s, bool Embed_Styles_On_Construct = false) : element(s, Embed_Styles_On_Construct){
                Progress = 0.0f;
            }
            
            /**
             * @brief Constructs a Bar object with the given style and optional embedding of styles.
             * 
             * @param s A rvalue reference to a STYLING_INTERNAL::styleBase object that defines the style for the Bar.
             * @param Embed_Styles_On_Construct A boolean flag indicating whether to embed styles during construction. 
             *        Defaults to false.
             */
            Bar(STYLING_INTERNAL::styleBase&& s, bool Embed_Styles_On_Construct = false) : Bar(s, Embed_Styles_On_Construct){}

            /**
             * @brief Default constructor for Progress_Bar.
             *
             * This constructor is explicitly defined as default, which means that the compiler will generate a default implementation for it.
             * This is needed because otherwise, the compiler would not generate a default constructor for this class, since we have a user-declared constructor.
             */
            Bar() = default;

            void setHeadCharacter(INTERNAL::compactString cs) { Head = cs; }
            void setBodyCharacter(INTERNAL::compactString cs) { Body = cs; }
            void setTailCharacter(INTERNAL::compactString cs) { Tail = cs; }
            void setEmptyCharacter(INTERNAL::compactString cs) { Empty = cs; }

            void setHeadColor(RGB color) { Head_Color = color; }
            void setBodyColor(RGB color) { Body_Color = color; }
            void setTailColor(RGB color) { Tail_Color = color; }
            void setEmptyColor(RGB color) { Empty_Color = color; }

            /**
             * @brief Returns the index of the head of the progress bar.
             * @details
             * This function returns the index of the head of the progress bar. The head is the character that is drawn at the end of the progress bar when it is not full.
             * The index is calculated by multiplying the width of the progress bar (minus the border on both sides) by the progress value.
             * The result is then rounded down to the nearest integer using the floor() function.
             * @return The index of the head of the progress bar.
             */
            unsigned int getIndexofHead();

            /**
             * @brief Colors the bar with the current progress value.
             * @details
             * This function colors the progress bar with the current progress value. It first colors the empty part of the bar, then fills in the progressed part, and finally replaces the head and tail parts.
             */
            void colorBar();

            /**
             * @brief Sets the progress value of the progress bar.
             * @details This function updates the progress value of the progress bar. 
             * If the given value exceeds 1.0, a warning is reported, and the function returns without updating.
             * It also updates the color of the progress bar and marks the render buffer as dirty.
             * @param New_Progress The new progress value to set (should be between 0.0 and 1.0).
             */
            void setProgress(float New_Progress);
            
            /**
             * @brief Returns the current progress value of the progress bar.
             * @details This function returns the current progress value of the progress bar, which is a float between 0.0 and 1.0.
             * @return The current progress value of the progress bar.
             */
            float getProgress();

            /**
             * @brief Updates the progress of the progress bar by adding the specified value.
             * 
             * This function increments the current progress by the given value, ensuring
             * that the progress does not exceed 1.0f. If the progress exceeds 1.0f after
             * adding the value, the function returns without making any changes.
             * 
             * After updating the progress, the function updates the color of the progress
             * bar, marks the render buffer as dirty to reflect the changes, and triggers
             * a frame update to re-render the progress bar.
             * 
             * @param add The value to add to the current progress. Should be a float
             *            between 0.0f and 1.0f.
             */
            void updateProgress(float add);

            /**
             * @brief Toggles the border visibility of the progress bar.
             * @details This function toggles the border visibility of the progress bar.
             *          If the state has changed, it updates the border enabled state, marks the element as dirty for border changes, and updates the frame.
             * @param b The desired state of the border visibility.
             */
            void showBorder(bool state) override;

            /**
             * @brief Destructor for the Progress_Bar class.
             *
             * This destructor is responsible for properly deallocating all the memory
             * allocated by the Progress_Bar object. It calls the base class destructor
             * to ensure all parent class resources are also cleaned up.
             */
            ~Bar() override = default;

            /**
             * @brief Returns the name of the Progress_Bar object.
             * @details This function returns a string that represents the name of the Progress_Bar object.
             *          The name is constructed by concatenating the name of the Progress_Bar with the 
             *          class name "Progress_Bar", separated by a "<" and a ">".
             * @return The name of the Progress_Bar object.
             */
            std::string getName() const override{
                return "progressBar<" + Name + ">";
            }

        protected:
            /**
             * @brief Renders the progress bar into the Render_Buffer.
             * @details This function processes the progress bar to generate a vector of UTF objects representing the current state.
             * It handles different stains such as CLASS, STRETCH, COLOR, EDGE, and DEEP to ensure the progress bar is rendered correctly.
             * @return A vector of UTF objects representing the rendered progress bar.
             */
            std::vector<GGUI::UTF>& render() override;
            
            /**
             * @brief Creates a deep copy of the Progress_Bar object.
             * @details This function creates a new Progress_Bar object and copies all the data from the current Progress_Bar object to the new one.
             *          This is useful for creating a new Progress_Bar object that is a modified version of the current one.
             * @return A pointer to the new Progress_Bar object.
             */
            element* createInstance() const override {
                return new Bar();
            }
        };
    }
}

#endif
#ifndef _SWITCH_H_
#define _SWITCH_H_

#include <vector>
#include <string>


namespace GGUI{

    class visualState : public STYLING_INTERNAL::styleBase {
    protected:
        const INTERNAL::compactString *Off, *On;
    public:

        /**
         * @brief Constructs a visualState object with specified off and on states and a default value state.
         * 
         * @param off A compact string representing the "off" visual state.
         * @param on A compact string representing the "on" visual state.
         * @param Default The default value state, which is of type VALUE_STATE. Defaults to VALUE_STATE::VALUE.
         */
        constexpr visualState(const INTERNAL::compactString& off, const INTERNAL::compactString& on, const VALUE_STATE Default = VALUE_STATE::VALUE) : styleBase(Default), Off(&off), On(&on) {}

        /**
         * @brief Constructs a `visualState` object as a constexpr by copying the values from another `GGUI::visualState` object.
         * 
         * @param other The `GGUI::visualState` object to copy from. Its `status`, `Off`, and `On` properties are used to initialize the new object.
         */
        constexpr visualState(const GGUI::visualState& other) : styleBase(other.status), Off(other.Off), On(other.On) {}

        /**
         * @brief Destructor for the visualState class.
         * 
         * This inline destructor explicitly calls the destructor of the base class
         * styleBase to ensure proper cleanup of resources. The override specifier
         * indicates that this destructor overrides a virtual destructor in the base class.
         */
        inline ~visualState() override { styleBase::~styleBase(); }

        /**
         * @brief Creates a copy of the current visualState object.
         * 
         * This method overrides the copy function from the base class
         * and returns a dynamically allocated copy of the current 
         * visualState instance. The caller is responsible for managing 
         * the memory of the returned object.
         * 
         * @return styleBase* A pointer to a newly created copy of the 
         * current visualState object.
         */
        inline styleBase* copy() const override {
            return new visualState(*this);
        }

        /**
         * @brief Overloaded assignment operator for the visualState class.
         * 
         * This operator assigns the values from another visualState object to the current object,
         * but only if the `status` of the other object is greater than or equal to the `status` 
         * of the current object. If the condition is met, it copies the `Off` and `On` states 
         * as well as the `status` value from the other object.
         * 
         * @param other The visualState object to copy from.
         * @return A reference to the current visualState object after assignment.
         */
        constexpr visualState& operator=(const visualState& other){
            // Only copy the information if the other is enabled.
            if (other.status >= status){
                Off = other.Off;
                On = other.On;

                status = other.status;
            }
            return *this;
        }

        inline void evaluate([[maybe_unused]] const styling* self, [[maybe_unused]] const styling* owner) override {};

        INTERNAL::STAIN_TYPE embedValue(styling* host, element* owner) override;
    };

    class singleSelect : public STYLING_INTERNAL::styleBase {
    public:
        /**
         * @brief Constructs a singleSelect object with a default value state.
         * 
         * @param Default The default value state for the singleSelect object. 
         *                It is of type VALUE_STATE and defaults to VALUE_STATE::VALUE.
         */
        constexpr singleSelect(const VALUE_STATE Default = VALUE_STATE::VALUE) : styleBase(Default) {}

        /**
         * @brief Constructs a `singleSelect` object as a constexpr copy of another `singleSelect` object.
         * 
         * @param other The `singleSelect` object to copy from.
         * 
         * This constructor initializes the `singleSelect` object by copying the `status` 
         * from the `styleBase` of the provided `other` object. It is marked as `constexpr`, 
         * allowing it to be evaluated at compile time if the input is also a constant expression.
         */
        constexpr singleSelect(const GGUI::singleSelect& other) : styleBase(other.status) {}

        /**
         * @brief Destructor for the singleSelect class.
         * 
         * This inline destructor explicitly calls the destructor of the base class
         * styleBase to ensure proper cleanup of resources associated with the base class.
         */
        inline ~singleSelect() override { styleBase::~styleBase(); }

        /**
         * @brief Creates a copy of the current singleSelect object.
         * 
         * This method overrides the copy function from the base class 
         * and returns a dynamically allocated copy of the current 
         * singleSelect instance. The caller is responsible for managing 
         * the memory of the returned object.
         * 
         * @return styleBase* A pointer to a new singleSelect object 
         *         that is a copy of the current instance.
         */
        inline styleBase* copy() const override {
            return new singleSelect(*this);
        }

        /**
         * @brief Overloaded assignment operator for the singleSelect class.
         * 
         * This operator assigns the state of another singleSelect object to the current object.
         * The assignment only occurs if the `status` of the other object is greater than or 
         * equal to the `status` of the current object. This ensures that only valid or higher 
         * priority states are copied.
         * 
         * @param other The singleSelect object to copy from.
         * @return A reference to the current singleSelect object after assignment.
         */
        constexpr singleSelect& operator=(const singleSelect& other){
            // Only copy the information if the other is enabled.
            if (other.status >= status){
                status = other.status;
            }
            return *this;
        }

        inline void evaluate([[maybe_unused]] const styling* self, [[maybe_unused]] const styling* owner) override {};

        INTERNAL::STAIN_TYPE embedValue(styling* host, element* owner) override;
    };

    class switchBox : public element{
    protected:
        bool State = false;
        bool SingleSelect = false;   // Represents whether switching this box should disable other single selected switchBoxes under the same parent.

        //Contains the unchecked version of the symbol and the checked version.
        const INTERNAL::compactString *Off = nullptr, *On = nullptr;

        textField Text;
    public:
        /**
         * @brief Constructs a Switch element with specified text, states, event handler, and styling.
         * @param s The styling for the switch.
         * @param Embed_Styles_On_Construct If true, the styling will be embedded into the switch's style. Only use if you know what you're doing!!!
         */
        switchBox(STYLING_INTERNAL::styleBase& s = STYLES::CONSTANTS::Default, bool Embed_Styles_On_Construct = false);

        /**
         * @brief Constructs a switchBox object with the given style and optional embedding of styles.
         * 
         * @param s A rvalue reference to a STYLING_INTERNAL::styleBase object that defines the styling for the switchBox.
         * @param Embed_Styles_On_Construct A boolean flag indicating whether to embed styles during construction. 
         *        Defaults to false.
         */
        switchBox(STYLING_INTERNAL::styleBase&& s, bool Embed_Styles_On_Construct = false) : switchBox(s, Embed_Styles_On_Construct){}

        /**
         * @brief Destructor for the switchBox class.
         *
         * This destructor ensures that the base class destructor for `element` 
         * is explicitly called to properly clean up resources associated with 
         * the base class.
         */
        ~switchBox() override = default;

        /**
         * @brief Toggles the state of the switch.
         * @details Flips the current state from checked to unchecked or vice versa,
         * and marks the switch as needing a state update.
         */
        void toggle();

        /**
         * @brief Sets the state of the switch box and updates its visual representation.
         * 
         * This function changes the internal state of the switch box to the specified
         * boolean value, marks the state as dirty for internal processing, and updates
         * the frame to reflect the new state.
         * 
         * @param b The new state to set for the switch box. `true` represents an active
         *          state, while `false` represents an inactive state.
         */
        void setState(bool b);

        /**
         * @brief Enables single selection mode for the switchBox.
         * 
         * When single selection mode is enabled, the switchBox ensures that 
         * only one option can be selected at a time.
         */
        void enableSingleSelect();

        /**
         * @brief Checks if the switch element is in single-select mode.
         * 
         * @return true if the switch is in single-select mode, false otherwise.
         */
        bool isSingleSelect() { return SingleSelect; }

        /**
         * @brief Checks if the switch element is currently selected.
         * 
         * @return true if the switch is selected, false otherwise.
         */
        bool isSelected() { return State; }

        /**
         * @brief Sets the text of the switch element.
         * @details This function sets the text of the switch element by first pausing the GGUI engine, then setting the text with a space character added to the beginning, and finally updating the switch element's dimensions to fit the new text. The text is then reset in the Render_Buffer nested buffer of the window.
         * @param text The new text for the switch element.
         */
        void setText(INTERNAL::compactString text);

        /**
         * @brief Toggles the visibility of the border for the switchBox element.
         * 
         * This function enables or disables the border of the switchBox element
         * based on the provided boolean value. When the border state changes, 
         * the width and height of the element are adjusted accordingly to 
         * accommodate the border. The element is marked as dirty to reflect 
         * the border changes, and a frame update is triggered to re-render 
         * the element.
         * 
         * @param b A boolean value indicating whether the border should be 
         *          enabled (true) or disabled (false).
         */
        void showBorder(bool b) override;

        /**
         * @brief Returns the name of the Switch object.
         * @details This function returns a string that represents the name of the Switch object.
         *          The name is constructed by concatenating the name of the Switch with the 
         *          class name "Switch", separated by a "<" and a ">".
         * @return The name of the Switch object.
         */
        std::string getName() const override{
            return "switchBox<" + Name + ">";
        }

        /**
         * @brief Retrieves the string representation of the current state.
         * 
         * This function returns a compact string that represents the current
         * state of the switch. If the state is `true`, it returns the string
         * pointed to by `On`. Otherwise, it returns the string pointed to by `Off`.
         * 
         * @return INTERNAL::compactString The string representation of the current state.
         */
        constexpr INTERNAL::compactString getStateString() const {
            return State ? *On : *Off;
        }

        /**
         * @brief Sets the state strings for the switch box.
         * 
         * This function assigns the provided strings to represent the "off" and "on" states
         * of the switch box. It also marks the switch as needing a state update and refreshes
         * its frame to reflect the changes.
         * 
         * @param off Pointer to a compactString representing the "off" state.
         * @param on Pointer to a compactString representing the "on" state.
         */
        void setStateString(const INTERNAL::compactString* off, const INTERNAL::compactString* on);

    protected:
        /**
         * @brief Renders the switch element and its children into the Render_Buffer nested buffer of the window.
         * @details This function processes the switch element to generate a vector of UTF objects representing the current state.
         * It handles different stains such as CLASS, STRETCH, COLOR, EDGE, and DEEP to ensure the switch element is rendered correctly.
         * @return A vector of UTF objects representing the rendered switch element.
         */
        std::vector<GGUI::UTF>& render() override;
        
        /**
         * @brief Creates a deep copy of the Switch object.
         * @details This function creates a new Switch object and copies all the data from the current Switch object to the new one.
         *          This is useful for creating a new Switch object that is a modified version of the current one.
         * @return A pointer to the new Switch object.
         */
        element* createInstance() const override {
            return new switchBox();
        }
    };

    class radioButton : public switchBox{
    public:
        /**
         * @brief Constructs a radioButton element with optional custom styling and embedding behavior.
         *
         * This constructor initializes a radioButton, inheriting from switchBox, with the specified style and visual state.
         * The visual state is set to display the appropriate radio button symbols for "off" and "on" states.
         *
         * @param s The style to apply to the radioButton. Defaults to STYLES::CONSTANTS::Default.
         * @param Embed_Styles_On_Construct If true, embeds the styles during construction. Defaults to false.
         */
        radioButton(STYLING_INTERNAL::styleBase& s = STYLES::CONSTANTS::Default, bool Embed_Styles_On_Construct = false) : switchBox(s | visualState(SYMBOLS::RADIOBUTTON_OFF, SYMBOLS::RADIOBUTTON_ON), Embed_Styles_On_Construct) {}
        
        /**
         * @brief Constructs a radioButton object with the specified style and optional embedding of styles.
         * 
         * @param s A `STYLING_INTERNAL::styleBase` object representing the style to be applied to the radio button.
         * @param Embed_Styles_On_Construct A boolean flag indicating whether to embed styles during construction. 
         *        Defaults to `false`.
         */
        radioButton(STYLING_INTERNAL::styleBase&& s, bool Embed_Styles_On_Construct = false) : radioButton(s, Embed_Styles_On_Construct){}

        /**
         * @brief Returns the state of the Radio_Button.
         * @details This function returns a boolean value indicating whether the Radio_Button is turned on or off.
         *          The state is represented by the Switch::State property.
         * @return The state of the Radio_Button.
         */
        bool getState(){
            return State;
        }
        
        /**
         * @brief Returns the name of the Radio_Button object.
         * @details This function returns a string that represents the name of the Radio_Button object.
         *          The name is constructed by concatenating the name of the Radio_Button with the 
         *          class name "Radio_Button", separated by a "<" and a ">".
         * @return The name of the Radio_Button object.
         */
        std::string getName() const override{
            // Return the formatted name of the Radio_Button.
            return "radioButton<" + Name + ">";
        }

        // Diabled, use the switchBox class type for search
        // element* createInstance() const override {
        //     return new radioButton();
        // }
    };

    class checkBox : public switchBox{
    public:
        /**
         * @brief Constructs a checkBox element with optional styling and embedding behavior.
         *
         * This constructor initializes a checkBox by applying the provided style and visual states
         * for checked and unchecked symbols. It also allows specifying whether to embed styles upon construction.
         *
         * @param s The style to apply to the checkBox. Defaults to STYLES::CONSTANTS::Default.
         * @param Embed_Styles_On_Construct If true, embeds styles during construction. Defaults to false.
         */
        checkBox(STYLING_INTERNAL::styleBase& s = STYLES::CONSTANTS::Default, bool Embed_Styles_On_Construct = false) : switchBox(s | visualState({SYMBOLS::EMPTY_CHECK_BOX, SYMBOLS::CHECKED_CHECK_BOX}), Embed_Styles_On_Construct) {}
        
        /**
         * @brief Constructor for the checkBox class.
         * 
         * This constructor initializes a checkBox object with the given style and an optional flag
         * to embed styles during construction. It forwards the parameters to another checkBox
         * constructor for initialization.
         * 
         * @param s A style object of type STYLING_INTERNAL::styleBase, used to define the appearance of the checkBox.
         * @param Embed_Styles_On_Construct A boolean flag indicating whether to embed styles during construction. 
         *                                   Defaults to false.
         */
        checkBox(STYLING_INTERNAL::styleBase&& s, bool Embed_Styles_On_Construct = false) : checkBox(s, Embed_Styles_On_Construct) {}

        /**
         * @brief Returns the current state of the Check_Box.
         * @details This function returns a boolean indicating whether the Check_Box is checked or unchecked.
         * @return The state of the Check_Box.
         */
        bool getState(){
            return State; // Return the current state of the Check_Box.
        }
        
        /**
         * @brief Returns the name of the Check_Box object.
         * @details This function returns a string that represents the name of the Check_Box object.
         *          The name is constructed by concatenating the name of the Check_Box with the 
         *          class name "Check_Box", separated by a "<" and a ">".
         * @return The name of the Check_Box object.
         */
        std::string getName() const override{
            return "checkBox<" + Name + ">";
        }

        // Disabled, use the switchBox class type.
        // element* createInstance() const override {
        //     return new checkBox();
        // }
    };

    namespace INTERNAL{
        /**
         * @brief Disables other switches in the same group, keeping only the specified switch active.
         * 
         * This function ensures that if the provided switchBox (`keepOn`) is part of a group
         * and is marked as single-select, it will remain active while all other single-select
         * switches in the same group are disabled. If the switch is not part of a group or is
         * not single-select, it toggles the state of the provided switchBox.
         * 
         * @param keepOn Pointer to the switchBox that should remain active or be toggled.
         * 
         * @note If the `keepOn` switchBox does not have a parent or is not part of a group,
         *       the function will simply toggle its state.
         */
        void DisableOthers(switchBox* keepOn);
    }
}

#endif
#ifndef _RENDERER_H_
#define _RENDERER_H_

#undef min
#undef max

#include <functional>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>



//GGUI uses the ANSI escape code
//https://en.wikipedia.org/wiki/ANSI_escape_code
namespace GGUI{

    
    /**
     * @brief Register a user defined cleanup callback to be invoked during de-initialization.
     * @details Callbacks are executed after internal threads are joined but before terminal state is restored.
     */
    extern void registerCleanupCallback(std::function<void()> Callback);

    /**
     * @brief Gracefully shuts down the application.
     *
     * This function performs a series of steps to gracefully shut down the application:
     * 1. Logs the initiation of the termination process.
     * 2. Signals subthreads to terminate.
     * 3. Waits for all subthreads to join.
     * 4. Reverts the console to its normal mode.
     * 5. Cleans up platform-specific resources and settings.
     * 6. Logs the successful shutdown of the application.
     * 7. Exits the application with the specified exit code.
     *
     * @param signum The exit code to be used when terminating the application.
     */
    extern void EXIT(int Signum = 0);

    /**
     * @brief Returns the Main element of the GGUI system.
     * @details This function returns the main element of the GGUI system, which is the root element of the GUI.
     * @return A pointer to the main element of the GGUI system.
     */
    extern element* getRoot();

    /**
     * @brief Updates the frame.
     * @details This function updates the frame. It's the main entry point for the rendering thread.
     * @note This function will return immediately if the rendering thread is paused.
     */
    extern void updateFrame();
    
    /**
     * @brief Pauses the rendering thread.
     * @details This function pauses the rendering thread. The thread will wait until the rendering thread is resumed.
     */
    extern void pauseGGUI();

    /**
     * @brief Resumes the rendering thread.
     * @details This function resumes the rendering thread after it has been paused.
     * @param restore_render_to The status to restore the rendering thread to.
     */
    extern void resumeGGUI();

    /**
     * @brief Pauses all other GGUI internal threads and calls the given function.
     * @details This function will pause all other GGUI internal threads and call the given function.
     * @param f The function to call.
     */
    extern void pauseGGUI(std::function<void()> f);

    /**
     * @brief Use GGUI in a simple way.
     * @details This is a simple way to use GGUI. It will pause all other GGUI internal threads, initialize GGUI, add all the elements to the root window, sleep for the given amount of milliseconds, and then exit GGUI.
     * @param App The whole GGUI Application that GGUI holds.
     * @param Sleep_For The amount of milliseconds to sleep after calling the given function.
     */
    extern void GGUI(STYLING_INTERNAL::styleBase& App, unsigned long long Sleep_For = 0);

    /**
     * @brief Calls the GGUI function with the provided style and sleep duration.
     *
     * This function forwards the given style object and sleep duration to another
     * overload of the GGUI function. It is typically used to initialize or update
     * the graphical user interface with specific styling and timing parameters.
     *
     * @param App An rvalue reference to a STYLING_INTERNAL::style_base object representing the application's style.
     * @param Sleep_For The duration, in microseconds, for which the function should sleep or delay execution.
     */
    extern void GGUI(STYLING_INTERNAL::styleBase&& App, unsigned long long Sleep_For = 0);

    /**
    * @brief Retrieves an element by name.
    * @details This function takes a string argument representing the name of the element
    *          and returns a pointer to the element if it exists in the global Element_Names map.
    * @param name The name of the element to retrieve.
    * @return A pointer to the element if it exists; otherwise, nullptr.
    */
    extern element* getElement(std::string name);

    /**
     * @brief Retrieves a vector of pointers to elements of type T.
     * 
     * This template function delegates the retrieval of elements to the INTERNAL::Main object.
     * It returns a std::vector containing pointers to elements of the specified type T.
     * 
     * @tparam T The type of elements to retrieve.
     * @return std::vector<T*> A vector of pointers to elements of type T.
     */
    template<typename T>
    std::vector<T*> getElements(){
        return getRoot()->getElements<T>();
    }
}

#endif
#ifndef _ADDONS_H_
#define _ADDONS_H_

namespace GGUI{
    
    /**
     * @brief Initializes all addons and adds them to the main internal structure.
     *
     * This function first calls the initializer for the inspect tool addon.
     * After all addons are loaded, it iterates through the list of addons
     * and adds each one to the main internal structure.
     */
    extern void initAddons();

    /**
     * @brief Initializes the inspect tool.
     * @details This function initializes the inspect tool which is a debug tool that displays the number of elements, render time, and event time.
     * @see GGUI::Update_Stats
     */
    extern void initInspectTool();
}

#endif
