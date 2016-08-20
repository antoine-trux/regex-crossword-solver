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


#ifndef REGEX_TOKEN_HPP
#define REGEX_TOKEN_HPP

#include "group_number.hpp"
#include "repetition_count.hpp"


// An instance of this class represents a regex token, which is the
// smallest lexical entity of a regex. Some tokens have an associated
// value, others have none.
//
// For example, the regex '(A|B)\1' contains the following tokens (from
// left to right):
// * OPEN_GROUP       - no associated value
// * SINGLE_CHARACTER - associated to character A
// * OR               - no associated value
// * SINGLE_CHARACTER - associated to character B
// * CLOSE_GROUP      - no associated value
// * BACKREFERENCE    - associated to group number 1
class RegexToken final
{
public:
    enum class Type
    {
        // '.'
        ANY_CHARACTER,

        // '\1', ..., '\9'
        BACKREFERENCE,

        // '-'
        CHARACTER_RANGE_SEPARATOR,

        // ']'
        CLOSE_CHARACTER_CLASS,

        // '}'
        CLOSE_COUNTED_REPETITION,

        // ')'
        CLOSE_GROUP,

        // '$', '\Z'
        EPSILON_AT_END,

        // '^', '\A'
        EPSILON_AT_START,

        // '\b'
        EPSILON_AT_WORD_BOUNDARY,

        // '\B'
        EPSILON_NOT_AT_WORD_BOUNDARY,

        // no more tokens
        END,

        // invalid token
        INVALID,

        // '*'
        KLEENE_STAR_REPETITION,

        // '^'
        NEGATE_CHARACTER_CLASS,

        // '['
        OPEN_CHARACTER_CLASS,

        // '{'
        OPEN_COUNTED_REPETITION,

        // '('
        OPEN_GROUP,

        // '|'
        OR,

        // '+'
        PLUS_REPETITION,

        // '?'
        QUESTION_MARK_REPETITION,

        // 'min' or 'max' in '{min}', '{min,}' or '{min,max}'
        REPETITION_COUNT,

        // ','
        REPETITION_COUNT_SEPARATOR,

        // '\d'
        SHORTHAND_DIGIT_CHARACTER,

        // '\D'
        SHORTHAND_NOT_DIGIT_CHARACTER,

        // '\S'
        SHORTHAND_NOT_SPACE_CHARACTER,

        // '\W'
        SHORTHAND_NOT_WORD_CHARACTER,

        // '\s'
        SHORTHAND_SPACE_CHARACTER,

        // '\w'
        SHORTHAND_WORD_CHARACTER,

        // 'a', '\[', ...
        SINGLE_CHARACTER
    };

    // instance creation and deletion
    RegexToken(const RegexToken& rhs);
    ~RegexToken();
    RegexToken& operator=(const RegexToken& rhs);
    static RegexToken create_backreference_token(
                        const GroupNumber& group_number);
    static RegexToken create_invalid_token(const std::string& error_message);
    static RegexToken create_repetition_count_token(
                        const RepetitionCount& repetition_count);
    static RegexToken create_single_character_token(char c);
    static RegexToken create_token(Type type);

    // accessing
    char character() const;
    std::string error_message() const;
    GroupNumber group_number() const;
    RepetitionCount repetition_count() const;
    Type type() const;

    // querying
    static bool is_negated_shorthand_character(Type type);
    static bool is_non_negated_shorthand_character(Type type);
    static bool is_shorthand_character(Type type);
    bool is_shorthand_character() const;

private:
    // instance creation and deletion
    RegexToken(Type type);
    void copy_union(const RegexToken& rhs);

    // data members

    // The type of this token.
    Type m_type;

    union
    {
        // The token value if 'm_type' is SINGLE_CHARACTER, undefined
        // otherwise.
        char m_character;

        // The token value (the referenced group number) if 'm_type' is
        // BACKREFERENCE, undefined otherwise.
        GroupNumber m_group_number;

        // The token value if 'm_type' is REPETITION_COUNT, undefined
        // otherwise.
        RepetitionCount m_repetition_count;

        // The error message if 'm_type' is INVALID, undefined
        // otherwise.
        std::string m_error_message;
    };
};


#endif // REGEX_TOKEN_HPP
