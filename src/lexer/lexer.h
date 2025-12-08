#ifndef _lexer_h_
#define _lexer_h_

/**
 * Takes std::string or a file as an input and outputs nested lexed::tokens.
 */

#include <string>
#include <vector>
#include <mutex>

namespace parser {
    namespace token {
        class base;
    }
}

namespace lexer{
    // contains all token-related utilities.
    namespace token{

        // represents the origin of said token
        class position{
        public:
            unsigned short x;       // represents the character offset
            unsigned short y;       // represents the line number
            unsigned short file_id; // points to an offset for docked files.

            position(unsigned short x_value = 0, unsigned short y_value = 0, unsigned short file_id_value = 0) : x(x_value), y(y_value), file_id(file_id_value) {}

            bool operator==(const position& other) const noexcept {
                return x == other.x && y == other.y && file_id == other.file_id;
            }

            bool operator<(const position& other) const noexcept {
                if (file_id != other.file_id) return file_id < other.file_id;
                if (y != other.y) return y < other.y;
                return x < other.x;
            }

            bool operator<=(const position& other) const noexcept {
                return *this < other || *this == other;
            }

            bool operator>(const position& other) const noexcept {
                return !(*this <= other);
            }

            bool operator>=(const position& other) const noexcept {
                return !(*this < other);
            }

            
        };

        enum class types{
            UNKNOWN,
            TEXT,
            OPERATOR,
            SEPARATOR,      // represents commas, spaces and newlines
            NUMBER,
            WRAPPER,        // represents strings, characters, parenthesis, brackets, braces and comments
            ESCAPE,         // represents escape sequences like \n, \t, \x00FF, etc.
            CONTROL         // EOF and similar
        };

        // Represents the base class for all lexical token classes.
        class base{
        protected:
            position start;     // start position of the token

            types type;         // type of the token

        public:
            bool redundant;     // used to flag if an token is flagged to be removed later on instead of messing up calculated indicies.

            parser::token::base* parsed = nullptr;    // in second stage we can remove this since object will be the largest size.

            base(position start_position, types token_type) : start(start_position), type(token_type), redundant(false) {}
            virtual ~base() {}

            position get_start() const { return start; }
            types get_type() const { return type; }
            
            // Pure virtual clone method for deep copying
            virtual base* clone() const {
                return new base(*this);
            }

            virtual std::string toString() {
                return "POSITION: (" + std::to_string(start.x) + ", " + std::to_string(start.y) + ")";
            }

            virtual std::string getData() { return ""; }
        };

        class text : public base{
        public:
            std::string data;   // the text itself

            text(position start_position, const std::string& text_value) : base(start_position, types::TEXT), data(text_value) {}
            
            base* clone() const override {
                return new text(*this);
            }

            std::string toString() override {
                return "[TEXT: \"" + data + "\" " + base::toString() + "]";
            }

            std::string getData() override { return data; }
        };

        class op : public base{
        public:
            std::string text;   // the operator itself

            op(position start_position, const std::string& text_value) : base(start_position, types::OPERATOR), text(text_value) {}
            
            base* clone() const override {
                return new op(*this);
            }

            std::string toString() override {
                return "[OPERATOR: \"" + text + "\" " + base::toString() + "]";
            }

            std::string getData() override { return text; }
        };

        class number : public base{
        public:
            std::string text;   // the number itself

            enum class types{
                INTEGER,
                FLOAT,
                HEX,
                BOOLEAN
            } number_type;

            number(position start_position, const std::string& text_value) : base(start_position, token::types::NUMBER), text(text_value) {
                // by default this code for decimal checking will never be of any use in tokenizer, since tokenizer only creates primitive tokens, where an decimal would be just an collection of two numbers and one dot operator.
                if(text.find('.') != std::string::npos) number_type = types::FLOAT;
                else number_type = types::INTEGER;
            }
            
            base* clone() const override {
                return new number(*this);
            }

