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


#include "set_of_characters.hpp"

#include "alphabet.hpp"

#include <cassert>
#include <numeric>

using namespace std;


// static data members
const vector<SetOfCharacters::Bitset> SetOfCharacters::m_bitmasks =
    SetOfCharacters::compute_bitmasks();


// SetOfCharacters
// ---------------

// instance creation and deletion

// Construct an empty set of characters.
SetOfCharacters::SetOfCharacters()
{
}

// Construct a set of characters which contains a single character, 'c'.
SetOfCharacters::SetOfCharacters(char c) :
  SetOfCharacters(get_bitset(c))
{
}

// Construct a set of characters which contains 'characters'.
SetOfCharacters::SetOfCharacters(const string& characters) :
  SetOfCharacters(compute_bitset(characters))
{
}

SetOfCharacters::SetOfCharacters(const Bitset& bitset) :
  m_bitset(bitset)
{
}

// accessing

// Return an iterator referring to the first element of this set of
// characters, or to the past-the-end element if this set of characters
// is empty.
SetOfCharacters::const_iterator
SetOfCharacters::begin() const
{
    return const_iterator(m_bitset, index_of_bit_set(m_bitset, 0));
}

// See description of 'SetOfCharacters::m_bitmasks'.
vector<SetOfCharacters::Bitset>
SetOfCharacters::compute_bitmasks()
{
    vector<Bitset> result;

    for (size_t i = 0; i != m_bitset_size; ++i)
    {
        Bitset bitmask;
        bitmask[i] = true;
        result.push_back(bitmask);
    }

    return result;
}

// Return the Bitset which corresponds to 'characters'.
//
// Precondition:
// * each element of 'characters' is in the alphabet
//
// For example, if the alphabet is { A, B, E }:
// * compute_bitset('AE') returns 00...0101
// * compute_bitset('BE') returns 00...0110
SetOfCharacters::Bitset
SetOfCharacters::compute_bitset(const string& characters)
{
    return accumulate(characters.cbegin(),
                      characters.cend(),
                      Bitset(),
                      [](const Bitset& sum, char c)
                      {
                          return sum | get_bitset(c);
                      });
}

// Return an iterator referring to the past-the-end element of this set
// of characters.
SetOfCharacters::const_iterator
SetOfCharacters::end() const
{
    return const_iterator(m_bitset, m_bitset_size);
}

// Return the Bitset which corresponds to 'c'.
//
// Precondition:
// * 'c' is in alphabet
//
// For example, if the alphabet is { A, B, E }:
// * get_bitset('A') returns 00...0001
// * get_bitset('B') returns 00...0010
// * get_bitset('E') returns 00...0100
SetOfCharacters::Bitset
SetOfCharacters::get_bitset(char c)
{
    assert(Alphabet::has_character(c));
    return m_bitmasks[Alphabet::index_of_character(c)];
}

// Return the smallest i in the interval
// [index_of_first_bit_to_consider, m_bitset_size) such that the i'th
// bit of 'a_bitset' is set. If there is no such i, return
// 'm_bitset_size'.
size_t
SetOfCharacters::index_of_bit_set(const Bitset& a_bitset,
                                  size_t        index_of_first_bit_to_consider)
{
    assert(a_bitset.size() == m_bitset_size);
    assert(index_of_first_bit_to_consider <= m_bitset_size);

    // In the contexts where this method is used, speed does not matter.
    // Therefore, we choose an inefficient but simple implementation,
    // instead of an implementation based on bit twiddling.

    for (size_t i = index_of_first_bit_to_consider; i != m_bitset_size; ++i)
    {
        if (a_bitset[i])
        {
            return i;
        }
    }

    return m_bitset_size;
}

// Return the number of characters in this set of characters.
size_t
SetOfCharacters::size() const
{
    return m_bitset.count();
}

// Return the complement of this set of characters with respect to the
// alphabet (that is, the elements of the alphabet which do not belong
// to this set of characters).
//
// For example, if the alphabet is { A, B, E }:
// * if this set of characters is { A, B }, ~ returns { E }
// * if this set of characters is { B }, ~ returns { A, E }
SetOfCharacters
SetOfCharacters::operator~() const
{
    return SetOfCharacters(~m_bitset & Alphabet::characters().m_bitset);
}

// Return the intersection of 'lhs' and 'rhs'.
SetOfCharacters
operator&(const SetOfCharacters& lhs, const SetOfCharacters& rhs)
{
    auto result = lhs;
    result &= rhs;
    return result;
}

// Return the intersection of 'lhs' and { c }.
SetOfCharacters
operator&(const SetOfCharacters& lhs, char c)
{
    return lhs & SetOfCharacters(c);
}

// Return the union of 'lhs' and 'rhs'.
SetOfCharacters
operator|(const SetOfCharacters& lhs, const SetOfCharacters& rhs)
{
    auto result = lhs;
    result |= rhs;
    return result;
}

// Return the elements of 'lhs' which do not belong to 'rhs'.
SetOfCharacters
operator-(const SetOfCharacters& lhs, const SetOfCharacters& rhs)
{
    auto result = lhs;
    result -= rhs;
    return result;
}

// querying

bool
SetOfCharacters::contains(char c) const
{
    return m_bitset[Alphabet::index_of_character(c)];
}

bool
SetOfCharacters::contains(const SetOfCharacters& rhs) const
{
    return (m_bitset | ~rhs.m_bitset).all();
}

bool
SetOfCharacters::empty() const
{
    return m_bitset.none();
}

// Return true iff the following conditions both hold:
// 1. this set of characters is not empty
// 2. this set of characters contains only non-word characters
//
// For a definition of word characters, see the explanations for '\w' in
// https://docs.python.org/3/library/re.html#regular-expression-syntax
bool
SetOfCharacters::has_only_non_word_characters() const
{
    return Alphabet::has_non_word_characters(*this) &&
           !Alphabet::has_word_characters(*this);
}

