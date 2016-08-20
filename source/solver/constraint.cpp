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


#include "constraint.hpp"

#include "alphabet.hpp"

#include <algorithm>
#include <cassert>
#include <iterator>

using namespace std;


// instance creation and deletion

Constraint::Constraint() :
  Constraint({})
{
}

Constraint::Constraint(const vector<SetOfCharacters>& constraint) :
  m_constraint(constraint)
{
}

Constraint::Constraint(const initializer_list<string>& elements)
{
    transform(begin(elements),
              end(elements),
              back_inserter(m_constraint),
              [](const string& s)
              {
                  return SetOfCharacters(s);
              });
}

// Return a constraint with 'size' elements, where each element
// contains all the alphabet characters.
Constraint
Constraint::all(size_t size)
{
    return Constraint(vector<SetOfCharacters>(size, Alphabet::characters()));
}

// Return a constraint with 'size' elements, where each element is an
// empty set of characters.
Constraint
Constraint::none(size_t size)
{
    return Constraint(vector<SetOfCharacters>(size, SetOfCharacters()));
}

// accessing

SetOfCharacters&
Constraint::operator[](size_t i)
{
    // As Scott Meyers writes in one of his books:
    //
    //     The result may not win any beauty contests, but it has the
    //     desired effect of avoiding code duplication by implementing
    //     the non const version of operator[] in terms of the const
    //     version.

    return const_cast<SetOfCharacters&>(
               static_cast<const Constraint&>(*this)[i]);
}

const SetOfCharacters&
Constraint::operator[](size_t i) const
{
    return m_constraint[i];
}

size_t
Constraint::size() const
{
    return m_constraint.size();
}

// Precondition:
// * 'lhs' and 'rhs' have the same size
Constraint
operator|(const Constraint& lhs, const Constraint& rhs)
{
    auto result = lhs;
    result |= rhs;
    return result;
}

// querying

bool
operator==(const Constraint& lhs, const Constraint& rhs)
{
    return lhs.m_constraint == rhs.m_constraint;
}

bool
operator!=(const Constraint& lhs, const Constraint& rhs)
{
    return !(lhs == rhs);
}

// Used by unit tests.
bool
operator<(const Constraint& lhs, const Constraint& rhs)
{
    return lexicographical_compare(lhs.m_constraint.begin(),
                                   lhs.m_constraint.end(),
                                   rhs.m_constraint.begin(),
                                   rhs.m_constraint.end());
}

// Return whether this constraint is of size zero.
bool
Constraint::empty() const
{
    return m_constraint.empty();
}

// For an definition of "impossible", see the class description.
bool
Constraint::is_impossible() const
{
    return any_of(m_constraint.cbegin(),
                  m_constraint.cend(),
                  [](const SetOfCharacters& set_of_characters)
                  {
                      return set_of_characters.empty();
                  });
}

bool
Constraint::is_possible() const
{
    return !is_impossible();
}

// Return whether, for each index i in [0, size()), the element at index
// i of this constraint is a (not necessarily strict) subset of 'rhs[i]'.
//
// Precondition:
// * 'rhs' has the same size as this constraint
bool
Constraint::is_tighter_than_or_equal_to(const Constraint& rhs) const
{
    const auto size_ = size();
    assert(size_ == rhs.size());

    for (size_t i = 0; i != size_; ++i)
    {
        if (!rhs[i].contains((*this)[i]))
        {
            return false;
        }
    }

    return true;
}

// modifying

// Precondition:
// * 'rhs' has the same size as this constraint
Constraint&
Constraint::operator|=(const Constraint& rhs)
{
    const auto size_ = size();
    assert(size_ == rhs.size());

    for (size_t i = 0; i != size_; ++i)
    {
        (*this)[i] |= rhs[i];
    }

    return *this;
}
