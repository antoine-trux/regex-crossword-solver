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


#ifndef GROUP_NUMBER_HPP
#define GROUP_NUMBER_HPP

#include <iosfwd>
#include <string>


// An instance of this class represents a group number.
//
// Groups are delimited by parentheses. Each group is numbered according
// to the position of its opening parenthesis. Numbering starts at 1
// (i.e., there is no group 0).
//
// For example, in regex '((A)B)\1', '\1' references '((A)B)'.
//
// Groups up to 9 can be backreferenced.
class GroupNumber final
{
public:
    // instance creation and deletion
    explicit GroupNumber(unsigned int value);

    // accessing
    constexpr static unsigned int max_backreference_value()
    {
        return 9;
    }
    unsigned int value() const;

    // querying
    bool exceeds_max_backreference_value() const;

    // converting
    std::string to_string() const;

    // modifying
    GroupNumber& operator++();

private:
    friend bool operator==(const GroupNumber& lhs, const GroupNumber& rhs);
    friend bool operator>=(const GroupNumber& lhs, const GroupNumber& rhs);

    // data members

    unsigned int m_value;
};

// querying
bool operator==(const GroupNumber& lhs, const GroupNumber& rhs);
bool operator!=(const GroupNumber& lhs, const GroupNumber& rhs);
bool operator>=(const GroupNumber& lhs, const GroupNumber& rhs);


#endif // GROUP_NUMBER_HPP