// Return true iff the following conditions both hold:
// 1. this set of characters is not empty
// 2. this set of characters contains only word characters
//
// For a definition of word characters, see the explanations for '\w' in
// https://docs.python.org/3/library/re.html#regular-expression-syntax
bool
SetOfCharacters::has_only_word_characters() const
{
    return Alphabet::has_word_characters(*this) &&
           !Alphabet::has_non_word_characters(*this);
}

bool
SetOfCharacters::not_empty() const
{
    return !empty();
}

bool
operator==(const SetOfCharacters& lhs, const SetOfCharacters& rhs)
{
    return lhs.m_bitset == rhs.m_bitset;
}

bool
operator!=(const SetOfCharacters& lhs, const SetOfCharacters& rhs)
{
    return !(lhs == rhs);
}

// Return whether 'lhs' is "less than" 'rhs'.
//
// The meaning of "less than" is not specified in this context, except
// that it satisfies the strict weak ordering requirements (irreflexive,
// antisymmetric, transitive), so this operator can be used, for
// example, for comparing the results of sorting operations.
//
// Used by unit tests.
bool
operator<(const SetOfCharacters& lhs,
          const SetOfCharacters& rhs)
{
    // This implementation is not fast, but what else? Comparing
    // lhs.to_ullong() and rhs.to_ullong() would not work, because the
    // bitset values may not fit in an unsigned long long (in which case
    // to_ullong() would throw an 'std::overflow_error').
    //
    // Anyway, this function is only used by unit tests, so speed is not
    // critical.

    for (size_t i = 0; i != SetOfCharacters::m_bitset_size; ++i)
    {
        if (lhs.m_bitset[i] ^ rhs.m_bitset[i])
        {
            return rhs.m_bitset[i];
        }
    }

    return false;
}

// converting

string
SetOfCharacters::to_string() const
{
    string result;

    // This range-based for loop uses 'SetOfCharacters::const_iterator'
    // under the hoods.
    for (auto c : *this)
    {
        result.push_back(c);
    }

    return result;
}

// modifying

// Modify this set of characters by replacing it with its intersection
// with 'rhs'. Return this set of characters.
SetOfCharacters&
SetOfCharacters::operator&=(const SetOfCharacters& rhs)
{
    m_bitset &= rhs.m_bitset;
    return *this;
}

// Modify this set of characters by replacing it with its union with
// 'rhs'. Return this set of characters.
SetOfCharacters&
SetOfCharacters::operator|=(const SetOfCharacters& rhs)
{
    m_bitset |= rhs.m_bitset;
    return *this;
}

// Modify this set of characters by replacing it with its difference
// with 'rhs'. Return this set of characters.
SetOfCharacters&
SetOfCharacters::operator-=(const SetOfCharacters& rhs)
{
    m_bitset &= ~rhs.m_bitset;
    return *this;
}

// Modify this set of characters by replacing it with its difference
// with { c }. Return this set of characters.
SetOfCharacters&
SetOfCharacters::operator-=(char c)
{
    m_bitset.reset(Alphabet::index_of_character(c));
    return *this;
}

void
SetOfCharacters::add_character_at(size_t index)
{
    assert(index < m_bitset_size);
    m_bitset.set(index);
}

void
SetOfCharacters::clear()
{
    m_bitset.reset();
}

// Remove from this set of characters any non-word characters it may
// contain.
//
// For a definition of word characters, see the explanations for '\w' in
// https://docs.python.org/3/library/re.html#regular-expression-syntax
void
SetOfCharacters::remove_non_word_characters()
{
    Alphabet::remove_non_word_characters(*this);
}

// Remove from this set of characters any word characters it may
// contain.
//
// For a definition of word characters, see the explanations for '\w' in
// https://docs.python.org/3/library/re.html#regular-expression-syntax
void
SetOfCharacters::remove_word_characters()
{
    Alphabet::remove_word_characters(*this);
}


// SetOfCharacters::const_iterator
// -------------------------------

// instance creation and deletion

SetOfCharacters::const_iterator::const_iterator(
                                   const SetOfCharacters::Bitset& a_bitset,
                                   size_t                         bit_index) :
  m_bitset(a_bitset),
  m_bit_index(bit_index)
{
    assert(class_invariant());
}

// accessing

char
SetOfCharacters::const_iterator::operator*() const
{
    assert(m_bit_index != m_bitset.size());
    assert(m_bitset[m_bit_index]);
    return Alphabet::character_at(m_bit_index);
}

// querying

bool
operator==(const SetOfCharacters::const_iterator& lhs,
           const SetOfCharacters::const_iterator& rhs)
{
    assert(lhs.class_invariant());
    assert(rhs.class_invariant());
    assert(lhs.m_bitset == rhs.m_bitset);
    return lhs.m_bit_index == rhs.m_bit_index;
}

bool
operator!=(const SetOfCharacters::const_iterator& lhs,
           const SetOfCharacters::const_iterator& rhs)
{
    return !(lhs == rhs);
}

bool
SetOfCharacters::const_iterator::class_invariant() const
{
    return m_bit_index <= m_bitset.size() &&
           (m_bit_index == m_bitset.size() || m_bitset[m_bit_index]);
}

// modifying

SetOfCharacters::const_iterator&
SetOfCharacters::const_iterator::operator++()
{
    assert(class_invariant());
    assert(m_bit_index != SetOfCharacters::m_bitset_size);

    m_bit_index = SetOfCharacters::index_of_bit_set(m_bitset, m_bit_index + 1);

    assert(class_invariant());
    return *this;
}
