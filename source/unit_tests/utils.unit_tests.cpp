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
#include "utils.hpp"

#include <gtest/gtest.h>
#include <sstream>

using namespace std;


TEST(Utils, filesystem_entity_exists)
{
    const string path = "this_file_or_directory_does_not_exist";
    EXPECT_FALSE(Utils::filesystem_entity_exists(path));
}

TEST(Utils, char_to_string)
{
    EXPECT_EQ("a", Utils::char_to_string('a'));
}

TEST(Utils, digit_to_int)
{
    EXPECT_EQ(0, Utils::digit_to_int('0'));
    EXPECT_EQ(9, Utils::digit_to_int('9'));
}

TEST(Utils, hex_digit_to_int)
{
    EXPECT_EQ( 0, Utils::hex_digit_to_int('0'));
    EXPECT_EQ( 9, Utils::hex_digit_to_int('9'));
    EXPECT_EQ(10, Utils::hex_digit_to_int('a'));
    EXPECT_EQ(15, Utils::hex_digit_to_int('f'));
    EXPECT_EQ(10, Utils::hex_digit_to_int('A'));
    EXPECT_EQ(15, Utils::hex_digit_to_int('F'));
}

TEST(Utils, is_ascii_letter)
{
    EXPECT_TRUE(Utils::is_ascii_letter('a'));
    EXPECT_TRUE(Utils::is_ascii_letter('z'));

    EXPECT_TRUE(Utils::is_ascii_letter('A'));
    EXPECT_TRUE(Utils::is_ascii_letter('Z'));

    EXPECT_FALSE(Utils::is_ascii_letter('0'));
    EXPECT_FALSE(Utils::is_ascii_letter('9'));
    EXPECT_FALSE(Utils::is_ascii_letter('@'));
    EXPECT_FALSE(Utils::is_ascii_letter('['));
    EXPECT_FALSE(Utils::is_ascii_letter('`'));
    EXPECT_FALSE(Utils::is_ascii_letter('{'));
}

TEST(Utils, is_octal_digit)
{
    EXPECT_TRUE(Utils::is_octal_digit('0'));
    EXPECT_TRUE(Utils::is_octal_digit('7'));

    EXPECT_FALSE(Utils::is_octal_digit('8'));
    EXPECT_FALSE(Utils::is_octal_digit('9'));
    EXPECT_FALSE(Utils::is_octal_digit('/'));
    EXPECT_FALSE(Utils::is_octal_digit(':'));
}

TEST(Utils, has_only_whitespace)
{
    EXPECT_TRUE(Utils::has_only_whitespace(""));
    EXPECT_TRUE(Utils::has_only_whitespace(" "));
    EXPECT_FALSE(Utils::has_only_whitespace(" x"));
}

TEST(Utils, starts_with)
{
    EXPECT_TRUE(Utils::starts_with("", ""));
    EXPECT_TRUE(Utils::starts_with("x", ""));
    EXPECT_FALSE(Utils::starts_with("", "x"));
    EXPECT_TRUE(Utils::starts_with("xy z", "xy"));
    EXPECT_TRUE(Utils::starts_with("xyz", "xyz"));
    EXPECT_FALSE(Utils::starts_with("xyz", "xa"));
}

TEST(Utils, quoted)
{
    EXPECT_EQ("'abc'", Utils::quoted("abc"));
}

TEST(Utils, split_into_lines)
{
    {
        const auto lines = Utils::split_into_lines("");
        EXPECT_TRUE(lines.empty());
    }

    {
        const auto lines = Utils::split_into_lines("line 1");
        ASSERT_EQ(1, lines.size());
        EXPECT_EQ("line 1", lines[0]);
    }

    {
        const auto lines = Utils::split_into_lines("line 1\n");
        ASSERT_EQ(1, lines.size());
        EXPECT_EQ("line 1", lines[0]);
    }

    {
        const auto lines = Utils::split_into_lines("line 1\nline 2\n");
        ASSERT_EQ(2, lines.size());
        EXPECT_EQ("line 1", lines[0]);
        EXPECT_EQ("line 2", lines[1]);
    }
}

TEST(Utils, string_to_unsigned)
{
    size_t number = 0;

    // empty string
    EXPECT_FALSE(Utils::string_to_unsigned("", &number));

    // extra initial input
    EXPECT_FALSE(Utils::string_to_unsigned(" 1", &number));

    // extra final input
    EXPECT_FALSE(Utils::string_to_unsigned("1 ", &number));

    // negative number
    EXPECT_FALSE(Utils::string_to_unsigned("-1", &number));

    {
        // too large to fit into 'unsigned long long int'
        const string max_ull_as_string =
            Utils::to_string(numeric_limits<unsigned long long int>::max());
        const string too_large_ull_as_string = max_ull_as_string + '0';
        EXPECT_FALSE(Utils::string_to_unsigned(too_large_ull_as_string,
                                               &number));
    }

    // The purpose of variable
    // 'unsigned_long_long_int_is_larger_than_size_t' is to avoid Visual
    // Studio 2015's warning C4127 (Conditional Expression is Constant),
    // which would be triggered by:
    //
    //    if (sizeof(unsigned long long int) > sizeof(size_t))
    //
    static const auto unsigned_long_long_int_is_larger_than_size_t =
        sizeof(unsigned long long int) > sizeof(size_t);
    if (unsigned_long_long_int_is_larger_than_size_t)
    {
        // not too large to fit into 'unsigned long long int', but too
        // large to fit into 'size_t'
        const auto max_size_t = numeric_limits<size_t>::max();
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4307) // integral constant overflow
#endif // _MSC_VER
        const auto too_large_size_t =
            static_cast<unsigned long long int>(max_size_t) + 1;
#ifdef _MSC_VER
#pragma warning(pop)
#endif // _MSC_VER
        EXPECT_FALSE(Utils::string_to_unsigned(
                              Utils::to_string(too_large_size_t), &number));
    }

    EXPECT_TRUE(Utils::string_to_unsigned("0", &number));
    EXPECT_EQ(0, number);

    EXPECT_TRUE(Utils::string_to_unsigned("100", &number));
    EXPECT_EQ(100, number);

    EXPECT_TRUE(Utils::string_to_unsigned("0100", &number));
    EXPECT_EQ(100, number);
}

TEST(Utils, skip)
{
    {
        istringstream iss("");
        const string s("");
        EXPECT_TRUE(Utils::skip(iss, s));
        EXPECT_EQ(EOF, iss.get());
    }

    {
        istringstream iss("x");
        const string s("");
        EXPECT_TRUE(Utils::skip(iss, s));
        char c;
        EXPECT_TRUE(static_cast<bool>(iss.get(c)));
        EXPECT_EQ('x', c);
    }

    {
        istringstream iss("");
        const string s("x");
        EXPECT_FALSE(Utils::skip(iss, s));
        EXPECT_EQ(EOF, iss.get());
    }

    {
        istringstream iss("xy z");
        const string s("xy");
        EXPECT_TRUE(Utils::skip(iss, s));
        char c;
        EXPECT_TRUE(static_cast<bool>(iss.get(c)));
        EXPECT_EQ(' ', c);
    }

    {
        istringstream iss("xyz");
        const string s("xyz");
        EXPECT_TRUE(Utils::skip(iss, s));
        EXPECT_EQ(EOF, iss.get());
    }

    {
        istringstream iss("xyz");
        const string s("xa");
        EXPECT_FALSE(Utils::skip(iss, s));
        char c;
        EXPECT_TRUE(static_cast<bool>(iss.get(c)));
        EXPECT_EQ('x', c);
    }
}
