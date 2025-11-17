#include "preprocessor.h"

void preprocessor::includer::factory(preprocessor::unit* currentUnit) {

    for (int i = 0; i < (int32_t)currentUnit->currentScope->children.size(); i++) {
        openInclude(currentUnit, i);
    }
}

void preprocessor::includer::openInclude(preprocessor::unit* currentUnit, int32_t& index) {
    if (currentUnit->currentScope->children[index]->flags != parser::token::type::INCLUDE) return;
    auto* include = dynamic_cast<parser::token::includer::base*>(currentUnit->currentScope->children[index]);

    if (include->includeType == parser::token::includer::types::END) return;

    if (currentUnit->stack->contains(include->source)) {
        // Just remove the include token, since it is already included
        // This way the END includer will not be produced, since there is no relative paths created from an already included file.
        currentUnit->currentScope->children.erase(
            currentUnit->currentScope->children.begin() + index
        );
    }
    else {
        parser::unit::lexerOutput inlined = docker::file::translate(include->source, currentUnit->environment);
    
        currentUnit->stack->add(include->source);

        // First transform the BEGIN includer into an END includer
        include->includeType = parser::token::includer::types::END;
    
        if (!inlined.empty()){
            // Now we can insert before the end token the inlined lexer tokens.
            currentUnit->currentScope->rawTokens.insert(
                currentUnit->currentScope->rawTokens.begin() + index,
                inlined.begin(),
                inlined.end()
            );
        }

        // Now we need to call the parser on the inlined tokens with the other tokens
        parser::unit::base* subParser = new parser::unit::base(parser::unit::pass::FIRST, currentUnit->currentScope);
        subParser->factory();

        index -= inlined.size();  // Tell the preprocessor to re-evaluate the current scope, since it's content has changed.
    }
}

void preprocessor::includer::closeInclude(preprocessor::unit* currentUnit, int32_t& index) {
    if (currentUnit->currentScope->children[index]->flags != parser::token::type::INCLUDE) return;
    auto* include = dynamic_cast<parser::token::includer::base*>(currentUnit->currentScope->children[index]);

    if (include->includeType == parser::token::includer::types::BEGIN) return;

    // Tells Docker to remove one from the current working directory stack.
    currentUnit->stack->pop();

    // Also we can remove the end includer
    currentUnit->currentScope->children.erase(
        currentUnit->currentScope->children.begin() + index
    );

    index--;
}

