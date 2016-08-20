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
#include "regex_crossword_solver_test.hpp"
#include "set_of_characters.hpp"

using namespace std;


class SetOfCharactersTest : public RegexCrosswordSolverTest
{
};


TEST_F(SetOfCharactersTest, operator_equal)
{
    Alphabet::set("ABCD");

    const SetOfCharacters characters_1("ABC");
    const SetOfCharacters characters_2("ABC");
    const SetOfCharacters characters_3("ACD");

    EXPECT_TRUE(characters_1 == characters_2);
    EXPECT_FALSE(characters_1 == characters_3);

    EXPECT_FALSE(characters_1 != characters_2);
    EXPECT_TRUE(characters_1 != characters_3);
}

TEST_F(SetOfCharactersTest, operator_less_than)
{
    Alphabet::set("ABCD");

    const SetOfCharacters characters_1("AB");
    const SetOfCharacters characters_2("ABC");

    EXPECT_TRUE(characters_1 < characters_2);
    EXPECT_FALSE(characters_2 < characters_1);
    EXPECT_FALSE(characters_1 < characters_1);
}

TEST_F(SetOfCharactersTest, operator_and_1)
{
    Alphabet::set("ABCD");

    EXPECT_EQ(SetOfCharacters("BC"),
              SetOfCharacters("ABC") & SetOfCharacters("BCD"));
}

TEST_F(SetOfCharactersTest, operator_and_2)
{
    Alphabet::set("ABCD");

    EXPECT_EQ(SetOfCharacters("B"), SetOfCharacters("ABC") & 'B');
    EXPECT_EQ(SetOfCharacters(), SetOfCharacters("AC") & 'B');
}

TEST_F(SetOfCharactersTest, operator_or)
{
    Alphabet::set("ABCD");

    EXPECT_EQ(SetOfCharacters("ABD"),
              SetOfCharacters("AB") | SetOfCharacters("BD"));
}

TEST_F(SetOfCharactersTest, operator_not)
{
    Alphabet::set("ABCD");

    EXPECT_EQ(SetOfCharacters("BD"), ~SetOfCharacters("AC"));
}

TEST_F(SetOfCharactersTest, operator_minus_set)
{
    Alphabet::set("ABCDE");

    EXPECT_EQ(SetOfCharacters("AE"),
              SetOfCharacters("ACE") - SetOfCharacters("BCD"));
}

TEST_F(SetOfCharactersTest, operator_minus_char)
{
    Alphabet::set("ABC");

    SetOfCharacters characters("AB");
    EXPECT_EQ(SetOfCharacters("AB"), characters);

    characters -= 'A';
    EXPECT_EQ(SetOfCharacters("B"), characters);

    characters -= 'B';
    EXPECT_TRUE(characters.empty());
    EXPECT_FALSE(characters.not_empty());
}

TEST_F(SetOfCharactersTest, no_characters)
{
    Alphabet::set("A");

    const SetOfCharacters characters("");
    EXPECT_EQ(0, characters.size());
    EXPECT_TRUE(characters.empty());
    EXPECT_FALSE(characters.not_empty());
    auto iter = begin(characters);
    EXPECT_TRUE(iter == end(characters));
}

TEST_F(SetOfCharactersTest, all_characters)
{
    Alphabet::set("ABCD");

    const auto characters = Alphabet::characters();
    EXPECT_FALSE(characters.empty());
    EXPECT_TRUE(characters.not_empty());

    size_t num_characters = 0;

    for (auto c : characters)
    {
        EXPECT_EQ(Alphabet::character_at(num_characters), c);
        ++num_characters;
    }

    EXPECT_EQ(Alphabet::characters().size(), num_characters);
}

