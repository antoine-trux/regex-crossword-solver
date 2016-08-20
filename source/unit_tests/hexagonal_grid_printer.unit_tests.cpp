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


#include "alphabet.hpp"
#include "disable_warnings_from_gtest.hpp"
#include "grid_cell.hpp"
#include "grid_line.hpp"
#include "hexagonal_grid.hpp"
#include "hexagonal_grid_printer.hpp"
#include "regex_crossword_solver_test.hpp"

#include <numeric>

using namespace std;


class HexagonalGridPrinterTest : public RegexCrosswordSolverTest
{
};


TEST_F(HexagonalGridPrinterTest, print)
{
    const vector<string>& regexes{
        "A*", "B*", "C*",
        "D*", "E*", "F*",
        "G*", "H*", "I*"
    };

    const size_t num_regexes_per_line = 1;
    HexagonalGrid grid(regexes, num_regexes_per_line);
    auto characters = Alphabet::characters();

    auto row_0 = grid.row_at(0);
    row_0->cell(0)->set_possible_characters(characters);
    characters -= 'A';
    row_0->cell(1)->set_possible_characters(characters);

    auto row_1 = grid.row_at(1);
    characters -= 'B';
    row_1->cell(0)->set_possible_characters(characters);
    characters -= 'C';
    row_1->cell(1)->set_possible_characters(characters);
    characters -= 'D';
    row_1->cell(2)->set_possible_characters(characters);

    auto row_2 = grid.row_at(2);
    characters -= 'E';
    row_2->cell(0)->set_possible_characters(characters);
    // No characters.
    row_2->cell(1)->set_possible_characters(SetOfCharacters());

    const auto verbose = true;
    const auto grid_lines = HexagonalGridPrinter::print(grid, verbose);
    const auto grid_string = accumulate(grid_lines.cbegin(),
                                        grid_lines.cend(),
                                        string(),
                                        [](const string& x, const string& y)
                                        {
                                            return x + y + '\n';
                                        });
    const string expected = R"(   / \ / \   )"               "\n"
                            R"(  |ABC|BCD|  )"               "\n"
                            R"(  |DEF|EFG|  )"               "\n"
                            R"(  |GHI|HI |  )"               "\n"
                            R"( / \ / \ / \ )"               "\n"
                            R"(|CDE|DEF|EFG|)"               "\n"
                            R"(|FGH|GHI|HI |)"               "\n"
                            R"(| I |   |   |)"               "\n"
                            R"( \ / \ / \ / )"               "\n"
                            R"(  |FGH|   |  )"               "\n"
                            R"(  | I |   |  )"               "\n"
                            R"(  |   |   |  )"               "\n"
                            R"(   \ / \ /   )"               "\n"
                            R"(alphabet: 'ABCDEFGHI')"       "\n"
                            R"(regexes:)"                    "\n"
                            R"(  west -> east:)"             "\n"
                            R"(    'A*')"                    "\n"
                            R"(    'B*')"                    "\n"
                            R"(    'C*')"                    "\n"
                            R"(  south-east -> north-west:)" "\n"
                            R"(    'D*')"                    "\n"
                            R"(    'E*')"                    "\n"
                            R"(    'F*')"                    "\n"
                            R"(  north-east -> south-west:)" "\n"
                            R"(    'G*')"                    "\n"
                            R"(    'H*')"                    "\n"
                            R"(    'I*')"                    "\n";

    EXPECT_EQ(expected, grid_string);
}
