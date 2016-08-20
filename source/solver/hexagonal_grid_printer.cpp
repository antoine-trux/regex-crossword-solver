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


#include "hexagonal_grid_printer.hpp"

#include "grid_line.hpp"
#include "hexagonal_grid.hpp"

#include <cassert>

using namespace std;


// instance creation and deletion

HexagonalGridPrinter::HexagonalGridPrinter(const HexagonalGrid& grid,
                                           bool                 verbose) :
  // 2nd argument: cell_body_width_must_be_odd = true, in order to be
  //               able to draw a vertical bar on top and in the middle
  //               of the hat
  // 3rd argument: min_cell_body_width = 3, in order to be able to draw
  //               the cell's hat
  GridPrinter(grid.max_length_of_possible_characters_as_string(),
              true,
              3,
              verbose),
  m_grid(grid)
{
}

// accessing

const vector<unique_ptr<GridLine>>&
HexagonalGridPrinter::grid_rows() const
{
    return m_grid.rows();
}

size_t
HexagonalGridPrinter::hat_height() const
{
    return (cell_body_width() - 1) / 2;
}

size_t
HexagonalGridPrinter::indentation_level(size_t row_index) const
{
    assert(is_valid_row_index(row_index));

    return m_grid.num_rows() - m_grid.num_cells(row_index);
}

size_t
HexagonalGridPrinter::last_row_index() const
{
    return m_grid.num_rows() - 1;
}

size_t
HexagonalGridPrinter::num_grid_rows() const
{
    return m_grid.num_rows();
}

// querying

bool
HexagonalGridPrinter::is_below_midrow(size_t row_index) const
{
    assert(is_valid_row_index(row_index));

    return row_index > m_grid.num_rows() / 2;
}

// printing

vector<string>
HexagonalGridPrinter::print(const HexagonalGrid& grid, bool verbose)
{
    const HexagonalGridPrinter printer(grid, verbose);
    return printer.GridPrinter::print();
}

vector<string>
HexagonalGridPrinter::print_cell_bottom_hat() const
{
    vector<string> result;

    const auto hat_height_ = hat_height();

    for (size_t i = 0; i != hat_height_; ++i)
    {
        const string side_padding(i, ' ');
        const string center_padding(2 * (hat_height_ - i) - 1, ' ');
        const string line(side_padding + '\\' + center_padding + '/' +
                          side_padding);
        result.push_back(line);
    }

    return result;
}

vector<string>
HexagonalGridPrinter::print_cell_bottom_hats() const
{
    auto result = print_padding_for_cell_bottom_hats();

    for (size_t i = 0; i != m_grid.num_cells(last_row_index()); ++i)
    {
        result = concat_hor(result, print_vert_border_of_cell_bottom_hat());
        result = concat_hor(result, print_cell_bottom_hat());
    }

    result = concat_hor(result, print_vert_border_of_cell_bottom_hat());
    result = concat_hor(result, print_padding_for_cell_bottom_hats());

    return result;
}

vector<string>
HexagonalGridPrinter::print_cell_top_hat() const
{
    vector<string> result;

    const auto hat_height_ = hat_height();

    for (size_t i = 0; i != hat_height_; ++i)
    {
        const string side_padding(hat_height_ - i - 1, ' ');
        const string center_padding(2 * i + 1, ' ');
        const string line(side_padding + '/' + center_padding + '\\' +
                          side_padding);
        result.push_back(line);
    }

    return result;
}

vector<string>
HexagonalGridPrinter::print_left_cell_bottom_half_hat() const
{
    vector<string> result;

    for (size_t i = 0; i != hat_height(); ++i)
    {
        string line = " ";
        line += string(i, ' ');
        line += '\\';
        line += string(hat_height() - i - 1, ' ');
        result.push_back(line);
    }

    return result;
}

vector<string>
HexagonalGridPrinter::print_left_padding(size_t row_index) const
{
    assert(is_valid_row_index(row_index));

    const auto indentation_level_ = indentation_level(row_index);

    if (is_below_midrow(row_index))
    {
        auto result = print_padding(indentation_level_ - 1, hat_height());
        result = concat_hor(result, print_left_cell_bottom_half_hat());
        result = concat_vert(result, print_padding(indentation_level_,
                                                   cell_body_height()));
        return result;
    }
    else
    {
        return print_padding(indentation_level_, cell_top_height());
    }
}

vector<string>
HexagonalGridPrinter::print_padding(size_t indentation_level,
                                    size_t height) const
{
    const auto width = indentation_level * (hat_height() + 1);
    return print_spaces(width, height);
}

vector<string>
HexagonalGridPrinter::print_padding_for_cell_bottom_hats() const
{
    return print_padding_for_row(last_row_index(), hat_height());
}

vector<string>
HexagonalGridPrinter::print_padding_for_row(size_t row_index,
                                            size_t height) const
{
    assert(is_valid_row_index(row_index));

    return print_padding(indentation_level(row_index), height);
}

vector<string>
HexagonalGridPrinter::print_regexes() const
{
    vector<string> result;

    result.push_back("regexes:");

    result.push_back("  west -> east:");
    for (size_t i = 0; i != m_grid.num_lines_per_direction(); ++i)
    {
        result.push_back("    " + m_grid.line_at(0, i)->regexes_as_string());
    }

    result.push_back("  south-east -> north-west:");
    for (size_t i = 0; i != m_grid.num_lines_per_direction(); ++i)
    {
        result.push_back("    " + m_grid.line_at(1, i)->regexes_as_string());
    }

    result.push_back("  north-east -> south-west:");
    for (size_t i = 0; i != m_grid.num_lines_per_direction(); ++i)
    {
        result.push_back("    " + m_grid.line_at(2, i)->regexes_as_string());
    }

    return result;
}

vector<string>
HexagonalGridPrinter::print_right_cell_bottom_half_hat() const
{
    vector<string> result;

    for (size_t i = 0; i != hat_height(); ++i)
    {
        string line = string(hat_height() - i - 1, ' ');
        line += '/';
        line += string(i, ' ');
        line += ' ';
        result.push_back(line);
    }

    return result;
}

vector<string>
HexagonalGridPrinter::print_right_padding(size_t row_index) const
{
    assert(is_valid_row_index(row_index));

    const auto indentation_level_ = indentation_level(row_index);

    if (is_below_midrow(row_index))
    {
        auto result = print_right_cell_bottom_half_hat();
        result = concat_hor(result, print_padding(indentation_level_ - 1,
                                                  hat_height()));
        result = concat_vert(result, print_padding(indentation_level_,
                                                   cell_body_height()));
        return result;
    }
    else
    {
        return print_padding(indentation_level_, cell_top_height());
    }
}

vector<string>
HexagonalGridPrinter::print_row(size_t row_index) const
{
    assert(is_valid_row_index(row_index));

    vector<string> result(cell_top_height());

    result = concat_hor(result, print_left_padding(row_index));
    result = concat_hor(result, print_cells(row_index));
    result = concat_hor(result, print_right_padding(row_index));

    return result;
}

vector<string>
HexagonalGridPrinter::print_spaces(size_t width, size_t height) const
{
    const string spaces(width, ' ');
    return vector<string>(height, spaces);
}
