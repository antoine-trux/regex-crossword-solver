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
#include "grid.hpp"
#include "grid_reader.hpp"
#include "regex_crossword_solver_exception.hpp"
#include "regex_crossword_solver_test.hpp"

using namespace std;


class GridReaderTest : public RegexCrosswordSolverTest
{
};


TEST_F(GridReaderTest, hexagonal)
{
    istringstream iss("shape = hexagonal\n"

                      "num_regexes_per_line = 1\n"

                      "'A'\n"
                      "'B'\n"
                      "'C'\n"

                      "'D'\n"
                      "'E'\n"
                      "'F'\n"

                      "'G'\n"
                      "'H'\n"
                      "'I'\n");
    const auto grid = GridReader::read(iss);
    EXPECT_EQ(3, grid->num_rows());
}

TEST_F(GridReaderTest, rectangular)
{
    istringstream iss("shape = rectangular\n"

                      "num_rows = 2\n"
                      "num_cols = 3\n"

                      "num_regexes_per_row = 1\n"
                      "num_regexes_per_col = 1\n"

                      "'A'\n"
                      "'B'\n"

                      "'C'\n"
                      "'D'\n"
                      "'E'\n");
    const auto grid = GridReader::read(iss);
    EXPECT_EQ(2, grid->num_rows());
    EXPECT_EQ(6, grid->all_cells().size());
}

TEST_F(GridReaderTest, dos_format)
{
    istringstream iss("shape = rectangular\r\n"

                      "num_rows = 1\r\n"
                      "num_cols = 1\r\n"

                      "num_regexes_per_row = 1\r\n"
                      "num_regexes_per_col = 1\r\n"

                      "'A'\r\n"

                      "'B'\r\n");
    const auto grid = GridReader::read(iss);
    EXPECT_EQ(1, grid->num_rows());
    EXPECT_EQ(1, grid->all_cells().size());
}

TEST_F(GridReaderTest, nonempty_line_with_only_whitespace)
{
    istringstream iss(" \n");
    EXPECT_THROW(GridReader::read(iss), InputFileException);
}

TEST_F(GridReaderTest, missing_shape)
{
    istringstream iss("\n");
    EXPECT_THROW(GridReader::read(iss), InputFileException);
}

TEST_F(GridReaderTest, invalid_shape)
{
    istringstream iss("shape = foo\n");
    EXPECT_THROW(GridReader::read(iss), InputFileException);
}

TEST_F(GridReaderTest, rectangular_missing_num_rows)
{
    {
        istringstream iss("shape = rectangular\n");
        EXPECT_THROW(GridReader::read(iss), InputFileException);
    }

    {
        istringstream iss("shape = rectangular\n"
                          "foo\n");
        EXPECT_THROW(GridReader::read(iss), InputFileException);
    }
}

TEST_F(GridReaderTest, rectangular_missing_equal_sign_after_num_rows)
{
    istringstream iss("shape = rectangular\n"
                      "num_rows\n");
    EXPECT_THROW(GridReader::read(iss), InputFileException);
}

TEST_F(GridReaderTest, rectangular_missing_num_rows_value)
{
    istringstream iss("shape = rectangular\n"
                      "num_rows = \n");
    EXPECT_THROW(GridReader::read(iss), InputFileException);
}

TEST_F(GridReaderTest, rectangular_invalid_num_rows_value)
{
    istringstream iss("shape = rectangular\n"
                      "num_rows = x\n");
    EXPECT_THROW(GridReader::read(iss), InputFileException);
}

TEST_F(GridReaderTest, regex_without_initial_quote)
{
    istringstream iss("shape = hexagonal\n"

                      "num_regexes_per_line = 1\n"

                      "A'\n"
                      "B'\n"
                      "C'\n");
    EXPECT_THROW(GridReader::read(iss), InputFileException);
}

TEST_F(GridReaderTest, regex_without_final_quote)
{
    istringstream iss("shape = hexagonal\n"

                      "num_regexes_per_line = 1\n"

                      "'A\n"
                      "'B\n"
                      "'C\n");
    EXPECT_THROW(GridReader::read(iss), InputFileException);
}

TEST_F(GridReaderTest, regex_with_single_quote)
{
    istringstream iss("shape = hexagonal\n"

                      "num_regexes_per_line = 1\n"

                      "'\n"
                      "'\n"
                      "'\n");
    EXPECT_THROW(GridReader::read(iss), InputFileException);
}
