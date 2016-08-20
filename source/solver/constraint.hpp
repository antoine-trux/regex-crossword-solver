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


#ifndef CONSTRAINT_HPP
#define CONSTRAINT_HPP

#include "set_of_characters.hpp"

#include <cstddef>
#include <initializer_list>
#include <string>
#include <vector>


// An instance of this class represents a line constraint, that is, a
// sequence of sets of characters, where each set of characters contains
// the possible characters for the corresponding cell of the line.
//
// For example, constraint { "ABC", "AB" } can be associated with a
// two-cell line:
// * the possible characters for the first cell are A, B, and C,
// * the possible characters for the second cell are A and B.
//
// A string s of length n is said to satisfy constraint c of same
// length if, for each index i in [0, n), s[i] belongs to c[i].
//
// For example, there are four strings which satisfy constraint
// { "AB, "A", "AC }: "AAA", "AAC", "BAA" and "BAC".
//
// If any element of a constraint is the empty set, the constraint is
// said to be impossible, since no string satisfies such a constraint.
// For example, constraint { "AB", "", "AC" } is impossible.
class Constraint final
{
public:
    // instance creation and deletion
    Constraint();
    explicit Constraint(const std::vector<SetOfCharacters>& constraint);
    explicit Constraint(const std::initializer_list<std::string>& elements);
    static Constraint all(size_t size);
    static Constraint none(size_t size);

    // accessing
    SetOfCharacters& operator[](size_t i);
    const SetOfCharacters& operator[](size_t i) const;
    size_t size() const;

    // querying
    bool empty() const;
    bool is_impossible() const;
    bool is_possible() const;
    bool is_tighter_than_or_equal_to(const Constraint& rhs) const;

    // modifying
    Constraint& operator|=(const Constraint& rhs);

private:
    friend bool operator==(const Constraint& lhs, const Constraint& rhs);
    friend bool operator< (const Constraint& lhs, const Constraint& rhs);

    // data members

    std::vector<SetOfCharacters> m_constraint;
};

bool operator==(const Constraint& lhs, const Constraint& rhs);
bool operator!=(const Constraint& lhs, const Constraint& rhs);

bool operator<(const Constraint& lhs, const Constraint& rhs);

Constraint operator|(const Constraint& lhs, const Constraint& rhs);


#endif // CONSTRAINT_HPP
