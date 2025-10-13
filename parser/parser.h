#ifndef _PARSER_H_
#define _PARSER_H_

#include "../utils/utils.h"
#include "../lexer/lexer.h"

namespace parser {

    namespace node {
        
        enum class type{
            DEFINITION      = 0 << 0,       // Any instance of two or more words. Removes the inherited words and makes the last word an Object type node.
            OBJECT          = 1 << 0,       // Any occurrence of known defined word.
        };
        
        class base {
        public:
            utils::bitmask<type, int> flags;
            
            base* parent = nullptr;

            base() {

            }

        };
    }

}

#endif