#ifndef _parser_tester_h_
#define _parser_tester_h_

#include "utils.h"
#include "../../src/parser/parser.h"
#include "../../src/lexer/lexer.h"

#include <string>
#include <vector>
#include <memory>

namespace tester {

    class parserTester : public utils::TestSuite {
    public:
        parserTester() : utils::TestSuite("parser Tester") {
            add_test("Simple Definition", "single type definition", test_simple_definition);
            add_test("Operation Order", "test operator precedence and associativity", test_operation_order);
            add_test("Simple Parenthesis", "test parenthesis handling in expressions", test_simple_parenthesis);
            add_test("Chained Parenthesis", "test nested parenthesis handling", test_chained_parenthesis);
            add_test("class Definition", "test class definition parsing", test_single_wrapper_context);
            add_test("Fetching", "test member fetching", test_fetching);
            add_test("Function Return Type", "test function return type fetching", test_fetching_At_Context_Return_Type);
            add_test("Caller Construction", "test caller construction", test_caller_construction);
            add_test("Condition Parsing", "test condition parsing", test_conditions);
            add_test("Loop Parsing", "test loop parsing", test_loops);
            add_test("Basic String Baking", "test string token baking", test_basic_string_baking);
            add_test("Hexed String Baking", "test hexed string token baking", test_hexed_string_baking);
            add_test("Escaped String Baking", "test escaped string token baking", test_escaped_string_baking);
        }

    private:
        static parser::token::scope::base* parse(std::string input) {
            auto lexerOutput = lexer::tokenize(input, 0);

            parser::token::scope::base* globalScope = new parser::token::scope::base(
                parser::token::info(
                    parser::token::types::SCOPE,
                    lexer::token::position(0, 0, 0),
                    nullptr,
                    "global"
                ),
                lexerOutput
            );

            parser::unit::base parse(parser::unit::pass::FIRST, globalScope);
            parse.factory();

            return globalScope;
        }
        
        static void test_simple_definition() {
            parser::token::scope::base* globalScope = parse(
                "int a;\n"
                "int b;\n"
            );

            ASSERT_EQ((size_t)2, globalScope->definitions.size());

            auto a_Def = static_cast<parser::token::definition::base*>(globalScope->definitions[0]);
            ASSERT_EQ((std::string_view)"a", a_Def->symbol);
            ASSERT_EQ((int)parser::token::types::DEFINITION, (int)a_Def->type);
            ASSERT_EQ((size_t)1, a_Def->inherited.size());
            ASSERT_EQ((std::string_view)"int", a_Def->inherited[0]);

            auto b_Def = static_cast<parser::token::definition::base*>(globalScope->definitions[1]);
            ASSERT_EQ((std::string_view)"b", b_Def->symbol);
            ASSERT_EQ((int)parser::token::types::DEFINITION, (int)b_Def->type);
            ASSERT_EQ((size_t)1, b_Def->inherited.size());
            ASSERT_EQ((std::string_view)"int", b_Def->inherited[0]);
        }

