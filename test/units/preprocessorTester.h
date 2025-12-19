#ifndef _preprocessor_tester_h_
#define _preprocessor_tester_h_

#include "utils.h"
#include "../../src/preprocessor/preprocessor.h"
#include "../../src/parser/parser.h"
#include "../../src/lexer/lexer.h"
#include "../../src/args/args.h"
#include "../../src/docker/docker.h"

#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include <memory>

namespace tester {

    class preprocessorTester : public utils::TestSuite {
    public:
        preprocessorTester() : utils::TestSuite("preprocessor Tester") {
            add_test("Inline If True Literal", "inlines body when condition is literal true", test_inline_if_true_literal);
            add_test("Inline If False Literal", "inlines else when condition is literal false", test_inline_if_false_literal);
            add_test("No Inline Runtime Condition", "does not inline runtime conditions", test_no_inline_runtime_condition);
            add_test("Include Inlines And Removes Markers", "inlines include tokens and removes include markers", test_include_inlines_and_removes_markers);
            add_test("Duplicate Include No Skip", "duplicate include does not re-inline and does not skip following tokens", test_duplicate_include_only_once_and_no_skip);
        }

    private:
        static void cleanup_test_file(const std::string& path) {
            if (std::filesystem::exists(path)) {
                std::filesystem::remove(path);
            }
        }

        static void create_test_file(const std::string& path, const std::string& content) {
            std::ofstream file(path);
            if (!file.is_open()) {
                throw std::runtime_error("Failed to create test file: " + path);
            }
            file << content;
            file.close();
        }

        static parser::token::scope::base* parse_and_preprocess(const std::string& input, args::base* env, docker::stack* stack) {
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

            preprocessor::unit preprocess(globalScope, env, stack);
            preprocess.factory();

            return globalScope;
        }

        static bool has_token_type(parser::token::scope::base* scope, parser::token::types type) {
            for (auto* t : scope->children) {
                if (t && t->type == type) return true;
            }
            return false;
        }

        static size_t count_token_type(parser::token::scope::base* scope, parser::token::types type) {
            size_t count = 0;
            for (auto* t : scope->children) {
                if (t && t->type == type) count++;
            }
            return count;
        }

        static parser::token::base* find_first_child_of_type(parser::token::scope::base* scope, parser::token::types type) {
            for (auto* t : scope->children) {
                if (t && t->type == type) return t;
            }
            return nullptr;
        }

        static size_t count_definitions(parser::token::scope::base* scope, std::string_view symbol) {
            size_t count = 0;
            for (auto* d : scope->definitions) {
                if (d && d->type == parser::token::types::DEFINITION && d->symbol == symbol) {
                    count++;
                }
            }
            return count;
        }

        static parser::token::Operator::base* find_assignment(parser::token::scope::base* scope, std::string_view lhsSymbol, std::string_view rhsSymbol) {
            for (auto* t : scope->children) {
                if (!t || t->type != parser::token::types::OPERATOR) continue;

                auto* op = dynamic_cast<parser::token::Operator::base*>(t);
                if (!op) continue;
                if (op->operationType != parser::token::Operator::types::ASSIGN) continue;
                if (!op->left || !op->right) continue;

                if (op->left->symbol == lhsSymbol && op->right->symbol == rhsSymbol) {
                    return op;
                }
            }
            return nullptr;
        }

        static std::vector<parser::token::Operator::base*> collect_assignments(parser::token::scope::base* scope) {
            std::vector<parser::token::Operator::base*> result;
            for (auto* t : scope->children) {
                if (!t || t->type != parser::token::types::OPERATOR) continue;

                auto* op = dynamic_cast<parser::token::Operator::base*>(t);
                if (!op) continue;
                if (op->operationType != parser::token::Operator::types::ASSIGN) continue;
                if (!op->left || !op->right) continue;

                result.push_back(op);
            }
            return result;
        }

        static std::vector<parser::token::Operator::base*> collect_assignments_to(parser::token::scope::base* scope, std::string_view lhsSymbol) {
            std::vector<parser::token::Operator::base*> result;
            for (auto* op : collect_assignments(scope)) {
                if (op->left && op->left->symbol == lhsSymbol) {
                    result.push_back(op);
                }
            }
            return result;
        }

        static void assert_assignment_node(parser::token::Operator::base* assignOp, std::string_view lhsSymbol, std::string_view rhsSymbol) {
            ASSERT_TRUE(assignOp != nullptr);
            ASSERT_EQ((int)parser::token::types::OPERATOR, (int)assignOp->type);
            ASSERT_EQ((int)parser::token::Operator::types::ASSIGN, (int)assignOp->operationType);
            ASSERT_EQ((std::string_view)"=", assignOp->symbol);

            ASSERT_TRUE(assignOp->left != nullptr);
            ASSERT_EQ(lhsSymbol, assignOp->left->symbol);

            ASSERT_TRUE(assignOp->right != nullptr);
            ASSERT_EQ(rhsSymbol, assignOp->right->symbol);
        }

        static parser::token::definition::base* find_definition(parser::token::scope::base* scope, std::string_view symbol) {
            for (auto* d : scope->definitions) {
                if (d && d->type == parser::token::types::DEFINITION && d->symbol == symbol) {
                    return static_cast<parser::token::definition::base*>(d);
                }
            }
            return nullptr;
        }

        static bool has_definition(parser::token::scope::base* scope, std::string_view symbol) {
            for (auto* d : scope->definitions) {
                if (d && d->type == parser::token::types::DEFINITION && d->symbol == symbol) return true;
            }
            return false;
        }

