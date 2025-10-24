#include "parser.h"

namespace parser {

    parser::unit::base parser::unit::base::operator&(utils::range limit) {
        return { passIndex, parent, tokens & limit };
    }

    parser::unit::base operator&(parser::unit::base* unit, utils::range limit) {
        return (*unit) & limit;
    }

    void unit::base::factory() {
        for (passIndex = pass::FIRST; passIndex < pass::LAST; ++passIndex) {
            
            for (utils::range index = tokens.begin(); index < tokens.end(); ++index) {

                unit::base subUnit = this & index;

                token::definition::factory(subUnit);
                token::Operator::base::factory(subUnit);

                // Remove exhausted subset of tokens from the larger set of tokens we have:
                if (index.min != index.max) {   
                    // If this passed through, it means that what ever function inside the loop did in fact exhaust some tokens of ours, so let's remove those

                    tokens ^= index;
                }
            }
        }
    }

    void token::definition::factory(unit::base& currentUnit) {
        if (currentUnit.passIndex != unit::pass::FIRST) return;    // Definitions are only created in the first pass.


    }

    void token::Operator::base::factory(unit::base& currentUnit) {
        if (currentUnit.passIndex != unit::pass::SECOND) return;    // maybe increase this, since first pass is for definitions...



    }

}