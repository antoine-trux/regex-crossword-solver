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
#include "grid.unit_tests.utils.hpp"
#include "rectangular_grid.hpp"
#include "regex_crossword_solver_exception.hpp"
#include "regex_crossword_solver_test.hpp"

using namespace std;


class RectangularGridTest : public RegexCrosswordSolverTest
{
};


TEST_F(RectangularGridTest, constructor)
{
    const string grid_contents("shape = rectangular\n"

                               "num_rows = 2\n"
                               "num_cols = 3\n"

                               "num_regexes_per_row = 1\n"
                               "num_regexes_per_col = 1\n"

                               "'A'\n"
                               "'B'\n"

                               "'C'\n"
                               "'D'\n"
                               "'E'\n");
    auto generic_grid = GridUnitTestsUtils::read_grid(grid_contents);
    const auto grid =
        GridUnitTestsUtils::downcast_grid<RectangularGrid>(move(generic_grid));

    EXPECT_EQ(2, grid->m_lines_per_direction.size());
    EXPECT_EQ(2, grid->m_lines_per_direction[0].size());
    EXPECT_EQ(3, grid->m_lines_per_direction[1].size());
    EXPECT_EQ(2, grid->num_rows());
    EXPECT_EQ(3, grid->num_cols());
}

TEST_F(RectangularGridTest, solve_1_solution)
{
    // http://regexcrossword.com/challenges/intermediate/puzzles/1

    const string grid_contents("shape = rectangular\n"

                               "num_rows = 2\n"
                               "num_cols = 3\n"

                               "num_regexes_per_row = 1\n"
                               "num_regexes_per_col = 1\n"

                               "'[NOTAD]*'\n"
                               "'WEL|BAL|EAR'\n"

                               "'UB|IE|AW'\n"
                               "'[TUBE]*'\n"
                               "'[BORF].'\n");
    const vector<vector<string>> expected_solutions({ { "ATO", "WEL" } });

    GridUnitTestsUtils::solve_and_check(grid_contents, expected_solutions);
}

TEST_F(RectangularGridTest, solve_0_solutions)
{
    // derived from
    // http://regexcrossword.com/challenges/intermediate/puzzles/1

    const string grid_contents("shape = rectangular\n"

                               "num_rows = 2\n"
                               "num_cols = 3\n"

                               "num_regexes_per_row = 1\n"
                               "num_regexes_per_col = 1\n"

                               "'[NOTD]*'\n"
                               "'WEL|BAL|EAR'\n"

                               "'UB|IE|AW'\n"
                               "'[TUBE]*'\n"
                               "'[BORF].'\n");
    const vector<vector<string>> expected_solutions;

    GridUnitTestsUtils::solve_and_check(grid_contents, expected_solutions);
}

TEST_F(RectangularGridTest, solve_4_solutions)
{
    // derived from
    // http://regexcrossword.com/challenges/intermediate/puzzles/1

    const string grid_contents("shape = rectangular\n"

                               "num_rows = 2\n"
                               "num_cols = 3\n"

                               "num_regexes_per_row = 1\n"
                               "num_regexes_per_col = 1\n"

                               "'[NOTADB]*'\n"
                               "'WEL|BAL|EAR'\n"

                               "'UB|IE|AW'\n"
                               "'[TUBE]*'\n"
                               "'[BORF].'\n");
    const vector<vector<string>> expected_solutions({ { "ABB", "WEL" },
                                                      { "ABO", "WEL" },
                                                      { "ATB", "WEL" },
                                                      { "ATO", "WEL" } });

    GridUnitTestsUtils::solve_and_check(grid_contents, expected_solutions);
}

TEST_F(RectangularGridTest, invalid_num_regexes)
{
    const string grid_contents("shape = rectangular\n"

                               "num_rows = 1\n"
                               "num_cols = 2\n"

                               "num_regexes_per_row = 2\n"
                               "num_regexes_per_col = 1\n"

                               // There should be 1 * 2 + 2 * 1 = 4
                               // regexes, but there is only 1.
                               "'A'\n");
    EXPECT_THROW(GridUnitTestsUtils::read_grid(grid_contents),
                 GridStructureException);
}
