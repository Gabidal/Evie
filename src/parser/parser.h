#ifndef _parser_h_
#define _parser_h_

/**
 * Takes as input from lexer tokens and creates a free-typed Surface-AST for preprocessor.
 */

#include "../utils/utils.h"
#include "../lexer/lexer.h"

#include <string>
#include <new>
#include <utility>
#include <charconv>

namespace parser {

    namespace token {
        class base;

        namespace scope {
            class base;
        }
        
        namespace Operator {
            // Ordered via the order of combination
            enum class types;
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
        enum class types {
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

        class base : public utils::linkable {
        public:
            types type;
            lexer::token::position position;
            scope::base* parent;    // Scope context, for local definition find order.
            std::string_view symbol;
            token::base* contextParent;   // For operators and non-scope parents.

            base(types Flags, lexer::token::position Position = {0, 0, 0}, scope::base* Parent = nullptr, std::string_view Symbol = "", token::base* Context = nullptr) : linkable(), type(Flags), position(Position), parent(Parent), symbol(Symbol), contextParent(Context) {}

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

            // By default no operation since a normal token does not hold any matchable objects.
            virtual void replace(token::base* /* Match */, token::base* /* Source */) {}    // Does nothing.
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
                else if (value == "true" || value == "false") numberType = lexer::token::number::types::BOOLEAN;
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

            template<typename T>
            T getValueWithBooleanOverrideAsNumber() {
                if (numberType == lexer::token::number::types::BOOLEAN) {
                    return value == "true" ? 1 : 0;
                } else {
                    if constexpr (std::is_integral<T>::value) {
                        return static_cast<T>(std::stoll(value));
                    } else {
                        return static_cast<T>(std::stod(value));
                    }
                }
            }

            lexer::token::number::types getProminentNumberType(lexer::token::number::types other, Operator::types t);

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

                void replace(token::base* match, token::base* source) override {
                    for (auto& def : definitions) {
                        if (def == match) {
                            def = source;
                        }
                    }
                    
                    for (auto& child : children) {
                        if (child == match) {
                            child = source;
                        }
                    }
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
                for (auto& wrapper : wrappers) {
                    if (wrapper) wrapper->contextParent = this;
                }
            }

            [[nodiscard]] token::base* findClosestDefinition(std::string_view Symbol) override {
                // Check self
                token::base* result = symbol == Symbol ? this : nullptr;

                for (int32_t i = 0; i < (int32_t)wrappers.size() && !result; i++) result = wrappers[i]->findClosestDefinition(Symbol);

                return result;
            }
            
            static void factory(unit::base* /*Current Translation Unit State*/, int32_t& /*Start Index*/);   

            void replace(token::base* match, token::base* source) override {
                for (auto& wrapper : wrappers) {
                    if (wrapper == match) {
                        // We assume source is also a scope::base* if it replaces a wrapper
                        if (auto* casted = dynamic_cast<scope::base*>(source)) {
                            wrapper = casted;
                        }
                    }
                }
            }
        };

        class caller : public token::object {
        public:
            std::vector<scope::base*> parameters; // <>[]()

            caller(info Info, definition::base* ref, std::vector<scope::base*> Contexts = {}) : token::object(Info, ref), parameters(Contexts) {
                type = types::CALLER;
                for (auto& param : parameters) {
                    if (param) param->contextParent = this;
                }
            }

            static void factory(unit::base* /*Current Translation Unit State*/, int32_t& /*Start Index*/);

            void replace(token::base* match, token::base* source) override {
                for (auto& param : parameters) {
                    if (param == match) {
                        if (auto* casted = dynamic_cast<scope::base*>(source)) {
                            param = casted;
                        }
                    }
                }
            }
        };

        namespace Operator {

            // Ordered via the order of combination
            enum class types {
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

            types toType(std::string_view symbol);

            class base : public token::base {
            public:
                types operationType;
                // By default an operator combines two nodes next to it.
                token::base* left;
                token::base* right;

                base(info Info, types Type, token::base* Left, token::base* Right) : token::base(Info), operationType(Type), left(Left), right(Right) {
                    if (Left) Left->contextParent = this;
                    if (Right) Right->contextParent = this;
                }

                static void factory(unit::base* /*Current Translation Unit State*/);
            
                static void combinator(unit::base* /*Current Translation Unit State*/, int32_t& /*Current Index*/, types /*Focused Type*/);

                void replace(token::base* match, token::base* source) override {
                    if (left == match) {
                        left = source;
                    }
                    if (right == match) {
                        right = source;
                    }
                }
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
                    
                    base(info Info, token::base* Operand, fix::type postOrPre) : token::base(Info), fixity(postOrPre), operand(Operand) {
                        if (Operand) Operand->contextParent = this;
                    }

                    static void combinator(unit::base* /*Current Translation Unit State*/, int32_t& /*Current Index*/);

                    void replace(token::base* match, token::base* source) override {
                        if (operand == match) {
                            operand = source;
                        }
                    }
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

                type getComparisonType(std::string_view symbol);
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
            token::scope::base* header;
            token::scope::base* body;

            condition(info Info, token::scope::base* Header, token::scope::base* Body) : token::base(Info), header(Header), body(Body) {
                if (header) header->contextParent = this;
                if (body) body->contextParent = this;
            }

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

            void replace(token::base* match, token::base* source) override {
                if (header == match) header = dynamic_cast<token::scope::base*>(source);
                if (body == match) body = dynamic_cast<token::scope::base*>(source);
            }
        };

        class looper : public token::base {
        public:
            token::base* init;       // int i = 0, call()
            token::base* condition;  // i < size, true
            token::base* footer;     // i++, call(&i)

            token::base* body;

            looper(info Info, token::base* Init, token::base* Condition, token::base* Footer, token::base* Body) : token::base(Info), init(Init), condition(Condition), footer(Footer), body(Body) {
                if (init) init->contextParent = this;
                if (condition) condition->contextParent = this;
                if (footer) footer->contextParent = this;
                if (body) body->contextParent = this;
            }

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

            void replace(token::base* match, token::base* source) override {
                if (init == match) init = source;
                if (condition == match) condition = source;
                if (footer == match) footer = source;
                if (body == match) body = source;
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

            extern void factory(unit::base* /*Current Translation Unit State*/, int32_t& /*Start Index*/);
        }
    }

    // static utilities:
    namespace token {
        template<typename Target, typename Source>
        static void replace(Target* target, Source* source) {
            if (target->contextParent) target->contextParent->replace(target, source);

            source->contextParent = target->contextParent;

            delete target;
        }
    }

}

#endif