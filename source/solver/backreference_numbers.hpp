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


#ifndef BACKREFERENCE_NUMBERS_HPP
#define BACKREFERENCE_NUMBERS_HPP

#include "group_number.hpp"

#include <bitset>


// An instance of this class represents a set of backreferenced group
// numbers.
class BackreferenceNumbers final
{
public:
    // instance creation and deletion
    BackreferenceNumbers() = default;

    // querying
    bool contains(const GroupNumber& group_number) const;

    // modifying
    void add(const GroupNumber& group_number);

private:
    // Group number 0 is not used, hence '+ 1'.
    std::bitset<GroupNumber::max_backreference_value() + 1> m_bitset;
};


#endif // BACKREFERENCE_NUMBERS_HPP
