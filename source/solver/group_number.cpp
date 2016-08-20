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


#include "group_number.hpp"

#include "utils.hpp"

#include <cassert>

using namespace std;


// instance creation and deletion

// Precondition:
// * 'value' is not 0
GroupNumber::GroupNumber(unsigned int value) :
  m_value(value)
{
    // Group numbers start at 1.
    assert(value != 0);
}

// accessing

unsigned int
GroupNumber::value() const
{
    return m_value;
}

// querying

bool
GroupNumber::exceeds_max_backreference_value() const
{
    return m_value > max_backreference_value();
}

bool
operator==(const GroupNumber& lhs, const GroupNumber& rhs)
{
    return lhs.m_value == rhs.m_value;
}

bool
operator!=(const GroupNumber& lhs, const GroupNumber& rhs)
{
    return !(lhs == rhs);
}

bool
operator>=(const GroupNumber& lhs, const GroupNumber& rhs)
{
    return lhs.m_value >= rhs.m_value;
}

// converting

string
GroupNumber::to_string() const
{
    return Utils::to_string(m_value);
}

// modifying

GroupNumber&
GroupNumber::operator++()
{
    ++m_value;
    return *this;
}
