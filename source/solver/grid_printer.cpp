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


#include "grid_printer.hpp"

#include "alphabet.hpp"
#include "grid_cell.hpp"
#include "grid_line.hpp"
#include "utils.hpp"

#include <algorithm>
#include <cassert>
#include <cmath>

using namespace std;


// instance creation and deletion

// Precondition:
// * min_cell_body_width != 0
GridPrinter::GridPrinter(size_t max_num_characters_per_cell,
                         bool   cell_body_width_must_be_odd,
                         size_t min_cell_body_width,
                         bool   verbose) :
  m_cell_body_width(cell_body_width(max_num_characters_per_cell,
                                    cell_body_width_must_be_odd,
                                    min_cell_body_width)),
  m_cell_body_height(cell_body_height(max_num_characters_per_cell,
                                      cell_body_width_must_be_odd,
                                      min_cell_body_width)),
  m_verbose(verbose)
{
}

GridPrinter::~GridPrinter() = default;

// accessing

// Precondition:
// * min_cell_body_width != 0
size_t
GridPrinter::cell_body_height(size_t max_num_characters_per_cell,
                              bool   cell_body_width_must_be_odd,
                              size_t min_cell_body_width)
{
    return characters_size(max_num_characters_per_cell,
                           cell_body_width_must_be_odd,
                           min_cell_body_width).second;
}

size_t
GridPrinter::cell_body_height() const
{
    return m_cell_body_height;
}

// Precondition:
// * min_cell_body_width != 0
size_t
GridPrinter::cell_body_width(size_t max_num_characters_per_cell,
                             bool   cell_body_width_must_be_odd,
                             size_t min_cell_body_width)
{
    return characters_size(max_num_characters_per_cell,
                           cell_body_width_must_be_odd,
                           min_cell_body_width).first;
}

size_t
GridPrinter::cell_body_width() const
{
    return m_cell_body_width;
}

size_t
GridPrinter::cell_top_height() const
{
    return hat_height() + m_cell_body_height;
}

size_t
GridPrinter::characters_height(const GridCell& cell) const
{
    const auto result = characters_size(cell).second;
    assert(result <= m_cell_body_height);
    return result;
}

// Return the 'characters_line_index'th line of characters to be printed
// in 'cell', including padding.
string
GridPrinter::characters_line(const GridCell& cell,
                             size_t          characters_line_index) const
{
    assert(characters_line_index < characters_height(cell));

    const auto characters_width_ = characters_width(cell);

    const auto characters = cell.possible_characters_as_string();

    const auto line_characters =
        characters.substr(characters_line_index * characters_width_,
                          characters_width_);

    const auto num_line_characters = line_characters.size();
    assert(num_line_characters <= cell_body_width());

    const auto length_of_left_padding =
        (m_cell_body_width - num_line_characters) / 2;
    const auto length_of_right_padding =
        m_cell_body_width - (length_of_left_padding + num_line_characters);

    const string left_padding(length_of_left_padding, ' ');
    const string right_padding(length_of_right_padding, ' ');

    return left_padding + line_characters + right_padding;
}

// Return (width, height) of the characters to be printed in 'cell'.
pair<size_t, size_t>
GridPrinter::characters_size(const GridCell& cell) const
{
    const auto characters = cell.possible_characters_as_string();
    const auto num_characters = characters.size();
    const size_t min_width = 1;
    const auto width_must_be_odd = false;

    return characters_size(num_characters,
                           width_must_be_odd,
                           min_width,
                           m_cell_body_width,
                           m_cell_body_height);
}

