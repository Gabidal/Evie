#include "parser.h"

namespace parser {

    unit::base::base(unit::lexerOutput& lexedTokens) : window(lexedTokens) {

    }

    unit::base::base(base& ParentTranslatorUnit, utils::range setLimits) : 
        passIndex(ParentTranslatorUnit.passIndex),
        parent(ParentTranslatorUnit.parent),
        window(ParentTranslatorUnit.window & setLimits)
    {

        

    }

    void unit::base::factory() {

    }

    void token::base::factory(unit::base& /* currentUnit */) {

    }

}