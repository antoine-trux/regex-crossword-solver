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


#ifndef SET_OF_CHARACTERS_HPP
#define SET_OF_CHARACTERS_HPP

#include "alphabet_capacity.hpp"

#include <bitset>
#include <string>
#include <vector>


// An instance of this class represents a set of characters in the
// alphabet.
class SetOfCharacters final
{
public:
    class const_iterator;

    // instance creation and deletion
    SetOfCharacters();
    explicit SetOfCharacters(char c);
    explicit SetOfCharacters(const std::string& characters);

    // accessing
    const_iterator begin() const;
    const_iterator end() const;
    size_t size() const;
    SetOfCharacters operator~() const;

    // querying
    bool contains(char c) const;
    bool contains(const SetOfCharacters& rhs) const;
    bool empty() const;
    bool has_only_non_word_characters() const;
    bool has_only_word_characters() const;
    bool not_empty() const;

    // converting
    std::string to_string() const;

    // modifying
    SetOfCharacters& operator&=(const SetOfCharacters& rhs);
    SetOfCharacters& operator|=(const SetOfCharacters& rhs);
    SetOfCharacters& operator-=(const SetOfCharacters& rhs);
    SetOfCharacters& operator-=(char c);
    void add_character_at(size_t index);
    void clear();
    void remove_non_word_characters();
    void remove_word_characters();

private:
    friend class const_iterator;
    friend bool operator==(const SetOfCharacters& lhs,
                           const SetOfCharacters& rhs);
    friend bool operator< (const SetOfCharacters& lhs,
                           const SetOfCharacters& rhs);

    static constexpr size_t m_bitset_size = Alphabet::capacity();

    using Bitset = std::bitset<m_bitset_size>;

    // instance creation and deletion
    explicit SetOfCharacters(const Bitset& bitset);

    // accessing
    static std::vector<Bitset> compute_bitmasks();
    static Bitset compute_bitset(const std::string& characters);
    static Bitset get_bitset(char c);
    static size_t index_of_bit_set(
        const Bitset& a_bitset, size_t index_of_first_bit_to_consider);

    // data members

    // 'm_bitmasks[i]' contains a Bitset with the 'i'th bit set and
    // the other bits unset:
    //     m_bitmasks[0] = 00...0001
    //     m_bitmasks[1] = 00...0010
    //     m_bitmasks[2] = 00...0100
    //     etc.
    static const std::vector<Bitset> m_bitmasks;

    // Each character is represented as a bit in 'm_bitset' - the index
    // of character 'c' in 'm_bitset' is
    // Alphabet::index_of_character(c).
    //
    // For example, if the alphabet is { A, B, E }:
    // * A is represented by m_bitset[0]
    // * B is represented by m_bitset[1]
    // * E is represented by m_bitset[2]
    Bitset m_bitset;
};

// accessing
SetOfCharacters operator&(const SetOfCharacters& lhs,
                          const SetOfCharacters& rhs);
SetOfCharacters operator&(const SetOfCharacters& lhs,
                          char                   c);
SetOfCharacters operator|(const SetOfCharacters& lhs,
                          const SetOfCharacters& rhs);
SetOfCharacters operator-(const SetOfCharacters& lhs,
                          const SetOfCharacters& rhs);

// querying
bool operator==(const SetOfCharacters& lhs, const SetOfCharacters& rhs);
bool operator!=(const SetOfCharacters& lhs, const SetOfCharacters& rhs);
bool operator< (const SetOfCharacters& lhs, const SetOfCharacters& rhs);


// An instance of this class iterates the elements of a SetOfCharacters.
//
// For example:
//
//     const auto s = SetOfCharacters("ABC");
//     for (auto it = s.begin(); it != s.end(); ++it)
//     {
//         cout << *it;
//     }
//
// or even:
//
//     const auto s = SetOfCharacters("ABC");
//     for (auto c : s) // SetOfCharacters::const_iterator used
//     {                // implicitly
//         cout << c;
//     }
//
// would both print "ABC" onto 'cout'.
class SetOfCharacters::const_iterator final
{
public:
    // instance creation and deletion
    const_iterator(const SetOfCharacters::Bitset& a_bitset,
                   size_t                         bit_index);

    // accessing
    char operator*() const;

    // modifying
    const_iterator& operator++();

private:
    friend bool operator==(const const_iterator& lhs,
                           const const_iterator& rhs);

    // querying
    bool class_invariant() const;

    // data members

    // The underlying data structure of the iterated SetOfCharacters.
    const SetOfCharacters::Bitset& m_bitset;

    // The index of an element of 'm_bitset' which is set, or
    // m_bitset.size() if this iterator is at the end position.
    //
    // For example, if the alphabet is { A, B, C, D, E }, and this
    // iterator iterates the SetOfCharacters s = { A, C, E }:
    // * auto it = s.begin(); // m_bit_index is 0 (A)
    // * ++it; // m_bit_index is 2 (C)
    // * ++it; // m_bit_index is 4 (E)
    // * ++it; // m_bit_index is m_bit_index.size() = 5 (end)
    size_t m_bit_index;
};

// querying
bool operator==(const SetOfCharacters::const_iterator& lhs,
                const SetOfCharacters::const_iterator& rhs);
bool operator!=(const SetOfCharacters::const_iterator& lhs,
                const SetOfCharacters::const_iterator& rhs);


#endif // SET_OF_CHARACTERS_HPP