        static void test_operation_order() {
            parser::token::scope::base* globalScope = parse(
            "int a, int b, int c, int d\n"
            "a = 1 + 2 * 3 - 4\n"
            "a == 5 && b != 6 || c < 7 >= d\n"
            );

            // Test: a = 1 + 2 * 3 - 4
            // Expected AST: a = ((1 + (2 * 3)) - 4)
            // The assignment should be the root, with 'a' on left and expression on right
            
            // Find the assignment operator in the raw tokens
            parser::token::Operator::base* assignOp = nullptr;
            for (auto token : globalScope->rawTokens) {
                if (token->get_type() == lexer::token::types::OPERATOR && 
                    token->parsed && 
                    token->parsed->type == parser::token::types::OPERATOR) {
                    auto op = static_cast<parser::token::Operator::base*>(token->parsed);
                    if (op->operationType == parser::token::Operator::types::ASSIGN) {
                        assignOp = op;
                        break;
                    }
                }
            }
            
            ASSERT_TRUE(assignOp != nullptr);
            ASSERT_EQ((std::string_view)"=", assignOp->symbol);
            
            // Left side should be object 'a'
            ASSERT_TRUE(assignOp->left != nullptr);
            ASSERT_EQ((int)parser::token::types::OBJECT, (int)assignOp->left->type);
            ASSERT_EQ((std::string_view)"a", assignOp->left->symbol);
            
            // Right side should be subtraction: (1 + 2 * 3) - 4
            ASSERT_TRUE(assignOp->right != nullptr);
            ASSERT_EQ((int)parser::token::types::OPERATOR, (int)assignOp->right->type);
            auto subOp = static_cast<parser::token::Operator::base*>(assignOp->right);
            ASSERT_EQ((int)parser::token::Operator::types::SUBTRACTION, (int)subOp->operationType);
            ASSERT_EQ((std::string_view)"-", subOp->symbol);
            
            // Subtraction left: 1 + 2 * 3
            ASSERT_TRUE(subOp->left != nullptr);
            ASSERT_EQ((int)parser::token::types::OPERATOR, (int)subOp->left->type);
            auto addOp = static_cast<parser::token::Operator::base*>(subOp->left);
            ASSERT_EQ((int)parser::token::Operator::types::ADDITION, (int)addOp->operationType);
            ASSERT_EQ((std::string_view)"+", addOp->symbol);
            
            // Subtraction right: 4
            ASSERT_TRUE(subOp->right != nullptr);
            ASSERT_EQ((int)parser::token::types::NUMBER, (int)subOp->right->type);
            ASSERT_EQ((std::string_view)"4", subOp->right->symbol);
            
            // Addition left: 1
            ASSERT_TRUE(addOp->left != nullptr);
            ASSERT_EQ((int)parser::token::types::NUMBER, (int)addOp->left->type);
            ASSERT_EQ((std::string_view)"1", addOp->left->symbol);
            
            // Addition right: 2 * 3
            ASSERT_TRUE(addOp->right != nullptr);
            ASSERT_EQ((int)parser::token::types::OPERATOR, (int)addOp->right->type);
            auto mulOp = static_cast<parser::token::Operator::base*>(addOp->right);
            ASSERT_EQ((int)parser::token::Operator::types::MULTIPLICATION, (int)mulOp->operationType);
            ASSERT_EQ((std::string_view)"*", mulOp->symbol);
            
            // Multiplication left: 2
            ASSERT_TRUE(mulOp->left != nullptr);
            ASSERT_EQ((int)parser::token::types::NUMBER, (int)mulOp->left->type);
            ASSERT_EQ((std::string_view)"2", mulOp->left->symbol);
            
            // Multiplication right: 3
            ASSERT_TRUE(mulOp->right != nullptr);
            ASSERT_EQ((int)parser::token::types::NUMBER, (int)mulOp->right->type);
            ASSERT_EQ((std::string_view)"3", mulOp->right->symbol);
            
            // Test: a == 5 && b != 6 || c < 7 >= d
            // Expected AST: ((a == 5) && (b != 6)) || (c < 7) >= d
            // The logical OR should be at the top level
            
            // Find the logical OR operator
            parser::token::Operator::base* orOp = nullptr;
            for (auto token : globalScope->rawTokens) {
                if (token->get_type() == lexer::token::types::OPERATOR && 
                    token->parsed && 
                    token->parsed->type == parser::token::types::OPERATOR) {
                    auto op = static_cast<parser::token::Operator::base*>(token->parsed);
                    if (op->operationType == parser::token::Operator::types::LOGICAL_OR) {
                        orOp = op;
                        break;
                    }
                }
            }
            
            ASSERT_TRUE(orOp != nullptr);
            ASSERT_EQ((std::string_view)"||", orOp->symbol);
            
            // Left side of ||: (a == 5) && (b != 6)
            ASSERT_TRUE(orOp->left != nullptr);
            ASSERT_EQ((int)parser::token::types::OPERATOR, (int)orOp->left->type);
            auto andOp = static_cast<parser::token::Operator::base*>(orOp->left);
            ASSERT_EQ((int)parser::token::Operator::types::LOGICAL_AND, (int)andOp->operationType);
            ASSERT_EQ((std::string_view)"&&", andOp->symbol);
            
            // Left side of &&: a == 5
            ASSERT_TRUE(andOp->left != nullptr);
            ASSERT_EQ((int)parser::token::types::OPERATOR, (int)andOp->left->type);
            auto eqOp = static_cast<parser::token::Operator::base*>(andOp->left);
            ASSERT_EQ((int)parser::token::Operator::types::COMPARISON, (int)eqOp->operationType);
            ASSERT_EQ((std::string_view)"==", eqOp->symbol);
            
            // a == 5 left: a
            ASSERT_TRUE(eqOp->left != nullptr);
            ASSERT_EQ((int)parser::token::types::OBJECT, (int)eqOp->left->type);
            ASSERT_EQ((std::string_view)"a", eqOp->left->symbol);
            
            // a == 5 right: 5
            ASSERT_TRUE(eqOp->right != nullptr);
            ASSERT_EQ((int)parser::token::types::NUMBER, (int)eqOp->right->type);
            ASSERT_EQ((std::string_view)"5", eqOp->right->symbol);
            
            // Right side of &&: b != 6
            ASSERT_TRUE(andOp->right != nullptr);
            ASSERT_EQ((int)parser::token::types::OPERATOR, (int)andOp->right->type);
            auto neOp = static_cast<parser::token::Operator::base*>(andOp->right);
            ASSERT_EQ((int)parser::token::Operator::types::COMPARISON, (int)neOp->operationType);
            ASSERT_EQ((std::string_view)"!=", neOp->symbol);
            
            // b != 6 left: b
            ASSERT_TRUE(neOp->left != nullptr);
            ASSERT_EQ((int)parser::token::types::OBJECT, (int)neOp->left->type);
            ASSERT_EQ((std::string_view)"b", neOp->left->symbol);
            
            // b != 6 right: 6
            ASSERT_TRUE(neOp->right != nullptr);
            ASSERT_EQ((int)parser::token::types::NUMBER, (int)neOp->right->type);
            ASSERT_EQ((std::string_view)"6", neOp->right->symbol);
            
            // Right side of ||: c < 7 >= d
            // Note: This should parse as (c < 7) >= d due to left-to-right associativity
            ASSERT_TRUE(orOp->right != nullptr);
            ASSERT_EQ((int)parser::token::types::OPERATOR, (int)orOp->right->type);
            auto geOp = static_cast<parser::token::Operator::base*>(orOp->right);
            ASSERT_EQ((int)parser::token::Operator::types::COMPARISON, (int)geOp->operationType);
            ASSERT_EQ((std::string_view)">=", geOp->symbol);
            
            // Left of >=: c < 7
            ASSERT_TRUE(geOp->left != nullptr);
            ASSERT_EQ((int)parser::token::types::OPERATOR, (int)geOp->left->type);
            auto ltOp = static_cast<parser::token::Operator::base*>(geOp->left);
            ASSERT_EQ((int)parser::token::Operator::types::COMPARISON, (int)ltOp->operationType);
            ASSERT_EQ((std::string_view)"<", ltOp->symbol);
            
            // c < 7 left: c
            ASSERT_TRUE(ltOp->left != nullptr);
            ASSERT_EQ((int)parser::token::types::OBJECT, (int)ltOp->left->type);
            ASSERT_EQ((std::string_view)"c", ltOp->left->symbol);
            
            // c < 7 right: 7
            ASSERT_TRUE(ltOp->right != nullptr);
            ASSERT_EQ((int)parser::token::types::NUMBER, (int)ltOp->right->type);
            ASSERT_EQ((std::string_view)"7", ltOp->right->symbol);
            
            // Right of >=: d
            ASSERT_TRUE(geOp->right != nullptr);
            ASSERT_EQ((int)parser::token::types::OBJECT, (int)geOp->right->type);
            ASSERT_EQ((std::string_view)"d", geOp->right->symbol);
        }
    
