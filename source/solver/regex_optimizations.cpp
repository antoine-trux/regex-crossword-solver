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


#include "regex_optimizations.hpp"

#include <cassert>


// instance creation and deletion

RegexOptimizations
RegexOptimizations::all()
{
    return RegexOptimizations(true, true, true);
}

RegexOptimizations
RegexOptimizations::none()
{
    return RegexOptimizations(false, false, false);
}

RegexOptimizations::RegexOptimizations(bool optimize_concatenations,
                                       bool optimize_groups,
                                       bool optimize_unions) :
  m_optimize_concatenations(optimize_concatenations),
  m_optimize_groups(optimize_groups),
  m_optimize_unions(optimize_unions)
{
}

// querying

bool
RegexOptimizations::optimize_concatenations() const
{
    return m_optimize_concatenations;
}

bool
RegexOptimizations::optimize_groups() const
{
    return m_optimize_groups;
}

bool
RegexOptimizations::optimize_unions() const
{
    return m_optimize_unions;
}

// modifying

void
RegexOptimizations::set(Type type, bool on_or_off)
{
    switch (type)
    {
    case Type::CONCATENATIONS:
        m_optimize_concatenations = on_or_off;
        break;

    case Type::GROUPS:
        m_optimize_groups = on_or_off;
        break;

    case Type::UNIONS:
        m_optimize_unions = on_or_off;
        break;

    default:
        assert(false);
        break;
    }
}
