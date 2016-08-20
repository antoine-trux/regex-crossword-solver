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


#ifndef REPETITION_COUNT_HPP
#define REPETITION_COUNT_HPP

#include <cstddef>
#include <string>


// An instance of this class represents how many times a (part of a)
// regex is repeated. Such a count can be finite or infinite.
//
// For example, in regex 'A*B{2,4}':
// * 'A' has a minimum repetition count equal to 0, and an infinite
//   repetition count
// * 'B' has a minimum repetition count equal to 2, and a maximum
//   repetition count equal to 4
class RepetitionCount final
{
public:
    // instance creation and deletion
    RepetitionCount(size_t count);
    static RepetitionCount infinite();

    // querying
    bool is_not_infinite() const;

    // converting
    std::string to_string() const;

private:
    friend RepetitionCount operator+(const RepetitionCount& a,
                                     const RepetitionCount& b);
    friend RepetitionCount operator-(const RepetitionCount& a,
                                     const RepetitionCount& b);
    friend bool operator==(const RepetitionCount& a, const RepetitionCount& b);
    friend bool operator<=(const RepetitionCount& a, const RepetitionCount& b);
    friend bool operator<(const RepetitionCount& a, const RepetitionCount& b);

    // instance creation and deletion
    RepetitionCount(bool is_infinite, size_t count);

    // data members

    // Whether this repetition count represents infinity.
    bool m_is_infinite;

    // The finite value of this repetition count if 'm_is_infinite' is
    // false, undefined if 'm_is_infinite' is true.
    size_t m_count;
};

// accessing
RepetitionCount operator+(const RepetitionCount& a, const RepetitionCount& b);
RepetitionCount operator-(const RepetitionCount& a, const RepetitionCount& b);

// querying
bool operator==(const RepetitionCount& a, const RepetitionCount& b);
bool operator!=(const RepetitionCount& a, const RepetitionCount& b);
bool operator<=(const RepetitionCount& a, const RepetitionCount& b);
bool operator<(const RepetitionCount& a, const RepetitionCount& b);


#endif // REPETITION_COUNT_HPP
