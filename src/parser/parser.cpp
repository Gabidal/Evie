#include "parser.h"
#include <charconv>
#include <limits>
#include <cfloat>

namespace parser {

    unit::base::base(unit::pass i, token::scope::base* p, bool InString) : passIndex(i), parent(p), tokens(p->rawTokens), inString(InString) {

    }

    void unit::base::factory() {
        for (passIndex = pass::FIRST; passIndex < pass::LAST; ++passIndex) {
            
            // Consider here looping through subsets first and then range in them, to boost performance 999+
            // NOTE: if you decide to use subset traversal then you cannot remove mid loop exhausted tokens, so use reverse traversal!
            for (int32_t index = 0; index < (int32_t)tokens.size(); ++index) {

                token::definition::base::factory(this, index);
                token::context::factory(this, index);   // Right after definition pattern
                token::number::factory(this, index);
                token::object::factory(this, index);
                token::caller::factory(this, index);    // Right after object pattern
                token::scope::parenthesis::factory(this, index);
                token::Operator::fetcher::factory(this, index);
                token::condition::factory(this, index);
                token::looper::factory(this, index);
                token::string::factory(this, index);
                token::string::escape::factory(this, index);
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

    utils::range unit::findSubsequentParsedTokens(base* Unit, int32_t startIndex) {
        utils::range result;
        for (
            result = {startIndex, startIndex}; 
            result.max < (int32_t)Unit->tokens.size() &&                  // Check that we are still within the bounds
            Unit->tokens[result.max]->parsed;                             // Check that the current token is parsed
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

        if (reference) return;  // Dont override existing parsed

        // If not, then we need to find the defined manually
        reference = currentUnit->parent->findClosestDefinition(currentText->data);

        if (!reference) return; // Keywords?

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

            if (!currentUnit->inString)
                currentNumber->text = newNumber->getValue(); // Update lexer token text to match parser token value
        }
    }

    void token::number::determineSize() {
        using namespace std;
        const char* begin = value.data();
        const char* end = value.data() + value.size();

        if (numberType == lexer::token::number::types::INTEGER) {
            long long LLvalue{};
            auto [ptr, ec] = from_chars(begin, end, LLvalue);
            if (ec == errc::result_out_of_range) { minRequiredByteSize = 8; return; }

            if (LLvalue >= numeric_limits<int8_t>::min() && LLvalue <= numeric_limits<int8_t>::max())
                minRequiredByteSize = 1;
            else if (LLvalue >= numeric_limits<int16_t>::min() && LLvalue <= numeric_limits<int16_t>::max())
                minRequiredByteSize = 2;
            else if (LLvalue >= numeric_limits<int32_t>::min() && LLvalue <= numeric_limits<int32_t>::max())
                minRequiredByteSize = 4;
            else
                minRequiredByteSize = 8;
        }
        else if (numberType == lexer::token::number::types::FLOAT) {
            double doubleValue = std::stod(value);
            float f = static_cast<float>(doubleValue);
            // check if conversion preserves value
            minRequiredByteSize = (static_cast<double>(f) == doubleValue) ? 4 : 8;
        }
        else if (numberType == lexer::token::number::types::HEX) {
            unsigned long long LLvalue{};
            auto [ptr, ec] = from_chars(begin, end, LLvalue, 2);
            if (ec == errc::result_out_of_range) { minRequiredByteSize = 8; return; }

            if (LLvalue <= std::numeric_limits<uint8_t>::max())
                minRequiredByteSize = 1;
            else if (LLvalue <= std::numeric_limits<uint16_t>::max())
                minRequiredByteSize = 2;
            else if (LLvalue <= std::numeric_limits<uint32_t>::max())
                minRequiredByteSize = 4;
            else
                minRequiredByteSize = 8;
        }
    }

    void token::number::transformHexIntoInt() {
        if (numberType != lexer::token::number::types::HEX) return;

        const char* begin = value.data();
        const char* end   = value.data() + value.size();

        // skip optional 0x/0X prefix
        if (end - begin >= 2 && begin[0] == '0' && (begin[1] == 'x' || begin[1] == 'X'))
            begin += 2;

        if (begin == end) throw std::runtime_error("Empty hex literal after 0x prefix.");

        unsigned long long LLvalue{};
        auto [ptr, ec] = std::from_chars(begin, end, LLvalue, 16);

        // require success and full consumption to avoid partial parses like "0xFF" -> parsed '0'
        if (ec == std::errc() && ptr == end) {
            value = std::to_string(LLvalue);
            numberType = lexer::token::number::types::INTEGER;
            return;
        }

        throw std::runtime_error("Failed to convert hex number to integer representation.");
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
        unit::base subParserUnit(unit::pass::FIRST, newScope, currentUnit->inString);
        subParserUnit.factory();

        // Now we can write the parsed scope into the wrapper token
        currentWrapper->parsed = newScope;
    }

    void token::string::factory(unit::base* currentUnit, int32_t& startIndex) {
        if (currentUnit->passIndex != unit::pass::FIRST) return;    // Parenthesis's aren't really dependant of anything else, so they can be one of the first to be parsed.

        if (currentUnit->at<lexer::token::base>(startIndex)->get_type() != lexer::token::types::WRAPPER) return;

        lexer::token::wrapper* currentWrapper = currentUnit->at<lexer::token::wrapper>(startIndex);

        if (currentWrapper->parsed) return;

        if (
            currentWrapper->type != lexer::token::wrapper::types::CHARACTER && 
            currentWrapper->type != lexer::token::wrapper::types::STRING
        ) return; // Not a parenthesis type wrapper

        token::string::base* newStringScope = new token::string::base(
            token::info(
                token::type::STRING,
                currentWrapper->get_start(),
                currentUnit->parent,
                std::string("") + currentWrapper->identity
            ),
            currentWrapper->tokens
        );

        // This is done so that HEX pattern and escape pattern can do their job.
        unit::base subParserUnit(unit::pass::FIRST, newStringScope, true);
        subParserUnit.factory();

        newStringScope->bakeTokensToString();

        // Now we can write the parsed string
        currentWrapper->parsed = newStringScope;
    }

    void token::string::escape::factory(unit::base* currentUnit, int32_t& startIndex) {
        if (currentUnit->passIndex != unit::pass::FIRST) return;
        if (!currentUnit->inString) return;
        if (currentUnit->at<lexer::token::base>(startIndex)->get_type() != lexer::token::types::ESCAPE) return;

        lexer::token::escape* esc = currentUnit->at<lexer::token::escape>(startIndex);

        if (esc->sequence[0] == 'n') {
            esc->sequence = "\n";
        } else if (esc->sequence[0] == 't') {
            esc->sequence = "\t";
        } else if (esc->sequence[0] == 'r') {
            esc->sequence = "\r";
        } else if (esc->sequence[0] == '\\') {
            esc->sequence = "\\";
        } else if (esc->sequence[0] == '\'') {
            esc->sequence = "\'";
        } else if (esc->sequence[0] == '\"') {
            esc->sequence = "\"";
        } else if (esc->sequence[0] == 'x') {
            std::string result = "";

            if ((esc->sequence.size() - 1) % 2 != 0) throw std::runtime_error("Invalid hex escape sequence length in string.");

            for (int i = 1; i < (int32_t)esc->sequence.size(); i += 2) {
                std::string hexByteStr = esc->sequence.substr(i, 2);
                char byte = static_cast<char>(std::stoi(hexByteStr, nullptr, 16));
                result += byte;
            }

            esc->sequence = result;
        }
    }

    void token::string::base::bakeTokensToString() {
        std::string result = "";
        result += lexer::token::wrapper::getWrapperCondition(symbol[0]).first;

        for (const auto& token : rawTokens) {
            result += token->getData();
        }

        result += lexer::token::wrapper::getWrapperCondition(symbol[0]).second;

        bakedString = result;
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

    void token::condition::condition::factory(unit::base* currentUnit, int32_t& startIndex) {
        if (currentUnit->passIndex != unit::pass::SECOND) return;    // Conditions need callers to be parsed first.
        
        // require: <text> <any token> <any token>
        if (currentUnit->at<lexer::token::base>(startIndex)->get_type() != lexer::token::types::TEXT) return;
        if (currentUnit->at<lexer::token::base>(startIndex)->parsed) return;    // Keywords should have no parsed data.

        std::string_view symbol = currentUnit->at<lexer::token::text>(startIndex)->data;
        token::base* header = nullptr;
        token::base* body = nullptr;

        utils::range nextTokens = unit::findSubsequentParsedTokens(
            currentUnit,
            startIndex + 1
        );

        if ((symbol == "if" || symbol == "else") && nextTokens.length() == 2) {
            header = currentUnit->at<lexer::token::base>(nextTokens.min)->parsed;
            body = currentUnit->at<lexer::token::base>(nextTokens.max-1)->parsed;
        }
        else if (symbol == "else" && nextTokens.length() == 1) {
            body = currentUnit->at<lexer::token::base>(nextTokens.min)->parsed;
        }
        else return;

        token::condition* newCondition = new token::condition(
            token::info(
                token::type::CONDITION,
                currentUnit->at<lexer::token::text>(startIndex)->get_start(),
                currentUnit->parent,
                currentUnit->at<lexer::token::text>(startIndex)->data
            ),
            header,
            body
        );

        currentUnit->at<lexer::token::text>(startIndex)->parsed = newCondition;

        // Remove
        currentUnit->tokens.erase(currentUnit->tokens.begin() + nextTokens.min, currentUnit->tokens.begin() + nextTokens.max);
    }

    void token::looper::factory(unit::base* currentUnit, int32_t& startIndex) {
        if (currentUnit->passIndex != unit::pass::SECOND) return;    // Loopers need callers to be parsed first.
        
        // require: <text> <any token> <any token>
        if (currentUnit->at<lexer::token::base>(startIndex)->get_type() != lexer::token::types::TEXT) return;
        if (currentUnit->at<lexer::token::base>(startIndex)->parsed) return;    // Keywords should have no parsed data.

        std::string_view symbol = currentUnit->at<lexer::token::text>(startIndex)->data;
        lexer::token::base* header = nullptr;
        token::base* body = nullptr;

        utils::range nextTokens = unit::findSubsequentParsedTokens(
            currentUnit,
            startIndex + 1
        );

        if ((symbol == "while" || symbol == "for") && nextTokens.length() == 2) {
            header = currentUnit->at<lexer::token::base>(nextTokens.min);
            body = currentUnit->at<lexer::token::base>(nextTokens.max-1)->parsed;
        }
        else return;

        token::base* init = nullptr;       // int i = 0, call()
        token::base* condition = nullptr;  // i < size, true
        token::base* footer = nullptr;     // i++, call(&i)

        // First lets check if its: while true, or while (...)
        if (header->get_type() == lexer::token::types::WRAPPER) {
            auto parsedHeader = dynamic_cast<token::scope::base*>(header->parsed);

            // Let's now see how many parsed tokens this wrapper token contains
            if (parsedHeader->rawTokens.size() >= 6) throw std::runtime_error("Loop parsedHeader contains too many tokens, maximum is 3 operations + 3 separators.");

            if (parsedHeader->rawTokens.size() == 1 && parsedHeader->rawTokens[0]->parsed) {
                condition = parsedHeader->rawTokens[0]->parsed;
            }
            else {
                // We need to discern between (; i < a;), (;; i++), (int i = 0; i < a; i++), (int i;;)
                std::vector<std::pair<int32_t, token::base*>> operandLocation;

                int32_t separatorCount = 0;

                for (int32_t i = 0; i < (int32_t)parsedHeader->rawTokens.size(); i++) {
                    // if we hit an separator lets log it
                    if (parsedHeader->rawTokens[i]->get_type() == lexer::token::types::SEPARATOR) separatorCount++;
                    // If we hit a parsed token we log it with the current separator count
                    else if (parsedHeader->rawTokens[i]->parsed) operandLocation.push_back({separatorCount, parsedHeader->rawTokens[i]->parsed});
                }

                for (const auto& [index, operand] : operandLocation) {
                    if (index == 0) {
                        init = operand;
                    }
                    else if (index == 1) {
                        condition = operand;
                    }
                    else if (index == 2) {
                        footer = operand;
                    }
                }
            }
        }
        else {
            // Either a straight boolean is given or function call so basic base suffices
            condition = currentUnit->at<lexer::token::base>(nextTokens.min)->parsed;
        }


        token::looper* newLooper = new token::looper(
            token::info(
                token::type::LOOP,
                currentUnit->at<lexer::token::text>(startIndex)->get_start(),
                currentUnit->parent,
                currentUnit->at<lexer::token::text>(startIndex)->data
            ),
            init,
            condition,
            footer,
            body
        );

        currentUnit->at<lexer::token::text>(startIndex)->parsed = newLooper;

        // Remove
        currentUnit->tokens.erase(currentUnit->tokens.begin() + nextTokens.min, currentUnit->tokens.begin() + nextTokens.max);
    
    }
}