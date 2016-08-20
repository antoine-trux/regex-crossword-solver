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


#include "regex_tokenizer.hpp"

#include "utils.hpp"

#include <cassert>

using RTT = RegexToken::Type;
using namespace std;


// instance creation and deletion

RegexTokenizer::RegexTokenizer(const string& regex_as_string) :
  m_regex_as_string(regex_as_string),
  m_next_char_index(0),
  m_in_counted_repetition(false),
  m_in_character_class(false),
  m_previous_token_is_open_character_class(false),
  m_previous_token_is_negate_character_class(false),
  m_previous_token_is_character_range_separator(false),
  m_previous_token_is_end_of_character_range(false)
{
}

// accessing

size_t
RegexTokenizer::num_remaining_chars() const
{
    return m_regex_as_string.size() - m_next_char_index;
}

// Return the next character without advancing the position.
//
// For example, if the current position is:
//     ABC
//      ^
// return 'B'.
char
RegexTokenizer::peek_char() const
{
    const size_t offset = 0;
    return peek_char(offset);
}

// Return the character located 'offset' characters after the next
// character, without advancing the position.
//
// For example, if the current position is:
//     ABC
//      ^
// and 'offset' is 1, return 'C'.
char
RegexTokenizer::peek_char(size_t offset) const
{
    assert(offset < num_remaining_chars());
    return m_regex_as_string[m_next_char_index + offset];
}

RegexToken
RegexTokenizer::peek_token()
{
    const auto token = consume_token();
    push_back_token(token);
    return token;
}

size_t
RegexTokenizer::position() const
{
    return m_next_char_index;
}

// querying

bool
RegexTokenizer::at_end_of_string() const
{
    return m_next_char_index >= m_regex_as_string.size();
}

bool
RegexTokenizer::has_pushed_back_tokens() const
{
    return !m_pushed_back_tokens.empty();
}

bool
RegexTokenizer::next_three_chars_are_octal_digits() const
{
    return num_remaining_chars() >= 3          &&
           Utils::is_octal_digit(peek_char(0)) &&
           Utils::is_octal_digit(peek_char(1)) &&
           Utils::is_octal_digit(peek_char(2));
}

bool
RegexTokenizer::not_at_end_of_string() const
{
    return !at_end_of_string();
}

// modifying

// Advance the position past the next character, and return that
// character.
//
// For example, if the current position is:
//     ABC
//      ^
// change the position to:
//     ABC
//       ^
// and return 'B'.
char
RegexTokenizer::consume_char()
{
    const auto next_char = peek_char();
    ++m_next_char_index;
    return next_char;
}

// The next characters are '\' and 'u'. Return the token made of that
// escape sequence and the next characters.
//
// Escaped Unicode characters are not supported, so this returns an
// INVALID token.
RegexToken
RegexTokenizer::consume_escape_32_bit_unicode_token()
{
    const size_t expected_num_hex_digits = 4;
    return consume_escape_unicode_token(expected_num_hex_digits);
}

// The next characters are '\' and 'U'. Return the token made of that
// escape sequence and the next characters.
//
// Escaped Unicode characters are not supported, so this returns an
// INVALID token.
RegexToken
RegexTokenizer::consume_escape_64_bit_unicode_token()
{
    const size_t expected_num_hex_digits = 8;
    return consume_escape_unicode_token(expected_num_hex_digits);
}

// The next characters are '\' and a digit. Return the token made of
// that escape sequence (possibly with other, subsequent digits).
RegexToken
RegexTokenizer::consume_escape_digit_token()
{
    if (m_in_character_class)
    {
        return consume_escape_digit_token_in_character_class();
    }
    else
    {
        return consume_escape_digit_token_outside_character_class();
    }
}

// The next characters are '\' and a digit. Return the token made of
// that escape sequence (possibly with other, subsequent digits), given
// that we are in a character class.
//
// For example, if the current position is:
//     [\141]
//      ^
// return a SINGLE_CHARACTER token with value 'a' (= character with
// octal value 141).
RegexToken
RegexTokenizer::consume_escape_digit_token_in_character_class()
{
    assert(m_in_character_class);
    assert(peek_char(0) == '\\');

    const auto digit_0 = peek_char(1);
    assert(isdigit(digit_0));

    if (!Utils::is_octal_digit(digit_0))
    {
        const auto error_message = "bad escape in character class";
        return RegexToken::create_invalid_token(error_message);
    }

    return consume_escape_octal_character_token();
}