        static void test_simple_parenthesis() {
            parser::token::scope::base* globalScope = parse(
            "int a, int b, int c, int d\n"
            "a = (c + b) * d\n"
            );

            ASSERT_EQ((size_t)5, globalScope->children.size());
            ASSERT_TRUE(globalScope->children[4]->type == parser::token::types::OPERATOR);
            auto assignOp = static_cast<parser::token::Operator::base*>(globalScope->children[4]);
            ASSERT_EQ((std::string_view)"=", assignOp->symbol);
            ASSERT_TRUE(assignOp->left->type == parser::token::types::OBJECT);
            ASSERT_EQ((std::string_view)"a", assignOp->left->symbol);
            ASSERT_TRUE(assignOp->right->type == parser::token::types::OPERATOR);
            auto mulOp = static_cast<parser::token::Operator::base*>(assignOp->right);
            ASSERT_EQ((std::string_view)"*", mulOp->symbol);
            ASSERT_TRUE(mulOp->right->type == parser::token::types::OBJECT);
            ASSERT_EQ((std::string_view)"d", mulOp->right->symbol);
            ASSERT_TRUE(mulOp->left->type == parser::token::types::SCOPE);
            auto parenScope = static_cast<parser::token::scope::base*>(mulOp->left);
            ASSERT_EQ((size_t)1, parenScope->children.size());
            ASSERT_TRUE(parenScope->children[0]->type == parser::token::types::OPERATOR);
            auto addOp = static_cast<parser::token::Operator::base*>(parenScope->children[0]);
            ASSERT_EQ((std::string_view)"+", addOp->symbol);
            ASSERT_TRUE(addOp->left->type == parser::token::types::OBJECT);
            ASSERT_EQ((std::string_view)"c", addOp->left->symbol);
            ASSERT_TRUE(addOp->right->type == parser::token::types::OBJECT);
            ASSERT_EQ((std::string_view)"b", addOp->right->symbol);
        }

