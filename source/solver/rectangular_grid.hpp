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


#ifndef RECTANGULAR_GRID_HPP
#define RECTANGULAR_GRID_HPP

#include "grid.hpp"

#include <iosfwd>

class GridLine;


// A rectangular grid contains:
// * horizontal lines of cells (rows),
// * vertical lines of cells (columns).
//
// Rows are referred to by line direction 0, columns by line direction 1.
//
// Each cell of the grid can be identified by its (x, y) coordinates,
// where:
// * x is the (0-based) row index,
// * y is the (0-based) column index.
//
// For example, cell (0, 2) is the cell on the first row and third
// column.
class RectangularGrid final : public Grid
{
public:
    // instance creation and deletion
    RectangularGrid(const std::vector<std::string>& regexes,
                    size_t                          num_rows,
                    size_t                          num_regexes_per_row,
                    size_t                          num_cols,
                    size_t                          num_regexes_per_col);

    // accessing
    size_t num_cols() const;
    size_t num_rows() const override;

private:
    friend class RectangularGridPrinter;

    // instance creation and deletion
    RectangularGrid(const RectangularGrid& rhs);
    std::vector<std::unique_ptr<GridLine>>
        make_lines(size_t line_direction) const override;

    // copying
    std::unique_ptr<Grid> clone() const override;

    // accessing
    size_t begin_y(size_t x) const override;
    std::vector<size_t> coordinates(size_t x, size_t y) const override;
    size_t end_y(size_t x) const override;
    size_t index_of_cell_on_line(size_t coordinate,
                                 size_t next_coordinate) const override;
    size_t num_line_directions() const override;

    // printing
    std::vector<std::string> do_print(bool verbose = true) const override;

    // data members

    size_t m_num_rows;
    size_t m_num_cols;
};


#endif // RECTANGULAR_GRID_HPP
