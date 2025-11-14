#ifndef _preprocessor_h_
#define _preprocessor_h_

/**
 * Takes Surface-AST as input from parser and and modifies conditions and includes via docker. 
 */

#include "../parser/parser.h"
#include "../docker/docker.h"

/**
 * Before the hardening process of the Surface-AST into Core-AST
 */
namespace preprocessor {


    class unit {
    public:
        // parser::unit::base   <- could throw catch to see if it is parsable?
    };

}

#endif