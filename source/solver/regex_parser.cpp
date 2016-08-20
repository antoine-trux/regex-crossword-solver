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


#include "regex_parser.hpp"

#include "character_block.hpp"
#include "regex.hpp"
#include "regex_crossword_solver_exception.hpp"
#include "utils.hpp"

#include <cassert>

using RTT = RegexToken::Type;
using namespace std;


// The regular expressions we support are recognized by this grammar:
//
//     (Uppercase identifiers are terminals - they are token types,
//     defined in 'regex_token.hpp'.)
//
//     regex ->
//         regex
//         OR
//         term
//         |
//         term
//
//     term ->
//         term
//         factor
//         |
//         factor
//
//     factor ->
//         non_empty_factor
//         |
//         epsilon
//
//     non_empty_factor ->
//         non_empty_factor
//         repetition
//         |
//         group_or_atom
//
//     repetition ->
//         KLEENE_STAR_REPETITION
//         |
//         PLUS_REPETITION
//         |
//         QUESTION_MARK_REPETITION
//         |
//         counted_repetition
//
//     counted_repetition ->
//         fixed_repetition
//         |
//         range_repetition
//         |
//         range_repetition_to_infinity
//
//     fixed_repetition ->
//         OPEN_COUNTED_REPETITION
//         REPETITION_COUNT
//         CLOSE_COUNTED_REPETITION
//
//     range_repetition ->
//         OPEN_COUNTED_REPETITION
//         REPETITION_COUNT
//         REPETITION_COUNT_SEPARATOR
//         REPETITION_COUNT
//         CLOSE_COUNTED_REPETITION
//
//     range_repetition_to_infinity ->
//         OPEN_COUNTED_REPETITION
//         REPETITION_COUNT
//         REPETITION_COUNT_SEPARATOR
//         CLOSE_COUNTED_REPETITION
//
//     group_or_atom ->
//         group
//         |
//         atom
//
//     group ->
//         OPEN_GROUP
//         regex
//         CLOSE_GROUP
//
//     atom ->
//         BACKREFERENCE
//         |
//         character_block
//         |
//         EPSILON_AT_START
//         |
//         EPSILON_AT_END
//         |
//         EPSILON_AT_WORD_BOUNDARY
//         |
//         EPSILON_NOT_AT_WORD_BOUNDARY
//
//     character_block ->
//         ANY_CHARACTER
//         |
//         SINGLE_CHARACTER
//         |
//         shorthand_character
//         |
//         character_class
//
//     shorthand_character ->
//         SHORTHAND_DIGIT_CHARACTER
//         |
//         SHORTHAND_SPACE_CHARACTER
//         |
//         SHORTHAND_WORD_CHARACTER
//         |
//         SHORTHAND_NOT_DIGIT_CHARACTER
//         |
//         SHORTHAND_NOT_SPACE_CHARACTER
//         |
//         SHORTHAND_NOT_WORD_CHARACTER
//
//     character_class ->
//         OPEN_CHARACTER_CLASS
//         character_class_blocks
//         CLOSE_CHARACTER_CLASS
//         |
//         OPEN_CHARACTER_CLASS
//         NEGATE_CHARACTER_CLASS
//         character_class_blocks
//         CLOSE_CHARACTER_CLASS
//
//     character_class_blocks ->
//         character_class_block
//         character_class_blocks'
//
//     character_class_blocks' ->
//         character_class_block
//         character_class_blocks'
//         |
//         epsilon
//
//     character_class_block ->
//         shorthand_character
//         |
//         character_range
//         |
//         SINGLE_CHARACTER
//
//     character_range ->
//         SINGLE_CHARACTER
//         CHARACTER_RANGE_SEPARATOR
//         SINGLE_CHARACTER
//
//     epsilon ->
//         ''
//
// Note that this production rule:
//
//     factor ->
//         epsilon
//
// is intentional. It generates legal (although unusual) regular
// expressions such as: "", "A|", "|A", "|", "||", "()"
//
// The first four rules of the above grammar can be rewritten in a form
// suitable to recursive descent parsing:
//
//     regex ->
//         term
//         regex'
//
//     regex' ->
//         OR
//         term
//         regex'
//         |
//         epsilon
//
//     term ->
//         factor
//         term'
//
//     term' ->
//         non_empty_factor
//         term'
//         |
//         epsilon
//
//     factor ->
//         non_empty_factor
//         |
//         epsilon
//
//     non_empty_factor ->
//         group_or_atom
//         non_empty_factor'
//
//     non_empty_factor' ->
//         repetition
//         non_empty_factor'
//         |
//         epsilon
//
//     (The other rules [repetition, ..., epsilon] are unchanged.)
//
// The implementation below is an unimaginative recursive descent parser
// with backtracking. We do not bother implementing a predictive parser,
// because the backtracking version is simpler, and fast enough in this
// context - for example, the program spends less than 1% of its time in
// parsing for the MIT puzzle.