// A cell is to contain 'num_characters' characters.
//
// Return a pair which contains:
// * first  -> width of lines (i.e., number of characters per line)
// * second -> number of lines
//
// Preconditions:
// * min_width != 0
// * max_width != 0
// * max_height != 0
// * min_width <= max_width
pair<size_t, size_t>
GridPrinter::characters_size(size_t num_characters,
                             bool   width_must_be_odd,
                             size_t min_width,
                             size_t max_width,
                             size_t max_height)
{
    assert(min_width != 0);
    assert(max_width != 0);
    assert(max_height != 0);
    assert(min_width <= max_width);

    // Implementation:
    // * we determine the ideal floating-point width which corresponds
    //   to an ideal aspect ratio
    // * we compute the two integer widths which frame the ideal width
    // * we compute the two integer heights which correspond to the two
    //   integer widths computed in the previous step
    // * we determine which of the two (width, height) pairs is closer
    //   to the ideal aspect ratio

    // In my editor, on my screen, 3 rows of 5 characters each
    // are approximately a square.
    const auto ideal_aspect_ratio = 5.0 / 3.0;

    const auto ideal_width =
        sqrt(ideal_aspect_ratio *
             static_cast<decltype(ideal_aspect_ratio)>(num_characters));

    auto width_1 = static_cast<decltype(min_width)>(floor(ideal_width));
    auto width_2 = static_cast<decltype(min_width)>(ceil(ideal_width));

    width_1 = max(width_1, min_width);
    width_2 = max(width_2, min_width);

    if (width_must_be_odd && width_1 % 2 == 0)
    {
        ++width_1;
    }
    if (width_must_be_odd && width_2 % 2 == 0)
    {
        ++width_2;
    }

    width_1 = min(width_1, max_width);
    width_2 = min(width_2, max_width);

    auto height_1 = (num_characters + width_1 - 1) / width_1;
    auto height_2 = (num_characters + width_2 - 1) / width_2;

    height_1 = max(height_1, static_cast<decltype(height_1)>(1));
    height_2 = max(height_2, static_cast<decltype(height_2)>(1));

    height_1 = min(height_1, max_height);
    height_2 = min(height_2, max_height);

    // Reducing the height may result in too small a width for all the
    // characters to fit. In such a case, the width must be adjusted.
    while (width_1 * height_1 < num_characters)
    {
        ++width_1;
    }
    while (width_2 * height_2 < num_characters)
    {
        ++width_2;
    }

    const auto aspect_ratio_1 =
        static_cast<decltype(ideal_aspect_ratio)>(width_1) /
        static_cast<decltype(ideal_aspect_ratio)>(height_1);
    const auto aspect_ratio_2 =
        static_cast<decltype(ideal_aspect_ratio)>(width_2) /
        static_cast<decltype(ideal_aspect_ratio)>(height_2);

    const auto difference_1_from_ideal_aspect_ratio =
        fabs(aspect_ratio_1 - ideal_aspect_ratio);
    const auto difference_2_from_ideal_aspect_ratio =
        fabs(aspect_ratio_2 - ideal_aspect_ratio);

    if (difference_1_from_ideal_aspect_ratio <
        difference_2_from_ideal_aspect_ratio)
    {
        return make_pair(width_1, height_1);
    }
    else
    {
        return make_pair(width_2, height_2);
    }
}

size_t
GridPrinter::characters_width(const GridCell& cell) const
{
    const auto result = characters_size(cell).first;
    assert(result <= m_cell_body_width);
    return result;
}

string
GridPrinter::empty_row_in_cell_body() const
{
    return string(m_cell_body_width, ' ');
}

size_t
GridPrinter::num_empty_bottom_rows_in_body(const GridCell& cell) const
{
    return m_cell_body_height -
           (num_empty_top_rows_in_body(cell) + characters_height(cell));
}

size_t
GridPrinter::num_empty_top_rows_in_body(const GridCell& cell) const
{
    return (m_cell_body_height - characters_height(cell)) / 2;
}

// querying

bool
GridPrinter::is_valid_row_index(size_t row_index) const
{
    return row_index < num_grid_rows();
}

// printing

// Return the horizontal concatenation of 'left_lines' and
// 'right_lines'.
//
// Precondition:
// * 'left_lines' and 'right_lines' have the same number of elements
//
// For example, if:
// * 'left_lines' is { "ABC", "DEF" }, and
// * 'right_lines' is { "GH", "I" },
// then this method returns { "ABCGH", "DEFI" }.
vector<string>
GridPrinter::concat_hor(const vector<string>& left_lines,
                        const vector<string>& right_lines)
{
    assert(left_lines.size() == right_lines.size());

    vector<string> result;

    for (size_t i = 0; i != left_lines.size(); ++i)
    {
        const auto& left_line = left_lines[i];
        const auto& right_line = right_lines[i];

        result.push_back(left_line + right_line);
    }

    return result;
}

