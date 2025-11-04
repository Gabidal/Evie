#include "parser.h"

namespace parser {

    unit::base::base(unit::pass i, token::scope::base* p) : passIndex(i), parent(p), tokens(p->rawTokens) {

    }

    void unit::base::factory() {
        for (passIndex = pass::FIRST; passIndex < pass::LAST; ++passIndex) {
            
            // Consider here looping through subsets first and then range in them, to boost performance 999+
            // NOTE: if you decide to use subset traversal then you cannot remove mid loop exhausted tokens, so use reverse traversal!
            for (size_t index = 0; index < tokens.size(); ++index) {

                // FACTORIES:
                // -_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_
                token::definition::factory(this, index);
                token::number::factory(this, index);
                token::object::factory(this, index);
                // -_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_
                
            }
            
            // SPECIAL FACTORIES:
            // -_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_
            token::Operator::base::factory(this);
        }
    }

    utils::range unit::findSubsequentTokens(base* Unit, lexer::token::types type, size_t startIndex) {
        utils::range result;
        for (
            result = {startIndex, startIndex}; 
            (size_t)result.max < Unit->tokens.size() &&                  // Check that we are still within the bounds
            Unit->tokens[result.max]->get_type() == type;        // Check that the current token is of the requested type
            result.max++
        );

        return result;
    }

    token::base* token::base::findClosestDefinition(std::string_view Symbol) const {
        // Default behaviour is to pipe this call to the parent hoping it might hit the scope class.
        return parent ? parent->findClosestDefinition(Symbol) : nullptr;
    }

    token::definition::definition(info Info, std::vector<std::string_view> toInherit) : token::base(Info), inherited(toInherit) {
        if (parent) {
            parent->definitions.push_back(this);
        }
    }

    void token::definition::factory(unit::base* currentUnit, size_t& startIndex) {
        if (currentUnit->passIndex != unit::pass::FIRST) return;    // Definitions are only created in the first pass.
        if (currentUnit->tokens.size() < 2) return;                 // Need at least two tokens to form a definition. one for type and one for name

        // Check if current text token is defined or not:
        // - If is, then check next token if it is also an text token and is defined:
        //   - If is, then iterate until found text token which is not defined.
        //   - If not, then that text token which is not defined the new definition name and inherits all prior defined text symbols as types.
        // - If not, return.

        utils::range textTokens = findSubsequentTokens(currentUnit, lexer::token::types::TEXT, startIndex);

        if (textTokens.length() < 2) return; // Need at least two text tokens to form a definition.

        lexer::token::text* name = currentUnit->at<lexer::token::text>(textTokens.max - 1);

        // We can simply check that the last element is not defined:
        if (currentUnit->parent->findClosestDefinition(name->data) != nullptr) {
            throw std::runtime_error("Redefinition of symbol '" + name->data + "'");
        }

        // Let's now also fetch the symbols of each inherited text token.
        std::vector<std::string_view> inherited;
        for (int32_t i = textTokens.min; i < textTokens.max - 1; ++i) {
            std::string_view symbol = currentUnit->at<lexer::token::text>(i)->data;

            inherited.push_back(symbol);
        }

        // Definition class automatically handles everything it needs to, keep this accessor in case we need to do some higher level abstract thingies.
        [[maybe_unused]] token::definition::base* newDefinition = new token::definition(
            token::info(
                token::type::DEFINITION,
                name->get_start(),
                currentUnit->parent,
                name->data
            ),
            inherited
        );

        // Instead of setting parsed on the name token, we will let the object factory do that for us.
        // NOTE: For overloads to work we need to inject the definition into the parsed, this gives the object factory a "TRUE" reference to the correct definition and not just the first occurring overload.
        name->parsed = newDefinition;

        // Now we exhaust the tokens needed to build this definition, but not he last token, since it will now represent an defined text token for later patterns to recognize.
        currentUnit->tokens.erase(currentUnit->tokens.begin() + textTokens.min, currentUnit->tokens.begin() + textTokens.max - 1);
        // Update index
        startIndex = textTokens.min;
    }

