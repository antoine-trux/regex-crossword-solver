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


#ifndef CHARACTER_BLOCK_HPP
#define CHARACTER_BLOCK_HPP

#include "regex_token.hpp"

#include <memory>
#include <string>
#include <vector>

class SetOfCharacters;


// class hierarchy
// ---------------
// CharacterBlock
//     AnyCharacter
//     CharacterClass
//     CharacterRange
//     CompositeCharacterBlock
//     ShorthandCharacter
//     SingleCharacter


// An instance of a class derived from this abstract class represents a
// set of alternative characters in a regex. The length of a character
// block is always exactly one character.
//
// For example, '[A-D]' is a CharacterClass, with possible characters
// A, B, C and D.
class CharacterBlock
{
public:
    // instance creation and deletion
    virtual ~CharacterBlock() = 0;

    // copying
    std::unique_ptr<CharacterBlock> clone() const;

    // accessing
    SetOfCharacters characters() const;
    std::string explicit_characters() const;

    // converting
    std::string to_string() const;

protected:
    // querying
    static bool is_composite(const CharacterBlock& character_block);

private:
    // copying
    virtual std::unique_ptr<CharacterBlock> do_clone() const = 0;

    // accessing
    virtual SetOfCharacters do_characters() const = 0;
    virtual std::string do_explicit_characters() const = 0;

    // querying
    virtual bool is_composite() const;

    // converting
    virtual std::string do_to_string() const = 0;
};


// An instance of this class represents any character of the alphabet,
// and is noted '.'.
class AnyCharacter final : public CharacterBlock
{
public:
    // instance creation and deletion
    AnyCharacter() = default;

private:
    // copying
    std::unique_ptr<CharacterBlock> do_clone() const override;

    // accessing
    SetOfCharacters do_characters() const override;
    std::string do_explicit_characters() const override;

    // converting
    std::string do_to_string() const override;
};


// An instance of this class represents a character class.
//
// For example: '[A-D]'.
class CharacterClass final : public CharacterBlock
{
public:
    // instance creation and deletion
    CharacterClass(
      bool                                           is_negated,
      std::vector<std::unique_ptr<CharacterBlock>>&& character_blocks);

private:
    // copying
    std::unique_ptr<CharacterBlock> do_clone() const override;

    // accessing
    SetOfCharacters do_characters() const override;
    std::string do_explicit_characters() const override;

    // converting
    std::string do_to_string() const override;

    // data members

    bool m_is_negated;
    std::vector<std::unique_ptr<CharacterBlock>> m_character_blocks;
};


// An instance of this class represents a character range within a
// character class.
//
// For example, '[A-DH-L]' contains two character ranges: 'A-D' and
// 'H-L'.
class CharacterRange final : public CharacterBlock
{
public:
    // instance creation and deletion
    CharacterRange(char low, char high);

private:
    // copying
    std::unique_ptr<CharacterBlock> do_clone() const override;

    // accessing
    SetOfCharacters do_characters() const override;
    std::string do_explicit_characters() const override;

    // converting
    std::string do_to_string() const override;

    // data members

    char m_low;
    char m_high;
};


// An instance of this class is the composition of several other
// character blocks. A composite character block can only be the result
// of a union optimization.
class CompositeCharacterBlock final : public CharacterBlock
{
public:
    // instance creation and deletion
    CompositeCharacterBlock(
      std::unique_ptr<CharacterBlock>&& character_block_1,
      std::unique_ptr<CharacterBlock>&& character_block_2);
    explicit CompositeCharacterBlock(
               std::vector<std::unique_ptr<CharacterBlock>>&& character_blocks);

private:
    // instance creation and deletion
    void append(std::unique_ptr<CharacterBlock>&& character_block);

    // copying
    std::unique_ptr<CharacterBlock> do_clone() const override;

    // accessing
    SetOfCharacters do_characters() const override;
    std::string do_explicit_characters() const override;

    // querying
    bool is_composite() const override;

    // converting
    std::string do_to_string() const override;

    // data members

    std::vector<std::unique_ptr<CharacterBlock>> m_character_blocks;
};


// An instance of this class represents a shorthand character.
//
// For example: '\d' (any digit).
class ShorthandCharacter final : public CharacterBlock
{
public:
    // instance creation and deletion
    explicit ShorthandCharacter(RegexToken::Type token_type);

private:
    // copying
    std::unique_ptr<CharacterBlock> do_clone() const override;

    // accessing
    SetOfCharacters do_characters() const override;
    std::string do_explicit_characters() const override;

    // converting
    std::string do_to_string() const override;

    // data members

    RegexToken::Type m_token_type;
};


// An instance of this class represents a single character.
//
// For example: 'a'.
class SingleCharacter final : public CharacterBlock
{
public:
    // instance creation and deletion
    explicit SingleCharacter(char c);

private:
    // copying
    std::unique_ptr<CharacterBlock> do_clone() const override;

    // accessing
    SetOfCharacters do_characters() const override;
    std::string do_explicit_characters() const override;

    // converting
    std::string do_to_string() const override;

    // data members

    char m_character;
};


#endif // CHARACTER_BLOCK_HPP
