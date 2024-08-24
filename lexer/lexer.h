#ifndef _lexer_h_
#define _lexer_h_

#include <string>
#include <vector>
#include <mutex>

namespace lexer{
    // contains all token-related utilities.
    namespace token{

        // represents the origin of said token
        class position{
        public:
            unsigned short x;       // represents the character offset
            unsigned short y;       // represents the line number
            unsigned short file_id; // points to an offset for docked files.

            position(unsigned short x, unsigned short y, unsigned short file_id) : x(x), y(y), file_id(file_id) {}
        };

        enum class types{
            UNKNOWN,
            TEXT,
            OPERATOR,
            SEPARATOR,      // represents commas, spaces and newlines
            NUMBER,
            WRAPPER,        // represents strings, characters, parenthesis, brackets, braces and comments
            CONTROL         // EOF and similar
        };

        // Represents the base class for all lexical token classes.
        class base{
        protected:
            position start;     // start position of the token

            types type;         // type of the token

        public:
            bool redundant;     // used to flag if an token is flagged to be removed later on instead of messing up calculated indicies.

            base(position start, types type) : start(start), type(type), redundant(false) {}
            virtual ~base() {}

            position get_start() const { return start; }
            types get_type() const { return type; }
        };

        class text : public base{
        public:
            std::string data;   // the text itself

            text(position start, const std::string& text) : base(start, types::TEXT), data(text) {}
        };

        class op : public base{
        public:
            std::string text;   // the operator itself

            op(position start, const std::string& text) : base(start, types::OPERATOR), text(text) {}
        };

        class number : public base{
        public:
            std::string text;   // the number itself

            enum class types{
                INTEGER,
                FLOAT
            } number_type;

            number(position start, const std::string& text) : base(start, token::types::NUMBER), text(text) {
                // by default this code for decimal checking will never be of any use in tokenizer, since tokenizer only creates primitive tokens, where an decimal would be just an collection of two numbers and one dot operator.
                if(text.find('.') != std::string::npos) number_type = types::FLOAT;
                else number_type = types::INTEGER;
            }
        };

        class separator : public base{
        public:
            enum class types{
                UNKNOWN,
                COMMA,
                SPACE,
                NEWLINE,
            } type = types::UNKNOWN;

            separator(position start, const std::string& text) : base(start, token::types::SEPARATOR) {
                if (text == ",") type = types::COMMA;
                else if (text == " ") type = types::SPACE;
                else if (text == "\n") type = types::NEWLINE;
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

            wrapper(position start, std::string& self) : base(start, token::types::WRAPPER){
                if (self.empty()){
                    // TODO: log error
                    return;
                }

                identity = self[0];

                if (identity == '"') type = types::STRING;
                else if (identity == '\'') type = types::CHARACTER;
                else if (identity == '#') type = types::COMMENT;
                else if (identity == '(' || identity == ')') type = types::ROUND_BRACKETS;
                else if (identity == '[' || identity == ']') type = types::SQUARE_BRACKETS;
                else if (identity == '{' || identity == '}') type = types::CURLY_BRACKETS;
            }
        };

        class control : public base{
        public:
            enum class types{
                UNKNOWN,
                START_OF_FILE,
                END_OF_FILE
            } type = types::UNKNOWN;

            control(position start, std::string& self) : base(start, token::types::CONTROL){
                if (self.empty()){
                    // TODO: log error
                    return;
                }

                if (self[0] == '\2') type = types::START_OF_FILE;
                else if (self[0] == '\3') type = types::END_OF_FILE;
            }

            control(position start, std::string self) : base(start, token::types::CONTROL){
                                if (self.empty()){
                    // TODO: log error
                    return;
                }

                if (self[0] == '\2') type = types::START_OF_FILE;
                else if (self[0] == '\3') type = types::END_OF_FILE;
            }
                    
        };

        namespace presets{
            control start_of_file({ 0, 0, 0 }, "\2");

            control end_of_file(position p){
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

            group(token::types type, char min, char max, bool sticky = false) : type(type), min(min), max(max), sticky(sticky) {}
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
            { token::types::OPERATOR, '=', '=' },
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
            { token::types::SEPARATOR, '\n', '\n' },

            { token::types::WRAPPER, '"', '"' },
            { token::types::WRAPPER, '\'', '\'' },

            { token::types::NUMBER, '0', '9', true },

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