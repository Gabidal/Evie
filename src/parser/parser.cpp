#include "parser.h"
#include <charconv>
#include <limits>
#include <cfloat>

namespace parser {

    unit::base::base(unit::pass i, token::scope::base* p) : passIndex(i), parent(p), tokens(p->rawTokens) {

    }

    void unit::base::factory() {
        for (passIndex = pass::FIRST; passIndex < pass::LAST; ++passIndex) {
            
            // Consider here looping through subsets first and then range in them, to boost performance 999+
            // NOTE: if you decide to use subset traversal then you cannot remove mid loop exhausted tokens, so use reverse traversal!
            for (int32_t index = 0; index < (int32_t)tokens.size(); ++index) {

                // FACTORIES:
                // -_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_
                token::definition::base::factory(this, index);
                token::context::factory(this, index);   // Right after definition pattern
                token::number::factory(this, index);
                token::object::factory(this, index);
                token::caller::factory(this, index);    // Right after object pattern
                token::scope::parenthesis::factory(this, index);
                token::Operator::fetcher::factory(this, index);
                // -_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_
                
            }
            
            // SPECIAL FACTORIES:
            // -_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_
            token::Operator::base::factory(this);
        }

        // Harvest parsed tokens and append them to scope children
        for (auto& t : tokens) {
            if (t->parsed) {
                parent->children.push_back(t->parsed);
            }
        }
    }

    utils::range unit::findSubsequentTokens(base* Unit, lexer::token::types type, int32_t startIndex) {
        utils::range result;
        for (
            result = {startIndex, startIndex}; 
            result.max < (int32_t)Unit->tokens.size() &&                  // Check that we are still within the bounds
            Unit->tokens[result.max]->get_type() == type;        // Check that the current token is of the requested type
            result.max++
        );

        return result;
    }

    ::utils::range unit::findSubsequentTokens(base* Unit, std::vector<std::pair<lexer::token::types, std::string_view>> Requirements, int32_t startIndex) {
        utils::range result;

        // Checks if the lexer token fills any of the requirements
        auto fits = [&Requirements](lexer::token::base* t){
            for (const auto& req : Requirements) {
                if (t->get_type() == req.first && (req.second.empty() ? true : t->getData() == req.second)) {
                    return true;
                }
            }
            return false;
        };

        for (
            result = {startIndex, startIndex}; 
            result.max < (int32_t)Unit->tokens.size() &&                  // Check that we are still within the bounds
            fits(Unit->tokens[result.max]);        // Check that the current token fits any of the requirements
            result.max++
        );

        return result;
    }

    void unit::replaceDefinition(token::base* old, token::base* New) {
        if (!old->parent) throw std::runtime_error("Cannot replace definition of a token with no parent scope.");

        for (auto& t : old->parent->definitions) {
            if (t == old) {
                t = New;
                return;
            }
        }
    }

    token::base* token::base::findClosestDefinition(std::string_view Symbol) {
        // Default behaviour is to pipe this call to the parent hoping it might hit the scope class.
        return parent ? parent->findClosestDefinition(Symbol) : nullptr;
    }

    token::definition::base::base(info Info, std::vector<std::string_view> toInherit, types defType) : token::base(Info), inherited(toInherit), definitionType(defType) {
        if (parent) {
            parent->definitions.push_back(this);
        }
    }

