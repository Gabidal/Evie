#include "preprocessor.h"

void preprocessor::unit::factory() {

    for (int i = 0; i < (int32_t)currentScope->children.size(); i++) {
        includer::openInclude(this, i);
        includer::closeInclude(this, i);

        walkThroughScopes(i);   // prioritize sub scopes and their inner contents before proceeding.

        unwrap::branches(this, i);

        solver::determineLifetimes(this, i);

        solver::interpreter::factory(this, i);
    }
}

void preprocessor::unit::walkThroughScopes(int32_t index) {
    for (auto innerScope : currentScope->children[index]->getWalkable()) {

        preprocessor::unit subPreprocessorUnit(innerScope, arguments, stack);
        subPreprocessorUnit.factory();

    }
}

void preprocessor::includer::openInclude(preprocessor::unit* currentUnit, int32_t& index) {
    if (currentUnit->currentScope->children[index]->type != parser::token::types::INCLUDE) return;
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
        parser::unit::lexerOutput inlined = docker::file::translate(include->source, currentUnit->arguments);
    
        currentUnit->stack->add(include->source);

        // First transform the BEGIN includer into an END includer
        include->includeType = parser::token::includer::types::END;
    
        // We put the included tokens into a tmp scope so that we can have more control over the parsed output
        parser::token::scope::base* tmpScopeForInline = new parser::token::scope::base(
            parser::token::info(
                parser::token::types::SCOPE,
                include->position,
                currentUnit->currentScope,
                "include_inline_scope_" + std::to_string(include->position.y) + "_" + std::to_string(include->position.x)
            ),
            inlined
        );

        // Now we need to call the parser on the inlined tokens with the other tokens
        parser::unit::base* subParser = new parser::unit::base(parser::unit::pass::FIRST, tmpScopeForInline);
        subParser->factory();

        currentUnit->currentScope->insert(tmpScopeForInline, index);
    }
}

