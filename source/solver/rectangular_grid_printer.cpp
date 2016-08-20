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


#include "rectangular_grid_printer.hpp"

#include "grid_line.hpp"
#include "rectangular_grid.hpp"

#include <cassert>

using namespace std;


// instance creation and deletion

RectangularGridPrinter::RectangularGridPrinter(const RectangularGrid& grid,
                                               bool                   verbose) :
  // 2nd argument: cell_body_width_must_be_odd = false
  // 3rd argument: min_cell_body_width = 1
  GridPrinter(grid.max_length_of_possible_characters_as_string(),
              false,
              1,
              verbose),
  m_grid(grid)
{
}

// accessing

const vector<unique_ptr<GridLine>>&
RectangularGridPrinter::grid_rows() const
{
    return m_grid.rows();
}

size_t
RectangularGridPrinter::hat_height() const
{
    return 1;
}

size_t
RectangularGridPrinter::num_grid_rows() const
{
    return m_grid.num_rows();
}

// printing

vector<string>
RectangularGridPrinter::print(const RectangularGrid& grid, bool verbose)
{
    const RectangularGridPrinter printer(grid, verbose);
    return printer.GridPrinter::print();
}

vector<string>
RectangularGridPrinter::print_cell_bottom_hat() const
{
    return vector<string>({ string(cell_body_width(), '-') });
}

vector<string>
RectangularGridPrinter::print_cell_bottom_hats() const
{
    vector<string> result(hat_height());

    for (size_t i = 0; i != m_grid.num_cols(); ++i)
    {
        result = concat_hor(result, print_vert_border_of_cell_bottom_hat());
        result = concat_hor(result, print_cell_bottom_hat());
    }

    return concat_hor(result, print_vert_border_of_cell_bottom_hat());
}

vector<string>
RectangularGridPrinter::print_cell_top_hat() const
{
    return vector<string>({ string(cell_body_width(), '-') });
}

vector<string>
RectangularGridPrinter::print_regexes() const
{
    vector<string> result({ "regexes:" });

    result.push_back("  rows:");
    for (size_t i = 0; i != m_grid.num_rows(); ++i)
    {
        result.push_back("    " + m_grid.line_at(0, i)->regexes_as_string());
    }

    result.push_back("  columns:");
    for (size_t i = 0; i != m_grid.num_cols(); ++i)
    {
        result.push_back("    " + m_grid.line_at(1, i)->regexes_as_string());
    }

    return result;
}

vector<string>
RectangularGridPrinter::print_row(size_t row_index) const
{
    assert(is_valid_row_index(row_index));
    return print_cells(row_index);
}
