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


    };

    // Since including multifiles and stacked directories this needs to be more of a tool suite handling it.
    namespace includer {

        extern void factory(preprocessor::unit* currentUnit);

        // Inserts tokens from includeble files into the current scope tokens.
        extern void openInclude(preprocessor::unit* currentUnit, int32_t index);

        // Tells Docker that it is the end of the opened file tokens for Docker to be more folder context aware.
        extern void closeInclude(preprocessor::unit* currentUnit, int32_t index);

    }

}

#endif