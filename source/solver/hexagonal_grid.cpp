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


#include "hexagonal_grid.hpp"

#include "grid_line.hpp"
#include "hexagonal_grid_printer.hpp"
#include "regex_crossword_solver_exception.hpp"
#include "utils.hpp"

#include <algorithm>
#include <cassert>
#include <iterator>

using namespace std;


namespace
{

// Return 'regexes' grouped by line, such that each group of regexes can
// be assigned to the successive lines, starting at the topmost row and
// going counterclockwise.
//
// Precondition:
// * num_regexes_per_line != 0
//
// For example, if 'regexes' contains 18 elements and
// 'num_regexes_per_line' is 2, then the returned vector contains the
// following nine vectors, in this order:
// * a vector with the two regexes for the top row
// * a vector with the two regexes for the middle row
// * a vector with the two regexes for the bottom row
// * a vector with the two regexes for the leftmost
//   south-east -> north-west line
// * a vector with the two regexes for the middle
//   south-east -> north-west line
// * a vector with the two regexes for the rightmost
//   south-east -> north-west line
// * a vector with the two regexes for the rightmost
//   north-east -> south-west line
// * a vector with the two regexes for the middle
//   north-east -> south-west line
// * a vector with the two regexes for the leftmost
//   north-east -> south-west line
vector<vector<string>>
group(const vector<string>& regexes, size_t num_regexes_per_line)
{
    assert(num_regexes_per_line != 0);

    using Utils::to_string;

    if (regexes.size() % num_regexes_per_line != 0)
    {
        const auto error_message =
            string("we have:\n")                                         +
            "    num_regexes_per_line = "                                +
            to_string(num_regexes_per_line) + '\n'                       +
            "and there are " + to_string(regexes.size()) + " regexes\n"  +
            "but " + to_string(regexes.size()) + " is not divisible by " +
            to_string(num_regexes_per_line) + '\n';
        throw GridStructureException(error_message);
    }

    const auto num_lines = regexes.size() / num_regexes_per_line;

    vector<vector<string>> result;

    auto regexes_it = begin(regexes);

    for (size_t i = 0; i != num_lines; ++i)
    {
        vector<string> line_group;
        copy_n(regexes_it, num_regexes_per_line, back_inserter(line_group));
        advance(regexes_it, num_regexes_per_line);
        result.push_back(line_group);
    }

    assert(regexes_it == end(regexes));

    return result;
}

}


// instance creation and deletion

HexagonalGrid::HexagonalGrid(const HexagonalGrid& rhs) :
  Grid(rhs),
  m_side_length(rhs.m_side_length)
{
    copy_lines_and_cells(rhs);
}

// Precondition:
// * num_regexes_per_line != 0
HexagonalGrid::HexagonalGrid(const vector<string>& regexes,
                             size_t                num_regexes_per_line) :
  HexagonalGrid(group(regexes, num_regexes_per_line))
{
}

HexagonalGrid::HexagonalGrid(const vector<vector<string>>& regex_groups) :
  m_side_length(side_length(regex_groups.size()))
{
    construct_grid(regex_groups);
}

// Used by unit tests.
HexagonalGrid::HexagonalGrid(size_t side_length) :
  m_side_length(side_length)
{
}

unique_ptr<GridLine>
HexagonalGrid::make_line(size_t line_direction,
                         size_t line_index_within_direction) const
{
    return Grid::make_line(line_direction,
                           line_index_within_direction,
                           num_cells(line_index_within_direction));
}

vector<unique_ptr<GridLine>>
HexagonalGrid::make_lines(size_t line_direction) const
{
    vector<unique_ptr<GridLine>> result;

    for (size_t line_index_within_direction = 0;
         line_index_within_direction != num_lines_per_direction();
         ++line_index_within_direction)
    {
        result.push_back(make_line(line_direction,
                                   line_index_within_direction));
    }

    return result;
}

