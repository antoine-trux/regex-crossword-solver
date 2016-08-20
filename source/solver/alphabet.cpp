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


#include "alphabet.hpp"

#include "alphabet_capacity.hpp"
#include "logger.hpp"
#include "regex_crossword_solver_exception.hpp"
#include "set_of_characters.hpp"
#include "utils.hpp"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <set>
#include <stdexcept>

using namespace std;


namespace
{

// data

// For each character 'c' in the alphabet, the index of 'c' in
// 'g_characters_as_string' is the same as its index in 'g_characters'
// and in 'g_word_characters' (if 'c' is a word character).
//
// The elements of 'g_characters_as_string' (and hence also those of
// 'g_characters' and of 'g_word_characters') are sorted according to
// the natural comparison for type 'char'.
//
// For example, if the alphabet is { A, B, E, = }:
// * g_characters_as_string is "=ABE"
// * g_characters contains characters =, A, B and E, at indices 0, 1,
//   2, and 3, respectively
// * g_word_characters contains characters A, B and E, at indices 1,
//   2, and 3, respectively
string g_characters_as_string;
SetOfCharacters g_characters;
SetOfCharacters g_word_characters;

// Whether Alphabet::set() has been called (and Alphabet::reset() has
// not been called subsequently).
bool g_alphabet_has_been_set = false;

// querying

// For a definition of word characters, see the explanations for '\w' in
// https://docs.python.org/3/library/re.html#regular-expression-syntax
bool
is_word_character(char c)
{
    // Visual Studio Community 2015 Update 2 crashes when a character
    // with a value less than -1 is passed to isalnum():
    //
    //     Debug Assertion Failed!
    //     File: minkernel\crts\ucrt\src\appcrt\convert\isctype.cpp
    //     Line: 36
    //     Expression: c >= -1 && c <= 255
    //
    // The Standard does not mention such a restriction, so this seems
    // to be a bug in Visual Studio. We work around this problem by
    // checking whether 'c' has a positive value.
    return c >= 0 && (isalnum(c) || c == '_');
}

// error handling

void
throw_alphabet_is_too_large(set<char> characters)
{
    assert(characters.size() > Alphabet::capacity());

    string message;

    message += "the alphabet has ";
    message += Utils::to_string(characters.size());
    message += " characters:\n";

    const string characters_as_string(characters.cbegin(), characters.cend());
    message += "    '" + characters_as_string + "'\n";

    message += "the capacity of the alphabet is " +
               Utils::to_string(Alphabet::capacity()) + " characters\n";

    message += "increase the value returned by Alphabet::capacity() ";
    message += "and try again";

    throw AlphabetException(message);
}

} // unnamed namespace


// accessing

// Return the i'th character.
char
Alphabet::character_at(size_t i)
{
    assert(i < characters_as_string().size());
    return characters_as_string()[i];
}

const SetOfCharacters&
Alphabet::characters()
{
    assert(g_alphabet_has_been_set);
    return g_characters;
}

const string&
Alphabet::characters_as_string()
{
    assert(g_alphabet_has_been_set);
    return g_characters_as_string;
}

// Return the characters of the alphabet that are not in
// 'characters_to_omit'.
//
// Precondition:
// * all the elements of 'characters_to_omit' are in the alphabet
SetOfCharacters
Alphabet::complement(const string& characters_to_omit)
{
    return complement(SetOfCharacters(characters_to_omit));
}

// Return the characters of the alphabet that are not in
// 'characters_to_omit'.
//
// Precondition:
// * all the elements of 'characters_to_omit' are in the alphabet
SetOfCharacters
Alphabet::complement(const SetOfCharacters& characters_to_omit)
{
    assert(characters().contains(characters_to_omit));
    return characters() - characters_to_omit;
}

// Precondition:
// * 'c' is in the alphabet
size_t
Alphabet::index_of_character(char c)
{
    const auto& characters_as_string_ = characters_as_string();
    // Since 'characters_as_string_' is sorted, lower_bound() can be
    // used to find 'c'. lower_bound() is more efficient than find(), so
    // we use lower_bound().
    auto it = lower_bound(characters_as_string_.cbegin(),
                          characters_as_string_.cend(),
                          c);
    assert(it != characters_as_string_.cend() && *it == c);
    return static_cast<decltype(index_of_character(c))>(
              it - characters_as_string_.cbegin());
}

// Remove non-word characters from 'characters'.
//
// For a definition of word characters, see the explanations for '\w' in
// https://docs.python.org/3/library/re.html#regular-expression-syntax
void
Alphabet::remove_non_word_characters(SetOfCharacters& characters)
{
    assert(g_alphabet_has_been_set);
    characters &= g_word_characters;
}

// Remove word characters from 'characters'.
//
// For a definition of word characters, see the explanations for '\w' in
// https://docs.python.org/3/library/re.html#regular-expression-syntax
void
Alphabet::remove_word_characters(SetOfCharacters& characters)
{
    assert(g_alphabet_has_been_set);
    characters &= ~g_word_characters;
}

// querying

// Return whether the alphabet contains 'c'.
bool
Alphabet::has_character(char c)
{
    const auto& characters_as_string_ = characters_as_string();
    return binary_search(characters_as_string_.cbegin(),
                         characters_as_string_.cend(),
                         c);
}

// Return whether 'characters' has non-word characters.
//
// For a definition of word characters, see the explanations for '\w' in
// https://docs.python.org/3/library/re.html#regular-expression-syntax
bool
Alphabet::has_non_word_characters(const SetOfCharacters& characters)
{
    assert(g_alphabet_has_been_set);
    return !(characters & ~g_word_characters).empty();
}

// Return whether 'characters' has word characters.
//
// For a definition of word characters, see the explanations for '\w' in
// https://docs.python.org/3/library/re.html#regular-expression-syntax
bool
Alphabet::has_word_characters(const SetOfCharacters& characters)
{
    assert(g_alphabet_has_been_set);
    return !(characters & g_word_characters).empty();
}

// modifying

void
Alphabet::reset()
{
    g_characters_as_string.clear();
    g_characters.clear();
    g_word_characters.clear();

    g_alphabet_has_been_set = false;
}

// Set the contents of the alphabet to 'characters'.
//
// 'characters' must not be empty, and may contain duplicates.
void
Alphabet::set(const string& characters)
{
    if (characters.empty())
    {
        throw AlphabetException("alphabet must not be empty");
    }

    // 'std::set':
    // 1. gets rid of possible duplicates.
    // 2. sorts 'characters' according to the natural comparison for
    //    type 'char'
    const std::set<char> characters_without_duplicates(characters.cbegin(),
                                                       characters.cend());

    if (characters_without_duplicates.size() > capacity())
    {
        throw_alphabet_is_too_large(characters_without_duplicates);
    }

    g_characters_as_string.assign(characters_without_duplicates.cbegin(),
                                  characters_without_duplicates.cend());

    LOG("setting alphabet to " + Utils::quoted(g_characters_as_string));

    g_characters.clear();
    for (size_t i = 0; i != g_characters_as_string.size(); ++i)
    {
        g_characters.add_character_at(i);

        if (is_word_character(g_characters_as_string[i]))
        {
            g_word_characters.add_character_at(i);
        }
    }

    g_alphabet_has_been_set = true;
}