    void token::definition::base::factory(unit::base* currentUnit, int32_t& startIndex) {
        if (currentUnit->passIndex != unit::pass::FIRST) return;    // Definitions are only created in the first pass.
        if (currentUnit->tokens.size() < 2) return;                 // Need at least two tokens to form a definition. one for type and one for name

        utils::range textTokens = findSubsequentTokens(
            currentUnit, 
            {
                {lexer::token::types::TEXT, ""},
                {lexer::token::types::OPERATOR, "."}    // For fetcher inheritances
            },
            startIndex
        );

        if (textTokens.length() < 2) return; // Need at least two text tokens to form a definition.

        // Check that if fetcher operators are present they are atleast processed.
        for (int32_t i = textTokens.min; i < textTokens.max; i++) {
            if (currentUnit->tokens[i]->get_type() == lexer::token::types::OPERATOR && currentUnit->tokens[i]->parsed == nullptr) {
                return;
            }
        }

        lexer::token::text* name = currentUnit->at<lexer::token::text>(textTokens.max - 1);

        if (name->parsed) {
            // This text token is already parsed, cannot be a definition name.
            return;
        }

        // Let's now also fetch the symbols of each inherited text token.
        std::vector<std::string_view> inherited;
        for (int32_t i = textTokens.min; i < textTokens.max - 1; ++i) {

            if (currentUnit->tokens[i]->get_type() == lexer::token::types::OPERATOR) {
                token::object* rightOperand = dynamic_cast<token::object*>(dynamic_cast<token::Operator::base*>(currentUnit->at<lexer::token::base>(i)->parsed)->right);

                for (auto operandInherit : rightOperand->reference->inherited) {
                    inherited.push_back(operandInherit);
                }
            }
            else {
                std::string_view symbol = currentUnit->at<lexer::token::text>(i)->data;
                inherited.push_back(symbol);
            }
        }

        // Definition class automatically handles everything it needs to, keep this accessor in case we need to do some higher level abstract thingies.
        [[maybe_unused]] token::definition::base* newDefinition = new token::definition::base(
            token::info(
                token::type::DEFINITION,
                name->get_start(),
                currentUnit->parent,
                name->data
            ),
            inherited
        );

        // NOTE: For overloads to work we need to inject the definition into the parsed, this gives the object factory a "TRUE" reference to the correct definition and not just the first occurring overload.
        name->parsed = newDefinition;

        // Now we exhaust the tokens needed to build this definition, but not he last token, since it will now represent an defined text token for later patterns to recognize.
        currentUnit->tokens.erase(currentUnit->tokens.begin() + textTokens.min, currentUnit->tokens.begin() + textTokens.max - 1);
        // Update index
        startIndex = textTokens.min;
    }

    void token::object::factory(unit::base* currentUnit, int32_t& startIndex) {
        if (currentUnit->passIndex != unit::pass::FIRST) return;    // Dependant of definition pattern, but can happen on same pass.
    
        lexer::token::text* currentText = currentUnit->at<lexer::token::text>(startIndex);

        if (!currentText) return; // Not a text token, cannot be an object.

        token::base* reference = currentText->parsed;

        if (reference) return;

        // If not, then we need to find the defined manually
        reference = currentUnit->parent->findClosestDefinition(currentText->data);

        token::object* newObject = new token::object(
            token::info(
                token::type::OBJECT,
                currentText->get_start(),
                currentUnit->parent,
                currentText->data
            ),
            static_cast<token::definition::base*>(reference)
        );

        // now we override the definition factory made parsed reference with the actual object parsed value.
        currentText->parsed = newObject;
    }