    void token::object::factory(unit::base* currentUnit, size_t& startIndex) {
        if (currentUnit->passIndex != unit::pass::SECOND) return;    // Definitions are only created in the first pass, so objects on second pass, this way we can use references before their declaration, good for headers :).
    
        lexer::token::text* currentText = currentUnit->at<lexer::token::text>(startIndex);

        if (!currentText) return; // Not a text token, cannot be an object.

        token::base* reference = currentText->parsed;

        // Check if this text token is residue of an output of definition factory:
        if (!reference || reference->flags != token::type::DEFINITION) {
            // If not, then we need to find the defined manually
            reference = currentUnit->parent->findClosestDefinition(currentText->data);
        }

        token::object* newObject = new token::object(
            token::info(
                token::type::OBJECT,
                currentText->get_start(),
                currentUnit->parent,
                currentText->data
            ),
            static_cast<token::definition*>(reference)
        );

        // now we override the definition factory made parsed reference with the actual object parsed value.
        currentText->parsed = newObject;
    }

    void token::number::factory(unit::base* currentUnit, size_t& startIndex) {
        if (currentUnit->passIndex != unit::pass::FIRST) return;    // numbers can be handled as soon as possible since they do not require complex AST to be determined

        if (auto currentNumber = currentUnit->at<lexer::token::number>(startIndex)) {
            token::number* newNumber = new token::number(
                token::info(
                    token::type::NUMBER,
                    currentNumber->get_start(),
                    currentUnit->parent,
                    currentNumber->text
                ),
                currentNumber->text
            );

            currentNumber->parsed = newNumber;
        }
    }

    token::Operator::type token::Operator::toType(std::string_view symbol) {
        if (symbol == ".") {
            return token::Operator::type::FETCHER;
        } else if (fix::is(symbol)) {
            return token::Operator::type::FIX;
        } else if (symbol == "*") {
            return token::Operator::type::MULTIPLICATION;
        } else if (symbol == "/") {
            return token::Operator::type::DIVISION;
        } else if (symbol == "%") {
            return token::Operator::type::MODULO;
        } else if (symbol == "+") {
            return token::Operator::type::ADDITION;
        } else if (symbol == "-") {
            return token::Operator::type::SUBTRACTION;
        } else if (symbol == "<<") {
            return token::Operator::type::BITSHIFT_LEFT;
        } else if (symbol == ">>") {
            return token::Operator::type::BITSHIFT_RIGHT;
        } else if (comparison::is(symbol)) {
            return token::Operator::type::COMPARISON;
        } else if (symbol == "&") {
            return token::Operator::type::AND;
        } else if (symbol == "¤") {
            return token::Operator::type::XOR;
        } else if (symbol == "|") {
            return token::Operator::type::OR;
        } else if (symbol == "&&") {
            return token::Operator::type::LOGICAL_AND;
        } else if (symbol == "||") {
            return token::Operator::type::LOGICAL_OR;
        } else if (assign::is(symbol)) {
            return token::Operator::type::ASSIGN;
        }
        
        return token::Operator::type::UNKNOWN;
    }

    bool token::Operator::fix::is(std::string_view symbol) {
        if (symbol == "++" || symbol == "--") {
            return true;
        }

        return false;
    }
    
    bool token::Operator::comparison::is(std::string_view symbol) {
        if (symbol == "<" || symbol == ">" || symbol == "<=" || symbol == ">=" || symbol == "==" || symbol == "!=") {
            return true;
        }

        return false;
    }

    bool token::Operator::assign::is(std::string_view symbol) {
        if (symbol == "=" || symbol == "+=" || symbol == "-=" || symbol == "*=" || symbol == "/=" || symbol == "%=" || symbol == "<<=" || symbol == ">>=" || symbol == "&=" || symbol == "|=") {
            return true;
        }

        return false;
    }

