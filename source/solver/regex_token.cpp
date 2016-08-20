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


#include "regex_token.hpp"

#include <cassert>

using namespace std;


// instance creation and deletion

RegexToken::RegexToken(Type type) :
  m_type(type)
{
}

RegexToken::RegexToken(const RegexToken& rhs) :
  m_type(rhs.m_type)
{
    copy_union(rhs);
}

RegexToken::~RegexToken()
{
    if (m_type == Type::INVALID)
    {
        m_error_message.~string();
    }
}

RegexToken&
RegexToken::operator=(const RegexToken& rhs)
{
    if (&rhs == this)
    {
        return *this;
    }

    if (m_type == Type::INVALID && rhs.m_type != Type::INVALID)
    {
        m_error_message.~string();
    }

    if (m_type == Type::INVALID && rhs.m_type == Type::INVALID)
    {
        m_error_message = rhs.m_error_message;
    }
    else
    {
        copy_union(rhs);
    }

    m_type = rhs.m_type;

    return *this;
}

void
RegexToken::copy_union(const RegexToken& rhs)
{
    switch (rhs.m_type)
    {
    case Type::BACKREFERENCE:
        m_group_number = rhs.m_group_number;
        break;

    case Type::INVALID:
        new (&m_error_message) string(rhs.m_error_message);
        break;

    case Type::REPETITION_COUNT:
        m_repetition_count = rhs.m_repetition_count;
        break;

    case Type::SINGLE_CHARACTER:
        m_character = rhs.m_character;
        break;

    default:
        // The union is not used.
        break;
    }
}

RegexToken
RegexToken::create_backreference_token(const GroupNumber& group_number)
{
    auto token = RegexToken(Type::BACKREFERENCE);
    token.m_group_number = group_number;
    return token;
}

RegexToken
RegexToken::create_invalid_token(const string& error_message)
{
    auto token = RegexToken(Type::INVALID);
    new (&token.m_error_message) string(error_message);
    return token;
}

RegexToken
RegexToken::create_repetition_count_token(
              const RepetitionCount& repetition_count)
{
    auto token = RegexToken(Type::REPETITION_COUNT);
    token.m_repetition_count = repetition_count;
    return token;
}

RegexToken
RegexToken::create_single_character_token(char c)
{
    auto token = RegexToken(Type::SINGLE_CHARACTER);
    token.m_character = c;
    return token;
}

RegexToken
RegexToken::create_token(Type type)
{
    // These token types have their own factory function.
    assert(type != Type::BACKREFERENCE);
    assert(type != Type::INVALID);
    assert(type != Type::REPETITION_COUNT);
    assert(type != Type::SINGLE_CHARACTER);

    return RegexToken(type);
}

// accessing

char
RegexToken::character() const
{
    assert(m_type == Type::SINGLE_CHARACTER);
    return m_character;
}

string
RegexToken::error_message() const
{
    assert(m_type == Type::INVALID);
    return m_error_message;
}

GroupNumber
RegexToken::group_number() const
{
    assert(m_type == Type::BACKREFERENCE);
    return m_group_number;
}

RepetitionCount
RegexToken::repetition_count() const
{
    assert(m_type == Type::REPETITION_COUNT);
    return m_repetition_count;
}

RegexToken::Type
RegexToken::type() const
{
    return m_type;
}

// querying

bool
RegexToken::is_negated_shorthand_character(Type type)
{
    return type == Type::SHORTHAND_NOT_DIGIT_CHARACTER ||
           type == Type::SHORTHAND_NOT_SPACE_CHARACTER ||
           type == Type::SHORTHAND_NOT_WORD_CHARACTER;
}

bool
RegexToken::is_non_negated_shorthand_character(Type type)
{
    return type == Type::SHORTHAND_DIGIT_CHARACTER ||
           type == Type::SHORTHAND_SPACE_CHARACTER ||
           type == Type::SHORTHAND_WORD_CHARACTER;
}

bool
RegexToken::is_shorthand_character(Type type)
{
    return is_negated_shorthand_character(type) ||
           is_non_negated_shorthand_character(type);
}

bool
RegexToken::is_shorthand_character() const
{
    return is_shorthand_character(m_type);
}
