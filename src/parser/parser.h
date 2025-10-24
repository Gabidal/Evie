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

        enum class pass {
            FIRST,
            SECOND,
            THIRD,
            // ...

            LAST
        };

        class base {
        public:
            pass passIndex = pass::FIRST;             // Describes which pass through of the input token loop-through we currently are from.
            token::scope::base* parent = nullptr;     // Gives data of the current scope.
            utils::superSet<lexerOutput> tokens;      // Gives a set of indicies for the current scope of lexed tokens
            
            base(lexerOutput& Tokens) : tokens(Tokens) {}
            base(pass i, token::scope::base* p, utils::superSet<lexerOutput> t) : passIndex(i), parent(p), tokens(t) {}

            // Delete copy
            base(const base&) = delete;

            void factory();

            template<typename lexerTokenType>
            lexerTokenType* at(uint32_t i) {
                return dynamic_cast<lexerTokenType*>(tokens[i]);
            }

            base operator&(utils::range limit);
        };

        ::utils::range findSubsequentTokens(base& /*Current Translation Unit*/, lexer::token::types /*Token type*/);
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
        
        namespace scope {
            class base;
        }

        class base {
        public:
            type flags;
            scope::base* parent;
            std::string_view symbol;

            base(type Flags, scope::base* Parent = nullptr, std::string_view Symbol = "") : flags(Flags), parent(Parent), symbol(Symbol) {}

            virtual ~base() = default;  // For our fallen comrades 🥀🥀🥀 smh tsm

            [[nodiscard]] virtual base* findClosestDefinition(std::string_view /* Symbol */) const;
            
            // Each token class introduces their own factory, which takes lexer::tokens as input and colors the area it will require which will be deleted upon exit.
            // Also Parent has to be a scope
            static void factory(unit::base& /*Current Translation Unit State*/) {}
        };

        // If we use this we can use it with no need to worry about slicing, although just using token::base as info packet is also fine tbh 🙄
        struct info final : public parser::token::base {
            using parser::token::base::base;
        };

        class definition : public token::base {
        public:
            std::vector<std::string_view> inherited;

            // Auto-adds itself to the current parent
            definition(info Info, std::vector<std::string_view> toInherit);
            
            static void factory(unit::base& /*Current Translation Unit State*/);
        };

        namespace scope {

            enum class type {
                UNKNOWN,
                FUNCTION,
                CLASS,
                CONDITION,
                LOOP,
                PARENTHESIS
            };

            class base : public token::base {
            public:
                std::vector<token::base*> children;
                utils::superSet<unit::lexerOutput> rawTokens;   // used by templates.

                base(info Info, utils::superSet<unit::lexerOutput> RawTokens) : token::base(Info), rawTokens(RawTokens) {}

                [[nodiscard]] token::base* findClosestDefinition(std::string_view Symbol) const override {
                    // Search in current scope first
                    for (auto it = children.rbegin(); it != children.rend(); ++it) {
                        if ((*it)->flags == token::type::DEFINITION && (*it)->symbol == Symbol) {
                            return *it;
                        }
                    }

                    // Pipe to parent scope
                    return parent ? parent->findClosestDefinition(Symbol) : nullptr;
                }
            };
        }

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