        static void test_chained_parenthesis() {
            parser::token::scope::base* globalScope = parse(
            "void foo(int a, int b) {\n"
            "  int c = a + b\n"
            "  int d = c - a\n"
            "}"
            );

            ASSERT_TRUE(globalScope->children.size() == 1);
            ASSERT_TRUE(globalScope->children[0]->type == parser::token::types::DEFINITION);
            auto context = static_cast<parser::token::context*>(globalScope->children[0]);
            ASSERT_TRUE(context->definitionType == parser::token::definition::types::CONTEXT);
            ASSERT_EQ((std::string_view)"foo", context->symbol);
            ASSERT_TRUE(context->wrappers.size() == 2);

            ASSERT_TRUE(context->wrappers[0]->type == parser::token::types::SCOPE);
            auto params = context->wrappers[0];

            ASSERT_TRUE(params->definitions.size() == 2);
            ASSERT_TRUE(params->definitions[0]->type == parser::token::types::DEFINITION);
            auto a_Def = static_cast<parser::token::definition::base*>(params->definitions[0]);
            ASSERT_EQ((std::string_view)"a", a_Def->symbol);
            ASSERT_TRUE(params->definitions[1]->type == parser::token::types::DEFINITION);
            auto b_Def = static_cast<parser::token::definition::base*>(params->definitions[1]);
            ASSERT_EQ((std::string_view)"b", b_Def->symbol);
            
            ASSERT_TRUE(context->wrappers[1]->type == parser::token::types::SCOPE);
            auto body = context->wrappers[1];
            
            ASSERT_TRUE(body->definitions.size() == 2);
            ASSERT_TRUE(body->definitions[0]->type == parser::token::types::DEFINITION);
            auto c_Def = static_cast<parser::token::definition::base*>(body->definitions[0]);
            ASSERT_EQ((std::string_view)"c", c_Def->symbol);
            ASSERT_TRUE(body->definitions[1]->type == parser::token::types::DEFINITION);
            auto d_Def = static_cast<parser::token::definition::base*>(body->definitions[1]);
            ASSERT_EQ((std::string_view)"d", d_Def->symbol);

            ASSERT_TRUE(body->children[0]->type == parser::token::types::OPERATOR);
            auto C_setOp = static_cast<parser::token::Operator::base*>(body->children[0]);
            ASSERT_EQ((std::string_view)"=", C_setOp->symbol);
            ASSERT_TRUE(C_setOp->left->type == parser::token::types::DEFINITION);
            ASSERT_EQ((std::string_view)"c", C_setOp->left->symbol);
            ASSERT_TRUE(C_setOp->right->type == parser::token::types::OPERATOR);
            auto C_addOp = static_cast<parser::token::Operator::base*>(C_setOp->right);
            ASSERT_EQ((std::string_view)"+", C_addOp->symbol);
            ASSERT_TRUE(C_addOp->left->type == parser::token::types::OBJECT);
            ASSERT_EQ((std::string_view)"a", C_addOp->left->symbol);
            ASSERT_TRUE(C_addOp->right->type == parser::token::types::OBJECT);
            ASSERT_EQ((std::string_view)"b", C_addOp->right->symbol);
            
            ASSERT_TRUE(body->children[1]->type == parser::token::types::OPERATOR);
            auto D_setOp = static_cast<parser::token::Operator::base*>(body->children[1]);
            ASSERT_EQ((std::string_view)"=", D_setOp->symbol);
            ASSERT_TRUE(D_setOp->left->type == parser::token::types::DEFINITION);
            ASSERT_EQ((std::string_view)"d", D_setOp->left->symbol);
            ASSERT_TRUE(D_setOp->right->type == parser::token::types::OPERATOR);
            auto D_subOp = static_cast<parser::token::Operator::base*>(D_setOp->right);
            ASSERT_EQ((std::string_view)"-", D_subOp->symbol);
            ASSERT_TRUE(D_subOp->left->type == parser::token::types::OBJECT);
            ASSERT_EQ((std::string_view)"c", D_subOp->left->symbol);
            ASSERT_TRUE(D_subOp->right->type == parser::token::types::OBJECT);
            ASSERT_EQ((std::string_view)"a", D_subOp->right->symbol);
        }

        static void test_single_wrapper_context() {
            parser::token::scope::base* globalScope = parse(
            "class a {\n"
            "  int b = 0\n"
            "  int c = b\n"
            "}\n"
            );

            ASSERT_TRUE(globalScope->children.size() == 1);
            ASSERT_TRUE(globalScope->children[0]->type == parser::token::types::DEFINITION);
            auto context = static_cast<parser::token::context*>(globalScope->children[0]);
            ASSERT_TRUE(context->definitionType == parser::token::definition::types::CONTEXT);
            ASSERT_EQ((std::string_view)"a", context->symbol);
            ASSERT_TRUE(context->wrappers.size() == 1);

            ASSERT_TRUE(context->wrappers[0]->type == parser::token::types::SCOPE);
            auto body = context->wrappers[0];
            ASSERT_TRUE(body->definitions.size() == 2);
            ASSERT_TRUE(body->definitions[0]->type == parser::token::types::DEFINITION);
            auto b_Def = static_cast<parser::token::definition::base*>(body->definitions[0]);
            ASSERT_EQ((std::string_view)"b", b_Def->symbol);
            ASSERT_TRUE(body->definitions[1]->type == parser::token::types::DEFINITION);
            auto c_Def = static_cast<parser::token::definition::base*>(body->definitions[1]);
            ASSERT_EQ((std::string_view)"c", c_Def->symbol);

            ASSERT_TRUE(body->children[0]->type == parser::token::types::OPERATOR);
            auto B_setOp = static_cast<parser::token::Operator::base*>(body->children[0]);
            ASSERT_EQ((std::string_view)"=", B_setOp->symbol);
            ASSERT_TRUE(B_setOp->left->type == parser::token::types::DEFINITION);
            ASSERT_EQ((std::string_view)"b", B_setOp->left->symbol);
            ASSERT_TRUE(B_setOp->right->type == parser::token::types::NUMBER);
            ASSERT_EQ((std::string_view)"0", B_setOp->right->symbol);

            ASSERT_TRUE(body->children[1]->type == parser::token::types::OPERATOR);
            auto C_setOp = static_cast<parser::token::Operator::base*>(body->children[1]);
            ASSERT_EQ((std::string_view)"=", C_setOp->symbol);
            ASSERT_TRUE(C_setOp->left->type == parser::token::types::DEFINITION);
            ASSERT_EQ((std::string_view)"c", C_setOp->left->symbol);
            ASSERT_TRUE(C_setOp->right->type == parser::token::types::OBJECT);
            ASSERT_EQ((std::string_view)"b", C_setOp->right->symbol);
        }

