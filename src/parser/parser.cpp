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
            
            // Consider here looping through subsets first and then range in them, to boost performance 999+
            // NOTE: if you decide to use subset traversal then you cannot remove mid loop exhausted tokens, so use reverse traversal!
            for (utils::range index = tokens.begin(); index < tokens.end(); ++index) {

                // Starting place.
                unit::base subUnit = this & index;

                // FACTORIES:
                // NOTE: each factory should have the right to exhaust tokens from subsequent factories via '^' operation.
                // -_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_
                token::definition::factory(subUnit);
                token::Operator::base::factory(subUnit);
                // -_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_

                // Remove exhausted subset of tokens from the larger set of tokens we have:
                if (index.min != index.max) {   
                    // If this passed through, it means that what ever function inside the loop did in fact exhaust some tokens of ours, so let's remove those

                    tokens ^= index;
                }
            }
        }
    }

    utils::range unit::findSubsequentTokens(base& Unit, lexer::token::types type) {
        utils::range result;
        for (
            result = Unit.tokens.begin(); 
            result < Unit.tokens.end() &&                   // Check that we are still within the bounds
            Unit.tokens[result.max]->get_type() == type;    // Check that the current token is of the requested type
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
            parent->children.push_back(this);
        }
    }

    void token::definition::factory(unit::base& currentUnit) {
        if (currentUnit.passIndex != unit::pass::FIRST) return;     // Definitions are only created in the first pass.
        if (currentUnit.tokens.getMaxSize() < 2) return;            // Need at least two tokens to form a definition. one for type and one for name

        // Check if current text token is defined or not:
        // - If is, then check next token if it is also an text token and is defined:
        //   - If is, then iterate until found text token which is not defined.
        //   - If not, then that text token which is not defined the new definition name and inherits all prior defined text symbols as types.
        // - If not, return.

        utils::range textTokens = findSubsequentTokens(currentUnit, lexer::token::types::TEXT);

        if (textTokens.length() < 2) return; // Need at least two text tokens to form a definition.

        // We can simply check that the last element is not defined:
        if (currentUnit.parent->findClosestDefinition(currentUnit.at<lexer::token::text>(textTokens.max - 1)->data) != nullptr) {
            throw std::runtime_error("Redefinition of symbol '" + currentUnit.at<lexer::token::text>(textTokens.max - 1)->data + "'");
        }

        // Let's now also fetch the symbols of each inherited text token.
        std::vector<std::string_view> inherited;
        for (int32_t i = textTokens.min; i < textTokens.max - 1; ++i) {
            std::string_view symbol = currentUnit.at<lexer::token::text>(i)->data;

            inherited.push_back(symbol);
        }

        // Definition class automatically handles everything it needs to, keep this accessor in case we need to do some higher level abstract thingies.
        [[maybe_unused]] token::definition::base* newDefinition = new token::definition(
            token::info(
                token::type::DEFINITION,
                currentUnit.at<lexer::token::text>(textTokens.max - 1)->get_start(),
                currentUnit.parent,
                currentUnit.at<lexer::token::text>(textTokens.max - 1)->data
            ),
            inherited
        );

        // Now we exhaust the tokens needed to build this definition, but not he last token, since it will now represent an defined text token for later patterns to recognize.
        currentUnit.tokens ^= utils::range(textTokens.min, textTokens.max - 1);
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

    void token::Operator::base::factory(unit::base& currentUnit) {
        if (currentUnit.passIndex != unit::pass::SECOND) return;    // maybe increase this, since first pass is for definitions...
        
        // Checks if the current token is a operator token type, if so it needs to be accommodated.
        if (currentUnit.at<lexer::token::base>(currentUnit.getCurrentIndex())->get_type() != lexer::token::types::OPERATOR) return;

        // Check if we have enough tokens to form an operation.
        if (currentUnit.tokens.getMaxSize() <= 1) throw std::runtime_error("Incomplete operator token at index " + std::to_string(currentUnit.getCurrentIndex()));

        // Now we can start calling the handlers for this specific operation symbol type:
        lexer::token::op* operatorToken = currentUnit.at<lexer::token::op>(currentUnit.getCurrentIndex());

        switch (token::Operator::toType(operatorToken->text)) {
            case token::Operator::type::FETCHER:
                token::Operator::base::combinator(currentUnit);
                break;
            
            // Other operators go here...

            default:
                throw std::runtime_error("Unhandled operator symbol at index " + std::to_string(currentUnit.getCurrentIndex()));
        }
    }

    
    void token::Operator::base::combinator(unit::base& currentUnit) {
        // Operator::factory already checks for basic checks, so no need to do them here again.
        lexer::token::op* currentOperator = currentUnit.at<lexer::token::op>(currentUnit.getCurrentIndex());
        utils::range exhausted = currentUnit.tokens.begin();
        
        // <bool a> = 1 == 1 && 1 & 1
        // a = <1 == 1> && <1 & 1>
        // a = <l && r>
        // <a = r>

        // Exhaust consumed tokens (<left if needed> + op + <right if needed>)
        currentUnit.tokens ^= exhausted;
    }

}