            std::string toString() override {
                return "[NUMBER: \"" + text + "\" " + base::toString() + "]";
            }

            std::string getData() override { return text; }
        };

        class separator : public base{
        public:
            enum class types{
                UNKNOWN,
                COMMA,
                SPACE,  // Is removed at tokenize::postprocess
                NEWLINE,
            } type = types::UNKNOWN;

            separator(position start_position, const std::string& text_value) : base(start_position, token::types::SEPARATOR) {
                if (text_value == "," || text_value == ";") type = types::COMMA;
                else if (text_value == " ") type = types::SPACE;
                else if (text_value == "\n") type = types::NEWLINE;
            }
            
            base* clone() const override {
                return new separator(*this);
            }

            std::string toString() override {
                return "[SEPARATOR: \"" + std::to_string(static_cast<int>(type)) + "\" " + base::toString() + "]";
            }

            std::string getData() override {
                switch (type) {
                    case types::COMMA: return ",";
                    case types::SPACE: return " ";
                    case types::NEWLINE: return "\n";
                    default: return "";
                }
            }
        };

        class wrapper : public base{
        public:
            std::vector<base*> tokens; // contains the contents of the wrapper

            char identity;

            enum class types{
                UNKNOWN,
                ROUND_BRACKETS,
                SQUARE_BRACKETS,
                CURLY_BRACKETS,
                STRING,
                CHARACTER,
                COMMENT
            } type = types::UNKNOWN;

            wrapper(position start_position, std::string& self_text) : base(start_position, token::types::WRAPPER){
                if (self_text.empty()){
                    // TODO: log error
                    return;
                }

                identity = self_text[0];

                if (identity == '"') type = types::STRING;
                else if (identity == '\'') type = types::CHARACTER;
                else if (identity == '#') type = types::COMMENT;
                else if (identity == '(' || identity == ')') type = types::ROUND_BRACKETS;
                else if (identity == '[' || identity == ']') type = types::SQUARE_BRACKETS;
                else if (identity == '{' || identity == '}') type = types::CURLY_BRACKETS;
            }
            
            base* clone() const override {
                wrapper* cloned = new wrapper(*this);
                // Deep copy the tokens vector
                cloned->tokens.clear();
                for (const auto& token : tokens) {
                    if (token) {
                        cloned->tokens.push_back(token->clone());
                    }
                }
                return cloned;
            }

            std::string toString() override {
                std::string result = "[WRAPPER: \"" + std::to_string(static_cast<int>(type)) + "\" " + base::toString() + ": {";
                for (const auto& token : tokens) {
                    result += token->toString() + ", ";
                }
                result += "}]";
                return result;
            }

            std::string getData() override { 
                std::string result = "";
                result += getWrapperCondition(identity).first;

                for (const auto& token : tokens) {
                    result += token->getData();
                }

                result += getWrapperCondition(identity).second;

                return result;
            }

            static std::pair<char, char> getWrapperCondition(char c) {
                std::pair<char, char> result;
    
                // find the right wrap condition based on the starting wrap_identity.
                for (auto pair : wrapper_pairs){
                    if (pair.first == c){
                        result = pair;
                        break;
                    }
                }

                return result;
            }
        private:
            static std::vector<std::pair<char, char>> wrapper_pairs;
        };

        class escape : public base{
        public:
            std::string sequence;   // the escape sequence itself (without the backslash)

            escape(position start_position, const std::string& sequence_value) : base(start_position, token::types::ESCAPE), sequence(sequence_value) {}
            
            base* clone() const override {
                return new escape(*this);
            }

            std::string toString() override {
                return "[ESCAPE: \"\\\\" + sequence + "\" " + base::toString() + "]";
            }

            std::string getData() override { return sequence; }
        };

        class control : public base{
        public:
            enum class types{
                UNKNOWN,
                START_OF_FILE,
                END_OF_FILE
            } type = types::UNKNOWN;

