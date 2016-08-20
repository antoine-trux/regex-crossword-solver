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
#include "group_number.hpp"

#include <gtest/gtest.h>

using namespace std;


TEST(GroupNumber, constructor)
{
    const unsigned int group_number_value = 1;
    const GroupNumber group_number(group_number_value);
    EXPECT_EQ(group_number_value, group_number.value());
}

TEST(GroupNumber, exceeds_max_backreference_value)
{
    const GroupNumber group_number(GroupNumber::max_backreference_value() + 1);
    EXPECT_TRUE(group_number.exceeds_max_backreference_value());
}

TEST(GroupNumber, to_string)
{
    const GroupNumber group_number(1);
    EXPECT_EQ("1", group_number.to_string());
}

TEST(GroupNumber, comparison)
{
    const GroupNumber group_number_1(1);
    const GroupNumber group_number_1_a(1);
    const GroupNumber group_number_2(2);

    EXPECT_TRUE(group_number_1 == group_number_1_a);
    EXPECT_FALSE(group_number_1 != group_number_1_a);
    EXPECT_FALSE(group_number_1 == group_number_2);
    EXPECT_TRUE(group_number_1 != group_number_2);
    EXPECT_TRUE(group_number_1 >= group_number_1_a);
    EXPECT_TRUE(group_number_2 >= group_number_1);
    EXPECT_FALSE(group_number_1 >= group_number_2);
}