        static void test_fetching() {
            parser::token::scope::base* globalScope = parse(
            "class a {\n"
            "  int b = 0\n"
            "}\n"
            "\n"
            "a b\n"
            "b.b = 1\n"
            );

            ASSERT_TRUE(globalScope->children.size() == 3);
            ASSERT_TRUE(globalScope->children[0]->type == parser::token::types::DEFINITION);
            auto classToken = static_cast<parser::token::context*>(globalScope->children[0]);
            ASSERT_TRUE(classToken->definitionType == parser::token::definition::types::CONTEXT);
            ASSERT_EQ((std::string_view)"a", classToken->symbol);
            ASSERT_TRUE(classToken->wrappers.size() == 1);

            ASSERT_TRUE(globalScope->children[1]->type == parser::token::types::DEFINITION);
            auto b_Def = static_cast<parser::token::definition::base*>(globalScope->children[1]);
            ASSERT_EQ((std::string_view)"b", b_Def->symbol);
            ASSERT_TRUE(b_Def->definitionType == parser::token::definition::types::VARIABLE);

            ASSERT_TRUE(globalScope->children[2]->type == parser::token::types::OPERATOR);
            auto fetchAssignOp = static_cast<parser::token::Operator::base*>(globalScope->children[2]);
            ASSERT_EQ((std::string_view)"=", fetchAssignOp->symbol);
            ASSERT_TRUE(fetchAssignOp->left->type == parser::token::types::OPERATOR);
            auto fetchOp = static_cast<parser::token::Operator::base*>(fetchAssignOp->left);
            ASSERT_EQ((std::string_view)".", fetchOp->symbol);
            ASSERT_TRUE(fetchOp->left->type == parser::token::types::OBJECT);
            ASSERT_EQ((std::string_view)"b", fetchOp->left->symbol);
            ASSERT_TRUE(fetchOp->right->type == parser::token::types::OBJECT);
            ASSERT_EQ((std::string_view)"b", fetchOp->right->symbol);
            ASSERT_TRUE(fetchAssignOp->right->type == parser::token::types::NUMBER);
            ASSERT_EQ((std::string_view)"1", fetchAssignOp->right->symbol);
        }

        void static test_fetching_At_Context_Return_Type() {
            parser::token::scope::base* globalScope = parse(
            "class a {\n"
            "  int b = 0\n"
            "}\n"
            "a.b foo(int c) {\n"
            "  int d = c\n"
            "}"
            );

            ASSERT_TRUE(globalScope->children.size() == 2);
            ASSERT_TRUE(globalScope->children[0]->type == parser::token::types::DEFINITION);
            auto classContext = static_cast<parser::token::context*>(globalScope->children[0]);
            ASSERT_TRUE(classContext->definitionType == parser::token::definition::types::CONTEXT);
            ASSERT_EQ((std::string_view)"a", classContext->symbol);
            ASSERT_TRUE(classContext->wrappers.size() == 1);
            ASSERT_TRUE(globalScope->children[1]->type == parser::token::types::DEFINITION);
            auto funcContext = static_cast<parser::token::context*>(globalScope->children[1]);
            ASSERT_TRUE(funcContext->definitionType == parser::token::definition::types::CONTEXT);
            ASSERT_EQ((std::string_view)"foo", funcContext->symbol);
            ASSERT_TRUE(funcContext->wrappers.size() == 2);
            
            // Check return type a.b -> int
            ASSERT_TRUE(funcContext->inherited.size() == 1);
            ASSERT_EQ((std::string_view)"int", funcContext->inherited[0]);
            
            // Check parameter c
            ASSERT_TRUE(funcContext->wrappers[0]->type == parser::token::types::SCOPE);
            auto params = funcContext->wrappers[0];
            ASSERT_TRUE(params->definitions.size() == 1);
            ASSERT_TRUE(params->definitions[0]->type == parser::token::types::DEFINITION);
            auto c_Def = static_cast<parser::token::definition::base*>(params->definitions[0]);
            ASSERT_EQ((std::string_view)"c", c_Def->symbol);
            
            // Check function body
            ASSERT_TRUE(funcContext->wrappers[1]->type == parser::token::types::SCOPE);
            auto body = funcContext->wrappers[1];
            ASSERT_TRUE(body->definitions.size() == 1);
            ASSERT_TRUE(body->definitions[0]->type == parser::token::types::DEFINITION);
            auto d_Def = static_cast<parser::token::definition::base*>(body->definitions[0]);
            ASSERT_EQ((std::string_view)"d", d_Def->symbol);
        }

