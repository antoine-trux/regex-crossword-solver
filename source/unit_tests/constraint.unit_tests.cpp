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
#include "constraint.hpp"
#include "disable_warnings_from_gtest.hpp"
#include "regex_crossword_solver_test.hpp"

using namespace std;


class ConstraintTest : public RegexCrosswordSolverTest
{
};


TEST_F(ConstraintTest, operator_equality_and_inequality)
{
    Alphabet::set("ABC");

    Constraint constraint_1({ "A", "BC" });
    Constraint constraint_2({ "A", "BC" });
    Constraint constraint_3({ "A", "BC", "" });
    Constraint constraint_4({ "A", "B" });

    EXPECT_TRUE(constraint_1 == constraint_2);
    EXPECT_FALSE(constraint_1 != constraint_2);

    EXPECT_FALSE(constraint_1 == constraint_3);
    EXPECT_TRUE(constraint_1 != constraint_3);

    EXPECT_FALSE(constraint_1 == constraint_4);
    EXPECT_TRUE(constraint_1 != constraint_4);
}

TEST_F(ConstraintTest, constructor_from_vector)
{
    Alphabet::set("ABC");

    const vector<SetOfCharacters>
        sets_of_characters({ SetOfCharacters("A"), SetOfCharacters("BC") });
    const Constraint constraint(sets_of_characters);

    EXPECT_EQ(sets_of_characters.size(), constraint.size());

    for (size_t i = 0; i != sets_of_characters.size(); ++i)
    {
        EXPECT_EQ(sets_of_characters[i], constraint[i]);
    }
}

#if defined(__clang__) && defined(__CYGWIN__)

// This test does not work when compiled with clang in Cygwin:
// * in debug, the for loop variable 's' contains garbage (and thus the
//   check fails)
// * in release, the Constraint constructor crashes

#else // defined(__clang__) && defined(__CYGWIN__)

TEST_F(ConstraintTest, constructor_from_initializer_list)
{
    Alphabet::set("ABC");

    initializer_list<string> strings({ "A", "BC" });

    const Constraint constraint(strings);

    EXPECT_EQ(strings.size(), constraint.size());

    size_t i = 0;
    for (const auto& s : strings)
    {
        EXPECT_EQ(SetOfCharacters(s), constraint[i]);
        ++i;
    }
}

#endif // defined(__clang__) && defined(__CYGWIN__)

TEST_F(ConstraintTest, all)
{
    Alphabet::set("ABC");
    EXPECT_EQ(Constraint({ "ABC", "ABC" }), Constraint::all(2));
}

TEST_F(ConstraintTest, none)
{
    EXPECT_EQ(Constraint({ "", "" }), Constraint::none(2));
}

TEST_F(ConstraintTest, operator_or)
{
    Alphabet::set("ABC");

    const Constraint constraint_1({  "A", "BC" });
    const Constraint constraint_2({ "BC",  "A" });

    EXPECT_EQ(Constraint({ "ABC", "ABC" }), constraint_1 | constraint_2);
}

TEST_F(ConstraintTest, operator_at)
{
    Alphabet::set("ABC");

    Constraint constraint({ "AB" });
    EXPECT_EQ(SetOfCharacters("AB"), constraint[0]);

    constraint[0] = SetOfCharacters("BC");
    EXPECT_EQ(SetOfCharacters("BC"), constraint[0]);
}

TEST_F(ConstraintTest, empty)
{
    Alphabet::set("ABC");

    const vector<SetOfCharacters> empty_vector;
    EXPECT_TRUE(Constraint(empty_vector).empty());

    EXPECT_FALSE(Constraint({ "" }).empty());
}

TEST_F(ConstraintTest, is_possible)
{
    Alphabet::set("ABC");

    EXPECT_TRUE(Constraint({ "A", "B" }).is_possible());
    EXPECT_FALSE(Constraint({ "A", "B" }).is_impossible());

    EXPECT_FALSE(Constraint({ "A", "" }).is_possible());
    EXPECT_TRUE(Constraint({ "A", "" }).is_impossible());
}

TEST_F(ConstraintTest, is_tighter_than_or_equal_to)
{
    Alphabet::set("ABC");

    {
        const vector<SetOfCharacters> empty_sets_of_characters;
        const Constraint a(empty_sets_of_characters);
        const Constraint b(empty_sets_of_characters);
        EXPECT_TRUE(a.is_tighter_than_or_equal_to(b));
    }

    {
        const Constraint a({ "A", "A" });
        const Constraint b({ "A", "AB" });
        EXPECT_TRUE(a.is_tighter_than_or_equal_to(b));
    }

    {
        const Constraint a({ "A", "AB" });
        const Constraint b({ "A", "A" });
        EXPECT_FALSE(a.is_tighter_than_or_equal_to(b));
    }

    {
        const Constraint a({ "A", "A" });
        const Constraint b({ "A", "B" });
        EXPECT_FALSE(a.is_tighter_than_or_equal_to(b));
    }
}