            control(position start_position, std::string& self_text) : base(start_position, token::types::CONTROL){
                if (self_text.empty()){
                    // TODO: log error
                    return;
                }

                if (self_text[0] == '\2') type = types::START_OF_FILE;
                else if (self_text[0] == '\3') type = types::END_OF_FILE;
            }

            control(position start_position, const char* self_text) : base(start_position, token::types::CONTROL){
                if (std::string(self_text).empty()){
                    // TODO: log error
                    return;
                }

                if (self_text[0] == '\2') type = types::START_OF_FILE;
                else if (self_text[0] == '\3') type = types::END_OF_FILE;
            }
            
            base* clone() const override {
                return new control(*this);
            }
            
            std::string toString() override {
                return "[CONTROL: \"" + std::to_string(static_cast<int>(type)) + "\" " + base::toString() + "]";
            }

            std::string getData() override {
                switch (type) {
                    case types::START_OF_FILE: return "\2";
                    case types::END_OF_FILE: return "\3";
                    default: return "";
                }
            }
        };

        namespace presets{
            inline control start_of_file({ 0, 0, 0 }, "\2");

            inline control end_of_file(position p){
                return control(p, "\3");
            }
        }

        extern base* create(types type, std::string& text);
    }

    namespace cluster{
        class group{
        public:
            char min;           // represents the minimum value of an character an character can be to be of this group
            char max;           // represents the maximum value of an character an character can be to be of this group
            
            bool sticky;        // if the group is NOT sticky, it will not be able to be combined with other groups

            token::types type;         // what type of an token this group represents

            group(token::types group_type, char min_value, char max_value, bool sticky_value = false) : min(min_value), max(max_value), sticky(sticky_value), type(group_type) {}
        };

        inline std::vector<group> groups = {
            { token::types::TEXT, 'A', 'Z', true },
            { token::types::TEXT, 'a', 'z', true },
            { token::types::TEXT, '_', '_', true },

            { token::types::OPERATOR, '%', '%' },
            { token::types::OPERATOR, '&', '&' },
            { token::types::OPERATOR, '*', '*' },
            { token::types::OPERATOR, '+', '+' },
            { token::types::OPERATOR, '-', '-' },
            { token::types::OPERATOR, '/', '/' },
            { token::types::OPERATOR, '=', '=', true },
            { token::types::OPERATOR, '!', '!' },
            { token::types::OPERATOR, '<', '<' },
            { token::types::OPERATOR, '>', '>' },
            { token::types::OPERATOR, '|', '|' },
            { token::types::OPERATOR, '^', '^' },
            { token::types::OPERATOR, '~', '~' },
            { token::types::OPERATOR, '?', '?' },
            { token::types::OPERATOR, ':', ':' },
            { token::types::OPERATOR, '.', '.' },

            { token::types::SEPARATOR, ' ', ' ' },
            { token::types::SEPARATOR, ',', ',' },
            { token::types::SEPARATOR, ';', ';' },
            { token::types::SEPARATOR, '\n', '\n' },

            { token::types::WRAPPER, '"', '"' },
            { token::types::WRAPPER, '\'', '\'' },

            { token::types::NUMBER, '0', '9', true },

            { token::types::ESCAPE, '\\', '\\' },

            { token::types::WRAPPER, '#', '#' },

            { token::types::WRAPPER, '(', '(' },
            { token::types::WRAPPER, ')', ')' },
            { token::types::WRAPPER, '[', '[' },
            { token::types::WRAPPER, ']', ']' },
            { token::types::WRAPPER, '{', '{' },
            { token::types::WRAPPER, '}', '}' },

            { token::types::CONTROL, '\2', '\2' },
            { token::types::CONTROL, '\3', '\3' }
        };

        // returns the group related to the string text
        extern group get_group(char c);
    }

    // tokenizes the given text into a vector of tokens.
    extern std::vector<token::base*> tokenize(std::string text, unsigned file_id);

}

#endif