TEST_F(SetOfCharactersTest, one_character)
{
    Alphabet::set("AB");

    const SetOfCharacters characters('A');
    EXPECT_EQ(1, characters.size());
    EXPECT_FALSE(characters.empty());
    EXPECT_TRUE(characters.not_empty());
    EXPECT_TRUE(characters.contains('A'));

    auto iter = begin(characters);
    EXPECT_EQ('A', *iter);

    ++iter;
    EXPECT_TRUE(iter == end(characters));
}

TEST_F(SetOfCharactersTest, two_characters)
{
    Alphabet::set("ABC");

    const SetOfCharacters characters("AB");
    EXPECT_EQ(2, characters.size());
    EXPECT_FALSE(characters.empty());
    EXPECT_TRUE(characters.not_empty());
    EXPECT_TRUE(characters.contains('A'));
    EXPECT_TRUE(characters.contains('B'));

    auto iter = begin(characters);
    EXPECT_EQ('A', *iter);

    ++iter;
    EXPECT_EQ('B', *iter);

    ++iter;
    EXPECT_TRUE(iter == end(characters));
}

TEST_F(SetOfCharactersTest, clear)
{
    Alphabet::set("ABCD");

    SetOfCharacters characters = Alphabet::characters();
    characters.clear();
    EXPECT_TRUE(characters.empty());
    EXPECT_FALSE(characters.not_empty());
}

TEST_F(SetOfCharactersTest, contains_set_of_characters)
{
    Alphabet::set("ABC");

    const SetOfCharacters empty;
    const SetOfCharacters a('A');
    const SetOfCharacters b('B');
    const SetOfCharacters ab("AB");

    EXPECT_TRUE(empty.contains(empty));
    EXPECT_FALSE(empty.contains(a));
    EXPECT_FALSE(empty.contains(b));
    EXPECT_FALSE(empty.contains(ab));

    EXPECT_TRUE(a.contains(empty));
    EXPECT_TRUE(a.contains(a));
    EXPECT_FALSE(a.contains(b));
    EXPECT_FALSE(a.contains(ab));

    EXPECT_TRUE(b.contains(empty));
    EXPECT_FALSE(b.contains(a));
    EXPECT_TRUE(b.contains(b));
    EXPECT_FALSE(b.contains(ab));

    EXPECT_TRUE(ab.contains(empty));
    EXPECT_TRUE(ab.contains(a));
    EXPECT_TRUE(ab.contains(b));
    EXPECT_TRUE(ab.contains(ab));
}

TEST_F(SetOfCharactersTest, to_string)
{
    Alphabet::set("ADE");

    EXPECT_EQ("",    SetOfCharacters().to_string());
    EXPECT_EQ("A",   SetOfCharacters('A').to_string());
    EXPECT_EQ("D",   SetOfCharacters('D').to_string());
    EXPECT_EQ("E",   SetOfCharacters('E').to_string());
    EXPECT_EQ("AD",  SetOfCharacters("AD").to_string());
    EXPECT_EQ("AE",  SetOfCharacters("AE").to_string());
    EXPECT_EQ("DE",  SetOfCharacters("DE").to_string());
    EXPECT_EQ("ADE", SetOfCharacters("ADE").to_string());
}

TEST_F(SetOfCharactersTest, word_characters)
{
    Alphabet::set("ABC_=/&");

    const auto empty = SetOfCharacters();
    EXPECT_FALSE(empty.has_only_word_characters());
    EXPECT_FALSE(empty.has_only_non_word_characters());

    auto characters = SetOfCharacters("AB_=/");
    EXPECT_FALSE(characters.has_only_word_characters());
    characters.remove_non_word_characters();
    EXPECT_TRUE(characters.has_only_word_characters());
    EXPECT_FALSE(characters.has_only_non_word_characters());

    characters = SetOfCharacters("AB_=/");
    EXPECT_FALSE(characters.has_only_non_word_characters());
    characters.remove_word_characters();
    EXPECT_FALSE(characters.has_only_word_characters());
    EXPECT_TRUE(characters.has_only_non_word_characters());
}
