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


#ifndef REGEX_OPTIMIZATIONS_HPP
#define REGEX_OPTIMIZATIONS_HPP


// An instance of this class represents a set of optimization methods to
// be applied to a regular expression after it has been parsed.
class RegexOptimizations final
{
public:
    enum class Type
    {
        CONCATENATIONS,
        GROUPS,
        UNIONS
    };

    // instance creation and deletion
    static RegexOptimizations all();
    static RegexOptimizations none();

    // querying
    bool optimize_concatenations() const;
    bool optimize_groups() const;
    bool optimize_unions() const;

    // modifying
    void set(Type type, bool on_or_off);

private:
    // instance creation and deletion
    RegexOptimizations(bool optimize_concatenations,
                       bool optimize_groups,
                       bool optimize_unions);

    // data members

    bool m_optimize_concatenations;
    bool m_optimize_groups;
    bool m_optimize_unions;
};


#endif // REGEX_OPTIMIZATIONS_HPP