        static void test_caller_construction() {
            parser::token::scope::base* globalScope = parse(
            "void foo(int c) {\n"
            "  int d = c\n"
            "}\n"
            "int a = foo(1)\n"
            );

            ASSERT_TRUE(globalScope->children.size() == 2);
            ASSERT_TRUE(globalScope->children[0]->type == parser::token::types::DEFINITION);
            auto funcContext = static_cast<parser::token::context*>(globalScope->children[0]);
            ASSERT_TRUE(funcContext->definitionType == parser::token::definition::types::CONTEXT);
            ASSERT_EQ((std::string_view)"foo", funcContext->symbol);
            ASSERT_TRUE(funcContext->wrappers.size() == 2);

            ASSERT_TRUE(globalScope->children[1]->type == parser::token::types::OPERATOR);
            auto assignOp = static_cast<parser::token::Operator::base*>(globalScope->children[1]);
            ASSERT_EQ((std::string_view)"=", assignOp->symbol);
            ASSERT_TRUE(assignOp->left->type == parser::token::types::DEFINITION);
            auto a_Def = static_cast<parser::token::definition::base*>(assignOp->left);
            ASSERT_EQ((std::string_view)"a", a_Def->symbol);
            ASSERT_TRUE(assignOp->right->type == parser::token::types::CALLER);
            auto caller = static_cast<parser::token::caller*>(assignOp->right);
            ASSERT_EQ((std::string_view)"foo", caller->symbol);
            ASSERT_TRUE(caller->parameters.size() == 1);
            ASSERT_TRUE(caller->parameters[0]->type == parser::token::types::SCOPE);
            auto paramScope = static_cast<parser::token::scope::base*>(caller->parameters[0]);
            ASSERT_TRUE(paramScope->children.size() == 1);
            ASSERT_TRUE(paramScope->children[0]->type == parser::token::types::NUMBER);
            ASSERT_EQ((std::string_view)"1", paramScope->children[0]->symbol);
        }

        static void test_conditions() {
            parser::token::scope::base* globalScope = parse(
            "int c, int a = 1, int b = 2\n"
            "if (a > b) {\n"
            "  c = a\n"
            "} else {\n"
            "  c = b\n"
            "}\n"
            // "if (sys.os == \"win\") {\n"
            // "  use \"std.e\"\n"
            // "}\n"
            );

            ASSERT_TRUE(globalScope->children.size() == 5);
            ASSERT_TRUE(globalScope->children[3]->type == parser::token::types::CONDITION);
            auto ifCondition = static_cast<parser::token::condition*>(globalScope->children[3]);
            ASSERT_EQ((std::string_view)"if", ifCondition->symbol);
            ASSERT_TRUE(ifCondition->header->type == parser::token::types::SCOPE);
            auto ifHeader = static_cast<parser::token::scope::base*>(ifCondition->header);

            ASSERT_TRUE(ifHeader->children.size() == 1);
            ASSERT_TRUE(ifHeader->children[0]->type == parser::token::types::OPERATOR);
            auto conditionOp = static_cast<parser::token::Operator::base*>(ifHeader->children[0]);
            ASSERT_EQ((std::string_view)">", conditionOp->symbol);
            ASSERT_TRUE(conditionOp->left->type == parser::token::types::OBJECT);
            ASSERT_EQ((std::string_view)"a", conditionOp->left->symbol);
            ASSERT_TRUE(conditionOp->right->type == parser::token::types::OBJECT);
            ASSERT_EQ((std::string_view)"b", conditionOp->right->symbol);

            ASSERT_TRUE(ifCondition->body->type == parser::token::types::SCOPE);
            auto ifBody = static_cast<parser::token::scope::base*>(ifCondition->body);
            ASSERT_TRUE(ifBody->children.size() == 1);
            ASSERT_TRUE(ifBody->children[0]->type == parser::token::types::OPERATOR);
            auto ifAssignOp = static_cast<parser::token::Operator::base*>(ifBody->children[0]);
            ASSERT_EQ((std::string_view)"=", ifAssignOp->symbol);
            ASSERT_TRUE(ifAssignOp->left->type == parser::token::types::OBJECT);
            ASSERT_EQ((std::string_view)"c", ifAssignOp->left->symbol);
            ASSERT_TRUE(ifAssignOp->right->type == parser::token::types::OBJECT);
            ASSERT_EQ((std::string_view)"a", ifAssignOp->right->symbol);

            ASSERT_TRUE(globalScope->children[4]->type == parser::token::types::CONDITION);
            auto elseCondition = static_cast<parser::token::condition*>(globalScope->children[4]);
            ASSERT_EQ((std::string_view)"else", elseCondition->symbol);
            ASSERT_TRUE(elseCondition->body->type == parser::token::types::SCOPE);
            ASSERT_TRUE(elseCondition->header == nullptr);
            auto elseBody = static_cast<parser::token::scope::base*>(elseCondition->body);
            ASSERT_TRUE(elseBody->children.size() == 1);
            ASSERT_TRUE(elseBody->children[0]->type == parser::token::types::OPERATOR);
            auto elseAssignOp = static_cast<parser::token::Operator::base*>(elseBody->children[0]);
            ASSERT_EQ((std::string_view)"=", elseAssignOp->symbol);
            ASSERT_TRUE(elseAssignOp->left->type == parser::token::types::OBJECT);
            ASSERT_EQ((std::string_view)"c", elseAssignOp->left->symbol);
            ASSERT_TRUE(elseAssignOp->right->type == parser::token::types::OBJECT);
            ASSERT_EQ((std::string_view)"b", elseAssignOp->right->symbol);
        }
    
