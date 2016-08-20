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


#include "repetition_count.hpp"

#include "utils.hpp"

#include <cassert>

using namespace std;


// instance creation and deletion

RepetitionCount::RepetitionCount(size_t count) :
  RepetitionCount(false, count)
{
}

RepetitionCount::RepetitionCount(bool is_infinite, size_t count) :
  m_is_infinite(is_infinite),
  m_count(count)
{
}

RepetitionCount
RepetitionCount::infinite()
{
    return RepetitionCount(true, 0);
}

// accessing

RepetitionCount
operator+(const RepetitionCount& a, const RepetitionCount& b)
{
    if (a.m_is_infinite || b.m_is_infinite)
    {
        return RepetitionCount::infinite();
    }
    else
    {
        return a.m_count + b.m_count;
    }
}

// Preconditions:
// * 'b' is not infinite
// * a >= b
RepetitionCount
operator-(const RepetitionCount& a, const RepetitionCount& b)
{
    assert(!b.m_is_infinite);
    assert(b <= a);

    if (a.m_is_infinite)
    {
        return RepetitionCount::infinite();
    }
    else
    {
        return a.m_count - b.m_count;
    }
}

// querying

bool
RepetitionCount::is_not_infinite() const
{
    return !m_is_infinite;
}

bool
operator==(const RepetitionCount& a, const RepetitionCount& b)
{
    if (a.m_is_infinite)
    {
        return b.m_is_infinite;
    }
    else if (b.m_is_infinite)
    {
        return false;
    }
    else
    {
        return a.m_count == b.m_count;
    }
}

bool
operator!=(const RepetitionCount& a, const RepetitionCount& b)
{
    return !(a == b);
}

bool
operator<=(const RepetitionCount& a, const RepetitionCount& b)
{
    if (a.m_is_infinite)
    {
        return b.m_is_infinite;
    }
    else if (b.m_is_infinite)
    {
        return true;
    }
    else
    {
        return a.m_count <= b.m_count;
    }
}

bool
operator<(const RepetitionCount& a, const RepetitionCount& b)
{
    if (a.m_is_infinite)
    {
        return false;
    }
    else if (b.m_is_infinite)
    {
        return true;
    }
    else
    {
        return a.m_count < b.m_count;
    }
}

// converting

string
RepetitionCount::to_string() const
{
    if (m_is_infinite)
    {
        return "infinite";
    }
    else
    {
        return Utils::to_string(m_count);
    }
}
