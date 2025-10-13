#ifndef _LEXER_TESTER_H_
#define _LEXER_TESTER_H_

#include "utils.h"
#include "../../src/lexer/lexer.h"

#include <string>
#include <vector>

namespace tester {

    class lexerTester : public utils::TestSuite {
    public:
        lexerTester() : utils::TestSuite("lexer Tester") {
            add_test("Cluster Group Classification", "get_group matches expected metadata", test_cluster_group_classification);
            add_test("Number Token Float", "number tokens detect decimal text", test_number_token_float);
            add_test("Separator Classification", "separator identifies newline type", test_separator_classification);
            add_test("Wrapper Classification", "wrapper infers identity kinds", test_wrapper_classification);
            add_test("Control Presets", "preset control tokens expose correct types", test_control_presets);
            add_test("Tokenize Simple Sequence", "basic tokenization splits text and separators", test_tokenize_simple_sequence);
            add_test("Tokenize Merge Text Numbers", "alphanumeric words stay as single text tokens", test_tokenize_merge_text_numbers);
            add_test("Tokenize Float Numbers", "floating literals merge around dot operator", test_tokenize_float_numbers);
            add_test("Tokenize String Wrapper", "string contents tuck into wrapper tokens", test_tokenize_string_wrapper);
            add_test("Tokenize Collapse Newlines", "consecutive newlines reduce to single token", test_tokenize_collapse_newlines);
            add_test("Token Position Tracking", "tokenize keeps column/line offsets", test_token_position_tracking);
        }

    private:
        using TokenPtr = lexer::token::base*;

        static void destroy_tokens(std::vector<TokenPtr>& tokens) {
            for (auto* token : tokens) {
                if (!token) continue;
                if (token->get_type() == lexer::token::types::WRAPPER) {
                    auto* wrap = static_cast<lexer::token::wrapper*>(token);
                    destroy_tokens(wrap->tokens);
                    wrap->tokens.clear();
                }
                delete token;
            }
            tokens.clear();
        }

        struct TokenGuard {
            std::vector<TokenPtr> tokens;
            ~TokenGuard() {
                lexerTester::destroy_tokens(tokens);
            }
        };

        static void test_cluster_group_classification() {
            auto text_group = lexer::cluster::get_group('A');
            ASSERT_TRUE(text_group.type == lexer::token::types::TEXT);
            ASSERT_TRUE(text_group.sticky);
            ASSERT_EQ('A', text_group.min);
            ASSERT_EQ('Z', text_group.max);

            auto op_group = lexer::cluster::get_group('+');
            ASSERT_TRUE(op_group.type == lexer::token::types::OPERATOR);
            ASSERT_FALSE(op_group.sticky);

            auto unknown_group = lexer::cluster::get_group('@');
            ASSERT_EQ('@', unknown_group.min);
            ASSERT_TRUE(unknown_group.type == lexer::token::types::UNKNOWN);
        }

        static void test_number_token_float() {
            std::string literal = "12.34";
            lexer::token::number number_token({0, 0, 0}, literal);
            ASSERT_TRUE(number_token.number_type == lexer::token::number::types::FLOAT);

            std::string integer_literal = "42";
            lexer::token::number integer_token({1, 0, 0}, integer_literal);
            ASSERT_TRUE(integer_token.number_type == lexer::token::number::types::INTEGER);
        }

        static void test_separator_classification() {
            lexer::token::separator sep({0, 0, 0}, "\n");
            ASSERT_TRUE(sep.type == lexer::token::separator::types::NEWLINE);

            lexer::token::separator space({0, 0, 0}, " ");
            ASSERT_TRUE(space.type == lexer::token::separator::types::SPACE);
        }

        static void test_wrapper_classification() {
            std::string string_wrapper_text = "\"";
            lexer::token::wrapper string_wrapper({0, 0, 0}, string_wrapper_text);
            ASSERT_EQ('"', string_wrapper.identity);
            ASSERT_TRUE(string_wrapper.type == lexer::token::wrapper::types::STRING);

            std::string comment_text = "#";
            lexer::token::wrapper comment_wrapper({0, 0, 0}, comment_text);
            ASSERT_TRUE(comment_wrapper.type == lexer::token::wrapper::types::COMMENT);

            std::string round_text = "(";
            lexer::token::wrapper round_wrapper({0, 0, 0}, round_text);
            ASSERT_TRUE(round_wrapper.type == lexer::token::wrapper::types::ROUND_BRACKETS);
        }