// instance creation and deletion

RegexParser::RegexParser(const string& regex_as_string) :
  m_regex_as_string(regex_as_string),
  m_tokenizer(regex_as_string),
  m_next_group_number(1)
{
}

// accessing

RegexToken
RegexParser::consume_and_check_token()
{
    const auto token = m_tokenizer.consume_token();

    if (token.type() == RTT::INVALID)
    {
        throw_parse_exception(token.error_message());
    }

    return token;
}

unique_ptr<Regex>
RegexParser::parse(const string& regex_as_string)
{
    RegexParser parser(regex_as_string);
    return parser.parse();
}

unique_ptr<Regex>
RegexParser::parse()
{
    auto regex(parse_regex());
    assert(regex != nullptr);

    if (consume_and_check_token().type() != RTT::END)
    {
        throw_parse_exception("extra input");
    }

    return regex;
}

// Return an AnyCharacter if the next token is ANY_CHARACTER, 'nullptr'
// otherwise.
unique_ptr<CharacterBlock>
RegexParser::parse_any_character()
{
    const auto token_any_character = peek_and_check_token();
    if (token_any_character.type() != RTT::ANY_CHARACTER)
    {
        return nullptr;
    }
    m_tokenizer.consume_token();

    return Utils::make_unique<AnyCharacter>();
}

// Implement this production rule:
//
//     atom ->
//         BACKREFERENCE
//         |
//         character_block
//         |
//         EPSILON_AT_START
//         |
//         EPSILON_AT_END
//         |
//         EPSILON_AT_WORD_BOUNDARY
//         |
//         EPSILON_NOT_AT_WORD_BOUNDARY
unique_ptr<Regex>
RegexParser::parse_atom()
{
    auto backreference(parse_backreference());
    if (backreference != nullptr)
    {
        return backreference;
    }

    auto character_block_regex(parse_character_block_regex());
    if (character_block_regex != nullptr)
    {
        return character_block_regex;
    }

    auto epsilon_at_start(parse_epsilon_at_start());
    if (epsilon_at_start != nullptr)
    {
        return epsilon_at_start;
    }

    auto epsilon_at_end(parse_epsilon_at_end());
    if (epsilon_at_end != nullptr)
    {
        return epsilon_at_end;
    }

    auto epsilon_at_word_boundary(parse_epsilon_at_word_boundary());
    if (epsilon_at_word_boundary != nullptr)
    {
        return epsilon_at_word_boundary;
    }

    auto epsilon_not_at_word_boundary(parse_epsilon_not_at_word_boundary());
    if (epsilon_not_at_word_boundary != nullptr)
    {
        return epsilon_not_at_word_boundary;
    }

    return nullptr;
}

unique_ptr<Regex>
RegexParser::parse_backreference()
{
    const auto token_backreference = peek_and_check_token();
    if (token_backreference.type() != RTT::BACKREFERENCE)
    {
        return nullptr;
    }
    m_tokenizer.consume_token();

    const auto group_number = token_backreference.group_number();

    if (group_number.exceeds_max_backreference_value())
    {
        const string error_message =
            "backreference exceeds maximum ("                        +
            Utils::to_string(GroupNumber::max_backreference_value()) +
            ')';
        throw_parse_exception(error_message);
    }

    if (group_number >= m_next_group_number)
    {
        throw_parse_exception("forward reference");
    }

    return Utils::make_unique<BackreferenceRegex>(group_number);
}

