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


#ifndef GRID_LINE_REGEX_HPP
#define GRID_LINE_REGEX_HPP

#include <memory>
#include <string>

class Constraint;
class Regex;
class RegexOptimizations;


// An instance of this class represents a regex in a grid line.
class GridLineRegex final
{
public:
    // instance creation and deletion
    explicit GridLineRegex(const std::string& regex_as_string);
    GridLineRegex(const GridLineRegex& rhs);
    GridLineRegex(GridLineRegex&& rhs) noexcept;
    GridLineRegex& operator=(GridLineRegex rhs);

    // accessing
    std::string as_string() const;
    std::string explicit_characters() const;

    // modifying
    Constraint constrain(const Constraint& constraint);
    void optimize(const RegexOptimizations& optimizations);

private:
    friend void swap(GridLineRegex& lhs, GridLineRegex& rhs) noexcept;

    // instance creation and deletion
    GridLineRegex() = default;

    // querying
    static bool is_universal_regex(const std::string& regex_as_string);
    bool is_universal_regex() const;

    // data members

    // A string representation of this regex.
    std::string m_regex_as_string;

    // The parsed regex, or nullptr if this regex is to be ignored.
    std::unique_ptr<Regex> m_regex;
};


#endif // GRID_LINE_REGEX_HPP
