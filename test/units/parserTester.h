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

            ASSERT_EQ((size_t)0, parse.output.size());
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

            ASSERT_EQ((size_t)0, parse.output.size());
        }
    };
}

#endif