// Implement this production rule:
//
//     character_block ->
//         ANY_CHARACTER
//         |
//         SINGLE_CHARACTER
//         |
//         shorthand_character
//         |
//         character_class
unique_ptr<CharacterBlock>
RegexParser::parse_character_block()
{
    auto any_character(parse_any_character());
    if (any_character != nullptr)
    {
        return any_character;
    }

    auto single_character(parse_single_character());
    if (single_character != nullptr)
    {
        return single_character;
    }

    auto shorthand_character(parse_shorthand_character());
    if (shorthand_character != nullptr)
    {
        return shorthand_character;
    }

    auto character_class(parse_character_class());
    if (character_class != nullptr)
    {
        return character_class;
    }

    return nullptr;
}

unique_ptr<Regex>
RegexParser::parse_character_block_regex()
{
    auto character_block(parse_character_block());
    if (character_block == nullptr)
    {
        return nullptr;
    }

    return Utils::make_unique<CharacterBlockRegex>(move(character_block));
}

// Implement this production rule:
//
//     character_class ->
//         OPEN_CHARACTER_CLASS
//         character_class_blocks
//         CLOSE_CHARACTER_CLASS
//         |
//         OPEN_CHARACTER_CLASS
//         NEGATE_CHARACTER_CLASS
//         character_class_blocks
//         CLOSE_CHARACTER_CLASS
unique_ptr<CharacterBlock>
RegexParser::parse_character_class()
{
    const auto token_open = peek_and_check_token();
    if (token_open.type() != RTT::OPEN_CHARACTER_CLASS)
    {
        return nullptr;
    }
    m_tokenizer.consume_token();

    const auto token_after_open = peek_and_check_token();
    const auto character_class_is_negated =
        token_after_open.type() == RTT::NEGATE_CHARACTER_CLASS;
    if (character_class_is_negated)
    {
        m_tokenizer.consume_token();
    }

    auto character_class_blocks(parse_character_class_blocks(
                                  character_class_is_negated));
    if (character_class_blocks == nullptr)
    {
        if (character_class_is_negated)
        {
            m_tokenizer.push_back_token(token_after_open);
        }

        m_tokenizer.push_back_token(token_open);

        return nullptr;
    }

    const auto token_close = consume_and_check_token();
    if (token_close.type() != RTT::CLOSE_CHARACTER_CLASS)
    {
        throw_parse_exception("missing ']'");
    }

    return character_class_blocks;
}

// Implement this production rule:
//
//     character_class_block ->
//         shorthand_character
//         |
//         character_range
//         |
//         SINGLE_CHARACTER
unique_ptr<CharacterBlock>
RegexParser::parse_character_class_block()
{
    auto shorthand_character(parse_shorthand_character());
    if (shorthand_character != nullptr)
    {
        return shorthand_character;
    }

    auto character_range(parse_character_range());
    if (character_range != nullptr)
    {
        return character_range;
    }

    auto single_character(parse_single_character());
    if (single_character != nullptr)
    {
        return single_character;
    }

    return nullptr;
}

// Implement this production rule:
//
//     character_class_blocks ->
//         character_class_block
//         character_class_blocks'
unique_ptr<CharacterBlock>
RegexParser::parse_character_class_blocks(bool character_class_is_negated)
{
    auto first_character_class_block(parse_character_class_block());
    if (first_character_class_block == nullptr)
    {
        return nullptr;
    }

    vector<unique_ptr<CharacterBlock>> character_class_blocks;
    character_class_blocks.push_back(move(first_character_class_block));

    parse_character_class_blocks_prime(character_class_blocks);

    return Utils::make_unique<CharacterClass>(character_class_is_negated,
                                              move(character_class_blocks));
}

