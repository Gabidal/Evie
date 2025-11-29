#include "preprocessor.h"

void preprocessor::unit::factory() {

    for (int i = 0; i < (int32_t)currentScope->children.size(); i++) {
        includer::openInclude(this, i);
        includer::closeInclude(this, i);

        solver::determineLifetimes(this, i);

        solver::interpreter::factory(this, i);
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

void preprocessor::unwrap::conditionals(preprocessor::unit* currentUnit, int32_t& index) {
    if (currentUnit->currentScope->children[index]->type != parser::token::types::CONDITION) return;
    auto* conditional = dynamic_cast<parser::token::condition*>(currentUnit->currentScope->children[index]);

    /**
     * For us to determine if this condition is unwraptable, we need to first call preprocessor to inline and simplify the condition.
     * If the resulting condition = 1, we replace the condition with the contents of the body (remember to also transfer any definitions made inside the condition)
     * If the resulting condition = 0, we remove the entire conditional from the AST.
     * If the resulting condition is anything else, we do nothing, this is because it is most likely a run-time condition.
     */

    preprocessor::unit subPreprocessorUnit(conditional->header, currentUnit->arguments, currentUnit->stack);
    subPreprocessorUnit.factory();

    // ...
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
    T finalValue;

    // For boolean returning operations
    utils::boolToInt intermediateFinalValue;

    switch (operation->operationType) {
        case parser::token::Operator::types::MULTIPLICATION:
            finalValue = leftValue * rightValue;
            break;
        case parser::token::Operator::types::DIVISION:
            if (rightValue == 0) throw std::runtime_error("Division by zero in compile-time evaluation.");
            finalValue = leftValue / rightValue;
            break;
        case parser::token::Operator::types::MODULO:
            if constexpr (std::is_integral<T>::value) {
                if (rightValue == 0) throw std::runtime_error("Modulo by zero in compile-time evaluation.");
                finalValue = leftValue % rightValue;
            } else {
                throw std::runtime_error("Modulo operator is not defined for floating point numbers in compile-time evaluation.");
            }
            break;
        case parser::token::Operator::types::ADDITION:
            finalValue = leftValue + rightValue;
            break;
        case parser::token::Operator::types::SUBTRACTION:
            finalValue = leftValue - rightValue;
            break;
        case parser::token::Operator::types::BITSHIFT_LEFT:
            if constexpr (std::is_integral<T>::value) {
                finalValue = leftValue << rightValue;
            } else {
                throw std::runtime_error("Bitshift operators are not defined for floating point numbers in compile-time evaluation.");
            }
            break;
        case parser::token::Operator::types::BITSHIFT_RIGHT:
            if constexpr (std::is_integral<T>::value) {
                finalValue = leftValue >> rightValue;
            } else {
                throw std::runtime_error("Bitshift operators are not defined for floating point numbers in compile-time evaluation.");
            }
            break;
        case parser::token::Operator::types::COMPARISON:

            switch (parser::token::Operator::comparison::getComparisonType(operation->symbol)) {
                case parser::token::Operator::comparison::type::LESS_THAN:
                    intermediateFinalValue = (leftValue < rightValue) ? utils::boolToInt::TRUE : utils::boolToInt::FALSE;
                    break;
                case parser::token::Operator::comparison::type::GREATER_THAN:
                    intermediateFinalValue = (leftValue > rightValue) ? utils::boolToInt::TRUE : utils::boolToInt::FALSE;
                    break;
                case parser::token::Operator::comparison::type::LESS_EQUAL:
                    intermediateFinalValue = (leftValue <= rightValue) ? utils::boolToInt::TRUE : utils::boolToInt::FALSE;
                    break;
                case parser::token::Operator::comparison::type::GREATER_EQUAL:
                    intermediateFinalValue = (leftValue >= rightValue) ? utils::boolToInt::TRUE : utils::boolToInt::FALSE;
                    break;
                case parser::token::Operator::comparison::type::EQUAL:
                    intermediateFinalValue = (leftValue == rightValue) ? utils::boolToInt::TRUE : utils::boolToInt::FALSE;
                    break;
                case parser::token::Operator::comparison::type::NOT_EQUAL:
                    intermediateFinalValue = (leftValue != rightValue) ? utils::boolToInt::TRUE : utils::boolToInt::FALSE;
                    break;
                default:
                    break;
            }
            
            // This will cause trues to become 1.0 instead of just 1.
            finalValue = static_cast<T>(static_cast<int>(intermediateFinalValue));
            break;
        case parser::token::Operator::types::AND:
            if constexpr (std::is_integral<T>::value) {
                finalValue = leftValue & rightValue;
            } else {
                throw std::runtime_error("Bitwise AND operator is not defined for floating point numbers in compile-time evaluation.");
            }
            break;
        case parser::token::Operator::types::XOR:
            if constexpr (std::is_integral<T>::value) {
                finalValue = leftValue ^ rightValue;
            } else {
                throw std::runtime_error("Bitwise XOR operator is not defined for floating point numbers in compile-time evaluation.");
            }
            break;
        case parser::token::Operator::types::OR:
            if constexpr (std::is_integral<T>::value) {
                finalValue = leftValue | rightValue;
            } else {
                throw std::runtime_error("Bitwise OR operator is not defined for floating point numbers in compile-time evaluation.");
            }
            break;
        case parser::token::Operator::types::LOGICAL_AND:
            intermediateFinalValue = (leftValue && rightValue) ? utils::boolToInt::TRUE : utils::boolToInt::FALSE;
            finalValue = static_cast<T>(static_cast<int>(intermediateFinalValue));
            break;
        case parser::token::Operator::types::LOGICAL_OR:
            intermediateFinalValue = (leftValue || rightValue) ? utils::boolToInt::TRUE : utils::boolToInt::FALSE;
            finalValue = static_cast<T>(static_cast<int>(intermediateFinalValue));
            break;
        // No need for assigns, since you cant really assign into a number, can you now?
        default:
            throw std::runtime_error("Unsupported operator type for number evaluation in compile-time interpreter.");
            break;
    }

    // Since the template already has the actual type we dont need to split this into float and int conversions.
    std::string resultValue = std::to_string(finalValue);

    return resultValue;
}

parser::token::number* preprocessor::solver::interpreter::evaluate(parser::token::number* left, parser::token::number* right, parser::token::Operator::base* operation) {
    lexer::token::number::types prominentNumberType = left->getProminentNumberType(right->numberType);
    std::string resultValue;

    // If 1.0 and other small floats suddenly start acting weird, we need to constraint them via this.
    [[maybe_unused]] uint8_t maxBitSize = std::max(left->minRequiredByteSize, right->minRequiredByteSize);

    if (prominentNumberType == lexer::token::number::types::INTEGER) {
        int64_t leftVal = static_cast<int64_t>(std::stoll(left->value));
        int64_t rightVal = static_cast<int64_t>(std::stoll(right->value));
        resultValue = templatedIntAndFloatEvaluator<int64_t>(leftVal, rightVal, operation);
    } else if (prominentNumberType == lexer::token::number::types::FLOAT) {
        double leftVal = std::stod(left->value);
        double rightVal = std::stod(right->value);
        resultValue = templatedIntAndFloatEvaluator<double>(leftVal, rightVal, operation);
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
