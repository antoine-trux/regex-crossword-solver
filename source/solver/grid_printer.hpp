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


#ifndef GRID_PRINTER_HPP
#define GRID_PRINTER_HPP

#include <limits>
#include <memory>
#include <string>
#include <utility>
#include <vector>

class GridCell;
class GridLine;


// An abstract class - the superclass of concrete classes
// RectangularGridPrinter and HexagonalGridPrinter.
class GridPrinter
{
public:
    // instance creation and deletion
    virtual ~GridPrinter() = 0;

protected:
    // instance creation and deletion
    GridPrinter(size_t max_num_characters_per_cell,
                bool   cell_body_width_must_be_odd,
                size_t min_cell_body_width,
                bool   verbose);

    // accessing
    size_t cell_body_height() const;
    size_t cell_body_width() const;
    size_t cell_top_height() const;

    // querying
    bool is_valid_row_index(size_t row_index) const;

    // printing
    static std::vector<std::string>
        concat_hor(const std::vector<std::string>& left_lines,
                   const std::vector<std::string>& right_lines);
    static std::vector<std::string>
        concat_vert(const std::vector<std::string>& top_lines,
                    const std::vector<std::string>& bottom_lines);
    std::vector<std::string> print() const;
    std::vector<std::string> print_cells(size_t row_index) const;
    std::vector<std::string> print_vert_border_of_cell_bottom_hat() const;

private:
    // accessing
    static size_t cell_body_height(size_t max_num_characters_per_cell,
                                   bool   cell_body_width_must_be_odd,
                                   size_t min_cell_body_width);
    static size_t cell_body_width(size_t max_num_characters_per_cell,
                                  bool   cell_body_width_must_be_odd,
                                  size_t min_cell_body_width);
    size_t characters_height(const GridCell& cell) const;
    std::string characters_line(const GridCell& cell,
                                size_t          characters_line_index) const;
    std::pair<size_t, size_t> characters_size(const GridCell& cell) const;
    static std::pair<size_t, size_t>
        characters_size(size_t num_characters,
                        bool   width_must_be_odd,
                        size_t min_width,
                        size_t max_width = std::numeric_limits<size_t>::max(),
                        size_t max_height = std::numeric_limits<size_t>::max());
    size_t characters_width(const GridCell& cell) const;
    std::string empty_row_in_cell_body() const;
    virtual const std::vector<std::unique_ptr<GridLine>>& grid_rows() const = 0;
    virtual size_t hat_height() const = 0;
    size_t num_empty_bottom_rows_in_body(const GridCell& cell) const;
    size_t num_empty_top_rows_in_body(const GridCell& cell) const;
    virtual size_t num_grid_rows() const = 0;

    // printing
    std::vector<std::string> print_alphabet() const;
    std::vector<std::string> print_cell_body(const GridCell& cell) const;
    std::vector<std::string> print_cell_body_empty_lines_at_bottom(
                               const GridCell& cell) const;
    std::vector<std::string> print_cell_body_empty_lines_at_top(
                               const GridCell& cell) const;
    virtual std::vector<std::string> print_cell_bottom_hats() const = 0;
    virtual std::vector<std::string> print_cell_top_hat() const = 0;
    std::vector<std::string> print_cells(
        const std::vector<std::shared_ptr<GridCell>>& row_of_cells) const;
    std::vector<std::string> print_characters(const GridCell& cell) const;
    std::vector<std::string> print_grid() const;
    virtual std::vector<std::string> print_regexes() const = 0;
    virtual std::vector<std::string> print_row(size_t row_index) const = 0;
    std::vector<std::string> print_top(const GridCell& cell) const;
    std::vector<std::string> print_vert_border_of_cell_top() const;

    // data members

    // In characters. For a description of "cell body width", see the
    // subclasses.
    size_t m_cell_body_width;

    // In characters. For a description of "cell body height", see the
    // subclasses.
    size_t m_cell_body_height;

    // Whether the alphabet and the regexes are also to be printed.
    bool m_verbose;
};


#endif // GRID_PRINTER_HPP