// Implement this production rule:
//
//     character_class_blocks' ->
//         character_class_block
//         character_class_blocks'
//         |
//         epsilon
void
RegexParser::parse_character_class_blocks_prime(
               vector<unique_ptr<CharacterBlock>>& character_class_blocks)
{
    auto character_class_block(parse_character_class_block());
    if (character_class_block == nullptr)
    {
        return;
    }

    character_class_blocks.push_back(move(character_class_block));

    parse_character_class_blocks_prime(character_class_blocks);
}

// Implement this production rule:
//
//     character_range ->
//         SINGLE_CHARACTER
//         CHARACTER_RANGE_SEPARATOR
//         SINGLE_CHARACTER
unique_ptr<CharacterBlock>
RegexParser::parse_character_range()
{
    const auto token_character_low = peek_and_check_token();
    if (token_character_low.type() != RTT::SINGLE_CHARACTER)
    {
        return nullptr;
    }
    m_tokenizer.consume_token();

    const auto token_separator = peek_and_check_token();
    if (token_separator.type() != RTT::CHARACTER_RANGE_SEPARATOR)
    {
        m_tokenizer.push_back_token(token_character_low);
        return nullptr;
    }
    m_tokenizer.consume_token();

    const auto token_character_high = peek_and_check_token();
    if (token_character_high.type() != RTT::SINGLE_CHARACTER)
    {
        m_tokenizer.push_back_tokens({ token_character_low, token_separator });
        return nullptr;
    }
    m_tokenizer.consume_token();

    const auto low = token_character_low.character();
    const auto high = token_character_high.character();

    if (low > high)
    {
        throw_parse_exception(
          "low limit of character range larger than high limit");
    }

    return Utils::make_unique<CharacterRange>(low, high);
}

// Implement this production rule:
//
//     counted_repetition ->
//         fixed_repetition
//         |
//         range_repetition
//         |
//         range_repetition_to_infinity
//
// Precondition:
// * the next token is of type OPEN_COUNTED_REPETITION
unique_ptr<Regex>
RegexParser::parse_counted_repetition(unique_ptr<Regex> regex)
{
    assert(peek_and_check_token().type() == RTT::OPEN_COUNTED_REPETITION);

    const Regex* const original_regex = regex.get();

    regex = parse_fixed_repetition(move(regex));
    if (regex.get() != original_regex)
    {
        return regex;
    }

    regex = parse_range_repetition(move(regex));
    if (regex.get() != original_regex)
    {
        return regex;
    }

    regex = parse_range_repetition_to_infinity(move(regex));
    if (regex.get() == original_regex)
    {
        throw_parse_exception("invalid repetition");
    }

    return regex;
}

unique_ptr<Regex>
RegexParser::parse_epsilon_at_end()
{
    const auto token = peek_and_check_token();
    if (token.type() != RTT::EPSILON_AT_END)
    {
        return nullptr;
    }
    m_tokenizer.consume_token();

    return Utils::make_unique<EpsilonAtEndRegex>();
}

unique_ptr<Regex>
RegexParser::parse_epsilon_at_start()
{
    const auto token = peek_and_check_token();
    if (token.type() != RTT::EPSILON_AT_START)
    {
        return nullptr;
    }
    m_tokenizer.consume_token();

    return Utils::make_unique<EpsilonAtStartRegex>();
}

unique_ptr<Regex>
RegexParser::parse_epsilon_at_word_boundary()
{
    const auto token = peek_and_check_token();
    if (token.type() != RTT::EPSILON_AT_WORD_BOUNDARY)
    {
        return nullptr;
    }
    m_tokenizer.consume_token();

    return Utils::make_unique<EpsilonAtWordBoundaryRegex>();
}

unique_ptr<Regex>
RegexParser::parse_epsilon_not_at_word_boundary()
{
    const auto token = peek_and_check_token();
    if (token.type() != RTT::EPSILON_NOT_AT_WORD_BOUNDARY)
    {
        return nullptr;
    }
    m_tokenizer.consume_token();

    return Utils::make_unique<EpsilonNotAtWordBoundaryRegex>();
}

