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


#ifndef REGEX_PARSER_HPP
#define REGEX_PARSER_HPP

#include "regex_tokenizer.hpp"

#include <gtest/gtest_prod.h>
#include <memory>
#include <string>
#include <vector>

class CharacterBlock;
class Regex;


// An instance of this class parses a regex string into a regex parse
// tree.
//
// For example, this regex:
//
//     AB
//
// is parsed into:
/*
 *             ConcatenationRegex
 *                     /\
 *                    /  \
 *                   /    \
 *                  /      \
 *     CharacterBlockRegex  \
 *              (A)       CharacterBlockRegex
 *                                 (B)
 */
class RegexParser final
{
public:
    // accessing
    static std::unique_ptr<Regex> parse(const std::string& regex_as_string);

private:
    FRIEND_TEST(RegexParserTest, parse_throws_consume_invalid_token);

    // instance creation and deletion
    explicit RegexParser(const std::string& regex_as_string);

    // accessing
    RegexToken                      consume_and_check_token();
    std::unique_ptr<Regex>          parse();
    std::unique_ptr<CharacterBlock> parse_any_character();
    std::unique_ptr<Regex>          parse_atom();
    std::unique_ptr<Regex>          parse_backreference();
    std::unique_ptr<Regex>          parse_capturing_group();
    std::unique_ptr<CharacterBlock> parse_character_block();
    std::unique_ptr<Regex>          parse_character_block_regex();
    std::unique_ptr<CharacterBlock> parse_character_class();
    std::unique_ptr<CharacterBlock> parse_character_class_block();
    std::unique_ptr<CharacterBlock> parse_character_class_blocks(
                                      bool character_class_is_negated);
    void                            parse_character_class_blocks_prime(
                                   std::vector<std::unique_ptr<CharacterBlock>>&
                                     character_class_blocks);
    std::unique_ptr<CharacterBlock> parse_character_range();
    std::unique_ptr<Regex>          parse_counted_repetition(
                                      std::unique_ptr<Regex> regex);
    std::unique_ptr<Regex>          parse_epsilon_at_end();
    std::unique_ptr<Regex>          parse_epsilon_at_start();
    std::unique_ptr<Regex>          parse_epsilon_at_word_boundary();
    std::unique_ptr<Regex>          parse_epsilon_not_at_word_boundary();
    std::unique_ptr<Regex>          parse_factor();
    std::unique_ptr<Regex>          parse_fixed_repetition(
                                      std::unique_ptr<Regex> regex);
    std::unique_ptr<Regex>          parse_group();
    std::unique_ptr<Regex>          parse_lookahead_or_group_or_atom();
    std::unique_ptr<Regex>          parse_non_capturing_group();
    std::unique_ptr<Regex>          parse_non_empty_factor();
    std::unique_ptr<Regex>          parse_non_empty_factor_prime(
                                      std::unique_ptr<Regex> regex);
    std::unique_ptr<Regex>          parse_positive_lookahead();
    std::unique_ptr<Regex>          parse_range_repetition(
                                      std::unique_ptr<Regex> regex);
    std::unique_ptr<Regex>          parse_range_repetition_to_infinity(
                                      std::unique_ptr<Regex> regex);
    std::unique_ptr<Regex>          parse_regex();
    std::unique_ptr<Regex>          parse_regex_prime();
    std::unique_ptr<Regex>          parse_repetition(
                                      std::unique_ptr<Regex> regex);
    std::unique_ptr<CharacterBlock> parse_shorthand_character();
    std::unique_ptr<CharacterBlock> parse_single_character();
    std::unique_ptr<Regex>          parse_term();
    std::unique_ptr<Regex>          parse_term_prime();
    RegexToken                      peek_and_check_token();

    // error handling
    void throw_parse_exception(const std::string& error_message) const;

    // data members

    // The regex string to be parsed.
    std::string m_regex_as_string;

    // 'm_tokenizer' splits 'm_regex_as_string' into regex tokens, which
    // the parser consumes.
    RegexTokenizer m_tokenizer;

    // The next available group number.
    GroupNumber m_next_group_number;
};


#endif // REGEX_PARSER_HPP
