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


#ifndef GRID_HPP
#define GRID_HPP

#include <gtest/gtest_prod.h>
#include <iosfwd>
#include <memory>
#include <string>
#include <vector>

class GridCell;
class GridLine;
class RegexOptimizations;


// An abstract class (the superclass of concrete classes
// RectangularGrid and HexagonalGrid).
//
// The way the cells are organized into lines, and accessed with
// coordinates, depends on the grid geometry. See the subclasses of
// this class for further details.
class Grid
{
public:
    // instance creation and deletion
    virtual ~Grid() = 0;

    // accessing
    GridLine* row_at(size_t row_index) const;

    // printing
    std::vector<std::string> print() const;
    std::vector<std::string> print_verbose() const;
    static void report_solutions(
        const std::vector<std::unique_ptr<Grid>>& solutions,
        unsigned int                              num_solutions_to_find);

    // modifying
    void optimize(const RegexOptimizations& optimizations);
    std::vector<std::unique_ptr<Grid>> solve(
                                         unsigned int num_solutions_to_find);

protected:
    // instance creation and deletion
    Grid();
    explicit Grid(const Grid& rhs);
    void construct_grid(
           const std::vector<std::vector<std::string>>& regex_groups);
    std::unique_ptr<GridLine> make_line(size_t line_direction,
                                        size_t line_index_within_direction,
                                        size_t num_cells) const;

    // copying
    void copy_lines_and_cells(const Grid& source_grid);

    // accessing
    GridLine* line_at(size_t line_direction, size_t line_index) const;
    size_t max_length_of_possible_characters_as_string() const;
    const std::vector<std::unique_ptr<GridLine>>& rows() const;

private:
    FRIEND_TEST(GridReaderTest, hexagonal);
    FRIEND_TEST(GridReaderTest, rectangular);
    FRIEND_TEST(GridReaderTest, dos_format);
    FRIEND_TEST(HexagonalGridTest, constructor);
    FRIEND_TEST(RectangularGridTest, constructor);

    // instance creation and deletion
    void build_cell(size_t x, size_t y);
    void build_cells();
    void build_grid_structure();
    void build_lines();
    void build_regexes(
           const std::vector<std::vector<std::string>>& regex_groups) const;
    void initialize_cells();
    std::shared_ptr<GridCell>
        make_cell(const std::vector<size_t>& coordinates);
    virtual std::vector<std::unique_ptr<GridLine>>
        make_lines(size_t line_direction) const = 0;
    void place_cell(std::shared_ptr<GridCell>  cell,
                    const std::vector<size_t>& coordinates,
                    size_t                     line_direction);
    void set_alphabet() const;

    // copying
    virtual std::unique_ptr<Grid> clone() const = 0;
    void copy_cell(const Grid& source_grid, size_t x, size_t y);
    void copy_cells(const Grid& source_grid);
    void copy_lines(const Grid& source_grid);
    void copy_regexes(const Grid& source_grid);
    std::shared_ptr<GridCell> make_cell(const GridCell& cell_to_copy);

    // accessing
    std::vector<std::shared_ptr<GridCell>> all_cells() const;
    std::vector<GridLine*> all_lines() const;
    virtual size_t begin_y(size_t x) const = 0;
    std::shared_ptr<GridCell>
        cell(const std::vector<size_t>& coordinates) const;
    std::shared_ptr<GridCell> cell_to_search() const;
    virtual std::vector<size_t> coordinates(size_t x, size_t y) const = 0;
    virtual size_t end_y(size_t x) const = 0;
    std::string explicit_regex_characters() const;
    virtual size_t index_of_cell_on_line(size_t coordinate,
                                         size_t next_coordinate) const = 0;
    std::vector<GridLine*> lines_through(
                             const std::vector<size_t>& coordinates) const;
    virtual size_t num_line_directions() const = 0;
    virtual size_t num_rows() const = 0;

    // querying
    bool is_solved() const;

    // printing
    virtual std::vector<std::string> do_print(bool verbose) const = 0;
    void log_constrain_result(bool success) const;

    // modifying
    bool constrain();
    bool constrain_no_log();
    std::vector<std::unique_ptr<Grid>>
         search_cell(const GridCell& cell,
                     unsigned int&   num_remaining_solutions_to_find);
    std::vector<std::unique_ptr<Grid>>
         search_cell(const GridCell& cell,
                     char            c,
                     unsigned int&   num_remaining_solutions_to_find);
    std::vector<std::unique_ptr<Grid>>
         search_grid(unsigned int& num_remaining_solutions_to_find);
    std::vector<std::unique_ptr<Grid>>
         solve_no_log(unsigned int& num_remaining_solutions_to_find);

    // data members

    // The lines are organized by line direction.
    //
    // For example, in a hexagonal grid, 'm_lines_per_direction'
    // contains 3 elements (because there are 3 line directions in a
    // hexagonal grid), and each of these elements contains the lines of
    // that direction.
    std::vector<std::vector<std::unique_ptr<GridLine>>> m_lines_per_direction;
};

std::ostream& operator<<(std::ostream& os, const Grid& grid);


#endif // GRID_HPP