// Return the length (i.e., number of cells) of each side of the
// hexagon, given that there are altogether 'num_lines' lines in the
// grid.
size_t
HexagonalGrid::side_length(size_t num_lines)
{
    using Utils::to_string;

    if (num_lines % 3 != 0)
    {
        const auto message = "number of lines ("  +
                             to_string(num_lines) +
                             ") is not a multiple of 3";
        throw GridStructureException(message);
    }

    const auto num_lines_per_direction = num_lines / 3;

    if (num_lines_per_direction % 2 == 0)
    {
        const auto message = "number of lines / line direction (" +
                             to_string(num_lines_per_direction)   +
                             ") is even - should be odd";
        throw GridStructureException(message);
    }

    return (num_lines_per_direction + 1) / 2;
}

// copying

unique_ptr<Grid>
HexagonalGrid::clone() const
{
    return unique_ptr<Grid>(new HexagonalGrid(*this));
}

// accessing

// Return the smallest possible y-coordinate of a cell whose
// x-coordinate is 'x'.
size_t
HexagonalGrid::begin_y(size_t x) const
{
    assert(is_valid_coordinate(x));

    if (m_side_length >= x + 1)
    {
        return m_side_length - (x + 1);
    }
    else
    {
        return 0;
    }
}

vector<size_t>
HexagonalGrid::coordinates(size_t x, size_t y) const
{
    return { x, y, z(x, y) };
}

// Return one past the largest possible y-coordinate of a cell whose
// x-coordinate is 'x'.
size_t
HexagonalGrid::end_y(size_t x) const
{
    assert(is_valid_coordinate(x));

    // We avoid undefined behavior, which would result from converting
    // large unsigned integers to signed integers, with extra variables
    // of a signed type (int).

    const auto x_int = static_cast<int>(x);
    const auto side_length_int = static_cast<int>(m_side_length);

    const auto result = 2 * side_length_int - 1 -
                        max(x_int - side_length_int + 1, 0);

    assert(result >= 0);
    return static_cast<decltype(end_y(x))>(result);
}

// Return the index of a cell on the line corresponding to 'coordinate',
// given that the cell's next coordinate is 'next_coordinate', where
// "next coordinate" means y for x, z for y, and x for z.
//
// For example, let's suppose that the length of this grid's side is 3,
// that 'coordinate' is y=1, and 'next_coordinate' is z=2. Then the
// index of the cell along the y-line of index 1 is 1 (i.e., second cell
// on that line). Thus, this function would return 1 in this example.
size_t
HexagonalGrid::index_of_cell_on_line(size_t coordinate,
                                     size_t next_coordinate) const
{
    // The cases:
    // * (coordinate=y, next_coordinate=z), and
    // * (coordinate=z, next_coordinate=x)
    // are both isomorphic to the case (coordinate=x, next_coordinate=y),
    // so we use that last case.
    const auto x = coordinate;
    const auto y = next_coordinate;

    assert(are_valid_coordinates(x, y));

    return y - begin_y(x);
}

size_t
HexagonalGrid::num_cells(size_t line_index_within_direction) const
{
    return num_lines_per_direction() -
           static_cast<decltype(num_lines_per_direction())>(
               abs(static_cast<int>(line_index_within_direction) + 1 -
                   static_cast<int>(m_side_length)));
}

size_t
HexagonalGrid::num_line_directions() const
{
    return 3;
}

size_t
HexagonalGrid::num_lines_per_direction() const
{
    return 2 * m_side_length - 1;
}

size_t
HexagonalGrid::num_rows() const
{
    return num_lines_per_direction();
}

// Return the z coordinate of a cell, given its x and y coordinates.
size_t
HexagonalGrid::z(size_t x, size_t y) const
{
    assert(are_valid_coordinates(x, y));

    return 3 * m_side_length - x - y - 3;
}

// querying

bool
HexagonalGrid::are_valid_coordinates(size_t x, size_t y) const
{
    return is_valid_coordinate(x) && begin_y(x) <= y && y < end_y(x);
}

bool
HexagonalGrid::is_valid_coordinate(size_t coordinate) const
{
    return coordinate < num_lines_per_direction();
}

// printing

vector<string>
HexagonalGrid::do_print(bool verbose) const
{
    return HexagonalGridPrinter::print(*this, verbose);
}