    void token::Operator::base::factory(unit::base* currentUnit) {
        if (currentUnit->passIndex != unit::pass::THIRD) return;

        // <bool a> = 1 == 1 && 1 & 1
        // a = <1 == 1> && <1 & 1>
        // a = <l && r>
        // <a = r>

        // '.'
        for (size_t i = 0; i < currentUnit->tokens.size(); i++) {
            token::Operator::fetcher::combinator(currentUnit, i);
        }

        // '++' '--'
        for (size_t i = 0; i < currentUnit->tokens.size(); i++) {
            token::Operator::fix::base::combinator(currentUnit, i);
        }

        // '*' '/' '%'
        for (size_t i = 0; i < currentUnit->tokens.size(); i++) {
            token::Operator::base::combinator(currentUnit, i, token::Operator::type::MULTIPLICATION);
            token::Operator::base::combinator(currentUnit, i, token::Operator::type::DIVISION);
            token::Operator::base::combinator(currentUnit, i, token::Operator::type::MODULO);
        }

        // '+' '-'
        for (size_t i = 0; i < currentUnit->tokens.size(); i++) {
            token::Operator::base::combinator(currentUnit, i, token::Operator::type::ADDITION);
            token::Operator::base::combinator(currentUnit, i, token::Operator::type::SUBTRACTION);
        }

        // '<<' '>>'
        for (size_t i = 0; i < currentUnit->tokens.size(); i++) {
            token::Operator::base::combinator(currentUnit, i, token::Operator::type::BITSHIFT_LEFT);
            token::Operator::base::combinator(currentUnit, i, token::Operator::type::BITSHIFT_RIGHT);
        }

        // Comparisons: '<' '>' '<=' '>=' '==' '!='
        for (size_t i = 0; i < currentUnit->tokens.size(); i++) {
            token::Operator::base::combinator(currentUnit, i, token::Operator::type::COMPARISON);
        }

        // '&' '¤' '|'
        for (size_t i = 0; i < currentUnit->tokens.size(); i++) {
            token::Operator::base::combinator(currentUnit, i, token::Operator::type::AND);
            token::Operator::base::combinator(currentUnit, i, token::Operator::type::XOR);
            token::Operator::base::combinator(currentUnit, i, token::Operator::type::OR);
        }

        // '&&'
        for (size_t i = 0; i < currentUnit->tokens.size(); i++) {
            token::Operator::base::combinator(currentUnit, i, token::Operator::type::LOGICAL_AND);
        }

        // '||'
        for (size_t i = 0; i < currentUnit->tokens.size(); i++) {
            token::Operator::base::combinator(currentUnit, i, token::Operator::type::LOGICAL_OR);
        }

        // Assignments
        for (size_t i = 0; i < currentUnit->tokens.size(); i++) {
            token::Operator::base::combinator(currentUnit, i, token::Operator::type::ASSIGN);
        }
    }

