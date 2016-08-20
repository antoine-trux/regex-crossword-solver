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


#include "alphabet.hpp"
#include "character_block.hpp"
#include "disable_warnings_from_gtest.hpp"
#include "regex_crossword_solver_test.hpp"
#include "set_of_characters.hpp"

using namespace std;


class CharacterBlockTest : public RegexCrosswordSolverTest
{
};


TEST_F(CharacterBlockTest, newline_not_in_any_character)
{
    Alphabet::set("ABC\n");
    const auto any_character = AnyCharacter();
    EXPECT_EQ("ABC", any_character.characters().to_string());
}

TEST_F(CharacterBlockTest, shorthand_character_to_string)
{
    using RTT = RegexToken::Type;

    {
        const ShorthandCharacter c(RTT::SHORTHAND_DIGIT_CHARACTER);
        EXPECT_EQ("\\d", c.to_string());
    }

    {
        const ShorthandCharacter c(RTT::SHORTHAND_NOT_DIGIT_CHARACTER);
        EXPECT_EQ("\\D", c.to_string());
    }

    {
        const ShorthandCharacter c(RTT::SHORTHAND_NOT_SPACE_CHARACTER);
        EXPECT_EQ("\\S", c.to_string());
    }

    {
        const ShorthandCharacter c(RTT::SHORTHAND_NOT_WORD_CHARACTER);
        EXPECT_EQ("\\W", c.to_string());
    }

    {
        const ShorthandCharacter c(RTT::SHORTHAND_SPACE_CHARACTER);
        EXPECT_EQ("\\s", c.to_string());
    }

    {
        const ShorthandCharacter c(RTT::SHORTHAND_WORD_CHARACTER);
        EXPECT_EQ("\\w", c.to_string());
    }
}
