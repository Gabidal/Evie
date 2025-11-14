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

    class unit {
    public:
        parser::token::scope::base* globalScope;    // We can expect this to be half baked if the parser has thrown an exception while trying to parse this, but what it could parse has been already parsed.
    };

}

#endif