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


#ifndef HEXAGONAL_GRID_HPP
#define HEXAGONAL_GRID_HPP

#include "grid.hpp"

#include <gtest/gtest_prod.h>
#include <iosfwd>

class GridLine;


// A hexagonal grid has its cells oriented along three line directions:
//
// (x) west -> east lines (= "rows")
//
//       -------------->
//
// (y) south-east -> north-west lines
/*
 *       ^
 *        \
 *         \
 *          \
 *           \
 *            \
 *
 * (z) north-east -> south-west lines
 *
 *            /
 *           /
 *          /
 *         /
 *        /
 *       V
 */
// The six sides of the hexagon can be grouped into three pairs,
// according to where the lines start:
/*
 *               (F)
 *          --------------
 *         /              \
 *    (A) /                \ (E)
 *       /                  \
 *      /                    \
 *     /                      \
 *     \                      /
 *      \                    /
 *       \                  /
 *    (B) \                / (D)
 *         \              /
 *          --------------
 *               (C)
 */
// * the north-west (A) and south-west sides (B),
//   where the west -> east (x) lines (i.e., rows) start
//
// * the south (C) and south-east (D) sides,
//   where the south-east -> north-west (y) lines start
//
// * the north-east (E) and north sides (F),
//   where the north-east -> south-west (z) lines start
//
// A line can be identified by its index amongst the lines of same
// direction, counterclockwise. For example, if each side of the hexagon
// has 7 cells, the topmost row has index 0, and the bottommost row has
// index 12.
//
// Each cell of the grid can be identified by its (x, y, z) coordinates,
// which are the indices of the x, y and z lines to which the cell
// belongs. Notice that two coordinates are sufficient to determine a
// cell; that is, the third coordinate can be deduced from the other
// two.
//
// For example, in a hexagonal grid where each side has 7 cells (such as
// the MIT puzzle), cell (1, 5, 12) is the cell which lies:
// * on the 2nd row,
// * on the 6th south-east -> north-west line,
// * on the 13th north-east -> south-west line.
class HexagonalGrid final : public Grid
{
public:
    // instance creation and deletion
    HexagonalGrid(const std::vector<std::string>& regexes,
                  size_t                          num_regexes_per_line);

private:
    friend class HexagonalGridPrinter;
    FRIEND_TEST(HexagonalGridTest, side_length);
    FRIEND_TEST(HexagonalGridTest, num_lines_per_direction);
    FRIEND_TEST(HexagonalGridTest, begin_end_y);
    FRIEND_TEST(HexagonalGridTest, num_cells);
    FRIEND_TEST(HexagonalGridTest, z);
    FRIEND_TEST(HexagonalGridTest, index_of_cell_on_line);
    FRIEND_TEST(HexagonalGridTest, constructor);

    // instance creation and deletion
    HexagonalGrid(const HexagonalGrid& rhs);
    explicit HexagonalGrid(
               const std::vector<std::vector<std::string>>& regex_groups);
    explicit HexagonalGrid(size_t side_length);
    std::unique_ptr<GridLine> make_line(
                                size_t line_direction,
                                size_t line_index_within_direction) const;
    std::vector<std::unique_ptr<GridLine>>
        make_lines(size_t line_direction) const override;
    size_t num_line_directions() const override;
    static size_t side_length(size_t num_lines);

    // copying
    std::unique_ptr<Grid> clone() const override;

    // accessing
    size_t begin_y(size_t x) const override;
    std::vector<size_t> coordinates(size_t x, size_t y) const override;
    size_t end_y(size_t x) const override;
    size_t index_of_cell_on_line(size_t coordinate,
                                 size_t next_coordinate) const override;
    size_t num_cells(size_t line_index_within_direction) const;
    size_t num_lines_per_direction() const;
    size_t num_rows() const override;
    size_t z(size_t x, size_t y) const;

    // querying
    bool are_valid_coordinates(size_t x, size_t y) const;
    bool is_valid_coordinate(size_t coordinate) const;

    // printing
    std::vector<std::string> do_print(bool verbose) const override;

    // data members

    // The number of cells along each side of the hexagon.
    //
    // For example, this is 7 for the MIT puzzle.
    size_t m_side_length;
};


#endif // HEXAGONAL_GRID_HPP
