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
            add_test("Tokenize Nested Parentheses", "recursive wrappers build parent-child chains", test_tokenize_nested_parentheses);
            add_test("Tokenize Nested Siblings", "sequential wrappers keep hierarchy intact", test_tokenize_nested_siblings);
            add_test("Tokenize Hex Literal", "0x prefixed numbers collapse into single hex token", test_tokenize_hex_literal);
            add_test("Tokenize Multiple Hex Literals", "multiple hex tokens keep separators intact", test_tokenize_multiple_hex_literals);
            add_test("Tokenize Combined Operators", "compound operators stay merged as single tokens", test_tokenize_combined_operators);
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
            // ASSERT_TRUE(text_group.sticky);
            ASSERT_EQ('A', text_group.min);
            ASSERT_EQ('Z', text_group.max);

            auto op_group = lexer::cluster::get_group('+');
            ASSERT_TRUE(op_group.type == lexer::token::types::OPERATOR);
            // ASSERT_FALSE(op_group.sticky);

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

        static void test_tokenize_nested_parentheses() {
            TokenGuard guard;
            guard.tokens = lexer::tokenize("(foo(bar(baz)))", 0);

            ASSERT_EQ(static_cast<std::size_t>(1), guard.tokens.size());
            ASSERT_TRUE(guard.tokens[0]->get_type() == lexer::token::types::WRAPPER);

            auto* outer = static_cast<lexer::token::wrapper*>(guard.tokens[0]);
            ASSERT_TRUE(outer->type == lexer::token::wrapper::types::ROUND_BRACKETS);
            ASSERT_EQ('(', outer->identity);
            ASSERT_EQ(static_cast<std::size_t>(2), outer->tokens.size());

            ASSERT_TRUE(outer->tokens[0]->get_type() == lexer::token::types::TEXT);
            auto* outer_text = static_cast<lexer::token::text*>(outer->tokens[0]);
            ASSERT_EQ(std::string("foo"), outer_text->data);

            ASSERT_TRUE(outer->tokens[1]->get_type() == lexer::token::types::WRAPPER);
            auto* middle = static_cast<lexer::token::wrapper*>(outer->tokens[1]);
            ASSERT_TRUE(middle->type == lexer::token::wrapper::types::ROUND_BRACKETS);
            ASSERT_EQ(static_cast<std::size_t>(2), middle->tokens.size());

            ASSERT_TRUE(middle->tokens[0]->get_type() == lexer::token::types::TEXT);
            auto* middle_text = static_cast<lexer::token::text*>(middle->tokens[0]);
            ASSERT_EQ(std::string("bar"), middle_text->data);

            ASSERT_TRUE(middle->tokens[1]->get_type() == lexer::token::types::WRAPPER);
            auto* inner = static_cast<lexer::token::wrapper*>(middle->tokens[1]);
            ASSERT_TRUE(inner->type == lexer::token::wrapper::types::ROUND_BRACKETS);
            ASSERT_EQ(static_cast<std::size_t>(1), inner->tokens.size());

            ASSERT_TRUE(inner->tokens[0]->get_type() == lexer::token::types::TEXT);
            auto* inner_text = static_cast<lexer::token::text*>(inner->tokens[0]);
            ASSERT_EQ(std::string("baz"), inner_text->data);
        }

        static void test_tokenize_nested_siblings() {
            TokenGuard guard;
            guard.tokens = lexer::tokenize("(outer(inner1)(inner2(inner3)))", 1);

            ASSERT_EQ(static_cast<std::size_t>(1), guard.tokens.size());
            ASSERT_TRUE(guard.tokens[0]->get_type() == lexer::token::types::WRAPPER);

            auto* outer = static_cast<lexer::token::wrapper*>(guard.tokens[0]);
            ASSERT_TRUE(outer->type == lexer::token::wrapper::types::ROUND_BRACKETS);
            ASSERT_EQ(static_cast<std::size_t>(3), outer->tokens.size());

            ASSERT_TRUE(outer->tokens[0]->get_type() == lexer::token::types::TEXT);
            auto* label = static_cast<lexer::token::text*>(outer->tokens[0]);
            ASSERT_EQ(std::string("outer"), label->data);

            ASSERT_TRUE(outer->tokens[1]->get_type() == lexer::token::types::WRAPPER);
            auto* first_child = static_cast<lexer::token::wrapper*>(outer->tokens[1]);
            ASSERT_TRUE(first_child->type == lexer::token::wrapper::types::ROUND_BRACKETS);
            ASSERT_EQ(static_cast<std::size_t>(1), first_child->tokens.size());
            ASSERT_TRUE(first_child->tokens[0]->get_type() == lexer::token::types::TEXT);
            auto* first_inner = static_cast<lexer::token::text*>(first_child->tokens[0]);
            ASSERT_EQ(std::string("inner1"), first_inner->data);

            ASSERT_TRUE(outer->tokens[2]->get_type() == lexer::token::types::WRAPPER);
            auto* second_child = static_cast<lexer::token::wrapper*>(outer->tokens[2]);
            ASSERT_TRUE(second_child->type == lexer::token::wrapper::types::ROUND_BRACKETS);
            ASSERT_EQ(static_cast<std::size_t>(2), second_child->tokens.size());

            ASSERT_TRUE(second_child->tokens[0]->get_type() == lexer::token::types::TEXT);
            auto* second_inner_label = static_cast<lexer::token::text*>(second_child->tokens[0]);
            ASSERT_EQ(std::string("inner2"), second_inner_label->data);

            ASSERT_TRUE(second_child->tokens[1]->get_type() == lexer::token::types::WRAPPER);
            auto* grand_child = static_cast<lexer::token::wrapper*>(second_child->tokens[1]);
            ASSERT_TRUE(grand_child->type == lexer::token::wrapper::types::ROUND_BRACKETS);
            ASSERT_EQ(static_cast<std::size_t>(1), grand_child->tokens.size());
            ASSERT_TRUE(grand_child->tokens[0]->get_type() == lexer::token::types::TEXT);
            auto* deep_text = static_cast<lexer::token::text*>(grand_child->tokens[0]);
            ASSERT_EQ(std::string("inner3"), deep_text->data);
        }

        static void test_tokenize_hex_literal() {
            TokenGuard guard;
            guard.tokens = lexer::tokenize("0xff", 0);

            ASSERT_EQ(static_cast<std::size_t>(1), guard.tokens.size());
            ASSERT_TRUE(guard.tokens[0]->get_type() == lexer::token::types::NUMBER);

            auto* number = static_cast<lexer::token::number*>(guard.tokens[0]);
            ASSERT_EQ(std::string("0xff"), number->text);
            ASSERT_TRUE(number->number_type == lexer::token::number::types::HEX);
        }

        static void test_tokenize_multiple_hex_literals() {
            TokenGuard guard;
            guard.tokens = lexer::tokenize("0xff 0x1a", 4);

            ASSERT_EQ(static_cast<std::size_t>(3), guard.tokens.size());

            ASSERT_TRUE(guard.tokens[0]->get_type() == lexer::token::types::NUMBER);
            auto* first = static_cast<lexer::token::number*>(guard.tokens[0]);
            ASSERT_EQ(std::string("0xff"), first->text);
            ASSERT_TRUE(first->number_type == lexer::token::number::types::HEX);

            ASSERT_TRUE(guard.tokens[1]->get_type() == lexer::token::types::SEPARATOR);
            auto* separator = static_cast<lexer::token::separator*>(guard.tokens[1]);
            ASSERT_TRUE(separator->type == lexer::token::separator::types::SPACE);

            ASSERT_TRUE(guard.tokens[2]->get_type() == lexer::token::types::NUMBER);
            auto* second = static_cast<lexer::token::number*>(guard.tokens[2]);
            ASSERT_EQ(std::string("0x1a"), second->text);
            ASSERT_TRUE(second->number_type == lexer::token::number::types::HEX);
        }

        static void test_tokenize_combined_operators() {
            TokenGuard guard;
            guard.tokens = lexer::tokenize("== != += -= -- ++ -> << >> <= >= && ||", 0);

            const std::vector<std::string> expected_ops = {"==", "!=", "+=", "-=", "--", "++", "->", "<<", ">>", "<=", ">=", "&&", "||"};
            ASSERT_EQ(static_cast<std::size_t>((signed)expected_ops.size() * 2 - 1), guard.tokens.size());

            for (std::size_t i = 0; i < expected_ops.size(); ++i) {
                const std::size_t op_index = i * 2;
                ASSERT_TRUE(guard.tokens[op_index]->get_type() == lexer::token::types::OPERATOR);
                auto* op_token = static_cast<lexer::token::op*>(guard.tokens[op_index]);
                ASSERT_EQ(expected_ops[i], op_token->text);

                if (i + 1 == expected_ops.size()) continue;
                ASSERT_TRUE(guard.tokens[op_index + 1]->get_type() == lexer::token::types::SEPARATOR);
                auto* separator = static_cast<lexer::token::separator*>(guard.tokens[op_index + 1]);
                ASSERT_TRUE(separator->type == lexer::token::separator::types::SPACE);
            }
        }
    };
}

#endif