// Implement this production rule:
//
//     factor ->
//         non_empty_factor
//         |
//         epsilon
unique_ptr<Regex>
RegexParser::parse_factor()
{
    return parse_non_empty_factor();
}

// Implement this production rule:
//
//     fixed_repetition ->
//         OPEN_COUNTED_REPETITION
//         REPETITION_COUNT
//         CLOSE_COUNTED_REPETITION
//
// Return the repeated regex, or 'regex' itself if this rule does not
// apply.
//
// Precondition:
// * the next token is of type OPEN_COUNTED_REPETITION
unique_ptr<Regex>
RegexParser::parse_fixed_repetition(unique_ptr<Regex> regex)
{
    const auto token_open = consume_and_check_token();
    assert(token_open.type() == RTT::OPEN_COUNTED_REPETITION);

    const auto token_count = peek_and_check_token();
    if (token_count.type() != RTT::REPETITION_COUNT)
    {
        m_tokenizer.push_back_token(token_open);
        return regex;
    }
    m_tokenizer.consume_token();

    const auto token_close = peek_and_check_token();
    if (token_close.type() != RTT::CLOSE_COUNTED_REPETITION)
    {
        m_tokenizer.push_back_tokens({ token_open, token_count });
        return regex;
    }
    m_tokenizer.consume_token();

    const auto fixed_count = token_count.repetition_count();
    return Utils::make_unique<FixedRepetitionRegex>(move(regex), fixed_count);
}

// Implement this production rule:
//
//     group ->
//         OPEN_GROUP
//         regex
//         CLOSE_GROUP
unique_ptr<Regex>
RegexParser::parse_group()
{
    const auto token_open = peek_and_check_token();
    if (token_open.type() != RTT::OPEN_GROUP)
    {
        return nullptr;
    }
    m_tokenizer.consume_token();

    GroupNumber group_number = m_next_group_number;
    ++m_next_group_number;

    auto regex(parse_regex());
    assert(regex != nullptr);

    const auto token_close = consume_and_check_token();
    if (token_close.type() != RTT::CLOSE_GROUP)
    {
        throw_parse_exception("missing ')'");
    }

    return Utils::make_unique<GroupRegex>(move(regex), group_number);
}

// Implement this production rule:
//
//     group_or_atom ->
//         group
//         |
//         atom
unique_ptr<Regex>
RegexParser::parse_group_or_atom()
{
    auto group(parse_group());
    if (group != nullptr)
    {
        return group;
    }

    return parse_atom();
}

// Implement this production rule:
//
//     non_empty_factor ->
//         group_or_atom
//         non_empty_factor'
unique_ptr<Regex>
RegexParser::parse_non_empty_factor()
{
    auto group_or_atom(parse_group_or_atom());
    if (group_or_atom == nullptr)
    {
        return nullptr;
    }

    return parse_non_empty_factor_prime(move(group_or_atom));
}

// Implement this production rule:
//
//     non_empty_factor' ->
//         repetition
//         non_empty_factor'
//         |
//         epsilon
unique_ptr<Regex>
RegexParser::parse_non_empty_factor_prime(unique_ptr<Regex> regex)
{
    const Regex* const original_regex = regex.get();

    regex = parse_repetition(move(regex));
    if (regex.get() == original_regex)
    {
        return regex;
    }

    return parse_non_empty_factor_prime(move(regex));
}

