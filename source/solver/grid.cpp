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


#include "grid.hpp"

#include "alphabet.hpp"
#include "grid_cell.hpp"
#include "grid_line.hpp"
#include "logger.hpp"
#include "utils.hpp"

#include <algorithm>
#include <cassert>
#include <iterator>
#include <numeric>

using namespace std;


namespace
{

// Return the first lines that appear in a report corresponding to the
// given arguments.
vector<string>
report_header(const vector<unique_ptr<Grid>>& solutions,
              unsigned int                    num_solutions_to_find)
{
    const auto num_solutions_found = solutions.size();

    if (num_solutions_found == 0)
    {
        return { "this grid has no solutions" };
    }
    else
    {
        if (num_solutions_found < num_solutions_to_find)
        {
            return { "found " + Utils::to_string(num_solutions_found) +
                     " solution(s) (there are no other solutions):" };
        }
        else
        {
            return { "first " + Utils::to_string(num_solutions_found) +
                     " solution(s) found (there might be other solutions):",
                     "",
                     "(if you want to see if there are other solutions,",
                     " increase the value of command line option "
                     "'--stop-after')" };
        }
    }
}

void
log_solutions(const vector<unique_ptr<Grid>>& solutions,
              unsigned int                    num_solutions_to_find)
{
    static_cast<void>(num_solutions_to_find);

    LOG_BLANK_LINE();
    LOG(report_header(solutions, num_solutions_to_find));

    INCREMENT_LOGGING_INDENTATION_LEVEL();

    for (const auto& solution : solutions)
    {
        static_cast<void>(solution);

        LOG_BLANK_LINE();
        LOG(solution->print_verbose());
    }

    DECREMENT_LOGGING_INDENTATION_LEVEL();
}

} // unnamed namespace


// instance creation and deletion

Grid::Grid()
{
}

Grid::Grid(const Grid& /*rhs*/)
{
    // copy_lines_and_cells() (indirectly) calls virtual functions, so
    // copy_lines_and_cells() is not called here. Instead, it is called
    // by the copy constructor of the subclasses.
}

Grid::~Grid() = default;

void
Grid::build_cell(size_t x, size_t y)
{
    const auto coordinates_ = coordinates(x, y);
    auto cell = make_cell(coordinates_);

    for (size_t line_direction = 0;
         line_direction != num_line_directions();
         ++line_direction)
    {
        place_cell(cell, coordinates_, line_direction);
    }
}

void
Grid::build_cells()
{
    for (size_t x = 0; x != num_rows(); ++x)
    {
        for (auto y = begin_y(x); y != end_y(x); ++y)
        {
            build_cell(x, y);
        }
    }
}

void
Grid::build_grid_structure()
{
    build_lines();
    build_cells();
}

void
Grid::build_lines()
{
    for (size_t line_direction = 0;
         line_direction != num_line_directions();
         ++line_direction)
    {
        m_lines_per_direction.push_back(make_lines(line_direction));
    }
}

void
Grid::build_regexes(const vector<vector<string>>& regex_groups) const
{
    const auto lines = all_lines();

    for (size_t i = 0; i != lines.size(); ++i)
    {
        const auto line = lines[i];
        line->build_regexes(regex_groups[i]);
    }
}

void
Grid::construct_grid(const vector<vector<string>>& regex_groups)
{
    build_grid_structure();
    build_regexes(regex_groups);
    set_alphabet();
    initialize_cells();
}

void
Grid::initialize_cells()
{
    for (const auto& cell : all_cells())
    {
        cell->set_possible_characters_to_all_characters();
    }
}

shared_ptr<GridCell>
Grid::make_cell(const vector<size_t>& coordinates)
{
    return make_shared<GridCell>(coordinates);
}

unique_ptr<GridLine>
Grid::make_line(size_t line_direction,
                size_t line_index_within_direction,
                size_t num_cells) const
{
    return Utils::make_unique<GridLine>(*this,
                                        line_direction,
                                        line_index_within_direction,
                                        num_cells);
}

void
Grid::place_cell(shared_ptr<GridCell>  cell,
                 const vector<size_t>& coordinates,
                 size_t                line_direction)
{
    const auto lines = lines_through(coordinates);
    const auto coordinate = coordinates[line_direction];
    const auto next_line_direction = (line_direction + 1) %
                                     num_line_directions();
    const auto next_coordinate = coordinates[next_line_direction];
    const auto line = lines[line_direction];
    const auto index_of_cell_on_line_ =
                   index_of_cell_on_line(coordinate, next_coordinate);
    line->set_cell(cell, index_of_cell_on_line_);
}

