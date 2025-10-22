#ifndef _PARSER_H_
#define _PARSER_H_

#include "../utils/utils.h"
#include "../lexer/lexer.h"

#include <string>

namespace parser {

    namespace token {
        class base;

        namespace scope {
            class base;
        }
    }

    namespace unit {
        using lexerOutput = std::vector<lexer::token::base*>;

        class base {
        public:
            std::size_t passIndex = 0;                // Describes which pass through of the input token loop-through we currently are from.
            token::scope::base* parent = nullptr;     // Gives data of the current scope.
            utils::set<lexerOutput> window;           // Gives a set of indicies for the current scope of lexed tokens
            
            base(lexerOutput& /*Lexed Tokens*/);
            base(base& /*Reference Of The Parent Unit*/, utils::range /*Set Limits*/);

            // Delete copy
            base(const base&) = delete;

            void factory();
        };

    }

    namespace token {
        
        // Un-ordered
        enum class type {
            UNKNOWN,        // ???
            DEFINITION,     // Any instance of two or more words. Removes the inherited words and makes the last word an Object type node.
            OBJECT,         // Any occurrence of known defined word.
            OPERATOR,       // All operator representor type.
            SCOPE,          // Any occurrence of a scope block (function, class, namespace, parenthesis, etc).
        };
        
        class base {
        public:
            type flags;
            base* parent;
            std::string_view symbol;

            base(type Flags, base* Parent = nullptr, std::string_view Symbol = "") : flags(Flags), parent(Parent), symbol(Symbol) {}

            virtual ~base() = default;  // For our fallen comrades 🥀🥀🥀 smh tsm

            [[nodiscard]] virtual base* findClosestDefinition(std::string_view /* Symbol */) const {
                // Default behaviour is to pipe this call to the parent hoping it might hit the scope class.
                return parent ? parent->findClosestDefinition(symbol) : nullptr;
            }
            
            // Each token class introduces their own factory, which takes lexer::tokens as input and colors the area it will require which will be deleted upon exit.
            // Also Parent has to be a scope
            static void factory(unit::base& /*Current Translation Unit State*/);
        };

        // If we use this we can use it with no need to worry about slicing, although just using token::base as info packet is also fine tbh 🙄
        struct info final : public parser::token::base {
            using parser::token::base::base;
        };

        namespace Operator {

            // Ordered via the order of combination
            enum class type {
                UNKNOWN,            // ???
                FETCHER,            // .        <- This works same for scopes and member fetchers.
                PREFIX,             // Abstract type for the prefix operator subset.
                POSTFIX,            // Abstract type for the postfix operator subset.
                MULTIPLICATION,     // '*'
                DIVISION,           // '/'
                MODULO,             // '%'
                ADDITION,           // '+'
                SUBTRACTION,        // '-'
                BITSHIFT_LEFT,      // '<<'
                BITSHIFT_RIGHT,     // '>>'
                CONDITION,          // Abstract  type for the condition type subset.
                AND,                // '&'
                XOR,                // '¤'      <- washing machine strikes again >:3
                OR,                 // '|',
                ASSIGN,             // Abstract type for all assignment operators.
            };

            class base : public token::base {
            public:
                // By default an operator combines two nodes around it.
                token::base* left;
                token::base* right;

                base(info Info, token::base* Left, token::base* Right) : token::base(Info), left(Left), right(Right) {}

                static void factory(unit::base& /*Current Translation Unit State*/);
            };

            namespace prefix {

                // Un-ordered
                enum class type {
                    UNKNOWN,            // ???
                    PLUSPLUS,           // '++'
                    MINUSMINUS,         // '--'
                };

            }

            namespace postfix {

                using type = prefix::type;      // I'm absolute big brain on this one 🗣️💡

            }

            namespace condition {

                // Un-ordered
                enum class type {
                    UNKNOWN,            // ???
                    LESS_THAN,          // '<'
                    GREATER_THAN,       // '>'
                    LESS_EQUAL,         // '<='
                    GREATER_EQUAL,      // '>='
                    EQUAL,              // '=='
                    NOT_EQUAL,          // '!='
                    LOGICAL_AND,        // '&&'
                    LOGICAL_OR,         // '||'
                };

            }

            namespace assign {
                
                // un-ordered
                enum class type {
                    UNKNOWN,                // ???
                    ASSIGNMENT,             // '='
                    ADDITION_ASSIGN,        // '+='
                    SUBTRACTION_ASSIGN,     // '-='
                    MULTIPLICATION_ASSIGN,  // '*='
                    DIVISION_ASSIGN,        // '/='
                    MODULO_ASSIGN,          // '%='
                    AND_ASSIGN,             // '&='
                    OR_ASSIGN,              // '|='
                    XOR_ASSIGN,             // '¤='
                    BITSHIFT_LEFT_ASSIGN,   // '<<='
                    BITSHIFT_RIGHT_ASSIGN,  // '>>='
                };
            }
        }
    }

}

#endif