    void token::Operator::base::combinator(unit::base* currentUnit, size_t& i, token::Operator::type t) {
        // Checks if the current token is a operator token type, if so it needs to be accommodated.
        if (currentUnit->at<lexer::token::base>(i)->get_type() != lexer::token::types::OPERATOR) return;

        if (toType(currentUnit->at<lexer::token::op>(i)->text) != t) return;

        // Check if we have enough tokens to form an operation.
        if (currentUnit->tokens.size() <= 1) throw std::runtime_error("Incomplete operator token at index " + std::to_string(i));

        lexer::token::op* currentOperator = currentUnit->at<lexer::token::op>(i);

        token::base* left = currentUnit->at<lexer::token::base>(i - 1)->parsed;
        token::base* right = currentUnit->at<lexer::token::base>(i + 1)->parsed;

        if (!left) throw std::runtime_error("Left operand not parsed or missing for: " + currentOperator->toString());
        if (!right) throw std::runtime_error("Right operand not parsed or missing for: " + currentOperator->toString());

        // Now we have all the criteria met to construct a combinator operator:
        token::Operator::base* newOperator = new token::Operator::base(
            token::info(
                token::type::OPERATOR,
                currentOperator->get_start(),
                currentUnit->parent,
                currentOperator->text
            ),
            token::Operator::toType(currentOperator->text),
            left,
            right
        );

        // write parsed
        currentUnit->at<lexer::token::op>(i)->parsed = newOperator;

        // Exhaust consumed tokens (<exhaust + <not exhaust> + <exhaust>)
        currentUnit->tokens.erase(currentUnit->tokens.begin() + i + 1);
        currentUnit->tokens.erase(currentUnit->tokens.begin() + i - 1);

        // Update index
        i--;
    }

    void token::Operator::fetcher::combinator(unit::base* currentUnit, size_t& i) {
        // Checks if the current token is a operator token type, if so it needs to be accommodated.
        if (currentUnit->at<lexer::token::base>(i)->get_type() != lexer::token::types::OPERATOR) return;
        if (toType(currentUnit->at<lexer::token::op>(i)->text) != token::Operator::type::FETCHER) return;

        lexer::token::op* currentOperator = currentUnit->at<lexer::token::op>(i);

        token::base* left = currentUnit->at<lexer::token::base>(i - 1)->parsed;

        // Since each fetcher will always at conception cache where the right side is from, we dont need full recursion here
        if (!left) throw std::runtime_error("Use of undefined scope: " +  currentUnit->at<lexer::token::base>(i - 1)->toString());

        token::definition* closestFetcherDefinition;

        if (left->flags == token::type::OPERATOR) {     // a.b.c -> <a.b>.c
            // Now we can take from left->right->baked_definition->search(right->symbol)
            token::Operator::base* leftOperator = dynamic_cast<token::Operator::base*>(left);
            if (!leftOperator) throw std::runtime_error("Internal parser error: failed to cast left operand to operator type in fetcher combinator.");
            
            token::object* fetcherObject = dynamic_cast<token::object*>(leftOperator->right);
            if (!fetcherObject) throw std::runtime_error("Left operand in fetcher is not an object type: " + leftOperator->right->toString());

            // Now we have an identified object which should contain its definition, where we can then find the right side definition from.
            token::definition* bakedDefinition = fetcherObject->reference;
            if (!bakedDefinition) throw std::runtime_error("Object '" + fetcherObject->toString() + "' has no associated definition.");

            closestFetcherDefinition = bakedDefinition;
        }
        else if (left->flags == token::type::OBJECT || left->flags == token::type::CALLER){   // a.b | a().b
            token::object* leftObject = dynamic_cast<token::object*>(left);
            if (!leftObject) throw std::runtime_error("Internal parser error: failed to cast left operand to object type in fetcher combinator.");

            token::definition* bakedDefinition = leftObject->reference;
            if (!bakedDefinition) throw std::runtime_error("Object '" + leftObject->toString() + "' has no associated definition.");

            closestFetcherDefinition = bakedDefinition;
        }
        else {
            throw std::runtime_error("Left operand in fetcher is not a valid type: " + left->toString());
        }

        lexer::token::text* rightSide = currentUnit->at<lexer::token::text>(i + 1);
        if (!rightSide) throw std::runtime_error("Right operand in fetcher is not a valid text token: " + currentUnit->at<lexer::token::base>(i + 1)->toString());

        // Now that we have the definition of our closest fetcher from the chain, we can swift through its inheritances[i]->(casted to scope)->definitions and see if any of them contain right->symbol
        for (const auto& inheritedSymbol : closestFetcherDefinition->inherited) {
            token::base* foundInScope = closestFetcherDefinition->parent->findClosestDefinition(inheritedSymbol);
            if (!foundInScope) continue;

            token::scope::base* foundScope = dynamic_cast<token::scope::base*>(foundInScope);
            if (!foundScope) continue;

            token::base* fetchedDefinition = foundScope->findClosestDefinition(rightSide->data);
            if (fetchedDefinition) {
                // We have found our target definition!
                
                // Now we can construct the fetcher operator:
                token::Operator::base* newFetcherOperator = new token::Operator::base(
                    token::info(
                        token::type::OPERATOR,
                        currentOperator->get_start(),
                        currentUnit->parent,
                        currentOperator->text
                    ),
                    token::Operator::type::FETCHER,
                    left,
                    fetchedDefinition
                );

                // write parsed
                currentUnit->at<lexer::token::op>(i)->parsed = newFetcherOperator;

                // Exhaust consumed tokens (<exhaust + <not exhaust> + <exhaust>)
                currentUnit->tokens.erase(currentUnit->tokens.begin() + i + 1);
                currentUnit->tokens.erase(currentUnit->tokens.begin() + i - 1);

                // Update index
                i--;

                return;
            }
        }
    }

