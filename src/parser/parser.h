#ifndef _parser_h_
#define _parser_h_

/**
 * Takes as input from lexer tokens and creates a free-typed Surface-AST for preprocessor.
 */

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

            bool inString;

            base(lexerOutput& Tokens) : tokens(Tokens), inString(false) {}
            base(pass i, token::scope::base* p, bool InString = false);

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

        extern ::utils::range findSubsequentTokens(base* /*Current Translation Unit*/, lexer::token::types /*Token type*/, int32_t /*Start Index*/);

        extern ::utils::range findSubsequentParsedTokens(base* /*Current Translation Unit*/, int32_t /*Start Index*/);

        extern ::utils::range findSubsequentTokens(base* /*Current Translation Unit*/, std::vector<std::pair<lexer::token::types, std::string_view>> /*Requirements*/, int32_t /*Start Index*/);

        extern void replaceDefinition(token::base* old, token::base* New);
    }

    namespace token {
        
        // Un-ordered
        enum class type {
            UNKNOWN,        // ???
            COMMENT,        // #...\n
            DEFINITION,     // Any instance of two or more words. Removes the inherited words and makes the last word an Object type node.
            OBJECT,         // Any occurrence of known defined word.
            STRING,         // "..." or '...'
            OPERATOR,       // All operator representor type.
            SCOPE,          // Any occurrence of a scope block (function, class, namespace, parenthesis, etc).
            CALLER,         // Function call operator.
            NUMBER,         // Any number in Real space
            CONDITION,      // If, elses
            LOOP,           // Loopers
            INCLUDE,        // Represents all include pattern matchers 
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

            [[nodiscard]] virtual base* findClosestDefinition(std::string_view /* Symbol */);
            
            // Each token class introduces their own factory, which takes lexer::tokens as input and colors the area it will require which will be deleted upon exit.
            // Also Parent has to be a scope
            static void factory(unit::base* /*Current Translation Unit State*/, int32_t /*Current Index*/) {}

            virtual std::string toString() { 
                return std::string(symbol) + ": (" + std::to_string(position.x) + ", " + std::to_string(position.y) + ")";
            }

            virtual std::string getValue() {
                return std::string(symbol);
            }
        };

        // If we use this we can use it with no need to worry about slicing, although just using token::base as info packet is also fine tbh 🙄
        struct info final : public parser::token::base {
            using parser::token::base::base;

            info(parser::token::base* other) : parser::token::base(*other) {}
        };

        namespace definition {
            enum class types {
                VARIABLE,
                CONTEXT
            };

            class base : public token::base {
            public:
                std::vector<std::string_view> inherited;
                types definitionType;
    
                // Auto-adds itself to the current parent
                base(info Info, std::vector<std::string_view> toInherit, types defType = types::VARIABLE);
                
                static void factory(unit::base* /*Current Translation Unit State*/, int32_t& /*Current Index*/);
            };

        }
        
        class object : public token::base {
        public:
            std::string_view name;
            
            definition::base* reference;

            object(info Info, definition::base* ref) : token::base(Info), name(ref->symbol), reference(ref) {}
            
            static void factory(unit::base* /*Current Translation Unit State*/, int32_t& /*Current Index*/);
        };

        class number : public token::base {
        public:
            std::string value;
            lexer::token::number::types numberType;
            uint8_t minRequiredByteSize;

            number(info Info, const std::string& TextValue) : parser::token::base(Info), value(TextValue) {
                if(value.find('.') != std::string::npos) numberType = lexer::token::number::types::FLOAT;
                else if (value.find('x') != std::string::npos || value.find('X') != std::string::npos) numberType = lexer::token::number::types::HEX;
                else numberType = lexer::token::number::types::INTEGER;

                determineSize();

                transformHexIntoInt();
            }

            std::string toString() override {
                return "[PARSER NUMBER: \"" + value + "\" " + parser::token::base::toString() + "]";
            }

            static void factory(unit::base* /*Current Translation Unit State*/, int32_t& /*Current Index*/);

            std::string getValue() override {
                return value;
            }

        private:
            void determineSize();

            void transformHexIntoInt();
        };

        namespace scope {

            class base : public token::base {
            public:
                std::vector<token::base*> definitions;
                std::vector<token::base*> children;

                unit::lexerOutput rawTokens;   // used by templates.

                base(info Info, unit::lexerOutput RawTokens) : token::base(Info), rawTokens(RawTokens) {}

                [[nodiscard]] token::base* findClosestDefinition(std::string_view Symbol) override {
                    // Search in current scope first
                    for (auto it = definitions.rbegin(); it != definitions.rend(); ++it) {
                        if ((*it)->symbol == Symbol) {
                            return *it;
                        }
                    }

                    // Pipe to parent scope
                    return parent ? parent->findClosestDefinition(Symbol) : nullptr;
                }
            };

            namespace parenthesis {
                extern void factory(unit::base* /*Current Translation Unit State*/, int32_t& /*Start Index*/);
            }

            namespace comment {
                extern void factory(unit::base* /*Current Translation Unit State*/, int32_t& /*Start Index*/);
            }
        }

        namespace string {
            class base : public scope::base {
            public:
                std::string bakedString;

                base(info Info, unit::lexerOutput RawTokens) : scope::base(Info, RawTokens), bakedString("") {}

                void bakeTokensToString();
            };

            extern void factory(unit::base* /*Current Translation Unit State*/, int32_t& /*Start Index*/);

            namespace escape {
                extern void factory(unit::base* /*Current Translation Unit State*/, int32_t& /*Start Index*/);
            }
        }

        // Represents functions, classes, namespaces and more?
        class context : public token::definition::base {
        public:
            std::vector<scope::base*> wrappers; // <>[](){}

            context(token::definition::base Info, std::vector<scope::base*> Contexts = {}) : token::definition::base(Info), wrappers(Contexts) {
                definitionType = token::definition::types::CONTEXT;
            }

            [[nodiscard]] token::base* findClosestDefinition(std::string_view Symbol) override {
                // Check self
                token::base* result = symbol == Symbol ? this : nullptr;

                for (int32_t i = 0; i < (int32_t)wrappers.size() && !result; i++) result = wrappers[i]->findClosestDefinition(Symbol);

                return result;
            }
            
            static void factory(unit::base* /*Current Translation Unit State*/, int32_t& /*Start Index*/);   
        };

        class caller : public token::object {
        public:
            std::vector<scope::base*> parameters; // <>[]()

            caller(info Info, definition::base* ref, std::vector<scope::base*> Contexts = {}) : token::object(Info, ref), parameters(Contexts) {
                flags = type::CALLER;
            }

            static void factory(unit::base* /*Current Translation Unit State*/, int32_t& /*Start Index*/);
        };

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
            
                static void combinator(unit::base* /*Current Translation Unit State*/, int32_t& /*Current Index*/, type /*Focused Type*/);
            };

            namespace fetcher {
                extern void factory(unit::base* /*Current Translation Unit State*/, int32_t& /*Current Index*/);
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

                    static void combinator(unit::base* /*Current Translation Unit State*/, int32_t& /*Current Index*/);
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
    
        class condition : public token::base {
        public:
            token::base* header;
            token::base* body;

            condition(info Info, token::base* Header, token::base* Body) : token::base(Info), header(Header), body(Body) {}

            static void factory(unit::base* /*Current Translation Unit State*/, int32_t& /*Start Index*/);

            [[nodiscard]] token::base* findClosestDefinition(std::string_view Symbol) override {
                // Check header first
                token::base* result = nullptr;
                
                if (header) result = header->findClosestDefinition(Symbol);

                // Then body
                if (!result) {
                    result = body->findClosestDefinition(Symbol);
                }

                return result;
            }
        };

        class looper : public token::base {
        public:
            token::base* init;       // int i = 0, call()
            token::base* condition;  // i < size, true
            token::base* footer;     // i++, call(&i)

            token::base* body;

            looper(info Info, token::base* Init, token::base* Condition, token::base* Footer, token::base* Body) : token::base(Info), init(Init), condition(Condition), footer(Footer), body(Body) {}

            static void factory(unit::base* /*Current Translation Unit State*/, int32_t& /*Start Index*/);

            [[nodiscard]] token::base* findClosestDefinition(std::string_view Symbol) override {
                // Check init first
                token::base* result = nullptr;
                
                if (init) result = init->findClosestDefinition(Symbol);

                // Then body
                if (!result && body) {
                    result = body->findClosestDefinition(Symbol);
                }

                return result;
            }
        };
    
        namespace includer {
            enum class types {
                BEGIN,  // Default, replaced via the inlined tokens
                END,    // For each include the last token of the include tells Docker when to go back by one in working dir stack.
            };

            class base : public token::base {
            public:
                std::string_view source;  // fileName, git repo, URL, project with cmake or meson buildable files.
                types includeType;

                base(info Info, std::string_view Location, types IncludeType) : token::base(Info), source(Location), includeType(IncludeType) {}
            };

            static void factory(unit::base* /*Current Translation Unit State*/, int32_t& /*Start Index*/);
        }
    }

}

#endif