        static void test_loops() {
            parser::token::scope::base* globalScope = parse(
            "int a, int b, int c\n"
            "while (a < b) {\n"
            "  a++\n"
            "}\n"
            "\n"
            "for (int i = 0; i < a; i++) {\n"
            "  b++\n"
            "}\n"
            "\n"            
            "for (; c < a; c++) {\n"
            "  a++\n"
            "}\n"
            "\n"
            "for (;b < a && c > a;) {\n"
            "  b++\n"
            "}\n"
            "\n"
            );

            // Should have: 3 definitions (a, b, c) + 4 loops = 7 children
            ASSERT_TRUE(globalScope->children.size() == 7);
            
            // Test while loop: while (a < b) { a++ }
            ASSERT_TRUE(globalScope->children[3]->type == parser::token::types::LOOP);
            auto whileLoop = static_cast<parser::token::looper*>(globalScope->children[3]);
            ASSERT_EQ((std::string_view)"while", whileLoop->symbol);
            ASSERT_TRUE(whileLoop->init == nullptr);
            ASSERT_TRUE(whileLoop->condition != nullptr);
            ASSERT_TRUE(whileLoop->footer == nullptr);
            ASSERT_TRUE(whileLoop->body != nullptr);
            
            // Check while condition: a < b
            ASSERT_TRUE(whileLoop->condition->type == parser::token::types::OPERATOR);
            auto whileCondOp = static_cast<parser::token::Operator::base*>(whileLoop->condition);
            ASSERT_EQ((std::string_view)"<", whileCondOp->symbol);
            ASSERT_TRUE(whileCondOp->left->type == parser::token::types::OBJECT);
            ASSERT_EQ((std::string_view)"a", whileCondOp->left->symbol);
            ASSERT_TRUE(whileCondOp->right->type == parser::token::types::OBJECT);
            ASSERT_EQ((std::string_view)"b", whileCondOp->right->symbol);
            
            // Check while body
            ASSERT_TRUE(whileLoop->body->type == parser::token::types::SCOPE);
            auto whileBody = static_cast<parser::token::scope::base*>(whileLoop->body);
            ASSERT_TRUE(whileBody->children.size() == 1);
            ASSERT_TRUE(whileBody->children[0]->type == parser::token::types::OPERATOR);
            auto whileIncOp = static_cast<parser::token::Operator::fix::base*>(whileBody->children[0]);
            ASSERT_EQ((std::string_view)"++", whileIncOp->symbol);
            
            // Test for loop with full syntax: for (int i = 0; i < a; i++) { b++ }
            ASSERT_TRUE(globalScope->children[4]->type == parser::token::types::LOOP);
            auto forLoop1 = static_cast<parser::token::looper*>(globalScope->children[4]);
            ASSERT_EQ((std::string_view)"for", forLoop1->symbol);
            ASSERT_TRUE(forLoop1->init != nullptr);
            ASSERT_TRUE(forLoop1->condition != nullptr);
            ASSERT_TRUE(forLoop1->footer != nullptr);
            ASSERT_TRUE(forLoop1->body != nullptr);
            
            // Check init: int i = 0
            ASSERT_TRUE(forLoop1->init->type == parser::token::types::OPERATOR);
            auto initOp = static_cast<parser::token::Operator::base*>(forLoop1->init);
            ASSERT_EQ((std::string_view)"=", initOp->symbol);
            ASSERT_TRUE(initOp->left->type == parser::token::types::DEFINITION);
            ASSERT_EQ((std::string_view)"i", initOp->left->symbol);
            ASSERT_TRUE(initOp->right->type == parser::token::types::NUMBER);
            ASSERT_EQ((std::string_view)"0", initOp->right->symbol);
            
            // Check condition: i < a
            ASSERT_TRUE(forLoop1->condition->type == parser::token::types::OPERATOR);
            auto condOp = static_cast<parser::token::Operator::base*>(forLoop1->condition);
            ASSERT_EQ((std::string_view)"<", condOp->symbol);
            ASSERT_TRUE(condOp->left->type == parser::token::types::OBJECT);
            ASSERT_EQ((std::string_view)"i", condOp->left->symbol);
            ASSERT_TRUE(condOp->right->type == parser::token::types::OBJECT);
            ASSERT_EQ((std::string_view)"a", condOp->right->symbol);
            
            // Check footer: i++
            ASSERT_TRUE(forLoop1->footer->type == parser::token::types::OPERATOR);
            auto footerOp = static_cast<parser::token::Operator::fix::base*>(forLoop1->footer);
            ASSERT_EQ((std::string_view)"++", footerOp->symbol);
            
            // Check body: b++
            ASSERT_TRUE(forLoop1->body->type == parser::token::types::SCOPE);
            auto forBody1 = static_cast<parser::token::scope::base*>(forLoop1->body);
            ASSERT_TRUE(forBody1->children.size() == 1);
            ASSERT_TRUE(forBody1->children[0]->type == parser::token::types::OPERATOR);
            
            // Test for loop with missing init: for (; c < a; c++) { a++ }
            ASSERT_TRUE(globalScope->children[5]->type == parser::token::types::LOOP);
            auto forLoop2 = static_cast<parser::token::looper*>(globalScope->children[5]);
            ASSERT_EQ((std::string_view)"for", forLoop2->symbol);
            ASSERT_TRUE(forLoop2->init == nullptr);
            ASSERT_TRUE(forLoop2->condition != nullptr);
            ASSERT_TRUE(forLoop2->footer != nullptr);
            ASSERT_TRUE(forLoop2->body != nullptr);
            
            // Check condition: c < a
            ASSERT_TRUE(forLoop2->condition->type == parser::token::types::OPERATOR);
            auto cond2Op = static_cast<parser::token::Operator::base*>(forLoop2->condition);
            ASSERT_EQ((std::string_view)"<", cond2Op->symbol);
            ASSERT_TRUE(cond2Op->left->type == parser::token::types::OBJECT);
            ASSERT_EQ((std::string_view)"c", cond2Op->left->symbol);
            ASSERT_TRUE(cond2Op->right->type == parser::token::types::OBJECT);
            ASSERT_EQ((std::string_view)"a", cond2Op->right->symbol);
            
            // Check footer: c++
            ASSERT_TRUE(forLoop2->footer->type == parser::token::types::OPERATOR);
            auto footer2Op = static_cast<parser::token::Operator::fix::base*>(forLoop2->footer);
            ASSERT_EQ((std::string_view)"++", footer2Op->symbol);
            
            // Test for loop with only condition: for (;b < a && c > a;) { b++ }
            ASSERT_TRUE(globalScope->children[6]->type == parser::token::types::LOOP);
            auto forLoop3 = static_cast<parser::token::looper*>(globalScope->children[6]);
            ASSERT_EQ((std::string_view)"for", forLoop3->symbol);
            ASSERT_TRUE(forLoop3->init == nullptr);
            ASSERT_TRUE(forLoop3->condition != nullptr);
            ASSERT_TRUE(forLoop3->footer == nullptr);
            ASSERT_TRUE(forLoop3->body != nullptr);
            
            // Check condition: b < a && c > a
            ASSERT_TRUE(forLoop3->condition->type == parser::token::types::OPERATOR);
            auto cond3Op = static_cast<parser::token::Operator::base*>(forLoop3->condition);
            ASSERT_EQ((std::string_view)"&&", cond3Op->symbol);
            
            // Left side of &&: b < a
            ASSERT_TRUE(cond3Op->left->type == parser::token::types::OPERATOR);
            auto leftCond = static_cast<parser::token::Operator::base*>(cond3Op->left);
            ASSERT_EQ((std::string_view)"<", leftCond->symbol);
            ASSERT_TRUE(leftCond->left->type == parser::token::types::OBJECT);
            ASSERT_EQ((std::string_view)"b", leftCond->left->symbol);
            ASSERT_TRUE(leftCond->right->type == parser::token::types::OBJECT);
            ASSERT_EQ((std::string_view)"a", leftCond->right->symbol);
            
            // Right side of &&: c > a
            ASSERT_TRUE(cond3Op->right->type == parser::token::types::OPERATOR);
            auto rightCond = static_cast<parser::token::Operator::base*>(cond3Op->right);
            ASSERT_EQ((std::string_view)">", rightCond->symbol);
            ASSERT_TRUE(rightCond->left->type == parser::token::types::OBJECT);
            ASSERT_EQ((std::string_view)"c", rightCond->left->symbol);
            ASSERT_TRUE(rightCond->right->type == parser::token::types::OBJECT);
            ASSERT_EQ((std::string_view)"a", rightCond->right->symbol);
        }
    
