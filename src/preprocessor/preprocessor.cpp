#include "preprocessor.h"




void preprocessor::includer::factory(preprocessor::unit* currentUnit) {

    for (int i = 0; i < currentUnit->currentScope->children.size(); i++) {



    }

}


void preprocessor::includer::openInclude(preprocessor::unit* currentUnit, int32_t index) {
    if (currentUnit->currentScope->children[index]->flags != parser::token::type::INCLUDE) return;

    
}

void preprocessor::includer::closeInclude(preprocessor::unit* currentUnit, int32_t index) {

}

