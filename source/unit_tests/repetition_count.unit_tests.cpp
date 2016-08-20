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
#include "repetition_count.hpp"

using namespace std;


class RepetitionCountTest : public RegexCrosswordSolverTest
{
};


TEST_F(RepetitionCountTest, is_not_infinite)
{
    EXPECT_TRUE(RepetitionCount(0).is_not_infinite());
    EXPECT_FALSE(RepetitionCount::infinite().is_not_infinite());
}

TEST_F(RepetitionCountTest, to_string)
{
    EXPECT_EQ("infinite", RepetitionCount::infinite().to_string());
    EXPECT_EQ("1", RepetitionCount(1).to_string());
}

TEST_F(RepetitionCountTest, operator_plus)
{
    EXPECT_EQ(RepetitionCount::infinite(),
              RepetitionCount::infinite() + RepetitionCount::infinite());
    EXPECT_EQ(RepetitionCount::infinite(), RepetitionCount::infinite() + 1);
    EXPECT_EQ(RepetitionCount::infinite(), 1 + RepetitionCount::infinite());
    EXPECT_EQ(RepetitionCount(3), RepetitionCount(1) + RepetitionCount(2));
}

TEST_F(RepetitionCountTest, operator_minus)
{
    EXPECT_EQ(RepetitionCount::infinite(), RepetitionCount::infinite() - 1);
    EXPECT_EQ(RepetitionCount(0), RepetitionCount(1) - RepetitionCount(1));
}

TEST_F(RepetitionCountTest, operator_equal)
{
    EXPECT_TRUE(RepetitionCount::infinite() == RepetitionCount::infinite());
    EXPECT_FALSE(RepetitionCount(1) == RepetitionCount::infinite());
    EXPECT_TRUE(RepetitionCount(1) == RepetitionCount(1));
}

TEST_F(RepetitionCountTest, operator_not_equal)
{
    EXPECT_TRUE(RepetitionCount(1) != RepetitionCount(2));
}

TEST_F(RepetitionCountTest, operator_less_than_or_equal)
{
    EXPECT_TRUE(RepetitionCount::infinite() <= RepetitionCount::infinite());
    EXPECT_TRUE(RepetitionCount(1) <= RepetitionCount::infinite());
    EXPECT_TRUE(RepetitionCount(1) <= RepetitionCount(1));
}

TEST_F(RepetitionCountTest, operator_less_than)
{
    EXPECT_FALSE(RepetitionCount::infinite() < RepetitionCount::infinite());
    EXPECT_TRUE(RepetitionCount(1) < RepetitionCount::infinite());
    EXPECT_FALSE(RepetitionCount(1) < RepetitionCount(1));
}
