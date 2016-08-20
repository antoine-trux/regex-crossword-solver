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


#include "grid_line.hpp"

#include "grid.hpp"
#include "grid_cell.hpp"
#include "grid_line_regex.hpp"
#include "logger.hpp"
#include "regex.hpp"
#include "utils.hpp"

#include <algorithm>
#include <cassert>
#include <iterator>
#include <numeric>

using namespace std;


// instance creation and deletion

GridLine::GridLine(const Grid& grid,
                   size_t      line_direction,
                   size_t      line_index_within_direction,
                   size_t      num_cells) :
  m_grid(grid),
  m_direction(line_direction),
  m_index_within_direction(line_index_within_direction),
  m_cells(num_cells, nullptr)
{
}

GridLine::~GridLine() = default;

void
GridLine::build_regexes(const vector<string>& regexes_as_strings)
{
    m_grid_line_regexes.clear();

    transform(regexes_as_strings.cbegin(),
              regexes_as_strings.cend(),
              back_inserter(m_grid_line_regexes),
              [](const string& regex_as_string)
              {
                  return GridLineRegex(regex_as_string);
              });
}

void
GridLine::set_cell(shared_ptr<GridCell> cell, size_t index_of_cell_on_line)
{
    m_cells[index_of_cell_on_line] = cell;
}

// copying

void
GridLine::copy_regexes(const GridLine& rhs)
{
    m_grid_line_regexes = rhs.m_grid_line_regexes;
    m_saved_constraint = rhs.m_saved_constraint;
}

// accessing

shared_ptr<GridCell>
GridLine::cell(size_t cell_index)
{
    return m_cells[cell_index];
}

const vector<shared_ptr<GridCell>>&
GridLine::cells() const
{
    return m_cells;
}

// Return the constraint of this line, as computed from each of its
// cells.
Constraint
GridLine::constraint_from_cells() const
{
    vector<SetOfCharacters> constraint;
    transform(m_cells.cbegin(),
              m_cells.cend(),
              back_inserter(constraint),
              [](const shared_ptr<GridCell>& cell)
              {
                  return cell->possible_characters();
              });
    return Constraint(constraint);
}

// Return the characters which appear explicitly in the regexes of this
// line.
string
GridLine::explicit_regex_characters() const
{
    return accumulate(m_grid_line_regexes.cbegin(),
                      m_grid_line_regexes.cend(),
                      string(),
                      [](const string&        sum,
                         const GridLineRegex& grid_line_regex)
                      {
                          return sum + grid_line_regex.explicit_characters();
                      });
}

size_t
GridLine::num_cells() const
{
    return m_cells.size();
}

string
GridLine::regexes_as_string() const
{
    assert(!m_grid_line_regexes.empty());
    const string separator = ", ";

    string result =
        accumulate(m_grid_line_regexes.cbegin(),
                   m_grid_line_regexes.cend(),
                   string(),
                   [&separator](const string&        sum,
                                const GridLineRegex& grid_line_regex)
                   {
                       return sum                                        +
                              Utils::quoted(grid_line_regex.as_string()) +
                              separator;
                   });

    // Remove the last separator from 'result'.
    result.resize(result.size() - separator.size());
    return result;
}

// querying

bool
GridLine::has_impossible_constraint() const
{
    return m_saved_constraint.is_impossible();
}

// printing

vector<string>
GridLine::print_verbose_grid() const
{
    return m_grid.print_verbose();
}

// converting

string
GridLine::to_string() const
{
    return "line(" + Utils::to_string(m_direction) + ", "     +
            Utils::to_string(m_index_within_direction) + ", " +
            regexes_as_string() + ')';
}

// modifying

// Constrain this line with the contents of its cells, and return
// whether the constraint was changed.
//
// For example, suppose this line contains three cells, has the single
// regex '(A|C)*B', and the current cell contents is { "ABC", "ABC",
// "ABC" } (i.e., the possible characters for each cell are A, B and C).
// Then, this function would:
// * update the possible characters of the cells to { "AC", "AC", "B" }
// * return true
bool
GridLine::constrain()
{
    assert(m_saved_constraint.is_possible());

    if (m_saved_constraint == constraint_from_cells())
    {
        LOG_BLANK_LINE();
        LOG("not constraining " + to_string() +
            ", because line constraints have not changed since last time");
        return false;
    }

    const auto new_constraint = constrain_regexes();
    const auto constraint_was_changed =
        (new_constraint != constraint_from_cells());
    m_saved_constraint = new_constraint;

    if (new_constraint.is_impossible())
    {
        LOG_BLANK_LINE();
        LOG("impossible constraint for " + to_string());
        return constraint_was_changed;
    }

    if (constraint_was_changed)
    {
        LOG_BLANK_LINE();
        LOG("updating cells for " + to_string());

        update_cells(new_constraint);

        LOG("new grid:");
        INCREMENT_LOGGING_INDENTATION_LEVEL();
        LOG(print_verbose_grid());
        DECREMENT_LOGGING_INDENTATION_LEVEL();
    }
    else
    {
        LOG_BLANK_LINE();
        LOG("no cells were updated for " + to_string());
    }

    return constraint_was_changed;
}

// Constrain this line with its regex(es), and return the new
// constraint.
Constraint
GridLine::constrain_regexes()
{
    auto constraint = constraint_from_cells();

    for (auto& grid_line_regex : m_grid_line_regexes)
    {
        constraint = grid_line_regex.constrain(constraint);

        if (constraint.is_impossible())
        {
            break;
        }
    }

    return constraint;
}

// Optimize the regex(es) of this line according to 'optimizations'.
void
GridLine::optimize(const RegexOptimizations& optimizations)
{
    for (auto& grid_line_regex : m_grid_line_regexes)
    {
        grid_line_regex.optimize(optimizations);
    }
}

// Update the cells of this line with 'new_constraint'.
void
GridLine::update_cells(const Constraint& new_constraint)
{
    INCREMENT_LOGGING_INDENTATION_LEVEL();

    const auto num_cells_ = num_cells();

    for (size_t cell_index = 0; cell_index != num_cells_; ++cell_index)
    {
        const auto cell_ = cell(cell_index);

        const auto& possible_characters_for_cell = new_constraint[cell_index];

        const auto current_possible_characters = cell_->possible_characters();
        const auto new_possible_characters = current_possible_characters &
                                             possible_characters_for_cell;

        if (new_possible_characters != current_possible_characters)
        {
            LOG("updating " + cell_->to_string() + ": " +
                Utils::quoted(current_possible_characters.to_string()) +
                " => " +
                Utils::quoted(new_possible_characters.to_string()));
            cell_->set_possible_characters(new_possible_characters);
        }
    }

    DECREMENT_LOGGING_INDENTATION_LEVEL();
}
