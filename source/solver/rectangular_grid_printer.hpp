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


#ifndef RECTANGULAR_GRID_PRINTER_HPP
#define RECTANGULAR_GRID_PRINTER_HPP

#include "grid_printer.hpp"

class RectangularGrid;


// An instance of this class prints a RectangularGrid to an
// 'std::vector<std::string>', each string corresponding to a line of
// the grid.
//
// Here is the terminology we use when printing cells of a rectangular
// grid:
//
//      -------  <-- top hat                         ^
//     |       |                ^                    |
//     |  ABC  | ^ characters   | cell body height   | cell top height
//     |  DE   | v height       |                    |
//     |       |                v                    v
//      -------  <-- bottom hat
//
//        <->
//    characters
//       width
//
//      <----->
//     cell body
//       width
//
// In the above example:
// * characters width  = 3
// * characters height = 2
// * cell body width   = 7
// * cell body height  = 4
// * hat height        = 1
// * cell top height   = 5
class RectangularGridPrinter final : public GridPrinter
{
public:
    // printing
    static std::vector<std::string> print(const RectangularGrid& grid,
                                          bool                   verbose);

private:
    // instance creation and deletion
    RectangularGridPrinter(const RectangularGrid& grid, bool verbose);

    // accessing
    const std::vector<std::unique_ptr<GridLine>>& grid_rows() const override;
    size_t hat_height() const override;
    size_t num_grid_rows() const override;

    // printing
    std::vector<std::string> print_cell_bottom_hat() const;
    std::vector<std::string> print_cell_bottom_hats() const override;
    std::vector<std::string> print_cell_top_hat() const override;
    std::vector<std::string> print_regexes() const override;
    std::vector<std::string> print_row(size_t row_index) const override;

    // data members

    const RectangularGrid& m_grid;
};


#endif // RECTANGULAR_GRID_PRINTER_HPP