// Set the alphabet, according to the characters in the regexes which
// were previously assigned to the grid lines.
void
Grid::set_alphabet() const
{
    Alphabet::set(explicit_regex_characters());
}

// copying

// Copy to this grid the cell in 'source_grid' at coordinates 'x' and 'y'.
void
Grid::copy_cell(const Grid& source_grid, size_t x, size_t y)
{
    const auto coordinates = source_grid.coordinates(x, y);
    const auto cell_to_copy = source_grid.cell(coordinates);
    const auto cell_copy = make_cell(*cell_to_copy);

    for (size_t line_direction = 0;
         line_direction != num_line_directions();
         ++line_direction)
    {
        place_cell(cell_copy, coordinates, line_direction);
    }
}

// Copy to this grid the cells of 'source_grid'.
void
Grid::copy_cells(const Grid& source_grid)
{
    for (size_t x = 0; x != num_rows(); ++x)
    {
        for (auto y = begin_y(x); y != end_y(x); ++y)
        {
            copy_cell(source_grid, x, y);
        }
    }
}

// Copy to this grid the lines of 'source_grid'.
void
Grid::copy_lines(const Grid& source_grid)
{
    build_lines();
    copy_regexes(source_grid);
}

// Copy to this grid the lines and cells of 'source_grid'.
void
Grid::copy_lines_and_cells(const Grid& source_grid)
{
    copy_lines(source_grid);
    copy_cells(source_grid);
}

// Copy to this grid the regexes of 'source_grid'.
void
Grid::copy_regexes(const Grid& source_grid)
{
    const auto lines = all_lines();
    const auto source_lines = source_grid.all_lines();

    for (size_t i = 0; i != lines.size(); ++i)
    {
        const auto line = lines[i];
        const auto source_line = source_lines[i];

        line->copy_regexes(*source_line);
    }
}

shared_ptr<GridCell>
Grid::make_cell(const GridCell& cell_to_copy)
{
    return make_shared<GridCell>(cell_to_copy);
}

// accessing

vector<shared_ptr<GridCell>>
Grid::all_cells() const
{
    vector<shared_ptr<GridCell>> result;

    for (const auto& row : rows())
    {
        const auto& row_cells = row->cells();
        result.insert(end(result), row_cells.cbegin(), row_cells.cend());
    }

    return result;
}

vector<GridLine*>
Grid::all_lines() const
{
    vector<GridLine*> result;

    for (const auto& lines : m_lines_per_direction)
    {
        for (const auto& line : lines)
        {
            result.push_back(line.get());
        }
    }

    return result;
}

shared_ptr<GridCell>
Grid::cell(const vector<size_t>& coordinates) const
{
    const auto x = coordinates[0];
    const auto y = coordinates[1];

    const auto& rows_ = rows();
    const auto& row = rows_[x];

    const auto index_on_row = index_of_cell_on_line(x, y);

    return row->cell(index_on_row);
}

// Precondition:
// * there is at least one cell in this grid which can be searched
//   (i.e., one cell with several possible characters)
shared_ptr<GridCell>
Grid::cell_to_search() const
{
    const auto cells = all_cells();

    vector<shared_ptr<GridCell>> cells_that_can_be_searched;
    copy_if(cells.cbegin(),
            cells.cend(),
            back_inserter(cells_that_can_be_searched),
            [](shared_ptr<GridCell> cell)
            {
                return cell->has_several_possible_characters();
            });
    assert(!cells_that_can_be_searched.empty());

    // It is heuristically best to search a cell with the least number
    // of possible characters, since the probability to find a solution
    // by selecting a character at random from such a cell is the
    // greatest.
    return *min_element(cells_that_can_be_searched.cbegin(),
                        cells_that_can_be_searched.cend(),
                        [](shared_ptr<GridCell> cell_1,
                           shared_ptr<GridCell> cell_2)
                        {
                            return cell_1->num_possible_characters() <
                                   cell_2->num_possible_characters();
                        });
}

