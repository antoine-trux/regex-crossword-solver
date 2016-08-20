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


#include "grid_line_regex.hpp"

#include "constraint.hpp"
#include "regex.hpp"

using namespace std;


// instance creation and deletion

GridLineRegex::GridLineRegex(const string& regex_as_string) :
  m_regex_as_string(regex_as_string),
  m_regex(is_universal_regex(regex_as_string) ?
          nullptr                             :
          Regex::parse(regex_as_string))
{
}

GridLineRegex::GridLineRegex(const GridLineRegex& rhs) :
  m_regex_as_string(rhs.m_regex_as_string),
  m_regex(rhs.m_regex ? rhs.m_regex->clone() : nullptr)
{
}

GridLineRegex::GridLineRegex(GridLineRegex&& rhs) noexcept :
  GridLineRegex()
{
    swap(*this, rhs);
}

GridLineRegex&
GridLineRegex::operator=(GridLineRegex rhs)
{
    swap(*this, rhs);
    return *this;
}

void swap(GridLineRegex& lhs, GridLineRegex& rhs) noexcept
{
    using std::swap;

    swap(lhs.m_regex_as_string, rhs.m_regex_as_string);
    swap(lhs.m_regex,           rhs.m_regex);
}

// accessing

string
GridLineRegex::as_string() const
{
    return m_regex_as_string;
}

string
GridLineRegex::explicit_characters() const
{
    if (is_universal_regex())
    {
        return "";
    }

    return m_regex->explicit_characters();
}

// querying

bool
GridLineRegex::is_universal_regex(const string& regex_as_string)
{
    // '.*' matches all strings, and thus constrains nothing.
    return regex_as_string == ".*";
}

bool
GridLineRegex::is_universal_regex() const
{
    return m_regex == nullptr;
}

// modifying

Constraint
GridLineRegex::constrain(const Constraint& constraint)
{
    if (is_universal_regex())
    {
        return constraint;
    }

    return m_regex->constrain(constraint);
}

void
GridLineRegex::optimize(const RegexOptimizations& optimizations)
{
    if (!is_universal_regex())
    {
        m_regex = Regex::optimize(move(m_regex), optimizations);
    }
}
