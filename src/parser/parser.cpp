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
                token::Operator::base::factory(this, index);
                // -_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_

            }
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

    void token::definition::factory(unit::base* currentUnit, size_t startIndex) {
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

        name->parsed = newDefinition;

        // Now we exhaust the tokens needed to build this definition, but not he last token, since it will now represent an defined text token for later patterns to recognize.
        currentUnit->tokens.erase(currentUnit->tokens.begin() + textTokens.min, currentUnit->tokens.begin() + textTokens.max - 1);
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
        } else if (condition::is(symbol)) {
            return token::Operator::type::CONDITION;
        } else if (symbol == "&") {
            return token::Operator::type::AND;
        } else if (symbol == "¤") {
            return token::Operator::type::XOR;
        } else if (symbol == "|") {
            return token::Operator::type::OR;
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
    
    bool token::Operator::condition::is(std::string_view symbol) {
        if (symbol == "<" || symbol == ">" || symbol == "<=" || symbol == ">=" || symbol == "==" || symbol == "!=" || symbol == "&&" || symbol == "||") {
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

    void token::Operator::base::factory(unit::base* currentUnit, size_t startIndex) {
        if (currentUnit->passIndex != unit::pass::SECOND) return;    // maybe increase this, since first pass is for definitions...
        
        // Checks if the current token is a operator token type, if so it needs to be accommodated.
        if (currentUnit->at<lexer::token::base>(startIndex)->get_type() != lexer::token::types::OPERATOR) return;

        // Check if we have enough tokens to form an operation.
        if (currentUnit->tokens.size() <= 1) throw std::runtime_error("Incomplete operator token at index " + std::to_string(startIndex));

        // Now we can start calling the handlers for this specific operation symbol type:
        lexer::token::op* operatorToken = currentUnit->at<lexer::token::op>(startIndex);

        switch (token::Operator::toType(operatorToken->text)) {
            case token::Operator::type::FIX:
                token::Operator::fix::base::combinator(currentUnit, startIndex);
                break;
            default:    // Basically almost all of operator types are {L, op, R} combinations.
                token::Operator::base::combinator(currentUnit, startIndex);
        }
    }

    void token::Operator::base::combinator(unit::base* currentUnit, size_t startIndex) {
        // Operator::factory already checks for basic checks, so no need to do them here again.
        int32_t i = startIndex;
        lexer::token::op* currentOperator = currentUnit->at<lexer::token::op>(i);

        // <bool a> = 1 == 1 && 1 & 1
        // a = <1 == 1> && <1 & 1>
        // a = <l && r>
        // <a = r>

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
    }

    void token::Operator::fix::base::combinator(unit::base* currentUnit, size_t startIndex) {
        // Operator::factory already checks for basic checks, so no need to do them here again.
        lexer::token::op* currentOperator = currentUnit->at<lexer::token::op>(startIndex);

        // Fix operators need at least 2 tokens: {OP, operand} or {operand, OP}
        if (currentUnit->tokens.size() < 2) throw std::runtime_error("Not enough tokens to form: " + currentOperator->toString());

        token::base* operand = nullptr;
        fix::type fixity = fix::type::UNKNOWN;
        utils::range operandIndex;

        // Determine if this is prefix or postfix by checking neighboring tokens
        // Check for postfix: operand is at i-1
        if (startIndex > 0) {
            operand = currentUnit->at<lexer::token::base>(startIndex - 1)->parsed;
            if (operand) {
                fixity = fix::type::POST;
            }
        }

        // Check for prefix: operand is at i+1
        if (!operand && startIndex + 1 < currentUnit->tokens.size()) {
            operand = currentUnit->at<lexer::token::base>(startIndex + 1)->parsed;
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
        currentUnit->at<lexer::token::base>(startIndex)->parsed = newFixOperator;

        // Exhaust consumed tokens (either {operand, OP} for postfix or {OP, operand} for prefix)
        // But keep the operator token itself (at position i) as it now represents the parsed operation
        if (fixity == fix::type::POST) {
            currentUnit->tokens.erase(currentUnit->tokens.begin() + startIndex - 1);
        } else if (fixity == fix::type::PRE) {
            currentUnit->tokens.erase(currentUnit->tokens.begin() + startIndex + 1);
        }
    }

}