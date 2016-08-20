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


#include "grid_cell.hpp"

#include "alphabet.hpp"
#include "utils.hpp"

using namespace std;


// instance creation and deletion

GridCell::GridCell(const vector<size_t>& coordinates) :
  m_coordinates(coordinates)
{
}

// accessing

const vector<size_t>&
GridCell::coordinates() const
{
    return m_coordinates;
}

size_t
GridCell::num_possible_characters() const
{
    return m_possible_characters.size();
}

SetOfCharacters
GridCell::possible_characters() const
{
    return m_possible_characters;
}

string
GridCell::possible_characters_as_string() const
{
    return m_possible_characters.to_string();
}

// querying

bool
GridCell::has_several_possible_characters() const
{
    return num_possible_characters() > 1;
}

bool
GridCell::is_solved() const
{
    return num_possible_characters() == 1;
}

// converting

string
GridCell::to_string() const
{
    string result = "cell(";

    auto it = m_coordinates.cbegin();

    result += Utils::to_string(*it++);

    while (it != m_coordinates.cend())
    {
        result += ", " + Utils::to_string(*it++);
    }

    result += ')';

    return result;
}

// modifying

void
GridCell::set_possible_characters(const SetOfCharacters& possible_characters)
{
    m_possible_characters = possible_characters;
}

void
GridCell::set_possible_characters(char c)
{
    set_possible_characters(SetOfCharacters(c));
}

void
GridCell::set_possible_characters_to_all_characters()
{
    set_possible_characters(Alphabet::characters());
}