// The next characters are '\' and a digit. Return the token made of
// that escape sequence (possibly with other, subsequent digits), given
// that we are outside a character class.
//
// For example, if the current position is:
//     (A)\1
//        ^
// return a BACKREFERENCE token which refers to group number 1.
RegexToken
RegexTokenizer::consume_escape_digit_token_outside_character_class()
{
    assert(!m_in_character_class);

    assert(peek_char() == '\\');
    consume_char();

    const auto digit_0 = peek_char();
    assert(isdigit(digit_0));

    if (digit_0 == '0' || next_three_chars_are_octal_digits())
    {
        push_back_char('\\');
        return consume_escape_octal_character_token();
    }

    consume_char();

    const auto group_number = Utils::digit_to_int(digit_0);
    return RegexToken::create_backreference_token(
                         GroupNumber(static_cast<unsigned int>(group_number)));
}

// The next characters are '\' and 'x'. Return the token made of that
// escape sequence and the next characters.
//
// For example, if the current position is:
//     X\x61
//      ^
// return a SINGLE_CHARACTER token with value 'a' (= character with
// hexadecimal value 61).
RegexToken
RegexTokenizer::consume_escape_hex_char_token()
{
    assert(peek_char() == '\\');
    consume_char();

    assert(peek_char() == 'x');
    consume_char();

    const size_t expected_num_hex_digits = 2;
    size_t num_hex_digits_read = 0;

    int char_value = 0;

    while (not_at_end_of_string() &&
           num_hex_digits_read != expected_num_hex_digits)
    {
        const auto c = peek_char();

        if (!isxdigit(c))
        {
            break;
        }

        char_value = 16 * char_value + Utils::hex_digit_to_int(c);

        consume_char();
        ++num_hex_digits_read;
    }

    if (num_hex_digits_read == expected_num_hex_digits)
    {
        const auto c = static_cast<char>(char_value);
        return RegexToken::create_single_character_token(c);
    }
    else
    {
        const auto error_message = "incomplete hexadecimal escape";
        return RegexToken::create_invalid_token(error_message);
    }
}

// The next characters are '\' and one or more octal digits. Return the
// token made of that escape sequence.
//
// For example, if the current position is:
//     X\041
//      ^
// return a SINGLE_CHARACTER token with value '!' (= character with
// hexadecimal value 41).
RegexToken
RegexTokenizer::consume_escape_octal_character_token()
{
    assert(peek_char() == '\\');
    consume_char();

    const auto digit_0 = consume_char();
    assert(Utils::is_octal_digit(digit_0));

    auto char_value = Utils::digit_to_int(digit_0);

    if (at_end_of_string() || !Utils::is_octal_digit(peek_char()))
    {
        // 1 octal digit (digit_0)
        const auto c = static_cast<char>(char_value);
        return RegexToken::create_single_character_token(c);
    }

    const auto digit_1 = consume_char();
    char_value = 8 * char_value + Utils::digit_to_int(digit_1);

    if (at_end_of_string() || !Utils::is_octal_digit(peek_char()))
    {
        // 2 octal digits (digit_0, digit_1)
        const auto c = static_cast<char>(char_value);
        return RegexToken::create_single_character_token(c);
    }

    // 3 octal digits (digit_0, digit_1, digit_2)

    const auto digit_2 = consume_char();
    char_value = 8 * char_value + Utils::digit_to_int(digit_2);

    if (char_value >= 256)
    {
        const auto error_message = "octal escape value out of range";
        return RegexToken::create_invalid_token(error_message);
    }

    const auto c = static_cast<char>(char_value);
    return RegexToken::create_single_character_token(c);
}