// Implement this production rule:
//
//     range_repetition ->
//         OPEN_COUNTED_REPETITION
//         REPETITION_COUNT
//         REPETITION_COUNT_SEPARATOR
//         REPETITION_COUNT
//         CLOSE_COUNTED_REPETITION
//
// Return the repeated regex, or 'regex' itself if this rule does not
// apply.
//
// Precondition:
// * the next token is of type OPEN_COUNTED_REPETITION
unique_ptr<Regex>
RegexParser::parse_range_repetition(unique_ptr<Regex> regex)
{
    const auto token_open = consume_and_check_token();
    assert(token_open.type() == RTT::OPEN_COUNTED_REPETITION);

    const auto token_min = peek_and_check_token();
    if (token_min.type() != RTT::REPETITION_COUNT)
    {
        m_tokenizer.push_back_token(token_open);
        return regex;
    }
    m_tokenizer.consume_token();

    const auto token_separator = peek_and_check_token();
    if (token_separator.type() != RTT::REPETITION_COUNT_SEPARATOR)
    {
        m_tokenizer.push_back_tokens({ token_open, token_min });
        return regex;
    }
    m_tokenizer.consume_token();

    const auto token_max = peek_and_check_token();
    if (token_max.type() != RTT::REPETITION_COUNT)
    {
        m_tokenizer.push_back_tokens({ token_open, token_min,
                                       token_separator });
        return regex;
    }
    m_tokenizer.consume_token();

    const auto token_close = peek_and_check_token();
    if (token_close.type() != RTT::CLOSE_COUNTED_REPETITION)
    {
        m_tokenizer.push_back_tokens({ token_open, token_min, token_separator,
                                       token_max });
        return regex;
    }
    m_tokenizer.consume_token();

    const auto min_count = token_min.repetition_count();
    const auto max_count = token_max.repetition_count();

    if (max_count < min_count)
    {
        throw_parse_exception("min count of repetition is larger "
                              "than max count");
    }

    return Utils::make_unique<RangeRepetitionRegex>(move(regex),
                                                    min_count, max_count);
}

// Implement this production rule:
//
//     range_repetition_to_infinity ->
//         OPEN_COUNTED_REPETITION
//         REPETITION_COUNT
//         REPETITION_COUNT_SEPARATOR
//         CLOSE_COUNTED_REPETITION
//
// Return the repeated regex, or 'regex' itself if this rule does not
// apply.
//
// Precondition:
// * the next token is of type OPEN_COUNTED_REPETITION
unique_ptr<Regex>
RegexParser::parse_range_repetition_to_infinity(unique_ptr<Regex> regex)
{
    const auto token_open = consume_and_check_token();
    assert(token_open.type() == RTT::OPEN_COUNTED_REPETITION);

    const auto token_min = peek_and_check_token();
    if (token_min.type() != RTT::REPETITION_COUNT)
    {
        m_tokenizer.push_back_token(token_open);
        return regex;
    }
    m_tokenizer.consume_token();

    const auto token_separator = peek_and_check_token();
    if (token_separator.type() != RTT::REPETITION_COUNT_SEPARATOR)
    {
        m_tokenizer.push_back_tokens({ token_open, token_min });
        return regex;
    }
    m_tokenizer.consume_token();

    const auto token_close = peek_and_check_token();
    if (token_close.type() != RTT::CLOSE_COUNTED_REPETITION)
    {
        m_tokenizer.push_back_tokens({ token_open, token_min,
                                       token_separator });
        return regex;
    }
    m_tokenizer.consume_token();

    const auto min_count = token_min.repetition_count();

    return Utils::make_unique<RangeRepetitionToInfinityRegex>(
                    move(regex), min_count);
}

// Implement this production rule:
//
//     regex ->
//         term
//         regex'
unique_ptr<Regex>
RegexParser::parse_regex()
{
    auto term(parse_term());
    assert(term != nullptr);

    auto regex_prime(parse_regex_prime());
    if (regex_prime == nullptr)
    {
        return term;
    }

    return Utils::make_unique<UnionRegex>(move(term), move(regex_prime));
}

// Implement this production rule:
//
//     regex' ->
//         OR
//         term
//         regex'
//         |
//         epsilon
unique_ptr<Regex>
RegexParser::parse_regex_prime()
{
    const auto token = peek_and_check_token();
    if (token.type() != RTT::OR)
    {
        return nullptr;
    }
    m_tokenizer.consume_token();

    auto term(parse_term());
    assert(term != nullptr);

    auto regex_prime(parse_regex_prime());
    if (regex_prime == nullptr)
    {
        return term;
    }

    return Utils::make_unique<UnionRegex>(move(term), move(regex_prime));
}

