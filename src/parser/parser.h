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
            lexerOutput& tokens;                      // Gives a set of indicies for the current scope of lexed tokens
            
            /**
             * @brief This is how it works:
             * All patterns output here, then if this is a subUnit the parentUnit will subjugate all subUnit output into specified pattern as an AST member.
             * @example:
             *  a = 1 + 2 <- here the '+' will be triggered first, collecting 1 and 2.
             *  Now the next pattern knowing that the right side is exhausted, but not outside capacity, means it is processed token found in the unit::base::output.
             *  a = <exhausted>
             */
            std::vector<token::base*> output;

            base(lexerOutput& Tokens) : tokens(Tokens) {}
            base(pass i, token::scope::base* p);

            // Delete copy
            base(const base&) = delete;

            void factory();

            template<typename lexerTokenType>
            lexerTokenType* at(uint32_t i) {
                if (i < tokens.size())
                    return dynamic_cast<lexerTokenType*>(tokens[i]);
                
                return nullptr;
            }
        };

        ::utils::range findSubsequentTokens(base* /*Current Translation Unit*/, lexer::token::types /*Token type*/, size_t /*Start Index*/);
    }

    namespace token {
        
        // Un-ordered
        enum class type {
            UNKNOWN,        // ???
            DEFINITION,     // Any instance of two or more words. Removes the inherited words and makes the last word an Object type node.
            OBJECT,         // Any occurrence of known defined word.
            OPERATOR,       // All operator representor type.
            SCOPE,          // Any occurrence of a scope block (function, class, namespace, parenthesis, etc).
            CALLER,         // Function call operator.
            NUMBER,         // Any number in Real space
        };
        
        namespace scope {
            class base;
        }

        class base {
        public:
            type flags;
            lexer::token::position position;
            scope::base* parent;
            std::string_view symbol;

            base(type Flags, lexer::token::position Position = {0, 0, 0}, scope::base* Parent = nullptr, std::string_view Symbol = "") : flags(Flags), position(Position), parent(Parent), symbol(Symbol) {}

            virtual ~base() = default;  // For our fallen comrades 🥀🥀🥀 smh tsm

            [[nodiscard]] virtual base* findClosestDefinition(std::string_view /* Symbol */) const;
            
            // Each token class introduces their own factory, which takes lexer::tokens as input and colors the area it will require which will be deleted upon exit.
            // Also Parent has to be a scope
            static void factory(unit::base* /*Current Translation Unit State*/, size_t /*Current Index*/) {}

            virtual std::string toString() { 
                return std::string(symbol) + ": (" + std::to_string(position.x) + ", " + std::to_string(position.y) + ")";
            }
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
            
            static void factory(unit::base* /*Current Translation Unit State*/, size_t& /*Current Index*/);
        };
        
        class object : public token::base {
        public:
            std::string_view name;
            
            definition* reference;

            object(info Info, definition* ref) : token::base(Info), name(ref->symbol), reference(ref) {}
            
            static void factory(unit::base* /*Current Translation Unit State*/, size_t& /*Current Index*/);
        };

        class number : public token::base, public lexer::token::number {
        public:
            using lexer::token::number::number;

            number(info Info, const std::string& TextValue) : parser::token::base(Info), lexer::token::number(Info.position, TextValue) {}

            std::string toString() override {
                return "[PARSER NUMBER: \"" + text + "\" " + parser::token::base::toString() + "]";
            }

            static void factory(unit::base* /*Current Translation Unit State*/, size_t& /*Current Index*/);
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
                std::vector<token::base*> definitions;
                std::vector<token::base*> children;

                unit::lexerOutput rawTokens;   // used by templates.

                base(info Info, unit::lexerOutput RawTokens) : token::base(Info), rawTokens(RawTokens) {}

                [[nodiscard]] token::base* findClosestDefinition(std::string_view Symbol) const override {
                    // Search in current scope first
                    for (auto it = definitions.rbegin(); it != definitions.rend(); ++it) {
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
                FIX,                // Abstract type for the prefix operator subset.
                MULTIPLICATION,     // '*'
                DIVISION,           // '/'
                MODULO,             // '%'
                ADDITION,           // '+'
                SUBTRACTION,        // '-'
                BITSHIFT_LEFT,      // '<<'
                BITSHIFT_RIGHT,     // '>>'
                COMPARISON,         // Abstract type for conditionals except for || and &&
                AND,                // '&'
                XOR,                // '¤'      <- washing machine strikes again >:3
                OR,                 // '|',
                LOGICAL_AND,        // '&&'
                LOGICAL_OR,         // '||'
                ASSIGN,             // Abstract type for all assignment operators.
            };

            type toType(std::string_view symbol);

            class base : public token::base {
            public:
                type operationType;
                // By default an operator combines two nodes next to it.
                token::base* left;
                token::base* right;

                base(info Info, type Type, token::base* Left, token::base* Right) : token::base(Info), operationType(Type), left(Left), right(Right) {}

                static void factory(unit::base* /*Current Translation Unit State*/);
            
                static void combinator(unit::base* /*Current Translation Unit State*/, size_t& /*Current Index*/, type /*Focused Type*/);
            };

            namespace fetcher {
                extern void combinator(unit::base* /*Current Translation Unit State*/, size_t& /*Current Index*/);
            }

            namespace fix {

                enum class type {
                    UNKNOWN,
                    POST,
                    PRE,
                };

                bool is(std::string_view symbol);

                class base : public token::base {
                    // Symbol is stored in this::base::symbol
                public:
                    fix::type fixity;
                    token::base* operand;
                    
                    base(info Info, token::base* Operand, fix::type postOrPre) : token::base(Info), fixity(postOrPre), operand(Operand) {}

                    static void combinator(unit::base* /*Current Translation Unit State*/, size_t& /*Current Index*/);
                };

            }

            namespace comparison {

                // Un-ordered
                enum class type {
                    UNKNOWN,            // ???
                    LESS_THAN,          // '<'
                    GREATER_THAN,       // '>'
                    LESS_EQUAL,         // '<='
                    GREATER_EQUAL,      // '>='
                    EQUAL,              // '=='
                    NOT_EQUAL,          // '!='
                };

                bool is(std::string_view symbol);

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

                bool is(std::string_view symbol);
            }
        }
    }

}

#endif