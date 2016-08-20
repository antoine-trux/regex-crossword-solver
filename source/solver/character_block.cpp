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


#include "character_block.hpp"

#include "alphabet.hpp"
#include "set_of_characters.hpp"
#include "utils.hpp"

#include <algorithm>
#include <cassert>
#include <iterator>
#include <numeric>

using namespace std;


// CharacterBlock
// --------------

// instance creation and deletion

CharacterBlock::~CharacterBlock() = default;

// copying

unique_ptr<CharacterBlock>
CharacterBlock::clone() const
{
    return do_clone();
}

// accessing

// Return the characters which can match this character block.
SetOfCharacters
CharacterBlock::characters() const
{
    return do_characters();
}

// Return the characters which appear explicitly in this character
// block.
string
CharacterBlock::explicit_characters() const
{
    return do_explicit_characters();
}

// querying

bool
CharacterBlock::is_composite(const CharacterBlock& character_block)
{
    return character_block.is_composite();
}

bool
CharacterBlock::is_composite() const
{
    return false;
}

// converting

string
CharacterBlock::to_string() const
{
    return do_to_string();
}


// AnyCharacter
// ------------

// copying

unique_ptr<CharacterBlock>
AnyCharacter::do_clone() const
{
    return Utils::make_unique<AnyCharacter>();
}

// accessing

SetOfCharacters
AnyCharacter::do_characters() const
{
    auto characters = Alphabet::characters();

    // '.' does not match '\n'.
    const auto newline = '\n';
    if (Alphabet::has_character(newline))
    {
        characters -= newline;
    }

    return characters;
}

string
AnyCharacter::do_explicit_characters() const
{
    return "";
}

// converting

string
AnyCharacter::do_to_string() const
{
    return ".";
}


// CharacterClass
// --------------

// instance creation and deletion

CharacterClass::CharacterClass(
                  bool                                 is_negated,
                  vector<unique_ptr<CharacterBlock>>&& character_blocks) :
  m_is_negated(is_negated),
  m_character_blocks(move(character_blocks))
{
}

// copying

unique_ptr<CharacterBlock>
CharacterClass::do_clone() const
{
    vector<unique_ptr<CharacterBlock>> character_blocks_copy;
    character_blocks_copy.reserve(m_character_blocks.size());

    transform(m_character_blocks.cbegin(),
              m_character_blocks.cend(),
              back_inserter(character_blocks_copy),
              [](const unique_ptr<CharacterBlock>& character_block)
              {
                  return character_block->clone();
              });

    return Utils::make_unique<CharacterClass>(m_is_negated,
                                              move(character_blocks_copy));
}

// accessing

SetOfCharacters
CharacterClass::do_characters() const
{
    const SetOfCharacters union_ =
        accumulate(m_character_blocks.cbegin(),
                   m_character_blocks.cend(),
                   SetOfCharacters(),
                   [](const SetOfCharacters&            sum,
                      const unique_ptr<CharacterBlock>& character_block)
                   {
                       return sum | character_block->characters();
                   });

    if (m_is_negated)
    {
        return Alphabet::complement(union_);
    }
    else
    {
        return union_;
    }
}

string
CharacterClass::do_explicit_characters() const
{
    return accumulate(m_character_blocks.cbegin(),
                      m_character_blocks.cend(),
                      string(),
                      [](const string&                     sum,
                         const unique_ptr<CharacterBlock>& character_block)
                      {
                          return sum + character_block->explicit_characters();
                      });
}

// converting

string
CharacterClass::do_to_string() const
{
    string inside = m_is_negated ? "^" : "";
    inside += accumulate(m_character_blocks.cbegin(),
                         m_character_blocks.cend(),
                         string(),
                         [](const string&                     sum,
                            const unique_ptr<CharacterBlock>& character_block)
                         {
                             return sum + character_block->to_string();
                         });
    return '[' + inside + ']';
}


// CompositeCharacterBlock
// -----------------------

// instance creation and deletion

CompositeCharacterBlock::CompositeCharacterBlock(
    unique_ptr<CharacterBlock>&& character_block_1,
    unique_ptr<CharacterBlock>&& character_block_2)
{
    append(move(character_block_1));
    append(move(character_block_2));
}

CompositeCharacterBlock::CompositeCharacterBlock(
    vector<unique_ptr<CharacterBlock>>&& character_blocks) :
  m_character_blocks(move(character_blocks))
{
}

void
CompositeCharacterBlock::append(unique_ptr<CharacterBlock>&& character_block)
{
    if (!CharacterBlock::is_composite(*character_block))
    {
        m_character_blocks.push_back(move(character_block));
        return;
    }

    // Although not strictly necessary, we prefer to flatten composite
    // character blocks. One advantage of non-nested composite character
    // blocks is that their string representation (i.e., the result of
    // to_string()) is easier to read.

    CompositeCharacterBlock& composite =
        static_cast<decltype(composite)>(*character_block);

    m_character_blocks.insert(
        end(m_character_blocks),
        make_move_iterator(composite.m_character_blocks.begin()),
        make_move_iterator(composite.m_character_blocks.end()));
}

// copying

unique_ptr<CharacterBlock>
CompositeCharacterBlock::do_clone() const
{
    vector<unique_ptr<CharacterBlock>> character_blocks_copy;
    character_blocks_copy.reserve(m_character_blocks.size());

    transform(m_character_blocks.cbegin(),
              m_character_blocks.cend(),
              back_inserter(character_blocks_copy),
              [](const unique_ptr<CharacterBlock>& character_block)
              {
                  return character_block->clone();
              });

    return Utils::make_unique<CompositeCharacterBlock>(
                    move(character_blocks_copy));
}

