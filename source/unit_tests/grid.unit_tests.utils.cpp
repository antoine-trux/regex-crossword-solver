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


#include "grid.unit_tests.utils.hpp"

#include "grid_cell.hpp"
#include "grid_line.hpp"
#include "grid_reader.hpp"
#include "hexagonal_grid.hpp"
#include "logger.hpp"
#include "rectangular_grid.hpp"
#include "regex_optimizations.hpp"

#include <algorithm>

using namespace std;


namespace
{

// 'expected_rows' contains the rows of an expected solution. Return
// whether 'solution' is equal to 'expected_rows'.
bool
are_equal(const vector<string>& expected_rows, const Grid& solution)
{
    const auto num_rows = expected_rows.size();

    for (size_t i = 0; i != num_rows; ++i)
    {
        const auto row = solution.row_at(i);
        const auto expected_row = expected_rows[i];
        const auto num_cells_in_row = row->num_cells();

        if (expected_row.size() != num_cells_in_row)
        {
            return false;
        }

        for (size_t cell_i = 0; cell_i != num_cells_in_row; ++cell_i)
        {
            const auto cell = row->cell(cell_i);
            const auto expected_character = expected_row[cell_i];

            if (cell->possible_characters() !=
                SetOfCharacters(expected_character))
            {
                return false;
            }
        }
    }

    return true;
}

// Check that 'solution' is equal to one of the elements of
// 'expected_solutions'.
//
// Each element of 'expected_solutions' contains the rows of an expected
// solution.
void
check_solution(const vector<vector<string>>& expected_solutions,
               const Grid&                   solution)
{
    EXPECT_TRUE(any_of(expected_solutions.cbegin(),
                       expected_solutions.cend(),
                       [&solution](const vector<string>& expected_solution)
                       {
                           return are_equal(expected_solution, solution);
                       }));
}

// Check that the elements of 'solutions' are equal to the elements of
// 'expected_solutions' (not necessarily in the same order).
//
// Each element of 'expected_solutions' contains the rows of an expected
// solution.
void
check_solutions(const vector<vector<string>>&   expected_solutions,
                const vector<unique_ptr<Grid>>& solutions)
{
    EXPECT_EQ(expected_solutions.size(), solutions.size());

    for (const auto& solution : solutions)
    {
         check_solution(expected_solutions, *solution);
    }
}

vector<string>
log_filepaths_to_test()
{
    // ""  => no log
    // "-" => use console for logging
#if ENABLE_LOGGING
    return { "", "-" };
#else // ENABLE_LOGGING
    return { "" };
#endif // ENABLE_LOGGING
}

void
solve_and_check(const string&                 grid_contents,
                const vector<vector<string>>& expected_solutions,
                const string&                 log_filepath,
                bool                          optimize,
                bool                          find_all_solutions)
{
    static_cast<void>(log_filepath);

    SET_LOG_FILEPATH(log_filepath);

    const auto grid = GridUnitTestsUtils::read_grid(grid_contents);

    const auto optimizations =
        optimize ? RegexOptimizations::all() : RegexOptimizations::none();
    grid->optimize(optimizations);

    const auto num_solutions_to_find =
        find_all_solutions ? numeric_limits<unsigned int>::max() : 1;
    const auto solutions = grid->solve(num_solutions_to_find);

    if (find_all_solutions)
    {
        check_solutions(expected_solutions, solutions);
    }
    else
    {
        if (expected_solutions.empty())
        {
            EXPECT_TRUE(solutions.empty());
        }
        else
        {
            ASSERT_EQ(1, solutions.size());
            EXPECT_TRUE(are_equal(expected_solutions.front(),
                                  *solutions.front()));
        }
    }
}

void
solve_and_check(const string&                 grid_contents,
                const vector<vector<string>>& expected_solutions,
                const string&                 log_filepath,
                bool                          optimize)
{
    bool find_all_solutions = true;
    solve_and_check(grid_contents,
                    expected_solutions,
                    log_filepath,
                    optimize,
                    find_all_solutions);

    find_all_solutions = false;
    solve_and_check(grid_contents,
                    expected_solutions,
                    log_filepath,
                    optimize,
                    find_all_solutions);
}

void
solve_and_check(const string&                 grid_contents,
                const vector<vector<string>>& expected_solutions,
                const string&                 log_filepath)
{
    auto optimize = false;
    solve_and_check(grid_contents, expected_solutions, log_filepath, optimize);

    optimize = true;
    solve_and_check(grid_contents, expected_solutions, log_filepath, optimize);
}

} // unnamed namespace


unique_ptr<Grid>
GridUnitTestsUtils::read_grid(const string& grid_contents)
{
    istringstream grid_iss(grid_contents);
    return GridReader::read(grid_iss);
}

void
GridUnitTestsUtils::solve_and_check(
                      const string&                 grid_contents,
                      const vector<vector<string>>& expected_solutions)
{
    for (const auto& log_filepath : log_filepaths_to_test())
    {
        ::solve_and_check(grid_contents, expected_solutions, log_filepath);
    }
}