// The next character is '\'. Return the token made of that escape
// character and the next character(s).
//
// For example, if the current position is:
//     X\b
//      ^
// return a SINGLE_CHARACTER token with value '\b' (= backspace).
RegexToken
RegexTokenizer::consume_escape_token()
{
    assert(peek_char() == '\\');
    consume_char();

    if (at_end_of_string())
    {
        const auto error_message = "incomplete escape";
        return RegexToken::create_invalid_token(error_message);
    }

    const auto next_char = consume_char();

    switch (next_char)
    {
    case 'a':
        // bell
        return RegexToken::create_single_character_token('\a');

    case 'A':
        return RegexToken::create_token(RTT::EPSILON_AT_START);

    case 'b':
        if (m_in_character_class)
        {
            // backspace
            return RegexToken::create_single_character_token('\b');
        }
        else
        {
            return RegexToken::create_token(RTT::EPSILON_AT_WORD_BOUNDARY);
        }

    case 'B':
        return RegexToken::create_token(RTT::EPSILON_NOT_AT_WORD_BOUNDARY);

    case 'd':
        return RegexToken::create_token(RTT::SHORTHAND_DIGIT_CHARACTER);

    case 'D':
        return RegexToken::create_token(RTT::SHORTHAND_NOT_DIGIT_CHARACTER);

    case 'f':
        // form feed
        return RegexToken::create_single_character_token('\f');

    case 'n':
        // line feed
        return RegexToken::create_single_character_token('\n');

    case 'r':
        // carriage return
        return RegexToken::create_single_character_token('\r');

    case 's':
        // space
        return RegexToken::create_token(RTT::SHORTHAND_SPACE_CHARACTER);

    case 'S':
        return RegexToken::create_token(RTT::SHORTHAND_NOT_SPACE_CHARACTER);

    case 't':
        // horizontal tab
        return RegexToken::create_single_character_token('\t');

    case 'u':
        push_back_char(next_char);
        push_back_char('\\');
        return consume_escape_32_bit_unicode_token();

    case 'U':
        push_back_char(next_char);
        push_back_char('\\');
        return consume_escape_64_bit_unicode_token();

    case 'v':
        // vertical tab
        return RegexToken::create_single_character_token('\v');

    case 'w':
        return RegexToken::create_token(RTT::SHORTHAND_WORD_CHARACTER);

    case 'W':
        return RegexToken::create_token(RTT::SHORTHAND_NOT_WORD_CHARACTER);

    case 'x':
        push_back_char(next_char);
        push_back_char('\\');
        return consume_escape_hex_char_token();

    case 'Z':
        return RegexToken::create_token(RTT::EPSILON_AT_END);

    default:
        if (Utils::is_ascii_letter(next_char))
        {
            const auto error_message = "bad escape";
            return RegexToken::create_invalid_token(error_message);
        }
        else if (isdigit(next_char))
        {
            push_back_char(next_char);
            push_back_char('\\');
            return consume_escape_digit_token();
        }
        else
        {
            return RegexToken::create_single_character_token(next_char);
        }
    }
}

// The next characters are '\' and 'u' or 'U'. Return the token made of
// that escape sequence and the next characters.
//
// Escaped Unicode characters are not supported, so this returns an
// INVALID token.
RegexToken
RegexTokenizer::consume_escape_unicode_token(size_t expected_num_hex_digits)
{
    assert(peek_char() == '\\');
    consume_char();

    assert((peek_char() == 'u' && expected_num_hex_digits == 4) ||
           (peek_char() == 'U' && expected_num_hex_digits == 8));
    consume_char();

    size_t num_hex_digits_read = 0;

    while (not_at_end_of_string() &&
           num_hex_digits_read != expected_num_hex_digits)
    {
        if (!isxdigit(peek_char()))
        {
            break;
        }

        // Unicode tokens are not supported, so we do not bother
        // recording the character value.

        consume_char();
        ++num_hex_digits_read;
    }

    if (num_hex_digits_read == expected_num_hex_digits)
    {
        const auto error_message = "unicode characters are not supported";
        return RegexToken::create_invalid_token(error_message);
    }
    else
    {
        const auto error_message = "incomplete unicode escape";
        return RegexToken::create_invalid_token(error_message);
    }
}

// Return the upcoming repetition count token.
//
// For example, if the current position is:
//
//     A{1,20}
//         ^
// return a REPETITION_COUNT token with repetition count value 20.
RegexToken
RegexTokenizer::consume_repetition_count_token()
{
    assert(!m_in_character_class);
    assert(m_in_counted_repetition);

    string repetition_count_as_string;

    while (not_at_end_of_string() && isdigit(peek_char()))
    {
        repetition_count_as_string += consume_char();
    }

    size_t repetition_count;

    if (!Utils::string_to_unsigned(repetition_count_as_string,
                                   &repetition_count))
    {
        const auto error_message = "invalid repetition count";
        return RegexToken::create_invalid_token(error_message);
    }

    return RegexToken::create_repetition_count_token(repetition_count);
}

RegexToken
RegexTokenizer::consume_token()
{
    if (has_pushed_back_tokens())
    {
        const auto token = m_pushed_back_tokens.top();
        m_pushed_back_tokens.pop();
        return token;
    }

    if (at_end_of_string())
    {
        return RegexToken::create_token(RTT::END);
    }

    return consume_token_from_string();
}

RegexToken
RegexTokenizer::consume_token_from_string()
{
    if (m_in_character_class)
    {
        const auto token = consume_token_in_character_class();

        m_in_character_class = (token.type() != RTT::CLOSE_CHARACTER_CLASS);
        m_previous_token_is_open_character_class = false;
        m_previous_token_is_negate_character_class =
            (token.type() == RTT::NEGATE_CHARACTER_CLASS);
        m_previous_token_is_end_of_character_range =
            m_previous_token_is_character_range_separator &&
            (token.type() == RTT::SINGLE_CHARACTER);
        m_previous_token_is_character_range_separator =
            (token.type() == RTT::CHARACTER_RANGE_SEPARATOR);

        return token;
    }
    else
    {
        const auto token = consume_token_outside_character_class();

        m_in_character_class = (token.type() == RTT::OPEN_CHARACTER_CLASS);
        m_previous_token_is_open_character_class =
            (token.type() == RTT::OPEN_CHARACTER_CLASS);
        m_previous_token_is_negate_character_class = false;
        m_previous_token_is_character_range_separator = false;
        m_previous_token_is_end_of_character_range = false;

        return token;
    }
}

