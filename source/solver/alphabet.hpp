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


#ifndef ALPHABET_HPP
#define ALPHABET_HPP

#include <string>

class SetOfCharacters;


// The alphabet contains all the explicit characters in the regular
// expressions of a grid. Metacharacters ('(', '*', etc.) are not
// taken into account.
//
// For example, if a grid contains regular expressions "A[BC]*=AE" and
// "(AB)+", the alphabet is { A, B, C, E, = }.
//
// Shorthand characters ('\d', '\D', '\w', '\W') are treated as if they
// explicitly listed the characters they represent. For example, a regex
// containing '\d' causes the alphabet to include all the digit
// characters.
//
// Alphabet::set() must be called before any other function.
namespace Alphabet
{

// accessing
char character_at(size_t i);
const SetOfCharacters& characters();
const std::string& characters_as_string();
SetOfCharacters complement(const std::string& characters_to_omit);
SetOfCharacters complement(const SetOfCharacters& characters_to_omit);
size_t index_of_character(char c);
void remove_non_word_characters(SetOfCharacters& characters);
void remove_word_characters(SetOfCharacters& characters);

// querying
bool has_character(char c);
bool has_non_word_characters(const SetOfCharacters& characters);
bool has_word_characters(const SetOfCharacters& characters);

// modifying
void reset();
void set(const std::string& characters);

} // namespace Alphabet


#endif // ALPHABET_HPP