// Return the characters which appear explicitly in the regexes of this
// grid.
string
Grid::explicit_regex_characters() const
{
    const auto lines = all_lines();

    return accumulate(lines.cbegin(),
                      lines.cend(),
                      string(),
                      [](const string& sum, const GridLine* line)
                      {
                          return sum + line->explicit_regex_characters();
                      });
}

GridLine*
Grid::line_at(size_t line_direction, size_t line_index) const
{
    assert(line_direction < num_line_directions());
    const auto& lines_for_direction = m_lines_per_direction[line_direction];

    assert(line_index < lines_for_direction.size());
    return lines_for_direction[line_index].get();
}

// Return the lines that go through the cell at 'coordinates'.
vector<GridLine*>
Grid::lines_through(const vector<size_t>& coordinates) const
{
    vector<GridLine*> result;

    size_t line_direction = 0;

    for (auto coordinate : coordinates)
    {
        result.push_back(
                 m_lines_per_direction[line_direction][coordinate].get());
        ++line_direction;
    }

    return result;
}

// Return the maximum length of the possible characters of cells, when
// printed.
//
// For example, if this grid has two cells with possible characters
// "ABC" (cell 1) and "AB" (cell 2), then:
// * cell 1 is represented as "ABC", which has length 3,
// * cell 2 is represented as "AB", which has length 2,
// so this function would return 3.
size_t
Grid::max_length_of_possible_characters_as_string() const
{
    const auto cells = all_cells();

    const auto& max_cell =
        *max_element(
           cells.cbegin(),
           cells.cend(),
           [](const shared_ptr<GridCell>& cell_1,
              const shared_ptr<GridCell>& cell_2)
           {
               return cell_1->possible_characters_as_string().size() <
                      cell_2->possible_characters_as_string().size();
           });

    return max_cell->possible_characters_as_string().size();
}

GridLine*
Grid::row_at(size_t row_index) const
{
    assert(row_index < num_rows());
    return rows()[row_index].get();
}

const vector<unique_ptr<GridLine>>&
Grid::rows() const
{
    return m_lines_per_direction[0];
}

// querying

bool
Grid::is_solved() const
{
    const auto cells = all_cells();

    return all_of(cells.cbegin(),
                  cells.cend(),
                  [](const shared_ptr<GridCell>& cell)
                  {
                      return cell->is_solved();
                  });
}

// printing

// Log the result of the constrain that was done to this grid. The
// constrain was either successful or unsuccessful, according to
// 'success'.
void
Grid::log_constrain_result(bool success) const
{
    LOG_BLANK_LINE();

    if (success)
    {
        LOG("grid was successfully constrained to:");
        INCREMENT_LOGGING_INDENTATION_LEVEL();
        LOG(print_verbose());
        DECREMENT_LOGGING_INDENTATION_LEVEL();
    }
    else
    {
        LOG("grid could not be constrained");
    }
}

vector<string>
Grid::print() const
{
    const auto verbose = false;
    return do_print(verbose);
}

vector<string>
Grid::print_verbose() const
{
    const auto verbose = true;
    return do_print(verbose);
}

void
Grid::report_solutions(const vector<unique_ptr<Grid>>& solutions,
                       unsigned int                    num_solutions_to_find)
{
    const auto header_lines = report_header(solutions, num_solutions_to_find);

    for (const auto& header_line : header_lines)
    {
        cout << header_line << endl;
    }

    for (const auto& solution : solutions)
    {
        cout << endl;
        cout << *solution;
    }
}

ostream&
operator<<(ostream& os, const Grid& grid)
{
    for (const auto& line : grid.print())
    {
        os << line << endl;
    }

    return os;
}

// modifying

// Constrain this grid.
//
// Return true if no contradiction was found. The grid, however, may not
// have been solved yet: some cells may still contain several possible
// characters.
//
// Return false if a contradiction was found (i.e., if this grid cannot
// be solved).
bool
Grid::constrain()
{
    LOG_BLANK_LINE();
    LOG("constraining grid:");
    INCREMENT_LOGGING_INDENTATION_LEVEL();
    LOG(print_verbose());
    DECREMENT_LOGGING_INDENTATION_LEVEL();

    INCREMENT_LOGGING_INDENTATION_LEVEL();

    const auto result = constrain_no_log();

    DECREMENT_LOGGING_INDENTATION_LEVEL();
    log_constrain_result(result);

    return result;
}

