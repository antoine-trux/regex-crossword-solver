// Copyright (c) 2016 Antoine Trux
//
// The original version is available at
// http://solving-regular-expression-crosswords.blogspot.com
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// The above copyright notice, the above original version notice, and
// this permission notice shall be included in all copies or substantial
// portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
// CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
// TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
// SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.


#include "disable_warnings_from_gtest.hpp"
#include "regex_crossword_solver_test.hpp"
#include "regex_tokenizer.hpp"
#include "utils.hpp"

using RTT = RegexToken::Type;
using namespace std;


class RegexTokenizerTest : public RegexCrosswordSolverTest
{
};


TEST_F(RegexTokenizerTest, outside_character_class_single_character)
{
    RegexTokenizer tokenizer("A");

    const auto token = tokenizer.consume_token();
    ASSERT_EQ(RTT::SINGLE_CHARACTER, token.type());
    EXPECT_EQ('A', token.character());
}

TEST_F(RegexTokenizerTest, outside_character_class_any_character)
{
    RegexTokenizer tokenizer(".");

    const auto token = tokenizer.consume_token();
    EXPECT_EQ(RTT::ANY_CHARACTER, token.type());
}

TEST_F(RegexTokenizerTest, outside_character_class_caret)
{
    RegexTokenizer tokenizer("^");

    const auto token = tokenizer.consume_token();
    EXPECT_EQ(RTT::EPSILON_AT_START, token.type());
}

TEST_F(RegexTokenizerTest, outside_character_class_dollar)
{
    RegexTokenizer tokenizer("$");

    const auto token = tokenizer.consume_token();
    EXPECT_EQ(RTT::EPSILON_AT_END, token.type());
}

TEST_F(RegexTokenizerTest, outside_character_class_kleene_star)
{
    RegexTokenizer tokenizer("A*");

    tokenizer.consume_token();

    const auto token = tokenizer.consume_token();
    EXPECT_EQ(RTT::KLEENE_STAR_REPETITION, token.type());
}

TEST_F(RegexTokenizerTest, outside_character_class_plus)
{
    RegexTokenizer tokenizer("A+");

    tokenizer.consume_token();

    const auto token = tokenizer.consume_token();
    EXPECT_EQ(RTT::PLUS_REPETITION, token.type());
}

TEST_F(RegexTokenizerTest, outside_character_class_question_mark)
{
    RegexTokenizer tokenizer("A?");

    tokenizer.consume_token();

    const auto token = tokenizer.consume_token();
    EXPECT_EQ(RTT::QUESTION_MARK_REPETITION, token.type());
}

TEST_F(RegexTokenizerTest, outside_character_class_open_brace)
{
    RegexTokenizer tokenizer("A{2}");

    tokenizer.consume_token();

    const auto token = tokenizer.consume_token();
    EXPECT_EQ(RTT::OPEN_COUNTED_REPETITION, token.type());
}

TEST_F(RegexTokenizerTest, outside_character_class_open_bracket)
{
    RegexTokenizer tokenizer("[A]");

    const auto token = tokenizer.consume_token();
    EXPECT_EQ(RTT::OPEN_CHARACTER_CLASS, token.type());
}

TEST_F(RegexTokenizerTest, outside_character_class_or)
{
    RegexTokenizer tokenizer("A|B");

    tokenizer.consume_token();

    const auto token = tokenizer.consume_token();
    EXPECT_EQ(RTT::OR, token.type());
}

TEST_F(RegexTokenizerTest, outside_character_class_open_close_group)
{
    RegexTokenizer tokenizer("(A)");

    const auto token_open = tokenizer.consume_token();
    EXPECT_EQ(RTT::OPEN_GROUP, token_open.type());

    tokenizer.consume_token();

    const auto token_close = tokenizer.consume_token();
    EXPECT_EQ(RTT::CLOSE_GROUP, token_close.type());
}

TEST_F(RegexTokenizerTest, outside_character_class_positive_lookahead)
{
    RegexTokenizer tokenizer("(?=A)");

    const auto token_open = tokenizer.consume_token();
    EXPECT_EQ(RTT::OPEN_POSITIVE_LOOKAHEAD, token_open.type());

    tokenizer.consume_token();

    const auto token_close = tokenizer.consume_token();
    EXPECT_EQ(RTT::CLOSE_GROUP, token_close.type());
}

