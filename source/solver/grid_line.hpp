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


#ifndef GRID_LINE_HPP
#define GRID_LINE_HPP

#include "constraint.hpp"

#include <memory>
#include <string>
#include <vector>

class Grid;
class GridCell;
class GridLineRegex;
class Regex;
class RegexOptimizations;


// An instance of this class represents a line of cells in a grid.
//
// In a rectangular grid, lines are horizontal (rows) and vertical
// (columns).
//
// In a hexagonal grid, there are lines in three directions.
class GridLine final
{
public:
    // instance creation and deletion
    GridLine(const Grid& grid,
             size_t      line_direction,
             size_t      line_index_within_direction,
             size_t      num_cells);
    ~GridLine();
    void build_regexes(const std::vector<std::string>& regexes_as_strings);
    void set_cell(std::shared_ptr<GridCell> cell,
                  size_t                    index_of_cell_on_line);

    // copying
    void copy_regexes(const GridLine& rhs);

    // accessing
    std::shared_ptr<GridCell> cell(size_t cell_index);
    const std::vector<std::shared_ptr<GridCell>>& cells() const;
    std::string explicit_regex_characters() const;
    size_t num_cells() const;
    std::string regexes_as_string() const;

    // querying
    bool has_impossible_constraint() const;

    // modifying
    bool constrain();
    void optimize(const RegexOptimizations& optimizations);

private:
    // accessing
    Constraint constraint_from_cells() const;

    // printing
    std::vector<std::string> print_verbose_grid() const;

    // converting
    std::string to_string() const;

    // modifying
    Constraint constrain_regexes();
    void update_cells(const Constraint& new_constraint);

    // data members

    // The grid to which this line belongs.
    const Grid& m_grid;

    // 'm_direction' identifies the line direction.
    //
    // A rectangular grid has two line directions, so in that case
    // 'm_direction' is a number in [0, 2).
    //
    // A hexagonal grid has three line directions, so in that case
    // 'm_direction' is a number in [0, 3).
    //
    // See the descriptions of rectangular and hexagonal grids (in the
    // header files of these classes) for further information on line
    // directions.
    size_t m_direction;

    // 'm_index_within_direction' identifies this line among the lines
    // of same direction.
    //
    // Examples:
    // * In a rectangular grid with 5 rows, 'm_index_within_direction'
    //   for rows is a number in [0, 5) - row 0 being at the top and row
    //   4 at the bottom.
    // * In a hexagonal grid where each side has 7 cells, there are 13
    //   lines per direction, so in that case 'm_index_within_direction'
    //   is a number in [0, 13).
    size_t m_index_within_direction;

    // Many grids have a single regex per line, some have two.
    std::vector<GridLineRegex> m_grid_line_regexes;

    // The same cell is shared by all the lines that go through it - for
    // example, in a rectangular grid, each cell is shared by one row
    // and one column - hence the use of 'std::shared_ptr'.
    std::vector<std::shared_ptr<GridCell>> m_cells;

    // The constraint of this line the last time it was computed.
    Constraint m_saved_constraint;
};


#endif // GRID_LINE_HPP