// Return the vertical concatenation of 'top_lines' and bottom_lines'.
//
// For example, if:
// * 'top_lines' is { "ABC", "DEF" }, and
// * 'bottom_lines' is { "GH" },
// then this method returns { "ABC", "DEF", "GH" }.
vector<string>
GridPrinter::concat_vert(const vector<string>& top_lines,
                         const vector<string>& bottom_lines)
{
    auto result = top_lines;
    result.insert(result.end(), bottom_lines.cbegin(), bottom_lines.cend());
    return result;
}

// Return lines containing the textual representation of the grid.
vector<string>
GridPrinter::print() const
{
    auto result = print_grid();

    if (m_verbose)
    {
        result = concat_vert(result, print_alphabet());
        result = concat_vert(result, print_regexes());
    }

    return result;
}

vector<string>
GridPrinter::print_alphabet() const
{
    return vector<string>({ "alphabet: " +
                            Utils::quoted(Alphabet::characters_as_string()) });
}

vector<string>
GridPrinter::print_cell_body(const GridCell& cell) const
{
    vector<string> result;
    result = concat_vert(result, print_cell_body_empty_lines_at_top(cell));
    result = concat_vert(result, print_characters(cell));
    result = concat_vert(result, print_cell_body_empty_lines_at_bottom(cell));
    return result;
}

vector<string>
GridPrinter::print_cell_body_empty_lines_at_bottom(const GridCell& cell) const
{
    vector<string> result;

    for (size_t i = 0; i != num_empty_bottom_rows_in_body(cell); ++i)
    {
        result.push_back(empty_row_in_cell_body());
    }

    return result;
}

vector<string>
GridPrinter::print_cell_body_empty_lines_at_top(const GridCell& cell) const
{
    vector<string> result;

    for (size_t i = 0; i != num_empty_top_rows_in_body(cell); ++i)
    {
        result.push_back(empty_row_in_cell_body());
    }

    return result;
}

vector<string>
GridPrinter::print_cells(size_t row_index) const
{
    assert(is_valid_row_index(row_index));

    const auto& rows = grid_rows();
    const auto& row = rows[row_index];
    const auto& row_of_cells = row->cells();

    return print_cells(row_of_cells);
}

// Return the character lines for 'row_of_cells'.
vector<string>
GridPrinter::print_cells(const vector<shared_ptr<GridCell>>& row_of_cells) const
{
    vector<string> result(cell_top_height());

    for (const auto& cell : row_of_cells)
    {
        result = concat_hor(result, print_vert_border_of_cell_top());
        result = concat_hor(result, print_top(*cell));
    }

    return concat_hor(result, print_vert_border_of_cell_top());
}

vector<string>
GridPrinter::print_characters(const GridCell& cell) const
{
    vector<string> result;

    for (size_t i = 0; i != characters_height(cell); ++i)
    {
        result.push_back(characters_line(cell, i));
    }

    return result;
}

vector<string>
GridPrinter::print_grid() const
{
    vector<string> result;

    for (size_t i = 0; i != num_grid_rows(); ++i)
    {
        result = concat_vert(result, print_row(i));
    }

    return concat_vert(result, print_cell_bottom_hats());
}

// Return lines containing the textual representation of the top of 'cell'.
//
// Only the top hat and the possible characters are returned.
//
// Examples:
//
//     rectangular grid:        hexagonal grid:
/*
 *          -------                   / \
 *          ABCDEFG                  /   \
 *          HIJKLMN                 /     \
 *            OPQ                   ABCDEFG
 *                                  HIJKLMN
 *                                    OPQ
 */
vector<string>
GridPrinter::print_top(const GridCell& cell) const
{
    vector<string> result;
    result = concat_vert(result, print_cell_top_hat());
    result = concat_vert(result, print_cell_body(cell));
    return result;
}

vector<string>
GridPrinter::print_vert_border_of_cell_bottom_hat() const
{
    vector<string> result;

    const auto hat_height_ = hat_height();

    for (size_t i = 0; i != hat_height_; ++i)
    {
        result.push_back(" ");
    }

    return result;
}

vector<string>
GridPrinter::print_vert_border_of_cell_top() const
{
    vector<string> result;

    const auto hat_height_ = hat_height();

    for (size_t i = 0; i != hat_height_; ++i)
    {
        result.push_back(" ");
    }

    for (size_t i = 0; i != m_cell_body_height; ++i)
    {
        result.push_back("|");
    }

    return result;
}