TEST_F(RegexTokenizerTest, outside_character_class_non_capturing_group)
{
    RegexTokenizer tokenizer("(?:A)");

    const auto token_open = tokenizer.consume_token();
    EXPECT_EQ(RTT::OPEN_NON_CAPTURING_GROUP, token_open.type());

    tokenizer.consume_token();

    const auto token_close = tokenizer.consume_token();
    EXPECT_EQ(RTT::CLOSE_GROUP, token_close.type());
}

TEST_F(RegexTokenizerTest, outside_character_class_unsupported_open_group)
{
    RegexTokenizer tokenizer("(?A)");

    const auto token = tokenizer.consume_token();
    EXPECT_EQ(RTT::INVALID, token.type());
}

TEST_F(RegexTokenizerTest, in_character_class_single_character)
{
    RegexTokenizer tokenizer("[A]");

    tokenizer.consume_token();

    const auto token = tokenizer.consume_token();
    ASSERT_EQ(RTT::SINGLE_CHARACTER, token.type());
    EXPECT_EQ('A', token.character());
}

TEST_F(RegexTokenizerTest, in_character_class_close_bracket_as_character_1)
{
    RegexTokenizer tokenizer("[]A]");

    tokenizer.consume_token();

    const auto token = tokenizer.consume_token();
    ASSERT_EQ(RTT::SINGLE_CHARACTER, token.type());
    EXPECT_EQ(']', token.character());
}

TEST_F(RegexTokenizerTest, in_character_class_close_bracket_as_character_2)
{
    RegexTokenizer tokenizer("[^]A]");

    tokenizer.consume_token();
    tokenizer.consume_token();

    const auto token = tokenizer.consume_token();
    ASSERT_EQ(RTT::SINGLE_CHARACTER, token.type());
    EXPECT_EQ(']', token.character());
}

TEST_F(RegexTokenizerTest,
       in_character_class_close_bracket_as_close_character_class)
{
    RegexTokenizer tokenizer("[A]");

    tokenizer.consume_token();
    tokenizer.consume_token();

    const auto token = tokenizer.consume_token();
    EXPECT_EQ(RTT::CLOSE_CHARACTER_CLASS, token.type());
}

TEST_F(RegexTokenizerTest, in_character_class_caret_as_negation)
{
    RegexTokenizer tokenizer("[^A]");

    tokenizer.consume_token();

    const auto token = tokenizer.consume_token();
    EXPECT_EQ(RTT::NEGATE_CHARACTER_CLASS, token.type());
}

TEST_F(RegexTokenizerTest, in_character_class_caret_as_character)
{
    RegexTokenizer tokenizer("[A^]");

    tokenizer.consume_token();
    tokenizer.consume_token();

    const auto token = tokenizer.consume_token();
    ASSERT_EQ(RTT::SINGLE_CHARACTER, token.type());
    EXPECT_EQ('^', token.character());
}

TEST_F(RegexTokenizerTest, in_character_class_dash_at_beginning_1)
{
    RegexTokenizer tokenizer("[-A]");

    tokenizer.consume_token();

    const auto token = tokenizer.consume_token();
    ASSERT_EQ(RTT::SINGLE_CHARACTER, token.type());
    EXPECT_EQ('-', token.character());
}

TEST_F(RegexTokenizerTest, in_character_class_dash_at_beginning_2)
{
    RegexTokenizer tokenizer("[^-A]");

    tokenizer.consume_token();
    tokenizer.consume_token();

    const auto token = tokenizer.consume_token();
    ASSERT_EQ(RTT::SINGLE_CHARACTER, token.type());
    EXPECT_EQ('-', token.character());
}

TEST_F(RegexTokenizerTest, in_character_class_dash_at_end)
{
    RegexTokenizer tokenizer("[A-]");

    tokenizer.consume_token();
    tokenizer.consume_token();

    const auto token = tokenizer.consume_token();
    ASSERT_EQ(RTT::SINGLE_CHARACTER, token.type());
    EXPECT_EQ('-', token.character());
}

TEST_F(RegexTokenizerTest, in_character_class_dash_as_range_separator)
{
    RegexTokenizer tokenizer("[A-B]");

    tokenizer.consume_token();
    tokenizer.consume_token();

    const auto token = tokenizer.consume_token();
    EXPECT_EQ(RTT::CHARACTER_RANGE_SEPARATOR, token.type());
}

TEST_F(RegexTokenizerTest, in_character_class_dash_after_character_range)
{
    RegexTokenizer tokenizer("[A-B-E-F]");

    tokenizer.consume_token();
    tokenizer.consume_token();
    tokenizer.consume_token();
    tokenizer.consume_token();

    const auto token = tokenizer.consume_token();
    ASSERT_EQ(RTT::SINGLE_CHARACTER, token.type());
    EXPECT_EQ('-', token.character());
}