// accessing

SetOfCharacters
CompositeCharacterBlock::do_characters() const
{
    return accumulate(m_character_blocks.cbegin(),
                      m_character_blocks.cend(),
                      SetOfCharacters(),
                      [](const SetOfCharacters&            sum,
                         const unique_ptr<CharacterBlock>& character_block)
                      {
                          return sum | character_block->characters();
                      });
}

string
CompositeCharacterBlock::do_explicit_characters() const
{
    return accumulate(m_character_blocks.cbegin(),
                      m_character_blocks.cend(),
                      string(),
                      [](const string&                     sum,
                         const unique_ptr<CharacterBlock>& character_block)
                      {
                          return sum + character_block->explicit_characters();
                      });
}

// querying

bool
CompositeCharacterBlock::is_composite() const
{
    return true;
}

// converting

string
CompositeCharacterBlock::do_to_string() const
{
    const auto inside =
        accumulate(m_character_blocks.cbegin(),
                   m_character_blocks.cend(),
                   string(),
                   [](const string&                     sum,
                      const unique_ptr<CharacterBlock>& character_block)
                   {
                       return sum + character_block->to_string();
                   });
    return '{' + inside + '}';
}


// CharacterRange
// --------------

// instance creation and deletion

CharacterRange::CharacterRange(char low, char high) :
  m_low(low),
  m_high (high)
{
    assert(low <= high);
}

// copying

unique_ptr<CharacterBlock>
CharacterRange::do_clone() const
{
    return Utils::make_unique<CharacterRange>(m_low, m_high);
}

// accessing

SetOfCharacters
CharacterRange::do_characters() const
{
    return SetOfCharacters(explicit_characters());
}

string
CharacterRange::do_explicit_characters() const
{
    string result;
    result.resize(static_cast<decltype(result.size())>(m_high - m_low + 1));
    iota(result.begin(), result.end(), m_low);
    return result;
}

// converting

string
CharacterRange::do_to_string() const
{
    return m_low + string("-") + m_high;
}


// ShorthandCharacter
// ------------------

// instance creation and deletion

ShorthandCharacter::ShorthandCharacter(RegexToken::Type token_type) :
  m_token_type(token_type)
{
    assert(RegexToken::is_shorthand_character(token_type));
}

// copying

unique_ptr<CharacterBlock>
ShorthandCharacter::do_clone() const
{
    return Utils::make_unique<ShorthandCharacter>(m_token_type);
}

// accessing

SetOfCharacters
ShorthandCharacter::do_characters() const
{
    if (RegexToken::is_negated_shorthand_character(m_token_type))
    {
        return Alphabet::complement(explicit_characters());
    }
    else
    {
        return SetOfCharacters(explicit_characters());
    }
}

string
ShorthandCharacter::do_explicit_characters() const
{
    switch (m_token_type)
    {
    case RegexToken::Type::SHORTHAND_DIGIT_CHARACTER:
    case RegexToken::Type::SHORTHAND_NOT_DIGIT_CHARACTER:
        return "0123456789";

    case RegexToken::Type::SHORTHAND_NOT_SPACE_CHARACTER:
    case RegexToken::Type::SHORTHAND_SPACE_CHARACTER:
        // Contrary to other regular expression implementations, we
        // treat '\s' and '\S' as referring only to a space (not to
        // '\t', etc.).
        return " ";

    case RegexToken::Type::SHORTHAND_NOT_WORD_CHARACTER:
    case RegexToken::Type::SHORTHAND_WORD_CHARACTER:
        return "abcdefghijklmnopqrstuvwxyz"
               "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
               "0123456789_";

    default:
        assert(false);
        return "";
    }
}

// converting

string
ShorthandCharacter::do_to_string() const
{
    string result = "\\";

    switch (m_token_type)
    {
    case RegexToken::Type::SHORTHAND_DIGIT_CHARACTER:
        result += 'd';
        break;

    case RegexToken::Type::SHORTHAND_NOT_DIGIT_CHARACTER:
        result += 'D';
        break;

    case RegexToken::Type::SHORTHAND_NOT_SPACE_CHARACTER:
        result += 'S';
        break;

    case RegexToken::Type::SHORTHAND_NOT_WORD_CHARACTER:
        result += 'W';
        break;

    case RegexToken::Type::SHORTHAND_SPACE_CHARACTER:
        result += 's';
        break;

    case RegexToken::Type::SHORTHAND_WORD_CHARACTER:
        result += 'w';
        break;

    default:
        assert(false);
        break;
    }

    return result;
}


// SingleCharacter
// ---------------

// instance creation and deletion

SingleCharacter::SingleCharacter(char c) :
  m_character(c)
{
}

// copying

unique_ptr<CharacterBlock>
SingleCharacter::do_clone() const
{
    return Utils::make_unique<SingleCharacter>(m_character);
}

// accessing

SetOfCharacters
SingleCharacter::do_characters() const
{
    return SetOfCharacters(m_character);
}

string
SingleCharacter::do_explicit_characters() const
{
    return Utils::char_to_string(m_character);
}

// converting

string
SingleCharacter::do_to_string() const
{
    return Utils::char_to_string(m_character);
}