RegexToken
RegexTokenizer::consume_token_in_character_class()
{
    assert(m_in_character_class);

    const auto next_char = consume_char();

    switch (next_char)
    {
    case ']':
        if (m_previous_token_is_open_character_class ||
            m_previous_token_is_negate_character_class)
        {
            return RegexToken::create_single_character_token(next_char);
        }
        else
        {
            return RegexToken::create_token(RTT::CLOSE_CHARACTER_CLASS);
        }

    case '^':
        if (m_previous_token_is_open_character_class)
        {
            return RegexToken::create_token(RTT::NEGATE_CHARACTER_CLASS);
        }
        else
        {
            return RegexToken::create_single_character_token(next_char);
        }

    case '-':
        if (m_previous_token_is_open_character_class   ||
            m_previous_token_is_negate_character_class ||
            m_previous_token_is_end_of_character_range)
        {
            return RegexToken::create_single_character_token(next_char);
        }
        else if (at_end_of_string() || peek_char() == ']')
        {
            return RegexToken::create_single_character_token(next_char);
        }
        else
        {
            return RegexToken::create_token(RTT::CHARACTER_RANGE_SEPARATOR);
        }

    case '\\':
    {
        push_back_char(next_char);
        return consume_escape_token();
    }

    default:
        return RegexToken::create_single_character_token(next_char);
    }
}

RegexToken
RegexTokenizer::consume_token_in_counted_repetition()
{
    assert(!m_in_character_class);
    assert(m_in_counted_repetition);

    auto next_char = consume_char();

    if (next_char == '}')
    {
        m_in_counted_repetition = false;
        return RegexToken::create_token(RTT::CLOSE_COUNTED_REPETITION);
    }

    if (next_char == ',')
    {
        return RegexToken::create_token(RTT::REPETITION_COUNT_SEPARATOR);
    }

    if (!isdigit(next_char))
    {
        const auto error_message = "invalid token in counted repetition";
        return RegexToken::create_invalid_token(error_message);
    }

    push_back_char(next_char);
    return consume_repetition_count_token();
}

RegexToken
RegexTokenizer::consume_token_outside_character_class()
{
    assert(!m_in_character_class);

    if (m_in_counted_repetition)
    {
        return consume_token_in_counted_repetition();
    }

    const auto next_char = consume_char();

    switch (next_char)
    {
    case '.':
        return RegexToken::create_token(RTT::ANY_CHARACTER);

    case '^':
        return RegexToken::create_token(RTT::EPSILON_AT_START);

    case '$':
        return RegexToken::create_token(RTT::EPSILON_AT_END);

    case '*':
        return RegexToken::create_token(RTT::KLEENE_STAR_REPETITION);

    case '+':
        return RegexToken::create_token(RTT::PLUS_REPETITION);

    case '?':
        return RegexToken::create_token(RTT::QUESTION_MARK_REPETITION);

    case '{':
        m_in_counted_repetition = true;
        return RegexToken::create_token(RTT::OPEN_COUNTED_REPETITION);

    case '[':
        return RegexToken::create_token(RTT::OPEN_CHARACTER_CLASS);

    case '|':
        return RegexToken::create_token(RTT::OR);

    case '(':
        if (not_at_end_of_string() && peek_char() == '?')
        {
            const auto error_message = "construct '(?' is not supported";
            return RegexToken::create_invalid_token(error_message);
        }
        else
        {
            return RegexToken::create_token(RTT::OPEN_GROUP);
        }

    case ')':
        return RegexToken::create_token(RTT::CLOSE_GROUP);

    case '\\':
        push_back_char(next_char);
        return consume_escape_token();

    default:
        return RegexToken::create_single_character_token(next_char);
    }
}

void
RegexTokenizer::push_back_char(char c)
{
    static_cast<void>(c);
    assert(m_next_char_index != 0);
    assert(m_regex_as_string[m_next_char_index - 1] == c);

    --m_next_char_index;
}

void
RegexTokenizer::push_back_token(const RegexToken& token)
{
    if (token.type() != RTT::END)
    {
        m_pushed_back_tokens.push(token);
    }
}

// The elements of 'tokens' are pushed back in their reverse order -
// that is, the last element of 'tokens' is pushed first, and the first
// element of 'tokens' is pushed last.
void
RegexTokenizer::push_back_tokens(const vector<RegexToken>& tokens)
{
    for (auto rit = tokens.crbegin(); rit != tokens.crend(); ++rit)
    {
        push_back_token(*rit);
    }
}