TEST_F(RegexTokenizerTest, escape_at_end)
{
    RegexTokenizer tokenizer(R"(\)");

    const auto token = tokenizer.consume_token();
    EXPECT_EQ(RTT::INVALID, token.type());
}

TEST_F(RegexTokenizerTest, escape_a)
{
    RegexTokenizer tokenizer(R"(\a)");

    const auto token = tokenizer.consume_token();
    ASSERT_EQ(RTT::SINGLE_CHARACTER, token.type());
    EXPECT_EQ('\a', token.character());
}

TEST_F(RegexTokenizerTest, escape_A)
{
    RegexTokenizer tokenizer(R"(\A)");

    const auto token = tokenizer.consume_token();
    EXPECT_EQ(RTT::EPSILON_AT_START, token.type());
}

TEST_F(RegexTokenizerTest, escape_b_in_character_class)
{
    RegexTokenizer tokenizer(R"([\b])");

    tokenizer.consume_token();

    const auto token = tokenizer.consume_token();
    ASSERT_EQ(RTT::SINGLE_CHARACTER, token.type());
    EXPECT_EQ('\b', token.character());
}

TEST_F(RegexTokenizerTest, escape_b_outside_character_class)
{
    RegexTokenizer tokenizer(R"(\b)");

    const auto token = tokenizer.consume_token();
    EXPECT_EQ(RTT::EPSILON_AT_WORD_BOUNDARY, token.type());
}

TEST_F(RegexTokenizerTest, escape_B)
{
    RegexTokenizer tokenizer(R"(\B)");

    const auto token = tokenizer.consume_token();
    EXPECT_EQ(RTT::EPSILON_NOT_AT_WORD_BOUNDARY, token.type());
}

TEST_F(RegexTokenizerTest, escape_d)
{
    RegexTokenizer tokenizer(R"(\d)");

    const auto token = tokenizer.consume_token();
    EXPECT_EQ(RTT::SHORTHAND_DIGIT_CHARACTER, token.type());
}

TEST_F(RegexTokenizerTest, escape_D)
{
    RegexTokenizer tokenizer(R"(\D)");

    const auto token = tokenizer.consume_token();
    EXPECT_EQ(RTT::SHORTHAND_NOT_DIGIT_CHARACTER, token.type());
}

TEST_F(RegexTokenizerTest, escape_f)
{
    RegexTokenizer tokenizer(R"(\f)");

    const auto token = tokenizer.consume_token();
    ASSERT_EQ(RTT::SINGLE_CHARACTER, token.type());
    EXPECT_EQ('\f', token.character());
}

TEST_F(RegexTokenizerTest, escape_n)
{
    RegexTokenizer tokenizer(R"(\n)");

    const auto token = tokenizer.consume_token();
    ASSERT_EQ(RTT::SINGLE_CHARACTER, token.type());
    EXPECT_EQ('\n', token.character());
}

TEST_F(RegexTokenizerTest, escape_r)
{
    RegexTokenizer tokenizer(R"(\r)");

    const auto token = tokenizer.consume_token();
    ASSERT_EQ(RTT::SINGLE_CHARACTER, token.type());
    EXPECT_EQ('\r', token.character());
}

TEST_F(RegexTokenizerTest, escape_s)
{
    RegexTokenizer tokenizer(R"(\s)");

    const auto token = tokenizer.consume_token();
    EXPECT_EQ(RTT::SHORTHAND_SPACE_CHARACTER, token.type());
}

TEST_F(RegexTokenizerTest, escape_S)
{
    RegexTokenizer tokenizer(R"(\S)");

    const auto token = tokenizer.consume_token();
    EXPECT_EQ(RTT::SHORTHAND_NOT_SPACE_CHARACTER, token.type());
}

TEST_F(RegexTokenizerTest, escape_t)
{
    RegexTokenizer tokenizer(R"(\t)");

    const auto token = tokenizer.consume_token();
    ASSERT_EQ(RTT::SINGLE_CHARACTER, token.type());
    EXPECT_EQ('\t', token.character());
}

TEST_F(RegexTokenizerTest, escape_v)
{
    RegexTokenizer tokenizer(R"(\v)");

    const auto token = tokenizer.consume_token();
    ASSERT_EQ(RTT::SINGLE_CHARACTER, token.type());
    EXPECT_EQ('\v', token.character());
}

