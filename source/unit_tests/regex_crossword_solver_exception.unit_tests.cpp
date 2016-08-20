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
#include "regex_crossword_solver_exception.hpp"
#include "regex_crossword_solver_test.hpp"

using namespace std;


class RegexCrosswordSolverExceptionTest : public RegexCrosswordSolverTest
{
};


TEST_F(RegexCrosswordSolverExceptionTest, alphabet_exception)
{
    AlphabetException exc("error message");
    EXPECT_STREQ("ERROR:\n"
                 "    error message", exc.what());
}

TEST_F(RegexCrosswordSolverExceptionTest, command_line_exception)
{
    CommandLineException exc("error message");
    EXPECT_STREQ("ERROR:\n"
                 "    error message\n"
                 "\n"
                 "For information on command line usage:\n"
                 "    program --help", exc.what());
}

TEST_F(RegexCrosswordSolverExceptionTest, grid_structure_exception)
{
    GridStructureException exc("error message");
    EXPECT_STREQ("ERROR:\n"
                 "    error message", exc.what());
}

TEST_F(RegexCrosswordSolverExceptionTest, input_file_exception_1)
{
    InputFileException exc("error message");
    EXPECT_STREQ("ERROR:\n"
                 "    error message", exc.what());
}

TEST_F(RegexCrosswordSolverExceptionTest, input_file_exception_2)
{
    InputFileException exc("path/to/file", 10, "line content", "error message");
    EXPECT_STREQ("ERROR:\n"
                 "    in 'path/to/file', line 10:\n"
                 "        'line content'\n"
                 "    error message", exc.what());
}

TEST_F(RegexCrosswordSolverExceptionTest, regex_parse_exception)
{
    const size_t error_position = 1;
    RegexParseException exc("some error message", "ABC", error_position);
    EXPECT_STREQ("ERROR:\n"
                 "    some error message:\n"
                 "        'ABC'\n"
                 "          ^", exc.what());
}

TEST_F(RegexCrosswordSolverExceptionTest, regex_structure_exception)
{
    RegexStructureException exc("error message");
    EXPECT_STREQ("ERROR:\n"
                 "    error message", exc.what());
}