        static void test_control_presets() {
            auto& sof = lexer::token::presets::start_of_file;
            ASSERT_TRUE(sof.get_type() == lexer::token::types::CONTROL);
            ASSERT_TRUE(sof.type == lexer::token::control::types::START_OF_FILE);

            auto eof = lexer::token::presets::end_of_file({0, 0, 0});
            ASSERT_TRUE(eof.get_type() == lexer::token::types::CONTROL);
            ASSERT_TRUE(eof.type == lexer::token::control::types::END_OF_FILE);
        }

        static void test_tokenize_simple_sequence() {
            TokenGuard guard;
            guard.tokens = lexer::tokenize("foo bar", 7);

            ASSERT_EQ(static_cast<std::size_t>(3), guard.tokens.size());
            ASSERT_TRUE(guard.tokens[0]->get_type() == lexer::token::types::TEXT);
            ASSERT_TRUE(guard.tokens[1]->get_type() == lexer::token::types::SEPARATOR);
            ASSERT_TRUE(guard.tokens[2]->get_type() == lexer::token::types::TEXT);

            auto* first = static_cast<lexer::token::text*>(guard.tokens[0]);
            auto* second = static_cast<lexer::token::text*>(guard.tokens[2]);
            ASSERT_EQ(std::string("foo"), first->data);
            ASSERT_EQ(std::string("bar"), second->data);
        }

        static void test_tokenize_merge_text_numbers() {
            TokenGuard guard;
            guard.tokens = lexer::tokenize("foo123", 0);

            ASSERT_EQ(static_cast<std::size_t>(1), guard.tokens.size());
            ASSERT_TRUE(guard.tokens[0]->get_type() == lexer::token::types::TEXT);
            auto* text = static_cast<lexer::token::text*>(guard.tokens[0]);
            ASSERT_EQ(std::string("foo123"), text->data);
        }

        static void test_tokenize_float_numbers() {
            TokenGuard guard;
            guard.tokens = lexer::tokenize("123.45", 0);

            ASSERT_EQ(static_cast<std::size_t>(1), guard.tokens.size());
            ASSERT_TRUE(guard.tokens[0]->get_type() == lexer::token::types::NUMBER);
            auto* number = static_cast<lexer::token::number*>(guard.tokens[0]);
            ASSERT_EQ(std::string("123.45"), number->text);
            ASSERT_TRUE(number->number_type == lexer::token::number::types::FLOAT);
        }

        static void test_tokenize_string_wrapper() {
            TokenGuard guard;
            guard.tokens = lexer::tokenize("\"hi\"", 0);

            ASSERT_EQ(static_cast<std::size_t>(1), guard.tokens.size());
            ASSERT_TRUE(guard.tokens[0]->get_type() == lexer::token::types::WRAPPER);

            auto* wrap = static_cast<lexer::token::wrapper*>(guard.tokens[0]);
            ASSERT_EQ(static_cast<std::size_t>(1), wrap->tokens.size());
            ASSERT_TRUE(wrap->tokens[0]->get_type() == lexer::token::types::TEXT);

            auto* inner = static_cast<lexer::token::text*>(wrap->tokens[0]);
            ASSERT_EQ(std::string("hi"), inner->data);
        }

        static void test_tokenize_collapse_newlines() {
            TokenGuard guard;
            guard.tokens = lexer::tokenize("\n\n\n", 0);

            ASSERT_EQ(static_cast<std::size_t>(1), guard.tokens.size());
            ASSERT_TRUE(guard.tokens[0]->get_type() == lexer::token::types::SEPARATOR);
            auto* newline = static_cast<lexer::token::separator*>(guard.tokens[0]);
            ASSERT_TRUE(newline->type == lexer::token::separator::types::NEWLINE);
        }

        static void test_token_position_tracking() {
            TokenGuard guard;
            guard.tokens = lexer::tokenize("a\nb", 2);

            ASSERT_EQ(static_cast<std::size_t>(3), guard.tokens.size());
            auto start_a = guard.tokens[0]->get_start();
            ASSERT_EQ(static_cast<unsigned short>(0), start_a.x);
            ASSERT_EQ(static_cast<unsigned short>(0), start_a.y);
            ASSERT_EQ(static_cast<unsigned short>(2), start_a.file_id);

            auto newline = guard.tokens[1]->get_start();
            ASSERT_EQ(static_cast<unsigned short>(1), newline.x);
            ASSERT_EQ(static_cast<unsigned short>(0), newline.y);

            auto start_b = guard.tokens[2]->get_start();
            ASSERT_EQ(static_cast<unsigned short>(0), start_b.x);
            ASSERT_EQ(static_cast<unsigned short>(1), start_b.y);
        }
    };
}

#endif
