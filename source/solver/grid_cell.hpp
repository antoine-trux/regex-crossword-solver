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


#ifndef GRID_CELL_HPP
#define GRID_CELL_HPP

#include "set_of_characters.hpp"

#include <vector>


// An instance of this class represents a cell in a grid.
//
// At the beginning, each cell is assigned all the possible characters
// (i.e., all the characters of the alphabet). Then, the set of possible
// characters for a cell is reduced, as constraints are applied. A
// solved grid has all its cells containing one single possible
// character.
class GridCell final
{
public:
    // instance creation and deletion
    explicit GridCell(const std::vector<size_t>& coordinates);
    GridCell(const GridCell& /*rhs*/) = default;

    // accessing
    const std::vector<size_t>& coordinates() const;
    size_t num_possible_characters() const;
    SetOfCharacters possible_characters() const;
    std::string possible_characters_as_string() const;

    // querying
    bool has_several_possible_characters() const;
    bool is_solved() const;

    // converting
    std::string to_string() const;

    // modifying
    void set_possible_characters(const SetOfCharacters& possible_characters);
    void set_possible_characters(char c);
    void set_possible_characters_to_all_characters();

private:
    // data members

    // Refer to the description of classes RectangularGrid and
    // HexagonalGrid for how the cell coordinates are to be interpreted.
    std::vector<size_t> m_coordinates;

    // The characters that this cell may contain.
    SetOfCharacters m_possible_characters;
};


#endif // GRID_CELL_HPP