TEST_F(RegexTokenizerTest, escape_w)
{
    RegexTokenizer tokenizer(R"(\w)");

    const auto token = tokenizer.consume_token();
    EXPECT_EQ(RTT::SHORTHAND_WORD_CHARACTER, token.type());
}

TEST_F(RegexTokenizerTest, escape_W)
{
    RegexTokenizer tokenizer(R"(\W)");

    const auto token = tokenizer.consume_token();
    EXPECT_EQ(RTT::SHORTHAND_NOT_WORD_CHARACTER, token.type());
}

TEST_F(RegexTokenizerTest, escape_Z)
{
    RegexTokenizer tokenizer(R"(\Z)");

    const auto token = tokenizer.consume_token();
    EXPECT_EQ(RTT::EPSILON_AT_END, token.type());
}

TEST_F(RegexTokenizerTest, escape_non_special_ascii_letter)
{
    RegexTokenizer tokenizer(R"(\c)");

    const auto token = tokenizer.consume_token();
    EXPECT_EQ(RTT::INVALID, token.type());
}

TEST_F(RegexTokenizerTest, escape_non_ascii_letter_non_digit)
{
    RegexTokenizer tokenizer(R"(\!)");

    const auto token = tokenizer.consume_token();
    ASSERT_EQ(RTT::SINGLE_CHARACTER, token.type());
    EXPECT_EQ('!', token.character());
}

TEST_F(RegexTokenizerTest, in_counted_repetition_close_brace_1)
{
    RegexTokenizer tokenizer("A{1}");

    tokenizer.consume_token();
    tokenizer.consume_token();
    tokenizer.consume_token();

    const auto token = tokenizer.consume_token();
    EXPECT_EQ(RTT::CLOSE_COUNTED_REPETITION, token.type());
}

TEST_F(RegexTokenizerTest, in_counted_repetition_close_brace_2)
{
    RegexTokenizer tokenizer("A{1,}");

    tokenizer.consume_token();
    tokenizer.consume_token();
    tokenizer.consume_token();
    tokenizer.consume_token();

    const auto token = tokenizer.consume_token();
    EXPECT_EQ(RTT::CLOSE_COUNTED_REPETITION, token.type());
}

TEST_F(RegexTokenizerTest, in_counted_repetition_close_brace_3)
{
    RegexTokenizer tokenizer("A{1,2}");

    tokenizer.consume_token();
    tokenizer.consume_token();
    tokenizer.consume_token();
    tokenizer.consume_token();
    tokenizer.consume_token();

    const auto token = tokenizer.consume_token();
    EXPECT_EQ(RTT::CLOSE_COUNTED_REPETITION, token.type());
}

TEST_F(RegexTokenizerTest, in_counted_repetition_comma)
{
    RegexTokenizer tokenizer("A{1,2}");

    tokenizer.consume_token();
    tokenizer.consume_token();
    tokenizer.consume_token();

    const auto token = tokenizer.consume_token();
    EXPECT_EQ(RTT::REPETITION_COUNT_SEPARATOR, token.type());
}

TEST_F(RegexTokenizerTest, in_counted_repetition_digits)
{
    RegexTokenizer tokenizer("A{10}");

    tokenizer.consume_token();
    tokenizer.consume_token();

    const auto token = tokenizer.consume_token();
    ASSERT_EQ(RTT::REPETITION_COUNT, token.type());
    EXPECT_EQ(10, token.repetition_count());
}

TEST_F(RegexTokenizerTest, in_counted_repetition_invalid_1)
{
    RegexTokenizer tokenizer("A{x}");

    tokenizer.consume_token();
    tokenizer.consume_token();

    const auto token = tokenizer.consume_token();
    EXPECT_EQ(RTT::INVALID, token.type());
}

TEST_F(RegexTokenizerTest, in_counted_repetition_invalid_2)
{
    RegexTokenizer tokenizer("A{X}");

    tokenizer.consume_token();
    tokenizer.consume_token();

    const auto token = tokenizer.consume_token();
    EXPECT_EQ(RTT::INVALID, token.type());
}

TEST_F(RegexTokenizerTest, escape_digit_outside_character_class_non_octal)
{
    RegexTokenizer tokenizer(R"(\8)");

    const auto token = tokenizer.consume_token();
    ASSERT_EQ(RTT::BACKREFERENCE, token.type());
    EXPECT_EQ(GroupNumber(8), token.group_number());
}

