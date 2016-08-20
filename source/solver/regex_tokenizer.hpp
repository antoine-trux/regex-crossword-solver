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


#ifndef REGEX_TOKENIZER_HPP
#define REGEX_TOKENIZER_HPP

#include "regex_token.hpp"

#include <gtest/gtest_prod.h>
#include <stack>
#include <string>
#include <vector>


// An instance of this class splits a regex string into regex tokens.
//
// The stream of regex tokens which make up the regex are retrieved by
// repeatedly calling methods consume_token() and peek_token().
//
// Regex tokens can also be pushed back (for later retrieval) to the
// stream of regex tokens with methods push_back_token() and
// push_back_tokens().
class RegexTokenizer final
{
public:
    // instance creation and deletion
    explicit RegexTokenizer(const std::string& regex_as_string);

    // accessing
    RegexToken peek_token();
    size_t position() const;

    // modifying
    RegexToken consume_token();
    void push_back_token(const RegexToken& token);
    void push_back_tokens(const std::vector<RegexToken>& tokens);

private:
    // instance creation and deletion
    RegexToken create_single_character_token(char c);
    RegexToken create_token(RegexToken::Type type);

    // accessing
    size_t num_remaining_chars() const;
    char peek_char() const;
    char peek_char(size_t offset) const;

    // querying
    bool at_end_of_string() const;
    bool has_pushed_back_tokens() const;
    bool next_three_chars_are_octal_digits() const;
    bool not_at_end_of_string() const;

    // modifying
    char consume_char();
    RegexToken consume_escape_32_bit_unicode_token();
    RegexToken consume_escape_64_bit_unicode_token();
    RegexToken consume_escape_digit_token();
    RegexToken consume_escape_digit_token_in_character_class();
    RegexToken consume_escape_digit_token_outside_character_class();
    RegexToken consume_escape_hex_char_token();
    RegexToken consume_escape_octal_character_token();
    RegexToken consume_escape_token();
    RegexToken consume_escape_unicode_token(size_t expected_num_hex_digits);
    RegexToken consume_repetition_count_token();
    RegexToken consume_token_from_string();
    RegexToken consume_token_in_character_class();
    RegexToken consume_token_in_counted_repetition();
    RegexToken consume_token_outside_character_class();
    void push_back_char(char c);

    // data members

    // The regex string to be tokenized.
    std::string m_regex_as_string;

    // The index of the next character to be read in 'm_regex_as_string'.
    size_t m_next_char_index;

    // Whether we are after an OPEN_COUNTED_REPETITION token, and before
    // the matching CLOSE_COUNTED_REPETITION token.
    bool m_in_counted_repetition;

    // Whether we are after an OPEN_CHARACTER_CLASS token, and before
    // the matching CLOSE_CHARACTER_CLASS token.
    bool m_in_character_class;

    // Whether the previous token, if any, is OPEN_CHARACTER_CLASS.
    bool m_previous_token_is_open_character_class;

    // Whether the previous token, if any, is NEGATE_CHARACTER_CLASS.
    bool m_previous_token_is_negate_character_class;

    // Whether the previous token, if any, is CHARACTER_RANGE_SEPARATOR.
    bool m_previous_token_is_character_range_separator;

    // Whether the previous token, if any, marks the end of a character
    // range.
    bool m_previous_token_is_end_of_character_range;

    // The arguments to methods push_back_token[s]() are stored in
    // 'm_pushed_back_tokens'. Thus, the stream of remaining tokens that
    // make up 'm_regex_as_string' consists of the following tokens:
    // * the elements of 'm_pushed_back_tokens'
    // * the tokens (yet to be analyzed) starting at
    //   'm_regex_as_string[m_next_char_index]'
    std::stack<RegexToken> m_pushed_back_tokens;
};

#endif // REGEX_TOKENIZER_HPP
