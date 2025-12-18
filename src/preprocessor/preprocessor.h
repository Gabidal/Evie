#ifndef _preprocessor_h_
#define _preprocessor_h_

/**
 * Takes Surface-AST as input from parser and and modifies conditions and includes via docker. 
 */

#include "../parser/parser.h"
#include "../docker/docker.h"
#include "../args/args.h"

/**
 * Before the hardening process of the Surface-AST into Core-AST
 */
namespace preprocessor {

    /** 
     * For compile time conditions to be parsed, the following invariants must hold:
     * 
     * Let S be the set of all scopes where:
     *   S = {globalScope} union {s | s in globalScope->children->scopes}
     * 
     * For all s in S:
     *   1. All includes in s must be inlined before processing s
     *   2. For all compile-time conditions c in s:
     *      All definitions used by c must exist in s or parent(s) and be inlined
     * 
     * Processing order: globalScope, then recursively process each child scope
    */
    class unit {
    public:
        parser::token::scope::base* currentScope;
        args::base* arguments;
        docker::stack* stack;

        void factory();

        unit(parser::token::scope::base* CurrentScope, args::base* Arguments, docker::stack* Stack) : currentScope(CurrentScope), arguments(Arguments), stack(Stack) {}
    };

    // Since including multifiles and stacked directories this needs to be more of a tool suite handling it.
    namespace includer {
        // Inserts tokens from includeble files into the current scope tokens.
        extern void openInclude(preprocessor::unit* currentUnit, int32_t& index);

        // Tells Docker that it is the end of the opened file tokens for Docker to be more folder context aware.
        extern void closeInclude(preprocessor::unit* currentUnit, int32_t& index);
    }

    namespace unwrap {

        extern void conditionals(preprocessor::unit* currentUnit, int32_t& index);

    }
    
    /**
     * For compile time variable solving, limitations are as follows:
     * 
     * For surface-AST only numbers and string literals are solvable.
     * For core-AST operator: operators, operator overloads, numbers and string literals.
     */
    namespace solver {

        // This is linked to a parser::token.
        class color {
        public:
            parser::token::base* value;     // The value this lifetime represents in the painted area.

            lexer::token::position start;   // Definition position isn't always the newset assign.
            lexer::token::position end;     // If more data is assigned or reference is taken.

            bool isEmpty() { return value; }

            color(parser::token::base* Value, lexer::token::position Start = {}, lexer::token::position End = {}) : value(Value), start(Start), end(End) {}
        };

        const inline color emptyColor = color(nullptr);

        class lifetimes : public utils::linkable {
        public:
            std::vector<color> colors;  // Contains all the different lifetimes of each value the linked definition has.

            void add(color c);

            color get(lexer::token::position location);
        };

        extern void determineLifetimes(preprocessor::unit* currentUnit, int32_t& index);

        extern parser::token::base* getLifetimeValueFrom(parser::token::base* unknown);

        namespace interpreter {
            extern void factory(preprocessor::unit* currentUnit, int32_t& index);

            extern void evaluate(parser::token::base* token); 

            extern parser::token::number* evaluate(parser::token::number* left, parser::token::number* right, parser::token::Operator::base* operation);
            extern parser::token::base* evaluate(parser::token::string::base* left, parser::token::string::base* right, parser::token::Operator::base* operation);
        }
    }

}

#endif