TEST_F(RegexTokenizerTest, escape_digit_outside_character_class_zero)
{
    RegexTokenizer tokenizer(R"(\0)");

    const auto token = tokenizer.consume_token();
    ASSERT_EQ(RTT::SINGLE_CHARACTER, token.type());
    EXPECT_EQ('\0', token.character());
}

TEST_F(RegexTokenizerTest,
       escape_digit_outside_character_class_zero_and_1_octal)
{
    RegexTokenizer tokenizer(R"(\07)");

    const auto token = tokenizer.consume_token();
    ASSERT_EQ(RTT::SINGLE_CHARACTER, token.type());
    EXPECT_EQ('\a', token.character());
}

TEST_F(RegexTokenizerTest,
       escape_digit_outside_character_class_zero_and_2_octal)
{
    RegexTokenizer tokenizer(R"(\041)");

    const auto token = tokenizer.consume_token();
    ASSERT_EQ(RTT::SINGLE_CHARACTER, token.type());
    EXPECT_EQ('!', token.character());
}

TEST_F(RegexTokenizerTest, escape_digit_outside_character_class_3_octal)
{
    RegexTokenizer tokenizer(R"(\141)");

    const auto token = tokenizer.consume_token();
    ASSERT_EQ(RTT::SINGLE_CHARACTER, token.type());
    EXPECT_EQ('a', token.character());
}

TEST_F(RegexTokenizerTest,
       escape_digit_outside_character_class_3_octal_too_large)
{
    RegexTokenizer tokenizer(R"(\400)");

    const auto token = tokenizer.consume_token();
    EXPECT_EQ(RTT::INVALID, token.type());
}

TEST_F(RegexTokenizerTest,
       escape_digit_outside_character_class_octal_backreference)
{
    RegexTokenizer tokenizer(R"(\48)");

    const auto token = tokenizer.consume_token();
    ASSERT_EQ(RTT::BACKREFERENCE, token.type());
    EXPECT_EQ(GroupNumber(4), token.group_number());
}

TEST_F(RegexTokenizerTest, escape_digit_in_character_class_non_octal)
{
    RegexTokenizer tokenizer(R"([\8])");

    tokenizer.consume_token();

    const auto token = tokenizer.consume_token();
    EXPECT_EQ(RTT::INVALID, token.type());
}

TEST_F(RegexTokenizerTest, escape_digit_in_character_class_1_octal)
{
    RegexTokenizer tokenizer(R"([\7])");

    tokenizer.consume_token();

    const auto token = tokenizer.consume_token();
    ASSERT_EQ(RTT::SINGLE_CHARACTER, token.type());
    EXPECT_EQ('\a', token.character());
}

TEST_F(RegexTokenizerTest, escape_digit_in_character_class_2_octal)
{
    RegexTokenizer tokenizer(R"([\41])");

    tokenizer.consume_token();

    const auto token = tokenizer.consume_token();
    ASSERT_EQ(RTT::SINGLE_CHARACTER, token.type());
    EXPECT_EQ('!', token.character());
}

TEST_F(RegexTokenizerTest, escape_digit_in_character_class_3_octal)
{
    RegexTokenizer tokenizer(R"([\141])");

    tokenizer.consume_token();

    const auto token = tokenizer.consume_token();
    ASSERT_EQ(RTT::SINGLE_CHARACTER, token.type());
    EXPECT_EQ('a', token.character());
}

TEST_F(RegexTokenizerTest, escape_digit_in_character_class_3_octal_too_large)
{
    RegexTokenizer tokenizer(R"([\400])");

    tokenizer.consume_token();

    const auto token = tokenizer.consume_token();
    EXPECT_EQ(RTT::INVALID, token.type());
}

TEST_F(RegexTokenizerTest, escape_32_bit_unicode)
{
    RegexTokenizer tokenizer(R"(\u1234)");

    const auto token = tokenizer.consume_token();
    EXPECT_EQ(RTT::INVALID, token.type());
}

TEST_F(RegexTokenizerTest, escape_32_bit_unicode_not_enough_digits_1)
{
    RegexTokenizer tokenizer(R"(\u123)");

    const auto token = tokenizer.consume_token();
    EXPECT_EQ(RTT::INVALID, token.type());
}

TEST_F(RegexTokenizerTest, escape_32_bit_unicode_not_enough_digits_2)
{
    RegexTokenizer tokenizer(R"(\u123G)");

    const auto token = tokenizer.consume_token();
    EXPECT_EQ(RTT::INVALID, token.type());
}

