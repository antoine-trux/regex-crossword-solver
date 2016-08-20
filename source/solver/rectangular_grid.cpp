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


#include "rectangular_grid.hpp"

#include "grid_line.hpp"
#include "rectangular_grid_printer.hpp"
#include "regex_crossword_solver_exception.hpp"
#include "utils.hpp"

#include <algorithm>
#include <cassert>
#include <iterator>

using namespace std;


namespace
{

// Return 'regexes' grouped by row and column, such that each group of
// regexes can be assigned to the successive rows and columns, starting
// with the rows (topmost row first), then continuing with the columns
// (leftmost column first).
//
// Preconditions:
// * num_rows != 0
// * num_cols != 0
// * num_regexes_per_row != 0
// * num_regexes_per_col != 0
//
// For example, if:
// * 'num_rows' is 1,
// * 'num_regexes_per_row' is 2,
// * 'num_cols' is 2,
// * 'num_regexes_per_col' is 1,
// then the returned vector contains the following three vectors, in
// this order:
// * a vector with the two regexes for the row
// * a vector with the regex for the first column
// * a vector with the regex for the second column
vector<vector<string>>
group(const vector<string>& regexes,
      size_t                num_rows,
      size_t                num_regexes_per_row,
      size_t                num_cols,
      size_t                num_regexes_per_col)
{
    assert(num_rows != 0);
    assert(num_cols != 0);
    assert(num_regexes_per_row != 0);
    assert(num_regexes_per_col != 0);

    using Utils::to_string;

    const auto expected_num_regexes = num_rows * num_regexes_per_row +
                                      num_cols * num_regexes_per_col;
    if (regexes.size() != expected_num_regexes)
    {
        const auto error_message =
            string("we have:\n")                                               +
            "    num_rows = " + to_string(num_rows) + '\n'                     +
            "    num_cols = " + to_string(num_cols) + '\n'                     +
            "    num_regexes_per_row = "                                       +
            to_string(num_regexes_per_row) + '\n'                              +
            "    num_regexes_per_col = "                                       +
            to_string(num_regexes_per_col) + '\n'                              +
            "so there should be:\n"                                            +
            "    "                                                             +
            to_string(num_rows) + '*' + to_string(num_regexes_per_row) + " + " +
            to_string(num_cols) + '*' + to_string(num_regexes_per_col) + " = " +
            to_string(expected_num_regexes) + " regexes\n"
            "but there are " + to_string(regexes.size()) + " regexes";
        throw GridStructureException(error_message);
    }

    vector<vector<string>> result;

    auto regexes_it = begin(regexes);

    for (size_t i = 0; i != num_rows; ++i)
    {
        vector<string> row_group;
        copy_n(regexes_it, num_regexes_per_row, back_inserter(row_group));
        advance(regexes_it, num_regexes_per_row);
        result.push_back(row_group);
    }

    for (size_t i = 0; i != num_cols; ++i)
    {
        vector<string> col_group;
        copy_n(regexes_it, num_regexes_per_col, back_inserter(col_group));
        advance(regexes_it, num_regexes_per_col);
        result.push_back(col_group);
    }

    assert(regexes_it == end(regexes));

    return result;
}

}


// instance creation and deletion

// The elements of 'regexes' are listed first by rows, then by columns.
//
// Preconditions:
// * num_rows != 0
// * num_cols != 0
// * num_regexes_per_row != 0
// * num_regexes_per_col != 0
//
// For example, if:
// * 'num_rows' is 1,
// * 'num_regexes_per_row' is 2,
// * 'num_cols' is 2,
// * 'num_regexes_per_row' is 1,
// then 'regexes' contains, in this order:
// * the two regexes for the row
// * the regex for the first column
// * the regex for the second column
RectangularGrid::RectangularGrid(const vector<string>& regexes,
                                 size_t                num_rows,
                                 size_t                num_regexes_per_row,
                                 size_t                num_cols,
                                 size_t                num_regexes_per_col) :
  m_num_rows(num_rows),
  m_num_cols(num_cols)
{
    construct_grid(group(regexes,
                         num_rows, num_regexes_per_row,
                         num_cols, num_regexes_per_col));
}

RectangularGrid::RectangularGrid(const RectangularGrid& rhs) :
  Grid(rhs),
  m_num_rows(rhs.m_num_rows),
  m_num_cols(rhs.m_num_cols)
{
    copy_lines_and_cells(rhs);
}

vector<unique_ptr<GridLine>>
RectangularGrid::make_lines(size_t line_direction) const
{
    vector<unique_ptr<GridLine>> result;

    const auto num_lines = (line_direction == 0) ? m_num_rows : m_num_cols;
    const auto num_cells = (line_direction == 0) ? m_num_cols : m_num_rows;

    for (size_t line_index_within_direction = 0;
         line_index_within_direction != num_lines;
         ++line_index_within_direction)
    {
        result.push_back(make_line(line_direction,
                                   line_index_within_direction,
                                   num_cells));
    }

    return result;
}

// copying

unique_ptr<Grid>
RectangularGrid::clone() const
{
    return unique_ptr<Grid>(new RectangularGrid(*this));
}

// accessing

// Return the smallest possible y-coordinate of a cell whose
// x-coordinate is 'x'.
size_t
RectangularGrid::begin_y(size_t /*x*/) const
{
    return 0;
}

vector<size_t>
RectangularGrid::coordinates(size_t x, size_t y) const
{
    return { x, y };
}

// Return one past the largest possible y-coordinate of a cell whose
// x-coordinate is 'x'.
size_t
RectangularGrid::end_y(size_t /*x*/) const
{
    return m_num_cols;
}

// If 'coordinate' is the row index, then 'next_coordinate' is the
// column index, and vice versa.
size_t
RectangularGrid::index_of_cell_on_line(size_t /*coordinate*/,
                                       size_t next_coordinate) const
{
    return next_coordinate;
}

size_t
RectangularGrid::num_cols() const
{
    return m_num_cols;
}

size_t
RectangularGrid::num_line_directions() const
{
    return 2;
}

size_t
RectangularGrid::num_rows() const
{
    return m_num_rows;
}

// printing

vector<string>
RectangularGrid::do_print(bool verbose) const
{
    return RectangularGridPrinter::print(*this, verbose);
}
