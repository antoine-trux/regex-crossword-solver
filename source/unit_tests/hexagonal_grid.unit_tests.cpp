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
#include "hexagonal_grid.hpp"
#include "regex_crossword_solver_exception.hpp"
#include "regex_crossword_solver_test.hpp"

using namespace std;


class HexagonalGridTest : public RegexCrosswordSolverTest
{
};


TEST_F(HexagonalGridTest, side_length)
{
    EXPECT_EQ(1, HexagonalGrid::side_length(3));
    EXPECT_EQ(7, HexagonalGrid::side_length(39));
    EXPECT_THROW(HexagonalGrid::side_length(10), GridStructureException);
    EXPECT_THROW(HexagonalGrid::side_length(12), GridStructureException);
}

TEST_F(HexagonalGridTest, num_lines_per_direction)
{
    {
        const size_t side_length = 1;
        const HexagonalGrid grid(side_length);

        EXPECT_EQ(1, grid.num_lines_per_direction());
    }

    {
        const size_t side_length = 7;
        const HexagonalGrid grid(side_length);

        EXPECT_EQ(13, grid.num_lines_per_direction());
    }
}

TEST_F(HexagonalGridTest, begin_end_y)
{
    {
        const size_t side_length = 1;
        const HexagonalGrid grid(side_length);

        EXPECT_EQ(0, grid.begin_y(0));
        EXPECT_EQ(1, grid.end_y(0));
    }

    {
        const size_t side_length = 7;
        const HexagonalGrid grid(side_length);

        EXPECT_EQ( 6, grid.begin_y(0));
        EXPECT_EQ(13, grid.end_y(0));
        EXPECT_EQ( 0, grid.begin_y(6));
        EXPECT_EQ(13, grid.end_y(6));
        EXPECT_EQ( 0, grid.begin_y(12));
        EXPECT_EQ( 7, grid.end_y(12));
    }
}

TEST_F(HexagonalGridTest, num_cells)
{
    {
        const size_t side_length = 1;
        const HexagonalGrid grid(side_length);

        EXPECT_EQ(1, grid.num_cells(0));
    }

    {
        const size_t side_length = 7;
        const HexagonalGrid grid(side_length);

        EXPECT_EQ( 7, grid.num_cells( 0));
        EXPECT_EQ(13, grid.num_cells( 6));
        EXPECT_EQ( 7, grid.num_cells(12));
    }
}

TEST_F(HexagonalGridTest, z)
{
    {
        const size_t side_length = 1;
        const HexagonalGrid grid(side_length);

        EXPECT_EQ(0, grid.z(0, 0));
    }

    {
        const size_t side_length = 7;
        const HexagonalGrid grid(side_length);

        EXPECT_EQ(12, grid.z( 0,  6));
        EXPECT_EQ( 6, grid.z( 0, 12));
        EXPECT_EQ(12, grid.z( 6,  0));
        EXPECT_EQ( 0, grid.z( 6, 12));
        EXPECT_EQ( 6, grid.z(12,  0));
        EXPECT_EQ( 0, grid.z(12,  6));
    }
}

TEST_F(HexagonalGridTest, index_of_cell_on_line)
{
    {
        const size_t side_length = 1;
        const HexagonalGrid grid(side_length);

        EXPECT_EQ(0, grid.index_of_cell_on_line(0, 0));
    }

    {
        const size_t side_length = 7;
        const HexagonalGrid grid(side_length);

        EXPECT_EQ( 0, grid.index_of_cell_on_line( 0,  6));
        EXPECT_EQ( 6, grid.index_of_cell_on_line( 0, 12));
        EXPECT_EQ( 0, grid.index_of_cell_on_line( 6,  0));
        EXPECT_EQ(12, grid.index_of_cell_on_line( 6, 12));
        EXPECT_EQ( 0, grid.index_of_cell_on_line(12,  0));
        EXPECT_EQ( 6, grid.index_of_cell_on_line(12,  6));
    }
}

TEST_F(HexagonalGridTest, constructor)
{
    const string grid_contents("shape = hexagonal\n"

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
    auto generic_grid = GridUnitTestsUtils::read_grid(grid_contents);
    const auto grid =
        GridUnitTestsUtils::downcast_grid<HexagonalGrid>(move(generic_grid));

    EXPECT_EQ(3, grid->m_lines_per_direction.size());
    EXPECT_EQ(3, grid->m_lines_per_direction[0].size());
    EXPECT_EQ(3, grid->m_lines_per_direction[1].size());
    EXPECT_EQ(3, grid->m_lines_per_direction[2].size());
    EXPECT_EQ(2, grid->m_side_length);
}

TEST_F(HexagonalGridTest, solve_1_solution)
{
    // The grid of this test is inspired from the top left corner of the
    // MIT puzzle.

    const string grid_contents("shape = hexagonal\n"

                               "num_regexes_per_line = 1\n"

                               "'.*H.*'\n"
                               "'(DI|O)*'\n"
                               "'([AO])\\1'\n"

                               "'..'\n"
                               "'.*(IN|SE|HI)'\n"
                               "'[^C]*'\n"

                               "'..'\n"
                               "'[CHMNOR]*I[CHMNOR]*'\n"
                               "'ND|ET|IN'\n");
    const vector<vector<string>> expected_solutions({ { "NH", "DIO", "OO" } });

    GridUnitTestsUtils::solve_and_check(grid_contents, expected_solutions);
}

TEST_F(HexagonalGridTest, solve_0_solutions)
{
    // The grid of this test is inspired from the top left corner of the
    // MIT puzzle.

    const string grid_contents("shape = hexagonal\n"

                               "num_regexes_per_line = 1\n"

                               "'.*H.*'\n"
                               "'(DI|O)*'\n"
                               "'[AO].*'\n"

                               "'..'\n"
                               "'.*(IN|SE|HI)'\n"
                               "'[^C]*'\n"

                               "'..'\n"
                               "'AI[CHMNOR]*'\n"
                               "'ND|ET|IN'\n");
    const vector<vector<string>> expected_solutions;

    GridUnitTestsUtils::solve_and_check(grid_contents, expected_solutions);
}

TEST_F(HexagonalGridTest, solve_3_solutions)
{
    // The grid of this test is inspired from the top left corner of the
    // MIT puzzle.

    const string grid_contents("shape = hexagonal\n"

                               "num_regexes_per_line = 1\n"

                               "'.*H.*'\n"
                               "'(DI|O)*'\n"
                               "'[AO].*'\n"

                               "'..'\n"
                               "'.*(IN|SE|HI)'\n"
                               "'[^C]*'\n"

                               "'.[ACD]'\n"
                               "'[CHMNOR]*I[CHMNOR]*'\n"
                               "'ND|ET|IN'\n");
    const vector<vector<string>> expected_solutions({ { "NH", "DIO", "OA" },
                                                      { "NH", "DIO", "OC" },
                                                      { "NH", "DIO", "OD" } });

    GridUnitTestsUtils::solve_and_check(grid_contents, expected_solutions);
}

TEST_F(HexagonalGridTest, num_regexes_not_divisible_by_num_regexes_per_line)
{
    const string grid_contents("shape = hexagonal\n"
                               "num_regexes_per_line = 2\n"
                               "'A'\n");
    EXPECT_THROW(GridUnitTestsUtils::read_grid(grid_contents),
                 GridStructureException);
}
