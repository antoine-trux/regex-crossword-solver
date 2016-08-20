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
#include "regex_token.hpp"

using RTT = RegexToken::Type;
using namespace std;


class RegexTokenTest : public RegexCrosswordSolverTest
{
};


namespace
{

RegexToken create_fake_invalid_token()
{
    return RegexToken::create_invalid_token("");
}

} // unnamed namespace


TEST_F(RegexTokenTest, is_shorthand_character)
{
    {
        const auto type = RTT::ANY_CHARACTER;
        const auto token = RegexToken::create_token(type);
        EXPECT_FALSE(token.is_shorthand_character());
    }

    {
        const auto type = RTT::SHORTHAND_DIGIT_CHARACTER;
        const auto token = RegexToken::create_token(type);
        EXPECT_TRUE(token.is_shorthand_character());
    }

    {
        const auto type = RTT::SHORTHAND_NOT_DIGIT_CHARACTER;
        const auto token = RegexToken::create_token(type);
        EXPECT_TRUE(token.is_shorthand_character());
    }

    {
        const auto type = RTT::SHORTHAND_NOT_SPACE_CHARACTER;
        const auto token = RegexToken::create_token(type);
        EXPECT_TRUE(token.is_shorthand_character());
    }

    {
        const auto type = RTT::SHORTHAND_NOT_WORD_CHARACTER;
        const auto token = RegexToken::create_token(type);
        EXPECT_TRUE(token.is_shorthand_character());
    }

    {
        const auto type = RTT::SHORTHAND_SPACE_CHARACTER;
        const auto token = RegexToken::create_token(type);
        EXPECT_TRUE(token.is_shorthand_character());
    }

    {
        const auto type = RTT::SHORTHAND_WORD_CHARACTER;
        const auto token = RegexToken::create_token(type);
        EXPECT_TRUE(token.is_shorthand_character());
    }
}

TEST_F(RegexTokenTest, is_negated_shorthand_character)
{
    {
        const auto type = RTT::ANY_CHARACTER;
        EXPECT_FALSE(RegexToken::is_negated_shorthand_character(type));
    }

    {
        const auto type = RTT::SHORTHAND_DIGIT_CHARACTER;
        EXPECT_FALSE(RegexToken::is_negated_shorthand_character(type));
    }

    {
        const auto type = RTT::SHORTHAND_NOT_DIGIT_CHARACTER;
        EXPECT_TRUE(RegexToken::is_negated_shorthand_character(type));
    }

    {
        const auto type = RTT::SHORTHAND_NOT_SPACE_CHARACTER;
        EXPECT_TRUE(RegexToken::is_negated_shorthand_character(type));
    }

    {
        const auto type = RTT::SHORTHAND_NOT_WORD_CHARACTER;
        EXPECT_TRUE(RegexToken::is_negated_shorthand_character(type));
    }

    {
        const auto type = RTT::SHORTHAND_SPACE_CHARACTER;
        EXPECT_FALSE(RegexToken::is_negated_shorthand_character(type));
    }

    {
        const auto type = RTT::SHORTHAND_WORD_CHARACTER;
        EXPECT_FALSE(RegexToken::is_negated_shorthand_character(type));
    }
}

TEST_F(RegexTokenTest, generic_token)
{
    const auto token = RegexToken::create_token(RTT::ANY_CHARACTER);
    EXPECT_EQ(RTT::ANY_CHARACTER, token.type());

    const auto token_copy = token;
    EXPECT_EQ(RTT::ANY_CHARACTER, token_copy.type());

    auto token_assigned = create_fake_invalid_token();
    EXPECT_EQ(RTT::INVALID, token_assigned.type());

    token_assigned = token;
    EXPECT_EQ(RTT::ANY_CHARACTER, token_assigned.type());
}

TEST_F(RegexTokenTest, backreference_token)
{
    const auto group_number = GroupNumber(2);

    const auto token = RegexToken::create_backreference_token(group_number);
    ASSERT_EQ(RTT::BACKREFERENCE, token.type());
    EXPECT_EQ(group_number, token.group_number());

    const auto token_copy = token;
    ASSERT_EQ(RTT::BACKREFERENCE, token_copy.type());
    EXPECT_EQ(group_number, token_copy.group_number());

    auto token_assigned = create_fake_invalid_token();
    EXPECT_EQ(RTT::INVALID, token_assigned.type());

    token_assigned = token;
    ASSERT_EQ(RTT::BACKREFERENCE, token_assigned.type());
    EXPECT_EQ(group_number, token_assigned.group_number());
}

TEST_F(RegexTokenTest, single_character_token)
{
    const auto c = 'A';

    const auto token = RegexToken::create_single_character_token(c);
    ASSERT_EQ(RTT::SINGLE_CHARACTER, token.type());
    EXPECT_EQ(c, token.character());

    const auto token_copy = token;
    ASSERT_EQ(RTT::SINGLE_CHARACTER, token_copy.type());
    EXPECT_EQ(c, token_copy.character());

    auto token_assigned = create_fake_invalid_token();
    EXPECT_EQ(RTT::INVALID, token_assigned.type());

    token_assigned = token;
    ASSERT_EQ(RTT::SINGLE_CHARACTER, token_assigned.type());
    EXPECT_EQ(c, token_assigned.character());
}

TEST_F(RegexTokenTest, invalid_token)
{
    const string error_message = "error message";

    const auto token = RegexToken::create_invalid_token(error_message);
    ASSERT_EQ(RTT::INVALID, token.type());
    EXPECT_EQ(error_message, token.error_message());

    const auto token_copy = token;
    ASSERT_EQ(RTT::INVALID, token_copy.type());
    EXPECT_EQ(error_message, token_copy.error_message());

    auto token_assigned = create_fake_invalid_token();
    EXPECT_EQ(RTT::INVALID, token_assigned.type());

    token_assigned = token;
    ASSERT_EQ(RTT::INVALID, token_assigned.type());
    EXPECT_EQ(error_message, token_assigned.error_message());
}

TEST_F(RegexTokenTest, repetition_count_token)
{
    const auto repetition_count = RepetitionCount(2);

    const auto token =
        RegexToken::create_repetition_count_token(repetition_count);
    ASSERT_EQ(RTT::REPETITION_COUNT, token.type());
    EXPECT_EQ(repetition_count, token.repetition_count());

    const auto token_copy = token;
    ASSERT_EQ(RTT::REPETITION_COUNT, token_copy.type());
    EXPECT_EQ(repetition_count, token_copy.repetition_count());

    auto token_assigned = create_fake_invalid_token();
    EXPECT_EQ(RTT::INVALID, token_assigned.type());

    token_assigned = token;
    ASSERT_EQ(RTT::REPETITION_COUNT, token_assigned.type());
    EXPECT_EQ(repetition_count, token_assigned.repetition_count());
}

TEST_F(RegexTokenTest, self_assignment)
{
    auto token = RegexToken::create_token(RTT::ANY_CHARACTER);
    EXPECT_EQ(RTT::ANY_CHARACTER, token.type());

    token = token;
    EXPECT_EQ(RTT::ANY_CHARACTER, token.type());
}
