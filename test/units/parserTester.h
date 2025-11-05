#ifndef _PARSER_TESTER_H_
#define _PARSER_TESTER_H_

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
        }

    private:
        
        static void test_simple_definition() {
            auto lexerOutput = lexer::tokenize(
            "int a;\n"
            "int b;\n"
            , 0);

            parser::token::scope::base* globalScope = new parser::token::scope::base(
                parser::token::info(
                    parser::token::type::SCOPE,
                    lexer::token::position(0, 0, 0),
                    nullptr,
                    "global"
                ),
                lexerOutput
            );

            parser::unit::base parse(parser::unit::pass::FIRST, globalScope);
            parse.factory();

            ASSERT_EQ((size_t)2, globalScope->definitions.size());

            auto a_Def = static_cast<parser::token::definition*>(globalScope->definitions[0]);
            ASSERT_EQ((std::string_view)"a", a_Def->symbol);
            ASSERT_EQ((int)parser::token::type::DEFINITION, (int)a_Def->flags);
            ASSERT_EQ((size_t)1, a_Def->inherited.size());
            ASSERT_EQ((std::string_view)"int", a_Def->inherited[0]);

            auto b_Def = static_cast<parser::token::definition*>(globalScope->definitions[1]);
            ASSERT_EQ((std::string_view)"b", b_Def->symbol);
            ASSERT_EQ((int)parser::token::type::DEFINITION, (int)b_Def->flags);
            ASSERT_EQ((size_t)1, b_Def->inherited.size());
            ASSERT_EQ((std::string_view)"int", b_Def->inherited[0]);
        }

        static void test_operation_order() {
            auto lexerOutput = lexer::tokenize(
            "int a, int b, int c, int d\n"
            "a = 1 + 2 * 3 - 4\n"
            "a == 5 && b != 6 || c < 7 >= d\n"
            , 0);

            parser::token::scope::base* globalScope = new parser::token::scope::base(
                parser::token::info(
                    parser::token::type::SCOPE,
                    lexer::token::position(0, 0, 0),
                    nullptr,
                    "global"
                ),
                lexerOutput
            );

            parser::unit::base parse(parser::unit::pass::FIRST, globalScope);
            parse.factory();

            // Test: a = 1 + 2 * 3 - 4
            // Expected AST: a = ((1 + (2 * 3)) - 4)
            // The assignment should be the root, with 'a' on left and expression on right
            
            // Find the assignment operator in the raw tokens
            parser::token::Operator::base* assignOp = nullptr;
            for (auto token : globalScope->rawTokens) {
                if (token->get_type() == lexer::token::types::OPERATOR && 
                    token->parsed && 
                    token->parsed->flags == parser::token::type::OPERATOR) {
                    auto op = static_cast<parser::token::Operator::base*>(token->parsed);
                    if (op->operationType == parser::token::Operator::type::ASSIGN) {
                        assignOp = op;
                        break;
                    }
                }
            }
            
            ASSERT_TRUE(assignOp != nullptr);
            ASSERT_EQ((std::string_view)"=", assignOp->symbol);
            
            // Left side should be object 'a'
            ASSERT_TRUE(assignOp->left != nullptr);
            ASSERT_EQ((int)parser::token::type::OBJECT, (int)assignOp->left->flags);
            ASSERT_EQ((std::string_view)"a", assignOp->left->symbol);
            
            // Right side should be subtraction: (1 + 2 * 3) - 4
            ASSERT_TRUE(assignOp->right != nullptr);
            ASSERT_EQ((int)parser::token::type::OPERATOR, (int)assignOp->right->flags);
            auto subOp = static_cast<parser::token::Operator::base*>(assignOp->right);
            ASSERT_EQ((int)parser::token::Operator::type::SUBTRACTION, (int)subOp->operationType);
            ASSERT_EQ((std::string_view)"-", subOp->symbol);
            
            // Subtraction left: 1 + 2 * 3
            ASSERT_TRUE(subOp->left != nullptr);
            ASSERT_EQ((int)parser::token::type::OPERATOR, (int)subOp->left->flags);
            auto addOp = static_cast<parser::token::Operator::base*>(subOp->left);
            ASSERT_EQ((int)parser::token::Operator::type::ADDITION, (int)addOp->operationType);
            ASSERT_EQ((std::string_view)"+", addOp->symbol);
            
            // Subtraction right: 4
            ASSERT_TRUE(subOp->right != nullptr);
            ASSERT_EQ((int)parser::token::type::NUMBER, (int)subOp->right->flags);
            ASSERT_EQ((std::string_view)"4", subOp->right->symbol);
            
            // Addition left: 1
            ASSERT_TRUE(addOp->left != nullptr);
            ASSERT_EQ((int)parser::token::type::NUMBER, (int)addOp->left->flags);
            ASSERT_EQ((std::string_view)"1", addOp->left->symbol);
            
            // Addition right: 2 * 3
            ASSERT_TRUE(addOp->right != nullptr);
            ASSERT_EQ((int)parser::token::type::OPERATOR, (int)addOp->right->flags);
            auto mulOp = static_cast<parser::token::Operator::base*>(addOp->right);
            ASSERT_EQ((int)parser::token::Operator::type::MULTIPLICATION, (int)mulOp->operationType);
            ASSERT_EQ((std::string_view)"*", mulOp->symbol);
            
            // Multiplication left: 2
            ASSERT_TRUE(mulOp->left != nullptr);
            ASSERT_EQ((int)parser::token::type::NUMBER, (int)mulOp->left->flags);
            ASSERT_EQ((std::string_view)"2", mulOp->left->symbol);
            
            // Multiplication right: 3
            ASSERT_TRUE(mulOp->right != nullptr);
            ASSERT_EQ((int)parser::token::type::NUMBER, (int)mulOp->right->flags);
            ASSERT_EQ((std::string_view)"3", mulOp->right->symbol);
            
            // Test: a == 5 && b != 6 || c < 7 >= d
            // Expected AST: ((a == 5) && (b != 6)) || (c < 7) >= d
            // The logical OR should be at the top level
            
            // Find the logical OR operator
            parser::token::Operator::base* orOp = nullptr;
            for (auto token : globalScope->rawTokens) {
                if (token->get_type() == lexer::token::types::OPERATOR && 
                    token->parsed && 
                    token->parsed->flags == parser::token::type::OPERATOR) {
                    auto op = static_cast<parser::token::Operator::base*>(token->parsed);
                    if (op->operationType == parser::token::Operator::type::LOGICAL_OR) {
                        orOp = op;
                        break;
                    }
                }
            }
            
            ASSERT_TRUE(orOp != nullptr);
            ASSERT_EQ((std::string_view)"||", orOp->symbol);
            
            // Left side of ||: (a == 5) && (b != 6)
            ASSERT_TRUE(orOp->left != nullptr);
            ASSERT_EQ((int)parser::token::type::OPERATOR, (int)orOp->left->flags);
            auto andOp = static_cast<parser::token::Operator::base*>(orOp->left);
            ASSERT_EQ((int)parser::token::Operator::type::LOGICAL_AND, (int)andOp->operationType);
            ASSERT_EQ((std::string_view)"&&", andOp->symbol);
            
            // Left side of &&: a == 5
            ASSERT_TRUE(andOp->left != nullptr);
            ASSERT_EQ((int)parser::token::type::OPERATOR, (int)andOp->left->flags);
            auto eqOp = static_cast<parser::token::Operator::base*>(andOp->left);
            ASSERT_EQ((int)parser::token::Operator::type::COMPARISON, (int)eqOp->operationType);
            ASSERT_EQ((std::string_view)"==", eqOp->symbol);
            
            // a == 5 left: a
            ASSERT_TRUE(eqOp->left != nullptr);
            ASSERT_EQ((int)parser::token::type::OBJECT, (int)eqOp->left->flags);
            ASSERT_EQ((std::string_view)"a", eqOp->left->symbol);
            
            // a == 5 right: 5
            ASSERT_TRUE(eqOp->right != nullptr);
            ASSERT_EQ((int)parser::token::type::NUMBER, (int)eqOp->right->flags);
            ASSERT_EQ((std::string_view)"5", eqOp->right->symbol);
            
            // Right side of &&: b != 6
            ASSERT_TRUE(andOp->right != nullptr);
            ASSERT_EQ((int)parser::token::type::OPERATOR, (int)andOp->right->flags);
            auto neOp = static_cast<parser::token::Operator::base*>(andOp->right);
            ASSERT_EQ((int)parser::token::Operator::type::COMPARISON, (int)neOp->operationType);
            ASSERT_EQ((std::string_view)"!=", neOp->symbol);
            
            // b != 6 left: b
            ASSERT_TRUE(neOp->left != nullptr);
            ASSERT_EQ((int)parser::token::type::OBJECT, (int)neOp->left->flags);
            ASSERT_EQ((std::string_view)"b", neOp->left->symbol);
            
            // b != 6 right: 6
            ASSERT_TRUE(neOp->right != nullptr);
            ASSERT_EQ((int)parser::token::type::NUMBER, (int)neOp->right->flags);
            ASSERT_EQ((std::string_view)"6", neOp->right->symbol);
            
            // Right side of ||: c < 7 >= d
            // Note: This should parse as (c < 7) >= d due to left-to-right associativity
            ASSERT_TRUE(orOp->right != nullptr);
            ASSERT_EQ((int)parser::token::type::OPERATOR, (int)orOp->right->flags);
            auto geOp = static_cast<parser::token::Operator::base*>(orOp->right);
            ASSERT_EQ((int)parser::token::Operator::type::COMPARISON, (int)geOp->operationType);
            ASSERT_EQ((std::string_view)">=", geOp->symbol);
            
            // Left of >=: c < 7
            ASSERT_TRUE(geOp->left != nullptr);
            ASSERT_EQ((int)parser::token::type::OPERATOR, (int)geOp->left->flags);
            auto ltOp = static_cast<parser::token::Operator::base*>(geOp->left);
            ASSERT_EQ((int)parser::token::Operator::type::COMPARISON, (int)ltOp->operationType);
            ASSERT_EQ((std::string_view)"<", ltOp->symbol);
            
            // c < 7 left: c
            ASSERT_TRUE(ltOp->left != nullptr);
            ASSERT_EQ((int)parser::token::type::OBJECT, (int)ltOp->left->flags);
            ASSERT_EQ((std::string_view)"c", ltOp->left->symbol);
            
            // c < 7 right: 7
            ASSERT_TRUE(ltOp->right != nullptr);
            ASSERT_EQ((int)parser::token::type::NUMBER, (int)ltOp->right->flags);
            ASSERT_EQ((std::string_view)"7", ltOp->right->symbol);
            
            // Right of >=: d
            ASSERT_TRUE(geOp->right != nullptr);
            ASSERT_EQ((int)parser::token::type::OBJECT, (int)geOp->right->flags);
            ASSERT_EQ((std::string_view)"d", geOp->right->symbol);
        }
    
        static void test_simple_parenthesis() {
            auto lexerOutput = lexer::tokenize(
            "int a, int b, int c, int d\n"
            "a = (c + b) * d\n"
            , 0);

            parser::token::scope::base* globalScope = new parser::token::scope::base(
                parser::token::info(
                    parser::token::type::SCOPE,
                    lexer::token::position(0, 0, 0),
                    nullptr,
                    "global"
                ),
                lexerOutput
            );

            parser::unit::base parse(parser::unit::pass::FIRST, globalScope);
            parse.factory();

            ASSERT_EQ((size_t)5, globalScope->children.size());
            ASSERT_TRUE(globalScope->children[4]->flags == parser::token::type::OPERATOR);
            auto assignOp = static_cast<parser::token::Operator::base*>(globalScope->children[4]);
            ASSERT_EQ((std::string_view)"=", assignOp->symbol);
            ASSERT_TRUE(assignOp->left->flags == parser::token::type::OBJECT);
            ASSERT_EQ((std::string_view)"a", assignOp->left->symbol);
            ASSERT_TRUE(assignOp->right->flags == parser::token::type::OPERATOR);
            auto mulOp = static_cast<parser::token::Operator::base*>(assignOp->right);
            ASSERT_EQ((std::string_view)"*", mulOp->symbol);
            ASSERT_TRUE(mulOp->right->flags == parser::token::type::OBJECT);
            ASSERT_EQ((std::string_view)"d", mulOp->right->symbol);
            ASSERT_TRUE(mulOp->left->flags == parser::token::type::SCOPE);
            auto parenScope = static_cast<parser::token::scope::base*>(mulOp->left);
            ASSERT_EQ((size_t)1, parenScope->children.size());
            ASSERT_TRUE(parenScope->children[0]->flags == parser::token::type::OPERATOR);
            auto addOp = static_cast<parser::token::Operator::base*>(parenScope->children[0]);
            ASSERT_EQ((std::string_view)"+", addOp->symbol);
            ASSERT_TRUE(addOp->left->flags == parser::token::type::OBJECT);
            ASSERT_EQ((std::string_view)"c", addOp->left->symbol);
            ASSERT_TRUE(addOp->right->flags == parser::token::type::OBJECT);
            ASSERT_EQ((std::string_view)"b", addOp->right->symbol);
        }

        static void test_chained_parenthesis() {
            auto lexerOutput = lexer::tokenize(
            "void foo(int a, int b) {\n"
            "  int c = a + b\n"
            "  int d = c - a\n"
            "}"
            , 0);

            parser::token::scope::base* globalScope = new parser::token::scope::base(
                parser::token::info(
                    parser::token::type::SCOPE,
                    lexer::token::position(0, 0, 0),
                    nullptr,
                    "global"
                ),
                lexerOutput
            );

            parser::unit::base parse(parser::unit::pass::FIRST, globalScope);
            parse.factory();

            ASSERT_TRUE(globalScope->children.size() == 1);
            ASSERT_TRUE(globalScope->children[0]->flags == parser::token::type::FUNCTION);
            auto funcScope = static_cast<parser::token::function::base*>(globalScope->children[0]);
            ASSERT_EQ((std::string_view)"foo", funcScope->symbol);
            ASSERT_TRUE(funcScope->parameters->definitions.size() == 2);
            ASSERT_TRUE(funcScope->body->children.size() == 2);

            ASSERT_TRUE(funcScope->parameters->definitions[0]->flags == parser::token::type::DEFINITION);
            ASSERT_EQ((std::string_view)"a", funcScope->parameters->definitions[0]->symbol);
            ASSERT_TRUE(funcScope->parameters->definitions[1]->flags == parser::token::type::DEFINITION);
            ASSERT_EQ((std::string_view)"b", funcScope->parameters->definitions[1]->symbol);

            ASSERT_TRUE(funcScope->body->definitions[0]->flags == parser::token::type::DEFINITION);
            auto c_Def = static_cast<parser::token::definition*>(funcScope->body->definitions[0]);
            ASSERT_EQ((std::string_view)"c", c_Def->symbol);
            ASSERT_TRUE(funcScope->body->definitions[1]->flags == parser::token::type::DEFINITION);
            auto d_Def = static_cast<parser::token::definition*>(funcScope->body->definitions[1]);
            ASSERT_EQ((std::string_view)"d", d_Def->symbol);

            ASSERT_TRUE(funcScope->body->children[0]->flags == parser::token::type::OPERATOR);
            auto assignCOp = static_cast<parser::token::Operator::base*>(funcScope->body->children[0]);
            ASSERT_EQ((std::string_view)"=", assignCOp->symbol);
            ASSERT_TRUE(assignCOp->left->flags == parser::token::type::OBJECT);
            ASSERT_EQ((std::string_view)"c", assignCOp->left->symbol);
            ASSERT_TRUE(assignCOp->right->flags == parser::token::type::OPERATOR);
            auto addOp = static_cast<parser::token::Operator::base*>(assignCOp->right);
            ASSERT_EQ((std::string_view)"+", addOp->symbol);
            ASSERT_TRUE(addOp->left->flags == parser::token::type::OBJECT);
            ASSERT_EQ((std::string_view)"a", addOp->left->symbol);
            ASSERT_TRUE(addOp->right->flags == parser::token::type::OBJECT);
            ASSERT_EQ((std::string_view)"b", addOp->right->symbol);
            ASSERT_TRUE(funcScope->body->children[1]->flags == parser::token::type::OPERATOR);
            auto assignDOp = static_cast<parser::token::Operator::base*>(funcScope->body->children[1]);
            ASSERT_EQ((std::string_view)"=", assignDOp->symbol);
            ASSERT_TRUE(assignDOp->left->flags == parser::token::type::OBJECT);
            ASSERT_EQ((std::string_view)"d", assignDOp->left->symbol);
            ASSERT_TRUE(assignDOp->right->flags == parser::token::type::OPERATOR);
            auto subOp = static_cast<parser::token::Operator::base*>(assignDOp->right);
            ASSERT_EQ((std::string_view)"-", subOp->symbol);
            ASSERT_TRUE(subOp->left->flags == parser::token::type::OBJECT);
            ASSERT_EQ((std::string_view)"c", subOp->left->symbol);
            ASSERT_TRUE(subOp->right->flags == parser::token::type::OBJECT);
            ASSERT_EQ((std::string_view)"a", subOp->right->symbol);
        }

        static void test_class_construct() {
            auto lexerOutput = lexer::tokenize(
            "class a {\n"
            "  int b = 0\n"
            "  int c = b\n"
            "}\n"
            , 0);

            parser::token::scope::base* globalScope = new parser::token::scope::base(
                parser::token::info(
                    parser::token::type::SCOPE,
                    lexer::token::position(0, 0, 0),
                    nullptr,
                    "global"
                ),
                lexerOutput
            );

            parser::unit::base parse(parser::unit::pass::FIRST, globalScope);
            parse.factory();

            ASSERT_TRUE(globalScope->children.size() == 1);
        }
    };
}

#endif