    void token::number::factory(unit::base* currentUnit, int32_t& startIndex) {
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

    void token::number::determineSize() {
        using namespace std;
        const char* begin = text.data();
        const char* end = text.data() + text.size();

        if (number_type == types::INTEGER) {
            long long value{};
            auto [ptr, ec] = from_chars(begin, end, value);
            if (ec == errc::result_out_of_range) { minRequiredByteSize = 8; return; }

            if (value >= numeric_limits<int8_t>::min() && value <= numeric_limits<int8_t>::max())
                minRequiredByteSize = 1;
            else if (value >= numeric_limits<int16_t>::min() && value <= numeric_limits<int16_t>::max())
                minRequiredByteSize = 2;
            else if (value >= numeric_limits<int32_t>::min() && value <= numeric_limits<int32_t>::max())
                minRequiredByteSize = 4;
            else
                minRequiredByteSize = 8;
        }
        else if (number_type == types::FLOAT) {
            double value = std::stod(text);
            float f = static_cast<float>(value);
            // check if conversion preserves value
            minRequiredByteSize = (static_cast<double>(f) == value) ? 4 : 8;
        }
        else if (number_type == types::HEX) {
            unsigned long long value{};
            auto [ptr, ec] = from_chars(begin, end, value, 2);
            if (ec == errc::result_out_of_range) { minRequiredByteSize = 8; return; }

            if (value <= std::numeric_limits<uint8_t>::max())
                minRequiredByteSize = 1;
            else if (value <= std::numeric_limits<uint16_t>::max())
                minRequiredByteSize = 2;
            else if (value <= std::numeric_limits<uint32_t>::max())
                minRequiredByteSize = 4;
            else
                minRequiredByteSize = 8;
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

        // '++' '--'
        for (int32_t i = 0; i < (int32_t)currentUnit->tokens.size(); i++) {
            token::Operator::fix::base::combinator(currentUnit, i);
        }

        // '*' '/' '%'
        for (int32_t i = 0; i < (int32_t)currentUnit->tokens.size(); i++) {
            token::Operator::base::combinator(currentUnit, i, token::Operator::type::MULTIPLICATION);
            token::Operator::base::combinator(currentUnit, i, token::Operator::type::DIVISION);
            token::Operator::base::combinator(currentUnit, i, token::Operator::type::MODULO);
        }

        // '+' '-'
        for (int32_t i = 0; i < (int32_t)currentUnit->tokens.size(); i++) {
            token::Operator::base::combinator(currentUnit, i, token::Operator::type::ADDITION);
            token::Operator::base::combinator(currentUnit, i, token::Operator::type::SUBTRACTION);
        }

        // '<<' '>>'
        for (int32_t i = 0; i < (int32_t)currentUnit->tokens.size(); i++) {
            token::Operator::base::combinator(currentUnit, i, token::Operator::type::BITSHIFT_LEFT);
            token::Operator::base::combinator(currentUnit, i, token::Operator::type::BITSHIFT_RIGHT);
        }

        // Comparisons: '<' '>' '<=' '>=' '==' '!='
        for (int32_t i = 0; i < (int32_t)currentUnit->tokens.size(); i++) {
            token::Operator::base::combinator(currentUnit, i, token::Operator::type::COMPARISON);
        }

        // '&' '¤' '|'
        for (int32_t i = 0; i < (int32_t)currentUnit->tokens.size(); i++) {
            token::Operator::base::combinator(currentUnit, i, token::Operator::type::AND);
            token::Operator::base::combinator(currentUnit, i, token::Operator::type::XOR);
            token::Operator::base::combinator(currentUnit, i, token::Operator::type::OR);
        }

        // '&&'
        for (int32_t i = 0; i < (int32_t)currentUnit->tokens.size(); i++) {
            token::Operator::base::combinator(currentUnit, i, token::Operator::type::LOGICAL_AND);
        }

        // '||'
        for (int32_t i = 0; i < (int32_t)currentUnit->tokens.size(); i++) {
            token::Operator::base::combinator(currentUnit, i, token::Operator::type::LOGICAL_OR);
        }

        // Assignments
        for (int32_t i = 0; i < (int32_t)currentUnit->tokens.size(); i++) {
            token::Operator::base::combinator(currentUnit, i, token::Operator::type::ASSIGN);
        }
    }

    void token::Operator::base::combinator(unit::base* currentUnit, int32_t& i, token::Operator::type t) {
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

    void token::Operator::fetcher::factory(unit::base* currentUnit, int32_t& i) {
        if (currentUnit->passIndex != unit::pass::FIRST) return;

        // Checks if the current token is a operator token type, if so it needs to be accommodated.
        if (currentUnit->at<lexer::token::base>(i)->get_type() != lexer::token::types::OPERATOR) return;
        if (toType(currentUnit->at<lexer::token::op>(i)->text) != token::Operator::type::FETCHER) return;

        lexer::token::op* currentOperator = currentUnit->at<lexer::token::op>(i);
        if (currentOperator->parsed) return;

        token::base* left = currentUnit->at<lexer::token::base>(i - 1)->parsed;

        // Since each fetcher will always at conception cache where the right side is from, we dont need full recursion here
        if (!left) throw std::runtime_error("Use of undefined scope: " +  currentUnit->at<lexer::token::base>(i - 1)->toString());

        token::definition::base* closestFetcherDefinition;

        if (left->flags == token::type::OPERATOR) {     // a.b.c -> <a.b>.c
            // Now we can take from left->right->baked_definition->search(right->symbol)
            token::Operator::base* leftOperator = dynamic_cast<token::Operator::base*>(left);
            if (!leftOperator) throw std::runtime_error("Internal parser error: failed to cast left operand to operator type in fetcher combinator.");
            
            token::object* fetcherObject = dynamic_cast<token::object*>(leftOperator->right);
            if (!fetcherObject) throw std::runtime_error("Left operand in fetcher is not an object type: " + leftOperator->right->toString());

            // Now we have an identified object which should contain its definition, where we can then find the right side definition from.
            token::definition::base* bakedDefinition = fetcherObject->reference;
            if (!bakedDefinition) throw std::runtime_error("Object '" + fetcherObject->toString() + "' has no associated definition.");

            closestFetcherDefinition = bakedDefinition;
        }
        else if (left->flags == token::type::OBJECT || left->flags == token::type::CALLER){   // a.b | a().b
            token::object* leftObject = dynamic_cast<token::object*>(left);
            if (!leftObject) throw std::runtime_error("Internal parser error: failed to cast left operand to object type in fetcher combinator.");

            token::definition::base* bakedDefinition = leftObject->reference;
            if (!bakedDefinition) throw std::runtime_error("Object '" + leftObject->toString() + "' has no associated definition.");

            closestFetcherDefinition = bakedDefinition;
        }
        else {
            throw std::runtime_error("Left operand in fetcher is not a valid type: " + left->toString());
        }

        lexer::token::text* rightSide = currentUnit->at<lexer::token::text>(i + 1);
        if (!rightSide) throw std::runtime_error("Right operand in fetcher is not a valid text token: " + currentUnit->at<lexer::token::base>(i + 1)->toString());

        std::vector<token::base*> inherited = {closestFetcherDefinition};

        // fill inherited
        for (const auto& inheritedSymbol : closestFetcherDefinition->inherited) {
            token::base* foundScope = closestFetcherDefinition->parent->findClosestDefinition(inheritedSymbol);
            if (foundScope) {
                inherited.push_back(foundScope);
            }
        }

        // Now that we have the definition of our closest fetcher from the chain, we can swift through its inheritances[i]->(casted to scope)->definitions and see if any of them contain right->symbol
        for (const auto& foundScope : inherited) {

            token::base* fetchedDefinition = foundScope->findClosestDefinition(rightSide->data);
            if (fetchedDefinition) {
                // We have found our target definition!

                // Let's make a object token from the fetched definition:
                token::object* fetchedObject = new token::object(
                    token::info(
                        token::type::OBJECT,
                        rightSide->get_start(),
                        currentUnit->parent,
                        rightSide->data
                    ),
                    dynamic_cast<token::definition::base*>(fetchedDefinition)
                );
                
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
                    fetchedObject
                );

                // write parsed
                currentUnit->at<lexer::token::op>(i)->parsed = newFetcherOperator;

                // Exhaust consumed tokens (<exhaust + <not exhaust> + <exhaust>)
                currentUnit->tokens.erase(currentUnit->tokens.begin() + i + 1);
                currentUnit->tokens.erase(currentUnit->tokens.begin() + i - 1);

                // Update index
                i -= 2; // Go back two times since it should be the last iterable factory, so that next factories re-read what this produced, mainly for definition factory.

                return;
            }
        }
    }

    void token::Operator::fix::base::combinator(unit::base* currentUnit, int32_t& i) {
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
        if (!operand && i + 1 < (int32_t)currentUnit->tokens.size()) {
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

    void token::scope::parenthesis::factory(unit::base* currentUnit, int32_t& startIndex) {
        if (currentUnit->passIndex != unit::pass::FIRST) return;    // Parenthesis's aren't really dependant of anything else, so they can be one of the first to be parsed.

        if (currentUnit->at<lexer::token::base>(startIndex)->get_type() != lexer::token::types::WRAPPER) return;

        lexer::token::wrapper* currentWrapper = currentUnit->at<lexer::token::wrapper>(startIndex);

        if (currentWrapper->parsed) return;

        if (
            currentWrapper->type != lexer::token::wrapper::types::ROUND_BRACKETS && 
            currentWrapper->type != lexer::token::wrapper::types::CURLY_BRACKETS &&
            currentWrapper->type != lexer::token::wrapper::types::SQUARE_BRACKETS
        ) return; // Not a parenthesis type wrapper

        parser::token::scope::base* contextualParent = currentUnit->parent;

        // This may be controversial, but:
        // For: [](){} <-- each prior parenthesis contains definitions that the next parenthesis may try to access
        // Same for: func(){} <-- where the parameters are defined in the prior parenthesis
        // Same for: func<>(){} <-- same here :)

        // Check if i-1 exists and if it is a wrapper
        if (startIndex > 0 && currentUnit->at<lexer::token::base>(startIndex - 1)->get_type() == lexer::token::types::WRAPPER) {
            lexer::token::wrapper* priorWrapper = currentUnit->at<lexer::token::wrapper>(startIndex - 1);

            // Check if prior wrapper is parsed and is a scope, since we do this for each wrapper all prior i-n scopes are already set
            if (priorWrapper->parsed && priorWrapper->parsed->flags == token::type::SCOPE) {
                contextualParent = static_cast<token::scope::base*>(priorWrapper->parsed);
            }
        }

        // First we need to create an local scope to give to our sub-parser
        token::scope::base* newScope = new token::scope::base(
            token::info(
                token::type::SCOPE,
                currentWrapper->get_start(),
                contextualParent,
                std::string("") + currentWrapper->identity
            ),
            currentWrapper->tokens
        );

        // Now we can create the sub-parser unit for this scope
        unit::base subParserUnit(unit::pass::FIRST, newScope);
        subParserUnit.factory();

        // Now we can write the parsed scope into the wrapper token
        currentWrapper->parsed = newScope;
    }

    void token::context::factory(unit::base* currentUnit, int32_t& startIndex) {
        if (currentUnit->passIndex != unit::pass::FIRST) return;    // Contexts need definitions to be parsed first.

        constexpr int minimumRequiredWrapperCountForContexts = 1;

        // <definition> <parenthesis (any)>...*n
        if (startIndex + minimumRequiredWrapperCountForContexts >= (int32_t)currentUnit->tokens.size()) return; // Not enough tokens to form a function
        if (currentUnit->at<lexer::token::base>(startIndex)->get_type() != lexer::token::types::TEXT) return;
        if (!currentUnit->at<lexer::token::text>(startIndex)->parsed) return;   // Require the symbol to be parsed

        utils::range wrapperRange = unit::findSubsequentTokens(
            currentUnit,
            lexer::token::types::WRAPPER,
            startIndex + 1
        );

        if (wrapperRange.length() < minimumRequiredWrapperCountForContexts) return; // Need at least one wrapper to create an context

        // Check that none of the wrappers have been parsed yet
        for (int32_t wi = wrapperRange.min; wi < wrapperRange.max; ++wi) {
            if (currentUnit->at<lexer::token::wrapper>(wi)->parsed) return; // One of the wrappers is already parsed, cannot form context here.
        }

        token::definition::base* SymbolDefinition = dynamic_cast<token::definition::base*>(currentUnit->at<lexer::token::text>(startIndex)->parsed);

        if (!SymbolDefinition) return;  // Probably a caller pattern.
        
        token::context* newContext = new token::context(*SymbolDefinition);

        unit::replaceDefinition(SymbolDefinition, newContext);

        // Now that the context object pointer has been set, we can parse the wrapper tokens
        for (int32_t wi = wrapperRange.min; wi < wrapperRange.max; ++wi) {
            token::scope::parenthesis::factory(currentUnit, wi);
            newContext->wrappers.push_back(dynamic_cast<token::scope::base*>(currentUnit->at<lexer::token::wrapper>(wi)->parsed));
        }

        currentUnit->at<lexer::token::text>(startIndex)->parsed = newContext;

        // remove i+wrappers.size()
        currentUnit->tokens.erase(currentUnit->tokens.begin() + wrapperRange.min, currentUnit->tokens.begin() + wrapperRange.max);
    }

    void token::caller::factory(unit::base* currentUnit, int32_t& startIndex) {
        if (currentUnit->passIndex != unit::pass::FIRST) return;    // Contexts need definitions to be parsed first.

        constexpr int minimumRequiredWrapperCountForCallers = 1;

        // <object> <parenthesis (any)>...*n
        if (startIndex + minimumRequiredWrapperCountForCallers >= (int32_t)currentUnit->tokens.size()) return; // Not enough tokens to form a function
        if (currentUnit->at<lexer::token::base>(startIndex)->get_type() != lexer::token::types::TEXT) return;
        if (!currentUnit->at<lexer::token::text>(startIndex)->parsed) return;   // Require the symbol to be parsed

        utils::range wrapperRange = unit::findSubsequentTokens(
            currentUnit,
            lexer::token::types::WRAPPER,
            startIndex + 1
        );

        if (wrapperRange.length() < minimumRequiredWrapperCountForCallers) return; // Need at least one wrapper to create an context

        // Check that none of the wrappers have been parsed yet
        for (int32_t wi = wrapperRange.min; wi < wrapperRange.max; ++wi) {
            if (currentUnit->at<lexer::token::wrapper>(wi)->parsed) return; // One of the wrappers is already parsed, cannot form context here.
        }

        token::object* Symbolobject = dynamic_cast<token::object*>(currentUnit->at<lexer::token::text>(startIndex)->parsed);

        if (!Symbolobject) return;  // Probably a context pattern.
        
        token::caller* newCaller = new token::caller(info(dynamic_cast<token::base*>(Symbolobject)), Symbolobject->reference);

        // Now that the context object pointer has been set, we can parse the wrapper tokens
        for (int32_t wi = wrapperRange.min; wi < wrapperRange.max; ++wi) {
            token::scope::parenthesis::factory(currentUnit, wi);
            newCaller->parameters.push_back(dynamic_cast<token::scope::base*>(currentUnit->at<lexer::token::wrapper>(wi)->parsed));
        }

        currentUnit->at<lexer::token::text>(startIndex)->parsed = newCaller;

        // remove i+wrappers.size()
        currentUnit->tokens.erase(currentUnit->tokens.begin() + wrapperRange.min, currentUnit->tokens.begin() + wrapperRange.max);
    }
}