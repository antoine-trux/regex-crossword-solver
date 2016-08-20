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


#include "utils.hpp"

#include "command_line.hpp"

#include <algorithm>
#include <cassert>
#include <fstream>
#include <iostream>

using namespace std;


// querying

// Return whether 'path' exists as anything (file, directory, etc.) in
// the filesystem.
bool
Utils::filesystem_entity_exists(const string& path)
{
    ifstream ifs(path);
    return !ifs.fail();
}

// Return whether 's' contains only whitespace characters, if any.
bool
Utils::has_only_whitespace(const string& s)
{
    return find_if(s.cbegin(),
                   s.cend(),
                   [](char c)
                   {
                       return !isspace(c);
                   }) == s.cend();
}

bool
Utils::is_ascii_letter(char c)
{
    return ('a' <= c && c <= 'z') ||
           ('A' <= c && c <= 'Z');
}

bool
Utils::is_octal_digit(char c)
{
    return '0' <= c && c <= '7';
}

// Return whether 's' starts with 'prefix'.
bool
Utils::starts_with(const string& s, const string& prefix)
{
    return s.compare(0, prefix.size(), prefix) == 0;
}

// printing

// If in verbose mode, print 'message' onto 'os', otherwise do nothing.
void
Utils::print_verbose_message(ostream& os, const string& message)
{
    if (CommandLine::is_verbose())
    {
        os << message << endl;
    }
}

// converting

// Return a 'string' which consists of the single character 'c'.
string
Utils::char_to_string(char c)
{
    return string(1, c);
}

int
Utils::digit_to_int(char c)
{
    assert(isdigit(c));
    return c - '0';
}

int
Utils::hex_digit_to_int(char c)
{
    assert(isxdigit(c));

    if (isdigit(c))
    {
        return digit_to_int(c);
    }
    else if (islower(c))
    {
        return c - 'a' + 10;
    }
    else
    {
        assert(isupper(c));
        return c - 'A' + 10;
    }
}

// Return 's' surrounded by single quotes.
string
Utils::quoted(const string& s)
{
    return '\'' + s + '\'';
}

// Return a vector which contains the lines of 's'.
//
// For example, if 's' is "line 1\nline 2\n", this function returns a
// vector with two elements: "line 1" and "line 2".
vector<string>
Utils::split_into_lines(const string& s)
{
    vector<string> result;
    istringstream iss(s);
    string line;

    while (getline(iss, line))
    {
        result.push_back(line);
    }

    return result;
}

// modifying

// If 's' is next in 'istream', advance 'istream' past 's', and return
// true.
//
// Otherwise, leave 'istream' unchanged, and return false.
bool
Utils::skip(istream& is, const string& s)
{
    // The straightforward:
    //
    //     equal(s.begin(), s.end(), istream_iterator<char>(is))
    //
    // cannot be used here. If the contents of 'is' happened to be a
    // strict prefix of 's', the 'istream_iterator' would be
    // dereferenced when at end position, which would result in
    // undefined behavior. For example:
    //
    //     const string s = "abc";
    //     istringstream is("a");
    //     equal(s.begin(),
    //           s.end(),
    //           istream_iterator<char>(is)); // undefined behavior

    const auto original_position = is.tellg();

    for (auto string_char : s)
    {
        char istream_char;

        if (!is.get(istream_char) || istream_char != string_char)
        {
            is.seekg(original_position);
            return false;
        }
    }

    return true;
}