void preprocessor::includer::closeInclude(preprocessor::unit* currentUnit, int32_t& index) {
    if (currentUnit->currentScope->children[index]->type != parser::token::types::INCLUDE) return;
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

void preprocessor::unwrap::branches(preprocessor::unit* currentUnit, int32_t& index) {
    if (currentUnit->currentScope->children[index]->type != parser::token::types::CONDITION) return;
    auto* conditional = dynamic_cast<parser::token::condition*>(currentUnit->currentScope->children[index]);

    /**
     * NOTE: casts do NOT work and make this brittle, this can be fixed with a linear transformation style approach of two linked parser tokens, source and the projected destination-
     * where the format and size can be changed to whatever it is changed into, be it integer into float or smaller to bigger or vise versa.
     */

    /**
     * For us to determine if this condition is unwraptable, we need to first call preprocessor to inline and simplify the condition.
     * If the resulting condition is zero, we remove the entire conditional from the AST.
     * If the result is non-zero number, we wll proceed to inline.
     * If the resulting condition is other than evaluatable type, we do nothing, this is because it is most likely a run-time condition.
     */

    preprocessor::unit subPreprocessorUnit(conditional->header, currentUnit->arguments, currentUnit->stack);
    subPreprocessorUnit.factory();

    // First for little bit more generalization, let's create a list with all the possible branches.
    std::vector<parser::token::condition*> allBranches(conditional->branches.size() + 1);

    // Fetch from the primary condition all secondary conditions.
    for  (size_t i = 0; i < conditional->branches.size(); i++) {
        allBranches[i + 1] = conditional->branches[i];
    }

    // Add the primary condition to the start.
    allBranches.front() = conditional;


    /**
     * Now we can start go through each conditional branch and check if it is able to be inlined or not.
     * We can assume that each iteration thought he branches means the previous condition did not result in inline.
     * If any of the current conditions contain non-compile time tokens, we stop.
     * If an inlinable condition contains secondary conditions which contain non-compile time tokens, we remove them since they will never reach to be run.
     */ 

    for (size_t i = 0; i < allBranches.size(); i++) {
        bool inlineBranch = false;
        bool isDefaultBranch = (i == allBranches.size() - 1) && (allBranches[i]->header == nullptr);
        bool isCompileTimeEvaluatable = (allBranches[i]->header && !allBranches[i]->header->children.empty() && (
            allBranches[i]->header->children.back()->type == parser::token::types::NUMBER ||    // Any value other than zero is held as true.
            allBranches[i]->header->children.back()->type == parser::token::types::OBJECT       // Objects will be inspected later on, much more closer.
        ));

        if (!isDefaultBranch) {
            if (!isCompileTimeEvaluatable) break;   // Weak try, smallest mismatch should end in inline attempt termination.

            auto* unknown = allBranches[i]->header->children.back();

            /**
             * Here we can split evaluation into two different categories:
             *  - A) Directly evaluatable values, like {numbers, boolean, overridden operator for classes?}
             *  - B) Indirectly evaluatable values, like pointers, where as long as there is a compile-time value bound to this pointer and the value is not nullptr, it can inline the condition. 
             * 
             */


            parser::token::base* heldValueAtIndex = solver::getLifetimeValueFrom(unknown);

            if (!heldValueAtIndex) continue;

            /**
             * Now the value held by the condition, can be interpretred by two ways:
             *   - A) If the condition is a direct, its value needs only to be checked to NOT be zero.
             *   - B) If the condition is an indirect type, then as long as the heldValueAtIndex is NOT nullptr, then its true.
             */

            if (unknown->type == parser::token::types::NUMBER) {
                auto* num = dynamic_cast<parser::token::number*>(unknown);

                // Here we check that the value is non zero.
                if (num->getValueWithBooleanOverrideAsNumber<int64_t>()) {
                    inlineBranch = true;
                }
            } else if (unknown->type == parser::token::types::OBJECT) {
                auto* obj = dynamic_cast<parser::token::object*>(unknown);

                if (obj->inherits(utils::KEYWORDS::PTR)) {
                    inlineBranch = true;    // Since the check for heldValueAtIndex != is checked above, we know that if we get here it means its a ptr variable who does not hold a nullptr value.
                } else {
                    // I don't really know what it means to be here exactly, we could recursively fetch x = getLifetimeValueFrom(x), until it reaches a nullptr or a number value...
                }
            }
        }
        else {
            inlineBranch = true;
        }

        if (!inlineBranch) continue;

        // Here we can safely inline the body of the condition at its location and removing the condition since if any of the branches is compile-time inlinable, then the other wont ever reach their body.
        currentUnit->currentScope->children.erase(
            currentUnit->currentScope->children.begin() + index
        );

        // Now we can inline the body of the condition
        if (allBranches[i]->body) {
            currentUnit->currentScope->insert(allBranches[i]->body, index);
        }

        // Ensure the new token at index is properly handled:
        index--;

        return;
    }

}

void preprocessor::solver::lifetimes::add(color c) {
    // If the last color has it's end same as its start it means we need to update that one to end where this one begins
    if (!colors.empty()) {
        color& last = colors.back();
        if (last.end.x == last.start.x && last.end.y == last.start.y && last.end.file_id == last.start.file_id) {
            last.end = c.start;
        }
    }

    colors.push_back(c);
}

preprocessor::solver::color preprocessor::solver::lifetimes::get(lexer::token::position position) {
    // Let's check where the index lands on the colors
    for (auto& currentColor : colors) {
        if (currentColor.end >= position && currentColor.start <= position) {
            return currentColor;
        }
    }

    return emptyColor;
}

void preprocessor::solver::determineLifetimes(preprocessor::unit* currentUnit, int32_t& index) {
    if (currentUnit->currentScope->children[index]->type != parser::token::types::OPERATOR) return;
    auto* op = dynamic_cast<parser::token::Operator::base*>(currentUnit->currentScope->children[index]);

    // check if it is a assign operator
    if (op->operationType != parser::token::Operator::types::ASSIGN) return;

    parser::token::definition::base* definition = nullptr;

    // Let's skip caller ptr assign left side operators, since those are dynamic of return.
    if (dynamic_cast<parser::token::caller*>(op->left)) {
        // do nothing...
        return;
    } else if (dynamic_cast<parser::token::object*>(op->left)) {
        // Since the definition should contain the lifetimes.
        definition = dynamic_cast<parser::token::object*>(op->left)->reference;

        if (!definition) return;    // This probably is because some more complex that surface-AST cant handle.
    } else if (dynamic_cast<parser::token::definition::base*>(op->left)) {
        // Doesn't matter whether it is a context or a variable definition, since lambdas can also be re-assigned.
        definition = dynamic_cast<parser::token::definition::base*>(op->left);
    } else {
        return; // not a definable left side
    }

    if (!definition->getConnected()) {
        // No lifetime yet registered for this definition, so let's create a new one:
        definition->connect(new solver::lifetimes());
    } else if (!dynamic_cast<solver::lifetimes*>(definition->getConnected())) {
        // Something is wrong here...
        throw std::runtime_error("Definition linked lifetime is not of lifetimes type.");
    }

    color assign = color(
        op->right,
        op->right->position,
        op->right->position   // by default end is same as start, unless modified later
    );

    dynamic_cast<solver::lifetimes*>(definition->getConnected())->add(assign);
}

parser::token::base* preprocessor::solver::getLifetimeValueFrom(parser::token::base* unknown) {

    auto* variable = dynamic_cast<parser::token::object*>(unknown);

    if (variable) {

        // Now we need to check wether the variable is indirect or direct type variable.
        auto* definition = variable->reference;

        if (!definition) throw std::runtime_error("CRITICAL: Variable " + variable->toString() + " missing definition!");

        auto* colors = dynamic_cast<solver::lifetimes*>(definition->getConnected());

        if (!colors) return nullptr;   // No compile-time value is held at this time.

        auto currentColor = colors->get(variable->position);

        if (currentColor.isEmpty()) return nullptr; // No compile-time value is held at this time.

        return currentColor.value;
    } else {
        // Probable straight up value, like [number, string]
        return unknown;
    }

}

void preprocessor::solver::interpreter::factory(preprocessor::unit* currentUnit, int32_t& index) {
    if (currentUnit->currentScope->children[index]->type != parser::token::types::OPERATOR) return;
    auto* op = dynamic_cast<parser::token::Operator::base*>(currentUnit->currentScope->children[index]);

    evaluate(op);
}

void preprocessor::solver::interpreter::evaluate(parser::token::base* token) {
    if (token->type != parser::token::types::OPERATOR) return;

    parser::token::Operator::base* op = dynamic_cast<parser::token::Operator::base*>(token);

    // Try to compute higher order e.g deeper AST nodes first
    evaluate(op->left);
    evaluate(op->right);

    if (op->left->type == parser::token::types::NUMBER && op->right->type == parser::token::types::NUMBER) {
        parser::token::number* left = dynamic_cast<parser::token::number*>(op->left);
        parser::token::number* right = dynamic_cast<parser::token::number*>(op->right);

        parser::token::number* result = evaluate(left, right, op);

        // Replace the operator token with the result number token
        parser::token::replace(op, result);
    } else if (op->left->type == parser::token::types::STRING && op->right->type == parser::token::types::STRING) {
        parser::token::string::base* left = dynamic_cast<parser::token::string::base*>(op->left);
        parser::token::string::base* right = dynamic_cast<parser::token::string::base*>(op->right);

        parser::token::base* result = evaluate(left, right, op);

        // Replace the operator token with the result string token
        parser::token::replace(op, result);
    }
}

template<typename T>
std::string templatedIntAndFloatEvaluator(T leftValue, T rightValue, parser::token::Operator::base* operation) {
    std::string finalValue;

    switch (operation->operationType) {
        case parser::token::Operator::types::MULTIPLICATION:
            finalValue = std::to_string(leftValue * rightValue);
            break;
        case parser::token::Operator::types::DIVISION:
            if (rightValue == 0) throw std::runtime_error("Division by zero in compile-time evaluation.");
            finalValue = std::to_string(leftValue / rightValue);
            break;
        case parser::token::Operator::types::MODULO:
            if constexpr (std::is_integral<T>::value) {
                if (rightValue == 0) throw std::runtime_error("Modulo by zero in compile-time evaluation.");
                finalValue = std::to_string(leftValue % rightValue);
            } else {
                throw std::runtime_error("Modulo operator is not defined for floating point numbers in compile-time evaluation.");
            }
            break;
        case parser::token::Operator::types::ADDITION:
            finalValue = std::to_string(leftValue + rightValue);
            break;
        case parser::token::Operator::types::SUBTRACTION:
            finalValue = std::to_string(leftValue - rightValue);
            break;
        case parser::token::Operator::types::BITSHIFT_LEFT:
            if constexpr (std::is_integral<T>::value) {
                finalValue = std::to_string(leftValue << rightValue);
            } else {
                throw std::runtime_error("Bitshift operators are not defined for floating point numbers in compile-time evaluation.");
            }
            break;
        case parser::token::Operator::types::BITSHIFT_RIGHT:
            if constexpr (std::is_integral<T>::value) {
                finalValue = std::to_string(leftValue >> rightValue);
            } else {
                throw std::runtime_error("Bitshift operators are not defined for floating point numbers in compile-time evaluation.");
            }
            break;
        case parser::token::Operator::types::AND:
            if constexpr (std::is_integral<T>::value) {
                finalValue = std::to_string(leftValue & rightValue);
            } else {
                throw std::runtime_error("Bitwise AND operator is not defined for floating point numbers in compile-time evaluation.");
            }
            break;
        case parser::token::Operator::types::XOR:
            if constexpr (std::is_integral<T>::value) {
                finalValue = std::to_string(leftValue ^ rightValue);
            } else {
                throw std::runtime_error("Bitwise XOR operator is not defined for floating point numbers in compile-time evaluation.");
            }
            break;
        case parser::token::Operator::types::OR:
            if constexpr (std::is_integral<T>::value) {
                finalValue = std::to_string(leftValue | rightValue);
            } else {
                throw std::runtime_error("Bitwise OR operator is not defined for floating point numbers in compile-time evaluation.");
            }
            break;
        // No need for compare and assign operators!!!
        default:
            throw std::runtime_error("Unsupported operator type for number evaluation in compile-time interpreter.");
            break;
    }

    return finalValue;
}

template<typename L, typename R>
std::string templatedIntAndFloatEvaluatorForBooleanOperators(L leftValue, R rightValue, parser::token::Operator::base* operation) {
    std::string finalValue;

    // For boolean returning operations
    bool intermediateFinalValue;

    switch (operation->operationType) {
        case parser::token::Operator::types::COMPARISON:

            switch (parser::token::Operator::comparison::getComparisonType(operation->symbol)) {
                case parser::token::Operator::comparison::type::LESS_THAN:
                    intermediateFinalValue = (leftValue < rightValue);
                    break;
                case parser::token::Operator::comparison::type::GREATER_THAN:
                    intermediateFinalValue = (leftValue > rightValue);
                    break;
                case parser::token::Operator::comparison::type::LESS_EQUAL:
                    intermediateFinalValue = (leftValue <= rightValue);
                    break;
                case parser::token::Operator::comparison::type::GREATER_EQUAL:
                    intermediateFinalValue = (leftValue >= rightValue);
                    break;
                case parser::token::Operator::comparison::type::EQUAL:
                    intermediateFinalValue = (leftValue == rightValue);
                    break;
                case parser::token::Operator::comparison::type::NOT_EQUAL:
                    intermediateFinalValue = (leftValue != rightValue);
                    break;
                default:
                    break;
            }
            
            // Convert the boolean to true/false as a string
            finalValue = intermediateFinalValue ? utils::boolToString::TRUE : utils::boolToString::FALSE;
            break;
        case parser::token::Operator::types::LOGICAL_AND:
            intermediateFinalValue = leftValue && rightValue;
            finalValue = intermediateFinalValue ? utils::boolToString::TRUE : utils::boolToString::FALSE;
            break;
        case parser::token::Operator::types::LOGICAL_OR:
            intermediateFinalValue = leftValue || rightValue;
            finalValue = intermediateFinalValue ? utils::boolToString::TRUE : utils::boolToString::FALSE;
            break;
        // Only for compare operators, no other operator should be here!
        default:
            throw std::runtime_error("Unsupported operator type for number evaluation in compile-time interpreter.");
            break;
    }

    return finalValue;
}

parser::token::number* preprocessor::solver::interpreter::evaluate(parser::token::number* left, parser::token::number* right, parser::token::Operator::base* operation) {
    lexer::token::number::types prominentNumberType = left->getProminentNumberType(right->numberType, operation->operationType);
    std::string resultValue;

    // If 1.0 and other small floats suddenly start acting weird, we need to constraint them via this.
    [[maybe_unused]] uint8_t maxBitSize = std::max(left->minRequiredByteSize, right->minRequiredByteSize);

    if (prominentNumberType == lexer::token::number::types::INTEGER) {
        int64_t leftVal = left->getValueWithBooleanOverrideAsNumber<int64_t>();
        int64_t rightVal = right->getValueWithBooleanOverrideAsNumber<int64_t>();
        resultValue = templatedIntAndFloatEvaluator<int64_t>(leftVal, rightVal, operation);
    } else if (prominentNumberType == lexer::token::number::types::FLOAT) {
        double leftVal = left->getValueWithBooleanOverrideAsNumber<double>();
        double rightVal = right->getValueWithBooleanOverrideAsNumber<double>();
        resultValue = templatedIntAndFloatEvaluator<double>(leftVal, rightVal, operation);
    } else if (prominentNumberType == lexer::token::number::types::BOOLEAN) {
        // Helper to dispatch based on type combinations
        auto evaluateBoolean = [&](auto leftVal, parser::token::number* rightAsUnknown) {
            if (rightAsUnknown->numberType == lexer::token::number::types::BOOLEAN) {
                bool rightVal = (rightAsUnknown->value == utils::boolToString::TRUE);
                return templatedIntAndFloatEvaluatorForBooleanOperators(leftVal, rightVal, operation); 
            } else if (rightAsUnknown->numberType == lexer::token::number::types::INTEGER) {
                int64_t rightVal = rightAsUnknown->getValueWithBooleanOverrideAsNumber<int64_t>();  // No need to worry about boolean override, since it literally isn't a boolean.
                return templatedIntAndFloatEvaluatorForBooleanOperators(leftVal, rightVal, operation);
            } else if (rightAsUnknown->numberType == lexer::token::number::types::FLOAT) {
                double rightVal = rightAsUnknown->getValueWithBooleanOverrideAsNumber<double>();    // No need to worry about boolean override, since it literally isn't a boolean.
                return templatedIntAndFloatEvaluatorForBooleanOperators(leftVal, rightVal, operation);
            } else {
                throw std::runtime_error("Unsupported number type for boolean operation evaluation.");
            }
        };

        if (left->numberType == lexer::token::number::types::BOOLEAN) {
            bool leftVal = (left->value == utils::boolToString::TRUE);
            resultValue = evaluateBoolean(leftVal, right);
        } else if (left->numberType == lexer::token::number::types::INTEGER) {
            int64_t leftVal = left->getValueWithBooleanOverrideAsNumber<int64_t>();  // No need to worry about boolean override, since it literally isn't a boolean.
            resultValue = evaluateBoolean(leftVal, right);
        } else if (left->numberType == lexer::token::number::types::FLOAT) {
            double leftVal = left->getValueWithBooleanOverrideAsNumber<double>();    // No need to worry about boolean override, since it literally isn't a boolean.
            resultValue = evaluateBoolean(leftVal, right);
        }
    } else {
        throw std::runtime_error("HEX number type should not appear in compile-time evaluation.");
    }

    parser::token::number* result = new parser::token::number(
        parser::token::info(
            parser::token::types::NUMBER,
            operation->position,
            left->parent,
            resultValue
        ),
        resultValue
    );

    return result;
}

parser::token::base* preprocessor::solver::interpreter::evaluate(parser::token::string::base* left, parser::token::string::base* right, parser::token::Operator::base* operation) {
    if (operation->operationType == parser::token::Operator::types::ADDITION) {
        std::string resultValue = left->bakedString + right->bakedString;
        
        parser::token::string::base* result = new parser::token::string::base(
            parser::token::info(
                parser::token::types::STRING,
                operation->position,
                left->parent,
                left->symbol
            ),
            parser::unit::lexerOutput{}
        );
        
        result->bakedString = resultValue;
        return result;
    } else if (operation->operationType == parser::token::Operator::types::COMPARISON) {
        bool comparisonResult = false;
        auto compType = parser::token::Operator::comparison::getComparisonType(operation->symbol);
        
        if (compType == parser::token::Operator::comparison::type::EQUAL) {
            comparisonResult = (left->bakedString == right->bakedString);
        } else if (compType == parser::token::Operator::comparison::type::NOT_EQUAL) {
            comparisonResult = (left->bakedString != right->bakedString);
        } else {
                throw std::runtime_error("Unsupported comparison operator for string evaluation.");
        }
        
        std::string resultValue = comparisonResult ? utils::boolToString::TRUE : utils::boolToString::FALSE;

        parser::token::number* result = new parser::token::number(
            parser::token::info(
                parser::token::types::NUMBER,
                operation->position,
                left->parent,
                resultValue
            ),
            resultValue
        );
        return result;
    } else {
        throw std::runtime_error("Unsupported operator type for string evaluation in compile-time interpreter.");
    }
}