TEST_F(RegexTokenizerTest, escape_64_bit_unicode)
{
    RegexTokenizer tokenizer(R"(\U12345678)");

    const auto token = tokenizer.consume_token();
    EXPECT_EQ(RTT::INVALID, token.type());
}

TEST_F(RegexTokenizerTest, escape_64_bit_unicode_not_enough_digits_1)
{
    RegexTokenizer tokenizer(R"(\U1234567)");

    const auto token = tokenizer.consume_token();
    EXPECT_EQ(RTT::INVALID, token.type());
}

TEST_F(RegexTokenizerTest, escape_64_bit_unicode_not_enough_digits_2)
{
    RegexTokenizer tokenizer(R"(\U1234567G)");

    const auto token = tokenizer.consume_token();
    EXPECT_EQ(RTT::INVALID, token.type());
}

TEST_F(RegexTokenizerTest, escape_hex_char)
{
    RegexTokenizer tokenizer(R"(\x61)");

    const auto token = tokenizer.consume_token();
    ASSERT_EQ(RTT::SINGLE_CHARACTER, token.type());
    EXPECT_EQ('a', token.character());
}

TEST_F(RegexTokenizerTest, escape_hex_char_not_enough_digits_1)
{
    RegexTokenizer tokenizer(R"(\x6)");

    const auto token = tokenizer.consume_token();
    EXPECT_EQ(RTT::INVALID, token.type());
}

TEST_F(RegexTokenizerTest, escape_hex_char_not_enough_digits_2)
{
    RegexTokenizer tokenizer(R"(\x6G)");

    const auto token = tokenizer.consume_token();
    EXPECT_EQ(RTT::INVALID, token.type());
}

TEST_F(RegexTokenizerTest, end)
{
    RegexTokenizer tokenizer(".");

    EXPECT_EQ(RTT::ANY_CHARACTER, tokenizer.consume_token().type());
    EXPECT_EQ(RTT::END, tokenizer.consume_token().type());
    EXPECT_EQ(RTT::END, tokenizer.consume_token().type());
}

TEST_F(RegexTokenizerTest, peek_token)
{
    RegexTokenizer tokenizer(".");

    EXPECT_EQ(RTT::ANY_CHARACTER, tokenizer.peek_token().type());
    EXPECT_EQ(RTT::ANY_CHARACTER, tokenizer.consume_token().type());

    EXPECT_EQ(RTT::END, tokenizer.peek_token().type());
    EXPECT_EQ(RTT::END, tokenizer.consume_token().type());

    EXPECT_EQ(RTT::END, tokenizer.peek_token().type());
    EXPECT_EQ(RTT::END, tokenizer.consume_token().type());
}

TEST_F(RegexTokenizerTest, push_back_token)
{
    RegexTokenizer tokenizer(".*");

    const auto token_1 = tokenizer.consume_token();
    EXPECT_EQ(RTT::ANY_CHARACTER, token_1.type());

    tokenizer.push_back_token(token_1);

    const auto token_2 = tokenizer.consume_token();
    EXPECT_EQ(RTT::ANY_CHARACTER, token_2.type());

    const auto token_3 = tokenizer.consume_token();
    EXPECT_EQ(RTT::KLEENE_STAR_REPETITION, token_3.type());

    EXPECT_EQ(RTT::END, tokenizer.consume_token().type());
    tokenizer.push_back_token(token_3);
    tokenizer.push_back_token(token_2);

    EXPECT_EQ(RTT::ANY_CHARACTER, tokenizer.consume_token().type());
    EXPECT_EQ(RTT::KLEENE_STAR_REPETITION, tokenizer.consume_token().type());
    EXPECT_EQ(RTT::END, tokenizer.consume_token().type());
}

TEST_F(RegexTokenizerTest, push_back_tokens)
{
    RegexTokenizer tokenizer(".*");

    const auto token_1 = tokenizer.consume_token();
    EXPECT_EQ(RTT::ANY_CHARACTER, token_1.type());

    const auto token_2 = tokenizer.consume_token();
    EXPECT_EQ(RTT::KLEENE_STAR_REPETITION, token_2.type());

    EXPECT_EQ(RTT::END, tokenizer.consume_token().type());

    tokenizer.push_back_tokens({ token_1, token_2 });

    EXPECT_EQ(RTT::ANY_CHARACTER, tokenizer.consume_token().type());
    EXPECT_EQ(RTT::KLEENE_STAR_REPETITION, tokenizer.consume_token().type());
    EXPECT_EQ(RTT::END, tokenizer.consume_token().type());
}
