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
#include "disable_warnings_from_gtest.hpp"
#include "regex_crossword_solver_exception.hpp"
#include "regex_crossword_solver_test.hpp"
#include "set_of_characters.hpp"

using namespace std;


class AlphabetTest : public RegexCrosswordSolverTest
{
};


TEST_F(AlphabetTest, character_at)
{
    Alphabet::set("ZA");
    EXPECT_EQ('A', Alphabet::character_at(0));
    EXPECT_EQ('Z', Alphabet::character_at(1));
}

TEST_F(AlphabetTest, characters)
{
    Alphabet::set("BA");
    const auto characters = Alphabet::characters();
    EXPECT_EQ(SetOfCharacters("AB"), characters);
}

TEST_F(AlphabetTest, characters_as_string)
{
    Alphabet::set("ACB");
    EXPECT_EQ("ABC", Alphabet::characters_as_string());
}

TEST_F(AlphabetTest, complement_1)
{
    Alphabet::set("ABC");
    const auto remaining_characters = Alphabet::complement("AB");
    EXPECT_EQ(SetOfCharacters('C'), remaining_characters);
}

TEST_F(AlphabetTest, complement_2)
{
    Alphabet::set("ABC");
    const auto remaining_characters = Alphabet::complement("");
    EXPECT_EQ(SetOfCharacters("ABC"), remaining_characters);
}

TEST_F(AlphabetTest, complement_3)
{
    Alphabet::set("ABC");
    const auto remaining_characters = Alphabet::complement("ABC");
    EXPECT_TRUE(remaining_characters.empty());
}

TEST_F(AlphabetTest, complement_4)
{
    Alphabet::set("ABCDE");
    EXPECT_EQ(SetOfCharacters("ACE"), Alphabet::complement("BD"));
}

TEST_F(AlphabetTest, has_character)
{
    Alphabet::set("AB");
    EXPECT_TRUE(Alphabet::has_character('A'));
    EXPECT_TRUE(Alphabet::has_character('B'));
    EXPECT_FALSE(Alphabet::has_character('C'));
}

TEST_F(AlphabetTest, set_to_empty)
{
    EXPECT_THROW(Alphabet::set(""), AlphabetException);
}

TEST_F(AlphabetTest, set_to_too_large)
{
    string characters;

    for (size_t i = 0; i != Alphabet::capacity() + 1; ++i)
    {
        characters += static_cast<char>(i);
    }

    EXPECT_THROW(Alphabet::set(characters), AlphabetException);
}