        static void test_inline_if_true_literal() {
            args::base env;
            docker::stack stack;

            parser::token::scope::base* globalScope = parse_and_preprocess(
                "int a = 0\n"
                "if (true) {\n"
                "  a = 2\n"
                "} else {\n"
                "  a = 3\n"
                "}\n",
                &env,
                &stack
            );

            ASSERT_TRUE(globalScope != nullptr);
            ASSERT_TRUE((size_t)2 == globalScope->children.size());
            auto* firstAssign = dynamic_cast<parser::token::Operator::base*>(globalScope->children[0]);
            ASSERT_TRUE(firstAssign != nullptr);
            assert_assignment_node(firstAssign, "a", "0");

            auto* secondAssign = dynamic_cast<parser::token::Operator::base*>(globalScope->children[1]);
            ASSERT_TRUE(secondAssign != nullptr);
            assert_assignment_node(secondAssign, "a", "2");
        }

        static void test_inline_if_false_literal() {
            args::base env;
            docker::stack stack;

            parser::token::scope::base* globalScope = parse_and_preprocess(
                "int a = 0\n"
                "if (false) {\n"
                "  a = 2\n"
                "} else {\n"
                "  a = 3\n"
                "}\n",
                &env,
                &stack
            );

            ASSERT_TRUE(globalScope != nullptr);
            ASSERT_TRUE((size_t)2 == globalScope->children.size());
            
            auto* firstAssign = dynamic_cast<parser::token::Operator::base*>(globalScope->children[0]);
            ASSERT_TRUE(firstAssign != nullptr);
            assert_assignment_node(firstAssign, "a", "0");

            auto* secondAssign = dynamic_cast<parser::token::Operator::base*>(globalScope->children[1]);
            ASSERT_TRUE(secondAssign != nullptr);
            assert_assignment_node(secondAssign, "a", "3");
        }

        static void test_no_inline_runtime_condition() {
            args::base env;
            docker::stack stack;

            parser::token::scope::base* globalScope = parse_and_preprocess(
                "int a = 0\n"
                "int b\n"
                "if (a > b) {\n"
                "  a = 2\n"
                "} else {\n"
                "  a = 3\n"
                "}\n",
                &env,
                &stack
            );

            ASSERT_TRUE(globalScope != nullptr);
            ASSERT_TRUE((size_t)3 == globalScope->children.size());

            auto* firstAssign = dynamic_cast<parser::token::Operator::base*>(globalScope->children[0]);
            ASSERT_TRUE(firstAssign != nullptr);
            assert_assignment_node(firstAssign, "a", "0");

            auto* bDefinition = dynamic_cast<parser::token::definition::base*>(globalScope->children[1]);
            ASSERT_TRUE(bDefinition);
            ASSERT_TRUE(bDefinition->symbol == "b");
        
            auto* condition = dynamic_cast<parser::token::condition*>(globalScope->children[2]);
            ASSERT_TRUE(condition != nullptr);
            ASSERT_TRUE((size_t)1 == condition->branches.size());
        }

        static void test_include_inlines_and_removes_markers() {
            const std::string includeFile = "test_include_inlines_and_removes_markers.e";

            cleanup_test_file(includeFile);
            create_test_file(
                includeFile,
                "int x = 9\n"
            );

            args::base env;
            docker::stack stack;

            parser::token::scope::base* globalScope = parse_and_preprocess(
                "include \"" + includeFile + "\"\n"
                "int y = 1\n",
                &env,
                &stack
            );

            ASSERT_TRUE(globalScope != nullptr);
            ASSERT_TRUE((size_t)2 == globalScope->children.size());

            auto* firstAssign = dynamic_cast<parser::token::Operator::base*>(globalScope->children[0]);
            ASSERT_TRUE(firstAssign != nullptr);
            assert_assignment_node(firstAssign, "x", "9");

            auto* secondAssign = dynamic_cast<parser::token::Operator::base*>(globalScope->children[1]);
            ASSERT_TRUE(secondAssign != nullptr);
            assert_assignment_node(secondAssign, "y", "1");

            cleanup_test_file(includeFile);
        }

        static void test_duplicate_include_only_once_and_no_skip() {
            const std::string includeFile = "test_duplicate_include_only_once_and_no_skip.e";

            cleanup_test_file(includeFile);
            create_test_file(
                includeFile,
                "int x = 9\n"
            );

            args::base env;
            docker::stack stack;

            parser::token::scope::base* globalScope = parse_and_preprocess(
                "include \"" + includeFile + "\"\n"
                "int a = 1\n"
                "include \"" + includeFile + "\"\n"
                "int b = 2\n",
                &env,
                &stack
            );

            ASSERT_TRUE(globalScope != nullptr);
            ASSERT_TRUE((size_t)3 == globalScope->children.size());

            auto* firstAssign = dynamic_cast<parser::token::Operator::base*>(globalScope->children[0]);
            ASSERT_TRUE(firstAssign != nullptr);
            assert_assignment_node(firstAssign, "x", "9");

            auto* secondAssign = dynamic_cast<parser::token::Operator::base*>(globalScope->children[1]);
            ASSERT_TRUE(secondAssign != nullptr);
            assert_assignment_node(secondAssign, "a", "1");

            auto* thirdAssign = dynamic_cast<parser::token::Operator::base*>(globalScope->children[2]);
            ASSERT_TRUE(thirdAssign != nullptr);
            assert_assignment_node(thirdAssign, "b", "2");

            cleanup_test_file(includeFile);
        }
    };
}

#endif