    void token::Operator::fix::base::combinator(unit::base* currentUnit, size_t& i) {
        // Checks if the current token is a operator token type, if so it needs to be accommodated.
        if (currentUnit->at<lexer::token::base>(i)->get_type() != lexer::token::types::OPERATOR) return;

        // Check if we have enough tokens to form an operation.
        if (currentUnit->tokens.size() <= 1) throw std::runtime_error("Incomplete operator token at index " + std::to_string(i));

        lexer::token::op* currentOperator = currentUnit->at<lexer::token::op>(i);

        if (!token::Operator::fix::is(currentOperator->text)) return;   // not ++ or --

        // Fix operators need at least 2 tokens: {OP, operand} or {operand, OP}
        if (currentUnit->tokens.size() < 2) throw std::runtime_error("Not enough tokens to form: " + currentOperator->toString());

        token::base* operand = nullptr;
        fix::type fixity = fix::type::UNKNOWN;
        utils::range operandIndex;

        // Determine if this is prefix or postfix by checking neighboring tokens
        // Check for postfix: operand is at i-1
        if (i > 0) {
            operand = currentUnit->at<lexer::token::base>(i - 1)->parsed;
            if (operand) {
                fixity = fix::type::POST;
            }
        }

        // Check for prefix: operand is at i+1
        if (!operand && i + 1 < currentUnit->tokens.size()) {
            operand = currentUnit->at<lexer::token::base>(i + 1)->parsed;
            if (operand) {
                fixity = fix::type::PRE;
            }
        }

        if (!operand) throw std::runtime_error("No valid operand found for: " + currentOperator->toString());
        if (fixity == fix::type::UNKNOWN) throw std::runtime_error("Could not determine fixity for: " + currentOperator->toString());

        // Now we have all the criteria met to construct a fix operator:
        token::Operator::fix::base* newFixOperator = new token::Operator::fix::base(
            token::info(
                token::type::OPERATOR,
                currentOperator->get_start(),
                currentUnit->parent,
                currentOperator->text
            ),
            operand,
            fixity
        );

        // write the operand for the next user, NOTE: the IR will have to deal with fixity order of pre/post self modification of the operand
        currentUnit->at<lexer::token::base>(i)->parsed = newFixOperator;

        // Exhaust consumed tokens (either {operand, OP} for postfix or {OP, operand} for prefix)
        // But keep the operator token itself (at position i) as it now represents the parsed operation
        if (fixity == fix::type::POST) {
            currentUnit->tokens.erase(currentUnit->tokens.begin() + i - 1);
            // Update index
            i--;
        } else if (fixity == fix::type::PRE) {
            currentUnit->tokens.erase(currentUnit->tokens.begin() + i + 1);
            // Index remains the same
        }
    }

}