// Same as constrain(), except that this version does not (directly) log.
bool
Grid::constrain_no_log()
{
    const auto lines = all_lines();
    const auto num_lines = lines.size();
    size_t num_consecutive_lines_constrained_without_changes = 0;
    size_t line_index = 0;

    while (num_consecutive_lines_constrained_without_changes != num_lines)
    {
        auto line = lines[line_index];

        const auto line_constraint_was_changed = line->constrain();

        if (line_constraint_was_changed)
        {
            if (line->has_impossible_constraint())
            {
                return false;
            }
            else
            {
                num_consecutive_lines_constrained_without_changes = 0;
            }
        }
        else
        {
            ++num_consecutive_lines_constrained_without_changes;
        }

        line_index = (line_index + 1) % num_lines;
    }

    return true;
}

// Optimize the regexes of this grid according to 'optimizations'.
void
Grid::optimize(const RegexOptimizations& optimizations)
{
    for (auto line : all_lines())
    {
        line->optimize(optimizations);
    }
}

// Return the solved grid(s) obtained from this grid by searching
// 'cell'.
vector<unique_ptr<Grid>>
Grid::search_cell(const GridCell& cell,
                  unsigned int&   num_remaining_solutions_to_find)
{
    vector<unique_ptr<Grid>> solutions;

    for (auto possible_character : cell.possible_characters())
    {
        auto solutions_for_character =
            search_cell(cell,
                        possible_character,
                        num_remaining_solutions_to_find);

        solutions.insert(end(solutions),
                         make_move_iterator(solutions_for_character.begin()),
                         make_move_iterator(solutions_for_character.end()));

        if (num_remaining_solutions_to_find == 0)
        {
            return solutions;
        }
    }

    return solutions;
}

// Return the solved grid(s) obtained from this grid by constraining
// 'cell' to contain 'c'.
vector<unique_ptr<Grid>>
Grid::search_cell(const GridCell& cell,
                  char            c,
                  unsigned int&   num_remaining_solutions_to_find)
{
    LOG_BLANK_LINE();
    LOG("searching cell:");
    INCREMENT_LOGGING_INDENTATION_LEVEL();

    auto copy_of_this_grid = clone();
    auto cell_in_copy = copy_of_this_grid->cell(cell.coordinates());

    LOG(cell_in_copy->to_string() + ": " +
        cell_in_copy->possible_characters_as_string() + " => " + c);

    cell_in_copy->set_possible_characters(c);
    auto result =
        copy_of_this_grid->solve_no_log(num_remaining_solutions_to_find);

    DECREMENT_LOGGING_INDENTATION_LEVEL();
    return result;
}

vector<unique_ptr<Grid>>
Grid::search_grid(unsigned int& num_remaining_solutions_to_find)
{
    LOG_BLANK_LINE();
    LOG("searching grid:");
    INCREMENT_LOGGING_INDENTATION_LEVEL();
    LOG(print_verbose());

    auto result = search_cell(*cell_to_search(),
                              num_remaining_solutions_to_find);

    DECREMENT_LOGGING_INDENTATION_LEVEL();
    return result;
}

// Solve this grid. Return no more than 'num_solutions_to_find'
// solutions.
//
// Precondition:
// * num_solutions_to_find != 0
vector<unique_ptr<Grid>>
Grid::solve(unsigned int num_solutions_to_find)
{
    assert(num_solutions_to_find != 0);

    LOG_BLANK_LINE();
    LOG("attemping to solve this grid:");
    INCREMENT_LOGGING_INDENTATION_LEVEL();
    LOG(print_verbose());
    DECREMENT_LOGGING_INDENTATION_LEVEL();

    auto num_remaining_solutions_to_find = num_solutions_to_find;
    auto solutions = solve_no_log(num_remaining_solutions_to_find);

    log_solutions(solutions, num_solutions_to_find);

    return solutions;
}

// Same as solve(), except that this version does not (directly) log.
vector<unique_ptr<Grid>>
Grid::solve_no_log(unsigned int& num_remaining_solutions_to_find)
{
    if (!constrain())
    {
        // No solutions.
        return {};
    }

    if (is_solved())
    {
        Utils::print_verbose_message(cout, "found a solution");

        vector<unique_ptr<Grid>> result;

        result.push_back(clone());

        assert(num_remaining_solutions_to_find != 0);
        --num_remaining_solutions_to_find;

        return result;
    }

    return search_grid(num_remaining_solutions_to_find);
}