        static void test_basic_string_baking() {
            parser::token::scope::base* globalScope = parse(
            "\"Hello, World!\""
            );

            ASSERT_EQ((size_t)1, globalScope->children.size());
            ASSERT_TRUE(globalScope->children[0]->type == parser::token::types::STRING);
            auto strToken = static_cast<parser::token::string::base*>(globalScope->children[0]);
            ASSERT_TRUE(std::string("\"Hello, World!\"") == strToken->bakedString);
        }

        static void test_hexed_string_baking() {
            parser::token::scope::base* globalScope = parse(
            "\"0xFF abc\""
            );

            ASSERT_EQ((size_t)1, globalScope->children.size());
            ASSERT_TRUE(globalScope->children[0]->type == parser::token::types::STRING);
            auto strToken = static_cast<parser::token::string::base*>(globalScope->children[0]);
            ASSERT_TRUE(std::string("\"0xFF abc\"") == strToken->bakedString);
        }

        static void test_escaped_string_baking() {
            parser::token::scope::base* globalScope = parse(
            "\"Line1\\nLine2\\tTabbed\\\\Backslash\\\"Quote\""
            );

            ASSERT_EQ((size_t)1, globalScope->children.size());
            ASSERT_TRUE(globalScope->children[0]->type == parser::token::types::STRING);
            auto strToken = static_cast<parser::token::string::base*>(globalScope->children[0]);
            ASSERT_TRUE(std::string("\"Line1\nLine2\tTabbed\\Backslash\"Quote\"") == strToken->bakedString);
        }
    };
}

#endif
