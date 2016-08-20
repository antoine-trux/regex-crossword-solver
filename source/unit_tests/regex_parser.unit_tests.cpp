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
#include "regex.hpp"
#include "regex_crossword_solver_exception.hpp"
#include "regex_crossword_solver_test.hpp"
#include "regex_parser.hpp"

using namespace std;


class RegexParserTest : public RegexCrosswordSolverTest
{
};


TEST_F(RegexParserTest, parse)
{
    const vector<string> regexes_as_string = { "",
                                               "A",
                                               ".",
                                               R"((A)\1)",
                                               "[A]",
                                               "[AB]",
                                               "()",
                                               "(A)",
                                               "([A])",
                                               "(AB)",
                                               "(A|B)",
                                               "(A*)",
                                               "(A+)",
                                               "(A?)",
                                               "((A))",
                                               "A*",
                                               "A+",
                                               "A?",
                                               "AB?",
                                               "AB",
                                               "ABC",
                                               "|",
                                               "||",
                                               "|A",
                                               "A|",
                                               "A|B",
                                               "A{1}",
                                               "A{1,}",
                                               "A{1,2}",
                                               "[^ABC]",
                                               R"(\d)",
                                               R"([\d])",
                                               R"(A\s[B\s])",
                                               "[A-Z]" };

    for (const auto& regex_as_string : regexes_as_string)
    {
        auto regex = Regex::parse(regex_as_string);
        EXPECT_EQ(regex_as_string, regex->to_string());
    }
}

TEST_F(RegexParserTest, parse_throws_backreference_has_invalid_group_number)
{
    EXPECT_THROW(Regex::parse(R"((A)\2)"), RegexParseException);
}

TEST_F(RegexParserTest, parse_throws_character_class_without_closing_bracket)
{
    EXPECT_THROW(Regex::parse("[A"), RegexParseException);
}

TEST_F(RegexParserTest, parse_throws_invalid_character_class)
{
    EXPECT_THROW(Regex::parse(R"([^\B)"), RegexParseException);
}

TEST_F(RegexParserTest, parse_throws_invalid_token)
{
    EXPECT_THROW(Regex::parse(R"(\)"), RegexParseException);
}

TEST_F(RegexParserTest, parse_throws_extra_input)
{
    EXPECT_THROW(Regex::parse("A)"), RegexParseException);
}

TEST_F(RegexParserTest, parse_throws_invalid_range_1)
{
    EXPECT_THROW(Regex::parse(R"([A-\d])"), RegexParseException);
}

TEST_F(RegexParserTest, parse_throws_invalid_range_2)
{
    EXPECT_THROW(Regex::parse("[Z-A]"), RegexParseException);
}

TEST_F(RegexParserTest, parse_throws_group_without_closing_parenthesis)
{
    EXPECT_THROW(Regex::parse("(A"), RegexParseException);
}

TEST_F(RegexParserTest, parse_throws_invalid_right_side_of_concatenation)
{
    EXPECT_THROW(Regex::parse("AB("), RegexParseException);
}

TEST_F(RegexParserTest, parse_throws_repetition_min_greater_than_max)
{
    EXPECT_THROW(Regex::parse("A{2,1}"), RegexParseException);
}

TEST_F(RegexParserTest, parse_throws_repetition_without_closing_brace)
{
    EXPECT_THROW(Regex::parse("A{1,2"), RegexParseException);
}

TEST_F(RegexParserTest, parse_throws_invalid_char_after_min)
{
    EXPECT_THROW(Regex::parse("A{1x,2"), RegexParseException);
}

TEST_F(RegexParserTest, parse_throws_invalid_repetition_count)
{
    EXPECT_THROW(Regex::parse("A{,1"), RegexParseException);
}

TEST_F(RegexParserTest, consume_and_check_token_throws)
{
    RegexParser parser(R"(\)");
    EXPECT_THROW(parser.consume_and_check_token(), RegexParseException);
}

TEST_F(RegexParserTest, check_no_self_references)
{
    EXPECT_THROW(Regex::parse(R"((\1))"), RegexStructureException);
}