// Implement this production rule:
//
//     repetition ->
//         KLEENE_STAR_REPETITION
//         |
//         PLUS_REPETITION
//         |
//         QUESTION_MARK_REPETITION
//         |
//         counted_repetition
//
// Return the repeated regex, or 'regex' itself if this rule does not
// apply.
unique_ptr<Regex>
RegexParser::parse_repetition(unique_ptr<Regex> regex)
{
    const auto token = peek_and_check_token();

    switch (token.type())
    {
    case RTT::KLEENE_STAR_REPETITION:
        m_tokenizer.consume_token();
        return Utils::make_unique<KleeneStarRegex>(move(regex));

    case RTT::PLUS_REPETITION:
        m_tokenizer.consume_token();
        return Utils::make_unique<PlusRegex>(move(regex));

    case RTT::QUESTION_MARK_REPETITION:
        m_tokenizer.consume_token();
        return Utils::make_unique<QuestionMarkRegex>(move(regex));

    case RTT::OPEN_COUNTED_REPETITION:
        return parse_counted_repetition(move(regex));

    default:
        return regex;
    }
}

// Implement this production rule:
//
//     shorthand_character ->
//         SHORTHAND_DIGIT_CHARACTER
//         |
//         SHORTHAND_SPACE_CHARACTER
//         |
//         SHORTHAND_WORD_CHARACTER
//         |
//         SHORTHAND_NOT_DIGIT_CHARACTER
//         |
//         SHORTHAND_NOT_SPACE_CHARACTER
//         |
//         SHORTHAND_NOT_WORD_CHARACTER
unique_ptr<CharacterBlock>
RegexParser::parse_shorthand_character()
{
    const auto token_shorthand_character = peek_and_check_token();
    if (!token_shorthand_character.is_shorthand_character())
    {
        return nullptr;
    }
    m_tokenizer.consume_token();

    return Utils::make_unique<ShorthandCharacter>(
                    token_shorthand_character.type());
}

unique_ptr<CharacterBlock>
RegexParser::parse_single_character()
{
    const auto token_character = peek_and_check_token();
    if (token_character.type() != RTT::SINGLE_CHARACTER)
    {
        return nullptr;
    }
    m_tokenizer.consume_token();

    return Utils::make_unique<SingleCharacter>(token_character.character());
}

// Implement this production rule:
//
//     term ->
//         factor
//         term'
unique_ptr<Regex>
RegexParser::parse_term()
{
    auto factor(parse_factor());
    if (factor == nullptr)
    {
        return Utils::make_unique<EpsilonRegex>();
    }

    auto term_prime(parse_term_prime());
    if (term_prime == nullptr)
    {
        return factor;
    }

    return Utils::make_unique<ConcatenationRegex>(move(factor),
                                                  move(term_prime));
}

// Implement this production rule:
//
//     term' ->
//         non_empty_factor
//         term'
//         |
//         epsilon
unique_ptr<Regex>
RegexParser::parse_term_prime()
{
    auto non_empty_factor(parse_non_empty_factor());
    if (non_empty_factor == nullptr)
    {
        return nullptr;
    }

    auto term_prime(parse_term_prime());
    if (term_prime == nullptr)
    {
        return non_empty_factor;
    }

    return Utils::make_unique<ConcatenationRegex>(move(non_empty_factor),
                                                  move(term_prime));
}

RegexToken
RegexParser::peek_and_check_token()
{
    const auto token = m_tokenizer.peek_token();

    if (token.type() == RTT::INVALID)
    {
        throw_parse_exception(token.error_message());
    }

    return token;
}

// error handling

void
RegexParser::throw_parse_exception(const string& error_message) const
{
    throw RegexParseException(
            error_message, m_regex_as_string, m_tokenizer.position());
}
