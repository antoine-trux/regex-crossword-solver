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


#include "regex.hpp"

#include "backreference_numbers.hpp"
#include "character_block.hpp"
#include "constraint.hpp"
#include "regex_crossword_solver_exception.hpp"
#include "regex_optimizations.hpp"
#include "regex_parser.hpp"
#include "utils.hpp"

#include <algorithm>
#include <cassert>
#include <iterator>
#include <numeric>
#include <utility>

using namespace std;


// Regex
// -----

// instance creation and deletion

Regex::Regex() :
  m_parent(nullptr),
  m_constraint_size(0),
  m_begin_pos(0)
{
}

Regex::~Regex() = default;

// Return a 'unique_ptr' to 'optimized_regex', which is an optimized
// version of 'regex'. 'parent' is to be the parent of
// 'optimized_regex'.
unique_ptr<Regex>
Regex::optimized(unique_ptr<Regex> regex, Regex* optimized_regex, Regex* parent)
{
    if (optimized_regex == regex.get())
    {
        // The optimization did not change 'regex' (i.e., no
        // optimization was done).
        assert(regex->parent() == parent);
        return regex;
    }
    else
    {
        optimized_regex->set_parent(parent);
        return unique_ptr<Regex>(optimized_regex);
    }
}

// copying

unique_ptr<Regex>
Regex::clone(const Regex& regex, Regex* parent)
{
    auto copy = regex.clone();
    copy->set_parent(parent);
    return copy;
}

unique_ptr<Regex>
Regex::clone() const
{
    return do_clone();
}

// accessing

// Return the backreferences which are equal to 'regex' or descent from
// 'regex', and which reference 'group_number'.
vector<const BackreferenceRegex*>
Regex::backreferences_to(const Regex&       regex,
                         const GroupNumber& group_number)
{
    return regex.backreferences_to(group_number);
}

size_t
Regex::begin_pos(const Regex& regex)
{
    return regex.begin_pos();
}

// Return the beginning position of this regex within its constraint.
size_t
Regex::begin_pos() const
{
    return m_begin_pos;
}

// Return the constraint that results from all possible values of this
// regex applied to 'constraint'.
//
// For example, suppose this regex is '(A|[BC]C)*D*' and 'constraint' is
// { "ABCD", "BC", "ABCD" }. The successive values of length 3 of this
// regex yield:
//
// * AAA    => impossible constraint (A in the middle)
// * AAD    => impossible constraint (A in the middle)
// * AB[BC] => possible constraint { "A", "B", "BC" }
// * ADD    => impossible constraint (D in the middle)
// * B[BC]A => possible constraint { "B", "BC", "A" }
// * B[BC]D => possible constraint { "B", "BC", "D" }
// * DDD    => impossible constraint (D in the middle)
//
// When the possible constraints are ORed, we get the overall
// constraint { "AB", "BC", "ABCD" }, which this method returns.
Constraint
Regex::constrain(const Constraint& constraint)
{
    set_constraint_size(constraint.size());

    auto new_constraint = Constraint::none(constraint.size());

    rewind();

    while (not_at_end())
    {
        if (value_fits_exactly())
        {
            auto constraint_copy = constraint;

            if (constrain_with_current_value(constraint_copy))
            {
                new_constraint |= constraint_copy;
            }
        }

        increment();
    }

    assert(new_constraint.is_tighter_than_or_equal_to(constraint));
    return new_constraint;
}

bool
Regex::constrain_as_positive_lookahead(Regex&      regex,
                                       Constraint& constraint,
                                       size_t      begin_pos)
{
    return regex.constrain_as_positive_lookahead(constraint, begin_pos);
}

bool
Regex::constrain_as_positive_lookahead(Constraint& constraint, size_t begin_pos)
{
    auto new_constraint = Constraint::none(constraint.size());

    rewind(begin_pos);

    while (not_at_end())
    {
        if (value_fits())
        {
            auto constraint_copy = constraint;

            if (constrain_with_current_value(constraint_copy))
            {
                new_constraint |= constraint_copy;
            }
        }

        increment();
    }

    assert(new_constraint.is_tighter_than_or_equal_to(constraint));
    constraint = new_constraint;

    return constraint.is_possible();
}

bool
Regex::constrain_once_with_current_value(Regex&      regex,
                                         Constraint& constraint,
                                         size_t      offset)
{
    return regex.constrain_once_with_current_value(constraint, offset);
}

bool
Regex::constrain_once_with_current_value(Constraint& constraint)
{
    const size_t offset = 0;
    return constrain_once_with_current_value(constraint, offset);
}

// Same as constrain_with_current_value(), except that this method does
// not check for possible backreference propagations.
//
// Return boolean success.
bool
Regex::constrain_once_with_current_value(Constraint& constraint, size_t offset)
{
    assert(class_invariant());
    assert(has_a_value());
    assert(m_begin_pos + offset <= m_constraint_size);

    return do_constrain_once_with_current_value(constraint, offset);
}

// Modify 'constraint' with the current value of this regex.
//
// More specifically, each position of 'constraint' is ANDed with the
// current value of this regex:
//
//     constraint[i] = constraint[i] AND regex_value[i]
//
// See Regex::constrain() for further details.
//
// Return boolean success.
//
// Precondition:
// * this regex has a value
bool
Regex::constrain_with_current_value(Constraint& constraint)
{
    assert(class_invariant());
    assert(has_a_value());

    // When backreferences are involved, constraints may propagate
    // backwards. In order to take this phenomenon into account, we loop
    // until stability is reached, i.e., until there are no more such
    // constraint propagations.
    //
    // For example, suppose the regex '([AB])\1' is to constrain the
    // constraint { "AB", "A" }:
    // * regex      = '([AB])\1'
    // * constraint = { "AB", "A" }
    //
    // 1. When the CharacterBlockRegex '[AB]' constrains "AB", nothing
    //    happens (the first element "AB" of the constraint is not
    //    restricted):
    //    * regex      = '([AB])\1'
    //    * constraint = { "AB", "A" }
    //
    // 2. When the BackreferenceRegex '\1' constrains '[AB]' (by calling
    //    CharacterBlockRegex::do_constrain_once_with_current_value()
    //    with 'offset' = 1) to '[A]', the constraint itself is not
    //    modified. At this stage, the constraint is still { "AB", "A" }:
    //    * regex      = '([A])\1'
    //    * constraint = { "AB", "A" }
    //
    // 3. Because the ConcatenationRegex '([AB])\1' was modified by a
    //    backreference (to '[A]'), the regex (now '([A])\1')
    //    constrains the constraint once more. This restricts the
    //    constraint to { "A", "A" }, as desired:
    //    * regex      = '([A])\1'
    //    * constraint = { "A", "A" }

    auto success = true;

    do
    {
        do_reset_characters_were_constrained_by_backreference();
        success = constrain_once_with_current_value(constraint);
        if (!success)
        {
            break;
        }
    }
    while (do_characters_were_constrained_by_backreference());

    do_reset_after_constrain();

    return success && constrain_word_boundaries_with_current_value(constraint);
}

bool
Regex::constrain_word_boundaries_with_current_value(Regex&      regex,
                                                    Constraint& constraint)
{
    return regex.constrain_word_boundaries_with_current_value(constraint);
}

// Apply any word boundary sub-regexes ('\b' and '\B') in 'regex' to
// 'constraint'.
//
// Return boolean success.
bool
Regex::constrain_word_boundaries_with_current_value(Constraint& constraint)
{
    // We do no propagate word boundary constraints to backreferences,
    // because Python does not. For example:
    //
    //     python3.5
    //     >>> import re
    //     >>> re.match(r'^a(\b) a', 'a aa')   # (1)
    //     <_sre.SRE_Match object; span=(0, 3), match='a a'>
    //     >>> re.match(r'^a(\b) a\b', 'a aa') # (2)
    //     >>> re.match(r'^a(\b) a\1', 'a aa') # (3)
    //     <_sre.SRE_Match object; span=(0, 3), match='a a'>
    // (1): match    - the first '\b' is indeed at a word boundary
    // (2): no match - the second '\b' is not at a word boundary, which
    //                 prevents the match
    // (3): match    - '\1' refers to '\b', but this does not prevent
    //                 the match

    assert(class_invariant());
    assert(has_a_value());

    return do_constrain_word_boundaries_with_current_value(constraint);
}

size_t
Regex::constraint_size() const
{
    return m_constraint_size;
}

// Used by unit tests.
//
// This is similar to Regex::constrain(), except that:
// * this regex is applied 'begin_pos' characters from the start of
//   'constraint' (instead of from the start of 'constraint')
// * this method takes into account values of this regex that do not
//   extend to the end of 'constraint' (instead of exactly to the end
//   of 'constraint')
// * this method returns a vector which contains the possible
//   constraints (instead of the ORed value of these constraints)
vector<Constraint>
Regex::constraints(const Constraint& constraint, size_t begin_pos)
{
    vector<Constraint> result;

    set_constraint_size(constraint.size());

    rewind(begin_pos);

    while (not_at_end())
    {
        auto constraint_copy = constraint;

        if (constrain_with_current_value(constraint_copy))
        {
            result.push_back(constraint_copy);
        }

        increment();
    }

    return result;
}

const GroupRegex*
Regex::enclosing_group(const Regex& regex)
{
    return regex.enclosing_group();
}

// Return the closest GroupRegex that encloses this regex, or 'nullptr'
// if there is no such GroupRegex.
//
// For example:
// * calling this method on sub-regex 'A' of '((A)|B)C' returns '(A)'
// * calling this method on sub-regex 'B' of '((A)|B)C' returns '((A)|B)'
// * calling this method on sub-regex 'C' of '((A)|B)C' returns 'nullptr'
const GroupRegex*
Regex::enclosing_group() const
{
    if (!has_parent())
    {
        return nullptr;
    }

    return m_parent->yourself_or_enclosing_group();
}

// Return the closest PositiveLookaheadRegex that encloses this regex,
// 'or nullptr' if there is no such PositiveLookaheadRegex.
//
// For example:
// * calling this method on sub-regex 'A' of '(?=A)B' returns '(?=A)'
// * calling this method on sub-regex 'B' of '(?=A)B' returns 'nullptr'
const PositiveLookaheadRegex*
Regex::enclosing_lookahead() const
{
    if (!has_parent())
    {
        return nullptr;
    }

    return m_parent->yourself_or_enclosing_lookahead();
}

size_t
Regex::end_pos(const Regex& regex)
{
    return regex.end_pos();
}

// Return the end position of this regex within its constraint.
size_t
Regex::end_pos() const
{
    return m_begin_pos + length_of_current_value();
}

// Return the characters which appear explicitly in this regex.
string
Regex::explicit_characters() const
{
    return do_explicit_characters();
}

// Add to 'used_backreference_numbers' the backreference numbers used
// in 'regex'.
//
// For example, if 'regex' is '(A)(B)\2', add 2 to
// 'used_backreference_numbers' (not 1, because group 1 is not
// backreferenced here).
void
Regex::get_used_backreference_numbers(
         const Regex&          regex,
         BackreferenceNumbers& used_backreference_numbers)
{
    regex.do_get_used_backreference_numbers(used_backreference_numbers);
}

// Return the groups contained in 'regex'.
vector<const GroupRegex*>
Regex::groups(const Regex& regex)
{
    return regex.groups();
}

size_t
Regex::invalid_begin_pos(size_t constraint_size)
{
    return constraint_size + 1;
}

size_t
Regex::length_of_current_value(const Regex& regex)
{
    return regex.length_of_current_value();
}

size_t
Regex::length_of_current_value() const
{
    return do_length_of_current_value();
}

Regex*
Regex::parent(const Regex& regex)
{
    return regex.parent();
}

Regex*
Regex::parent() const
{
    return m_parent;
}

unique_ptr<Regex>
Regex::parse(const string& regex_as_string)
{
    auto regex = RegexParser::parse(regex_as_string);
    regex->check_no_self_references();
    regex->check_lookaheads_are_not_referenced_from_outside();
    assert(regex->parents_are_correctly_setup());
    return regex;
}

// Return the rightmost active GroupRegex numbered 'group_number' which
// is, in the parse tree, to the left of the path which goes from the
// root to 'from_child', knowing that 'from_child' is a child of
// 'regex'. If there is no such GroupRegex, return 'nullptr'.
//
// Precondition:
// * 'from_child' is a (direct) child of 'regex'
//
// Example 1:
//
//     whole regex = (A)B\1, and:
/*
 *                         . <-- root (a ConcatenationRegex)
 *                        / \
 *                       /   \
 *     GroupRegex 1 --> ()    . <-- 'regex' (a ConcatenationRegex)
 *                      |    / \
 *                      |   /   \
 *                      A   B   \1 <-- 'from_child'
 */
//     'group_number' = 1
//
//     This method returns GroupRegex 1.
//
// Example 2:
//
//     whole regex = ((A)|B)\2, and:
/*
 *                           . <-- root (a ConcatenationRegex)
 *                          / \    = 'regex'
 *                         /   \
 *     GroupRegex 1 -->   ()   \2 <-- 'from_child'
 *                        |
 *                        |
 *                       '|' <-- UnionRegex
 *                        /\
 *                       /  \
 *     GroupRegex 2 --> ()   B
 *     (active child of |
 *      the UnionRegex) |
 *                      A
 */
//     'group_number' = 2
//
//     During the iteration process, the active child of the UnionRegex
//     is its left child (GroupRegex 2). The value of the whole regex is
//     "AA".
//
//     This method returns GroupRegex 2.
//
// Example 3:
//
//     whole regex = ((A)|B)\2, and:
/*
 *                           . <-- root (a ConcatenationRegex) = 'regex'
 *                          / \
 *                         /   \
 *     GroupRegex 1 -->   ()   \2 <-- 'from_child'
 *                        |
 *                        |
 *                       '|' <-- UnionRegex
 *                        /\
 *                       /  \
 *     GroupRegex 2 --> ()   B (active child of the UnionRegex)
 *                      |
 *                      |
 *                      A
 */
//     'group_number' = 2
//
//     During the iteration process, the active child of the UnionRegex
//     is its right child (B). The whole regex has no value.
//
//     This method returns 'nullptr'.
GroupRegex*
Regex::rightmost_group(Regex&             regex,
                       const GroupNumber& group_number,
                       const Regex*       from_child)
{
    return regex.do_rightmost_group(group_number, from_child);
}

// Return the rightmost active GroupRegex numbered 'group_number'
// which, in the parse tree, originates at 'regex'. If there is no such
// GroupRegex, return 'nullptr'.
GroupRegex*
Regex::rightmost_group(Regex& regex, const GroupNumber& group_number)
{
    return regex.do_rightmost_group(group_number);
}

GroupRegex*
Regex::rightmost_group_from_parent(const GroupNumber& group_number) const
{
    if (m_parent == nullptr)
    {
        return nullptr;
    }

    return m_parent->do_rightmost_group(group_number, this);
}

// Return the root of the parse tree of this regex.
const Regex&
Regex::root() const
{
    if (!has_parent())
    {
        return *this;
    }

    return parent()->root();
}

// If this regex is a group regex, return it. Otherwise, return the
// group regex that encloses this regex, or 'nullptr' if there is no
// such group regex.
const GroupRegex*
Regex::yourself_or_enclosing_group() const
{
    return enclosing_group();
}

// If this regex is a lookahead regex, return it. Otherwise, return the
// lookahead regex that encloses this regex, or 'nullptr' if there is no
// such lookahead regex.
const PositiveLookaheadRegex*
Regex::yourself_or_enclosing_lookahead() const
{
    return enclosing_lookahead();
}

// querying

bool
Regex::at_end(const Regex& regex)
{
    return regex.at_end();
}

// Return whether 'regex' is positioned at one past its last value.
//
// For a description of the iteration process as a whole, see
// Regex::rewind().
bool
Regex::at_end() const
{
    return do_at_end();
}

bool
Regex::begin_pos_is_not_set() const
{
    return m_begin_pos == invalid_begin_pos(m_constraint_size);
}

bool
Regex::can_be_concatenated(const Regex& regex)
{
    return regex.do_can_be_concatenated();
}

bool
Regex::can_be_unified(const Regex& regex)
{
    return regex.do_can_be_unified();
}

bool
Regex::characters_were_constrained_by_backreference(const Regex& regex)
{
    return regex.do_characters_were_constrained_by_backreference();
}

// Since this method (indirectly) calls virtual methods, it is best not
// to call it in constructors of classes that have subclasses.
bool
Regex::class_invariant() const
{
    return constraint_size_is_not_set() ||
           begin_pos_is_not_set() || has_no_value() || value_fits();
}

bool
Regex::constraint_size_is_not_set() const
{
    return m_constraint_size == 0;
}

bool
Regex::do_can_be_concatenated() const
{
    return false;
}

bool
Regex::do_can_be_unified() const
{
    return false;
}

bool
Regex::do_has_a_value() const
{
    return not_at_end();
}

bool
Regex::do_is_character_block() const
{
    return false;
}

bool
Regex::do_is_concatenation() const
{
    return false;
}

bool
Regex::do_is_empty() const
{
    return false;
}

bool
Regex::do_is_string() const
{
    return false;
}

bool
Regex::do_is_union() const
{
    return false;
}

bool
Regex::has_a_value(const Regex& regex)
{
    return regex.has_a_value();
}

bool
Regex::has_a_value() const
{
    return do_has_a_value();
}

bool
Regex::has_a_value_which_fits(const Regex& regex)
{
    return regex.has_a_value_which_fits();
}

bool
Regex::has_a_value_which_fits() const
{
    return has_a_value() && value_fits();
}

// Return whether 'regex' is an ancestor of this regex.
bool
Regex::has_ancestor(const Regex* regex) const
{
    if (!has_parent())
    {
        return false;
    }

    if (m_parent == regex)
    {
        return true;
    }
    else
    {
        return m_parent->has_ancestor(regex);
    }
}

bool
Regex::has_no_value() const
{
    return !has_a_value();
}

bool
Regex::has_parent() const
{
    return m_parent != nullptr;
}

bool
Regex::is_character_block(const Regex& regex)
{
    return regex.do_is_character_block();
}

bool
Regex::is_concatenation(const Regex& regex)
{
    return regex.do_is_concatenation();
}

bool
Regex::is_empty(const Regex& regex)
{
    return regex.do_is_empty();
}

bool
Regex::is_root() const
{
    return m_parent == nullptr;
}

bool
Regex::is_string(const Regex& regex)
{
    return regex.do_is_string();
}

bool
Regex::is_union(const Regex& regex)
{
    return regex.do_is_union();
}

// Return whether 'regex' is not positioned at the end during iteration.
bool
Regex::not_at_end(const Regex& regex)
{
    return regex.not_at_end();
}

bool
Regex::not_at_end() const
{
    return !at_end();
}

bool
Regex::parents_are_correctly_setup(const Regex& regex)
{
    return regex.parents_are_correctly_setup();
}

bool
Regex::parents_are_correctly_setup() const
{
    return do_parents_are_correctly_setup();
}

bool
Regex::value_fits() const
{
    assert(has_a_value());

    return end_pos() <= m_constraint_size;
}

bool
Regex::value_fits_exactly() const
{
    assert(has_a_value());

    return end_pos() == m_constraint_size;
}

// Return whether the current value of 'regex' is epsilon.
bool
Regex::value_is_epsilon(const Regex& regex)
{
    assert(regex.has_a_value());

    return regex.m_begin_pos == regex.end_pos();
}

// converting

// Precondition:
// * 'regex' is an instance of 'TargetType', or of a type which publicly
//   inherits 'TargetType'
template<typename TargetType>
TargetType&
Regex::downcast(Regex& regex)
{
    assert(dynamic_cast<TargetType*>(&regex) != nullptr);

    return static_cast<TargetType&>(regex);
}

// Precondition:
// * '*regex' is an instance of 'TargetType', or of a type which publicly
//   inherits 'TargetType'
template<typename TargetType>
unique_ptr<TargetType>
Regex::downcast(unique_ptr<Regex> regex)
{
    assert(dynamic_cast<TargetType*>(regex.get()) != nullptr);

    const auto raw_pointer = static_cast<TargetType*>(regex.release());
    return unique_ptr<TargetType>(raw_pointer);
}

string
Regex::to_string() const
{
    return do_to_string();
}

// modifying

Regex*
Regex::do_optimize_concatenations_on_left()
{
    return do_optimize_concatenations();
}

Regex*
Regex::do_optimize_concatenations_on_right()
{
    return do_optimize_concatenations();
}

void
Regex::do_set_constraint_size(size_t constraint_size)
{
    m_constraint_size = constraint_size;
    m_begin_pos = invalid_begin_pos(constraint_size);

    set_constraint_size_of_children(constraint_size);
}

void
Regex::increment(Regex& regex)
{
    regex.increment();
}

// Advance this regex to the next value which fits the constraint size.
//
// If this regex has no next value which fits the constraint size,
// calling this method causes this regex to be positioned at the end.
//
// Precondition:
// * this regex is not positioned at the end
//
// For example, if this regex is '[AB]|C*', 'm_begin_pos' is 0 and
// 'm_constraint_size' is 2, the successive values are:
// 1. [AB]         - value length = 1
// 2. empty string - value length = 0
// 3. C            - value length = 1
// 4. CC           - value length = 2
// 5. (end position - no value)
//
// For a description of the iteration process as a whole, see
// Regex::rewind().
void
Regex::increment()
{
    assert(class_invariant());
    assert(not_at_end());

    do_increment();

    assert(at_end() || has_a_value_which_fits());
}

// Optimize 'root' according to 'optimizations'.
unique_ptr<Regex>
Regex::optimize(unique_ptr<Regex>         root,
                const RegexOptimizations& optimizations)
{
    // The order of optimizations is important in order to get the best
    // results.

    if (optimizations.optimize_groups())
    {
        root = optimize_groups_of_root(move(root));
    }

    if (optimizations.optimize_unions())
    {
        root = optimize_unions_of_root(move(root));
    }

    if (optimizations.optimize_concatenations())
    {
        root = optimize_concatenations_of_root(move(root));
    }

    return root;
}

unique_ptr<Regex>
Regex::optimize_concatenations(unique_ptr<Regex> regex)
{
    const auto parent = regex->parent();
    const auto optimized_regex = regex->do_optimize_concatenations();
    return optimized(move(regex), optimized_regex, parent);
}

// Optimize the ConcatenationRegex'es of 'root'.
unique_ptr<Regex>
Regex::optimize_concatenations_of_root(unique_ptr<Regex> root)
{
    assert(root->is_root());

    root = optimize_concatenations(move(root));
    assert(root->parents_are_correctly_setup());
    return root;
}

unique_ptr<Regex>
Regex::optimize_concatenations_on_left(unique_ptr<Regex> regex)
{
    const auto parent = regex->parent();
    const auto optimized_regex = regex->do_optimize_concatenations_on_left();
    return optimized(move(regex), optimized_regex, parent);
}

unique_ptr<Regex>
Regex::optimize_concatenations_on_right(unique_ptr<Regex> regex)
{
    const auto parent = regex->parent();
    const auto optimized_regex = regex->do_optimize_concatenations_on_right();
    return optimized(move(regex), optimized_regex, parent);
}

unique_ptr<Regex>
Regex::optimize_groups(unique_ptr<Regex>           regex,
                       const BackreferenceNumbers& used_backreference_numbers)
{
    const auto parent = regex->parent();
    const auto optimized_regex =
        regex->do_optimize_groups(used_backreference_numbers);
    return optimized(move(regex), optimized_regex, parent);
}

// Optimize the GroupRegex'es of 'root'.
unique_ptr<Regex>
Regex::optimize_groups_of_root(unique_ptr<Regex> root)
{
    assert(root->is_root());

    BackreferenceNumbers used_backreference_numbers;
    get_used_backreference_numbers(*root, used_backreference_numbers);

    root = optimize_groups(move(root), used_backreference_numbers);
    assert(root->parents_are_correctly_setup());
    return root;
}

unique_ptr<Regex>
Regex::optimize_unions(unique_ptr<Regex> regex)
{
    const auto parent = regex->parent();
    const auto optimized_regex = regex->do_optimize_unions();
    return optimized(move(regex), optimized_regex, parent);
}

// Optimize the UnionRegex'es of 'root'.
unique_ptr<Regex>
Regex::optimize_unions_of_root(unique_ptr<Regex> root)
{
    assert(root->is_root());

    root = optimize_unions(move(root));
    assert(root->parents_are_correctly_setup());
    return root;
}

void
Regex::reset_after_constrain(Regex& regex)
{
    regex.do_reset_after_constrain();
}

void
Regex::reset_characters_were_constrained_by_backreference(Regex& regex)
{
    regex.do_reset_characters_were_constrained_by_backreference();
}

void
Regex::rewind(Regex& regex, size_t begin_pos)
{
    regex.rewind(begin_pos);
}

void
Regex::rewind()
{
    const size_t begin_pos = 0;
    rewind(begin_pos);
}

// Position this regex to its first value (if any), so it begins at the
// 'begin_pos'th character within the constraint.
//
// Iteration through the possible values of a regex starting at a
// certain position and fitting within a certain length is done with the
// following steps:
// 1. call rewind(),
// 2. while not_at_end() returns true, call increment().
void
Regex::rewind(size_t begin_pos)
{
    assert(class_invariant());

    set_begin_pos(begin_pos);
    do_rewind();

    assert(at_end() || has_a_value_which_fits());
}

void
Regex::set_begin_pos(size_t begin_pos)
{
    assert(begin_pos <= m_constraint_size);

    m_begin_pos = begin_pos;
}

void
Regex::set_constraint_size(Regex& regex, size_t constraint_size)
{
    regex.set_constraint_size(constraint_size);
}

void
Regex::set_constraint_size(size_t constraint_size)
{
    do_set_constraint_size(constraint_size);
}

void
Regex::set_parent(Regex& regex, Regex* parent)
{
    regex.set_parent(parent);
}

void
Regex::set_parent(Regex* parent)
{
    m_parent = parent;
}

// error handling

// Check that the lookaheads of this regex, if any, do not contain
// groups which are backreferenced from outside the lookahead.
//
// For example:
// * '(?=A(B))' passes the check, because its only group ('(B)') is not
//   backreferenced
// * '(?=A(B)\1)' passes the check, because its only group ('(B)') is
//   referenced only from within the lookahead which contains that group
// * '(?=A(B)\1)\1' does not pass the check, because it contains a group
//   ('(B)') which is referenced from outside the lookahead which
//   contains that group (namely, by the second instance of '\1')
void
Regex::check_lookaheads_are_not_referenced_from_outside() const
{
    for (auto group : groups())
    {
        const auto lookahead = group->enclosing_lookahead();

        if (lookahead == nullptr)
        {
            // 'group' is not within a lookahead, so 'group' is fine as
            // far as this check is concerned.
            continue;
        }

        for (auto backreference : backreferences_to(group->number()))
        {
            if (backreference->has_ancestor(lookahead))
            {
                // Both 'backreference' and 'group' are within the same
                // lookahead, so 'backreference' is fine as far as this
                // check is concerned.
                continue;
            }

            // 'backreference' it outside the lookahead to which
            // 'group' belongs.
            const string message =
                "in "                                 +
                Utils::quoted(to_string())            +
                ",\nlookahead "                       +
                Utils::quoted(lookahead->to_string()) +
                " contains a group ("                 +
                Utils::quoted(group->to_string())     +
                ")\nwhich is referenced outside the lookahead";
            throw RegexStructureException(message);
        }
    }
}

void
Regex::check_no_self_references(const Regex& regex)
{
    regex.check_no_self_references();
}

void
Regex::check_no_self_references() const
{
    do_check_no_self_references();
}


// NullaryRegex
// ------------

// instance creation and deletion

NullaryRegex::NullaryRegex() :
  m_at_end(false)
{
}

// accessing

vector<const BackreferenceRegex*>
NullaryRegex::backreferences_to(const GroupNumber& /*group_number*/) const
{
    return {};
}

// See Regex::constrain_once_with_current_value().
bool
NullaryRegex::do_constrain_once_with_current_value(
                Constraint& /*constraint*/, size_t /*offset*/)
{
    return true;
}

// See Regex::constrain_word_boundaries_with_current_value().
bool
NullaryRegex::do_constrain_word_boundaries_with_current_value(
                Constraint& /*constraint*/)
{
    return true;
}

string
NullaryRegex::do_explicit_characters() const
{
    return "";
}

void
NullaryRegex::do_get_used_backreference_numbers(
                BackreferenceNumbers& /*used_backreference_numbers*/) const
{
}

size_t
NullaryRegex::do_length_of_current_value() const
{
    return 0;
}

// See Regex::rightmost_group(Regex&, const GroupNumber&, const Regex*).
GroupRegex*
NullaryRegex::do_rightmost_group(const GroupNumber& /*group_number*/,
                                 const Regex*       /*from_child*/)
{
    // A NullaryRegex does not have children (by definition), so
    // 'from_child' cannot be a child of this regex, thus violating the
    // precondition of this method.
    assert(false);

    return nullptr;
}

// See Regex::rightmost_group(Regex&, const GroupNumber&).
GroupRegex*
NullaryRegex::do_rightmost_group(const GroupNumber& /*group_number*/)
{
    return nullptr;
}

vector<const GroupRegex*>
NullaryRegex::groups() const
{
    return {};
}

// querying

bool
NullaryRegex::do_at_end() const
{
    return m_at_end;
}

bool
NullaryRegex::do_characters_were_constrained_by_backreference() const
{
    return false;
}

// modifying

void
NullaryRegex::do_increment()
{
    m_at_end = true;
}

Regex*
NullaryRegex::do_optimize_concatenations()
{
    return this;
}

Regex*
NullaryRegex::do_optimize_groups(
                const BackreferenceNumbers& /*used_backreference_numbers*/)
{
    return this;
}

Regex*
NullaryRegex::do_optimize_unions()
{
    return this;
}

bool
NullaryRegex::do_parents_are_correctly_setup() const
{
    return true;
}

void
NullaryRegex::do_reset_after_constrain()
{
}

void
NullaryRegex::do_reset_characters_were_constrained_by_backreference()
{
}

void
NullaryRegex::do_rewind()
{
    m_at_end = false;

    if (!has_a_value_which_fits(*this))
    {
        m_at_end = true;
    }
}

void
NullaryRegex::set_constraint_size_of_children(size_t /*constraint_size*/)
{
}

// error handling

void
NullaryRegex::do_check_no_self_references() const
{
}


// EmptyRegex
// ----------

// instance creation and deletion

EmptyRegex::EmptyRegex()
{
    assert(class_invariant());
}

// copying

unique_ptr<Regex>
EmptyRegex::do_clone() const
{
    return Utils::make_unique<EmptyRegex>();
}

// querying

bool
EmptyRegex::do_at_end() const
{
    return true;
}

bool
EmptyRegex::do_can_be_unified() const
{
    return true;
}

bool
EmptyRegex::do_is_empty() const
{
    return true;
}

// converting

string
EmptyRegex::do_to_string() const
{
    return "empty";
}


// EpsilonRegex
// ------------

// instance creation and deletion

EpsilonRegex::EpsilonRegex()
{
    assert(class_invariant());
}

// copying

unique_ptr<Regex>
EpsilonRegex::do_clone() const
{
    return Utils::make_unique<EpsilonRegex>();
}

// querying

bool
EpsilonRegex::do_can_be_concatenated() const
{
    return true;
}

// converting

string
EpsilonRegex::do_to_string() const
{
    return "";
}


// EpsilonAtStartRegex
// -------------------

// instance creation and deletion

EpsilonAtStartRegex::EpsilonAtStartRegex()
{
    assert(class_invariant());
}

// copying

unique_ptr<Regex>
EpsilonAtStartRegex::do_clone() const
{
    return Utils::make_unique<EpsilonAtStartRegex>();
}

// accessing

// See Regex::constrain_once_with_current_value().
bool
EpsilonAtStartRegex::do_constrain_once_with_current_value(
                       Constraint& /*constraint*/, size_t offset)
{
    return begin_pos() + offset == 0;
}

// converting

string
EpsilonAtStartRegex::do_to_string() const
{
    return "^";
}


// EpsilonAtEndRegex
// -----------------

// instance creation and deletion

EpsilonAtEndRegex::EpsilonAtEndRegex()
{
    assert(class_invariant());
}

// copying

unique_ptr<Regex>
EpsilonAtEndRegex::do_clone() const
{
    return Utils::make_unique<EpsilonAtEndRegex>();
}

// accessing

// See Regex::constrain_once_with_current_value().
bool
EpsilonAtEndRegex::do_constrain_once_with_current_value(
                     Constraint& constraint, size_t offset)
{
    return begin_pos() + offset == constraint.size();
}

// converting

string
EpsilonAtEndRegex::do_to_string() const
{
    return "$";
}


// EpsilonAtWordBoundaryRegex
// --------------------------

// instance creation and deletion

EpsilonAtWordBoundaryRegex::EpsilonAtWordBoundaryRegex()
{
    assert(class_invariant());
}

// copying

unique_ptr<Regex>
EpsilonAtWordBoundaryRegex::do_clone() const
{
    return Utils::make_unique<EpsilonAtWordBoundaryRegex>();
}

// accessing

// See Regex::constrain_word_boundaries_with_current_value().
bool
EpsilonAtWordBoundaryRegex::do_constrain_word_boundaries_with_current_value(
                              Constraint& constraint)
{
    const auto begin_pos_ = begin_pos();

    if (constraint.empty())
    {
        assert(begin_pos_ == 0);
        // python3.5
        // >>> import re
        // >>> re.match(r'\b', '')
        // >>> # no match
        return false;
    }

    if (begin_pos_ == 0)
    {
        constraint[0].remove_non_word_characters();
        return constraint[0].not_empty();
    }

    if (begin_pos_ == constraint.size())
    {
        constraint[constraint.size() - 1].remove_non_word_characters();
        return constraint[constraint.size() - 1].not_empty();
    }

    const auto only_word_characters_before =
        constraint[begin_pos_ - 1].has_only_word_characters();

    const auto only_non_word_characters_before =
        constraint[begin_pos_ - 1].has_only_non_word_characters();

    const auto only_word_characters_after =
        constraint[begin_pos_].has_only_word_characters();

    const auto only_non_word_characters_after =
        constraint[begin_pos_].has_only_non_word_characters();

    if (only_word_characters_before)
    {
        constraint[begin_pos_].remove_word_characters();
    }

    if (only_non_word_characters_before)
    {
        constraint[begin_pos_].remove_non_word_characters();
    }

    if (only_word_characters_after)
    {
        constraint[begin_pos_ - 1].remove_word_characters();
    }

    if (only_non_word_characters_after)
    {
        constraint[begin_pos_ - 1].remove_non_word_characters();
    }

    return constraint[begin_pos_ - 1].not_empty() &&
           constraint[begin_pos_].not_empty();
}

// converting

string
EpsilonAtWordBoundaryRegex::do_to_string() const
{
    return "\\b";
}


// EpsilonNotAtWordBoundaryRegex
// -----------------------------

// instance creation and deletion

EpsilonNotAtWordBoundaryRegex::EpsilonNotAtWordBoundaryRegex()
{
    assert(class_invariant());
}

// copying

unique_ptr<Regex>
EpsilonNotAtWordBoundaryRegex::do_clone() const
{
    return Utils::make_unique<EpsilonNotAtWordBoundaryRegex>();
}

// accessing

// See Regex::constrain_word_boundaries_with_current_value().
bool
EpsilonNotAtWordBoundaryRegex::do_constrain_word_boundaries_with_current_value(
                                 Constraint& constraint)
{
    const auto begin_pos_ = begin_pos();

    if (constraint.empty())
    {
        assert(begin_pos_ == 0);

        // python3.5
        // >>> import re
        // >>> re.match(r'\B', '')
        // >>> # no match
        return false;
    }

    if (begin_pos_ == 0)
    {
        constraint[0].remove_word_characters();
        return constraint[0].not_empty();
    }

    if (begin_pos_ == constraint.size())
    {
        constraint[constraint.size() - 1].remove_word_characters();
        return constraint[constraint.size() - 1].not_empty();
    }

    const auto only_word_characters_before =
        constraint[begin_pos_ - 1].has_only_word_characters();

    const auto only_non_word_characters_before =
        constraint[begin_pos_ - 1].has_only_non_word_characters();

    const auto only_word_characters_after =
        constraint[begin_pos_].has_only_word_characters();

    const auto only_non_word_characters_after =
        constraint[begin_pos_].has_only_non_word_characters();

    if (only_word_characters_before)
    {
        constraint[begin_pos_].remove_non_word_characters();
    }

    if (only_non_word_characters_before)
    {
        constraint[begin_pos_].remove_word_characters();
    }

    if (only_word_characters_after)
    {
        constraint[begin_pos_ - 1].remove_non_word_characters();
    }

    if (only_non_word_characters_after)
    {
        constraint[begin_pos_ - 1].remove_word_characters();
    }

    return constraint[begin_pos_ - 1].not_empty() &&
           constraint[begin_pos_].not_empty();
}

// converting

string
EpsilonNotAtWordBoundaryRegex::do_to_string() const
{
    return "\\B";
}


// PositiveLookaheadRegex
// ----------------------

// instance creation and deletion

PositiveLookaheadRegex::PositiveLookaheadRegex(unique_ptr<Regex> regex) :
  m_regex(move(regex))
{
    set_parent(*m_regex, this);
    assert(class_invariant());
}

// copying

unique_ptr<Regex>
PositiveLookaheadRegex::do_clone() const
{
    return Utils::make_unique<PositiveLookaheadRegex>(m_regex->clone());
}

// accessing

vector<const BackreferenceRegex*>
PositiveLookaheadRegex::backreferences_to(const GroupNumber& group_number) const
{
    return Regex::backreferences_to(*m_regex, group_number);
}

// See Regex::constrain_word_boundaries_with_current_value().
bool
PositiveLookaheadRegex::do_constrain_once_with_current_value(
                          Constraint& constraint, size_t offset)
{
    const auto constraint_comes_from_backreference = (offset != 0);
    if (constraint_comes_from_backreference)
    {
        // This method is called as the result of a backreference
        // constraining a group which is an ancestor of this lookahead.
        // In such a context, this lookahead is to be treated as an
        // epsilon (i.e., a string of zero-length), which constrains
        // nothing.
        return true;
    }

    assert(offset == 0);
    return constrain_as_positive_lookahead(*m_regex, constraint, begin_pos());
}

string
PositiveLookaheadRegex::do_explicit_characters() const
{
    return m_regex->explicit_characters();
}

void
PositiveLookaheadRegex::do_get_used_backreference_numbers(
    BackreferenceNumbers& used_backreference_numbers) const
{
    get_used_backreference_numbers(*m_regex, used_backreference_numbers);
}

vector<const GroupRegex*>
PositiveLookaheadRegex::groups() const
{
    return Regex::groups(*m_regex);
}

const PositiveLookaheadRegex*
PositiveLookaheadRegex::yourself_or_enclosing_lookahead() const
{
    return this;
}

// converting

string
PositiveLookaheadRegex::do_to_string() const
{
    return "(?=" + m_regex->to_string() + ')';
}

// modifying

Regex*
PositiveLookaheadRegex::do_optimize_concatenations()
{
    m_regex = optimize_concatenations(move(m_regex));
    return this;
}

Regex*
PositiveLookaheadRegex::do_optimize_groups(
    const BackreferenceNumbers& used_backreference_numbers)
{
    m_regex = optimize_groups(move(m_regex), used_backreference_numbers);
    return this;
}

Regex*
PositiveLookaheadRegex::do_optimize_unions()
{
    m_regex = optimize_unions(move(m_regex));
    return this;
}

void
PositiveLookaheadRegex::set_constraint_size_of_children(size_t constraint_size)
{
    set_constraint_size(*m_regex, constraint_size);
}

// error handling

void
PositiveLookaheadRegex::do_check_no_self_references() const
{
    check_no_self_references(*m_regex);
}


// CharacterBlockRegex
// -------------------

// instance creation and deletion

CharacterBlockRegex::CharacterBlockRegex(
                       unique_ptr<CharacterBlock> character_block) :
  m_character_block(move(character_block)),
  m_characters_are_initialized(false),
  m_constrained_characters_are_initialized(false),
  m_characters_were_constrained_by_backreference(false)
{
}

// Precondition:
// * 'regex_1' and 'regex_2' are either character block regexes or empty
//   regexes
CharacterBlockRegex::CharacterBlockRegex(unique_ptr<Regex> regex_1,
                                         unique_ptr<Regex> regex_2) :
  m_characters_are_initialized(false),
  m_constrained_characters_are_initialized(false),
  m_characters_were_constrained_by_backreference(false)
{
    append_character_block_of_regex(move(regex_1));
    append_character_block_of_regex(move(regex_2));
}

// Precondition:
// * 'regex' is either a character block or an empty regex
void
CharacterBlockRegex::append_character_block_of_regex(unique_ptr<Regex> regex)
{
    if (is_empty(*regex))
    {
        // An EmptyRegex has no character blocks, so nothing to do.
        return;
    }

    assert(is_character_block(*regex));

    auto character_block_regex = downcast<CharacterBlockRegex>(move(regex));
    auto character_block = character_block_regex->steal_character_block();

    if (m_character_block)
    {
        m_character_block =
            Utils::make_unique<CompositeCharacterBlock>(
                     move(m_character_block), move(character_block));
    }
    else
    {
        m_character_block = move(character_block);
    }
}

// copying

unique_ptr<Regex>
CharacterBlockRegex::do_clone() const
{
    return Utils::make_unique<CharacterBlockRegex>(m_character_block->clone());
}

// accessing

const SetOfCharacters&
CharacterBlockRegex::characters() const
{
    if (!m_characters_are_initialized)
    {
        m_characters = compute_characters();
        m_characters_are_initialized = true;
    }

    return m_characters;
}

SetOfCharacters
CharacterBlockRegex::compute_characters() const
{
    return m_character_block->characters();
}

// See Regex::constrain_once_with_current_value().
bool
CharacterBlockRegex::do_constrain_once_with_current_value(
                       Constraint& constraint, size_t offset)
{
    if (!m_constrained_characters_are_initialized)
    {
        m_constrained_characters = characters();
        m_constrained_characters_are_initialized = true;
    }

    const auto begin_pos_ = begin_pos() + offset;

    const auto constrained_characters_before = m_constrained_characters;

    // Statement (S1) below possibly restricts
    // 'm_constrained_characters' for the case the regex to which this
    // CharacterBlockRegex belongs would contain a BackreferenceRegex
    // which references this CharacterBlockRegex.
    //
    // For example, suppose the regex '([AB])\1' is to constrain the
    // constraint { "A", "AB" }:
    // * regex      = '([AB])\1'
    // * constraint = { "A", "AB" }
    //
    // The constraining proceeds as follows:
    //
    // 1. When the CharacterBlockRegex '[AB]' constrains "A" (by calling
    //    this method with 'offset' = 0), statement (S1) restricts
    //    'm_constrained_characters' from "AB" to "A", and statement
    //    (S2) leaves 'constraint[0]' equal to "A":
    //    * regex      = '([A])\1'
    //    * constraint = { "A", "AB" }
    //
    // 2. When the BackreferenceRegex '\1' constrains "AB" (by calling
    //    this method on CharacterBlockRegex '[AB]' with 'offset' = 1),
    //    statement (S1) leaves 'm_constrained_characters' equal to "A",
    //    and statement (S2) restricts 'constraint[1]' from "AB" to "A":
    //    * regex      = '([A])\1'
    //    * constraint = { "A", "A" }
    //
    // The end result of the above steps if that 'constraint' is
    // restricted from { "A", "AB" } to { "A", "A" }, as desired.
    //
    // If step 1 had not restricted 'm_constrained_characters' from "AB"
    // to "A" with statement (S1), step 2 would not have restricted
    // 'constraint[1]' from "AB" to "A" with statement (S2). Thus, the
    // end result is that 'constraint' would have been left equal to
    // { "A", "AB" }.

    m_constrained_characters &= constraint[begin_pos_]; // (S1)
    constraint[begin_pos_] = m_constrained_characters;  // (S2)

    const auto constraint_comes_from_backreference = (offset != 0);

    if (constraint_comes_from_backreference)
    {
        m_characters_were_constrained_by_backreference =
            (m_constrained_characters != constrained_characters_before);
    }

    return m_constrained_characters.not_empty();
}

string
CharacterBlockRegex::do_explicit_characters() const
{
    return m_character_block->explicit_characters();
}

size_t
CharacterBlockRegex::do_length_of_current_value() const
{
    return 1;
}

unique_ptr<CharacterBlock>
CharacterBlockRegex::steal_character_block()
{
    return move(m_character_block);
}

// querying

bool
CharacterBlockRegex::do_can_be_concatenated() const
{
    return true;
}

bool
CharacterBlockRegex::do_can_be_unified() const
{
    return true;
}

bool
CharacterBlockRegex::do_characters_were_constrained_by_backreference() const
{
    return m_characters_were_constrained_by_backreference;
}

bool
CharacterBlockRegex::do_is_character_block() const
{
    return true;
}

// converting

string
CharacterBlockRegex::do_to_string() const
{
    return m_character_block->to_string();
}

// modifying

void
CharacterBlockRegex::do_reset_after_constrain()
{
    m_constrained_characters = characters();
}

void
CharacterBlockRegex::do_reset_characters_were_constrained_by_backreference()
{
    m_characters_were_constrained_by_backreference = false;
}


// StringRegex
// -----------

// instance creation and deletion

StringRegex::StringRegex(vector<unique_ptr<CharacterBlock>>&&
                           character_blocks) :
  m_character_blocks(move(character_blocks)),
  m_characters_are_initialized(false),
  m_constrained_characters_are_initialized(false),
  m_characters_were_constrained_by_backreference(false)
{
}

// Precondition:
// * 'regex_1' and 'regex_2' are character block, string, or epsilon
//   regexes
StringRegex::StringRegex(unique_ptr<Regex> regex_1,
                         unique_ptr<Regex> regex_2) :
  m_characters_are_initialized(false),
  m_constrained_characters_are_initialized(false),
  m_characters_were_constrained_by_backreference(false)
{
    append_character_blocks_of_regex(move(regex_1));
    append_character_blocks_of_regex(move(regex_2));
}

// Precondition:
// * 'regex' is a character block, a string, or an epsilon regex
void
StringRegex::append_character_blocks_of_regex(unique_ptr<Regex> regex)
{
    if (is_character_block(*regex))
    {
        auto character_block = downcast<CharacterBlockRegex>(move(regex));
        m_character_blocks.push_back(character_block->steal_character_block());
        return;
    }

    if (is_string(*regex))
    {
        auto string_ = downcast<StringRegex>(move(regex));
        m_character_blocks.insert(
            end(m_character_blocks),
            make_move_iterator(string_->m_character_blocks.begin()),
            make_move_iterator(string_->m_character_blocks.end()));
        return;
    }

    // 'regex' is an EpsilonRegex, and an EpsilonRegex has no character
    // blocks, so nothing to do.
}

// copying

unique_ptr<Regex>
StringRegex::do_clone() const
{
    vector<unique_ptr<CharacterBlock>> clones;

    transform(m_character_blocks.cbegin(),
              m_character_blocks.cend(),
              back_inserter(clones),
              [](const unique_ptr<CharacterBlock>& character_block)
              {
                  return character_block->clone();
              });

    return Utils::make_unique<StringRegex>(move(clones));
}

// accessing

vector<SetOfCharacters>&
StringRegex::characters() const
{
    if (!m_characters_are_initialized)
    {
        initialize_characters();
        m_characters_are_initialized = true;
    }

    return m_characters;
}

// See Regex::constrain_once_with_current_value().
bool
StringRegex::do_constrain_once_with_current_value(Constraint& constraint,
                                                  size_t      offset)
{
    if (!m_constrained_characters_are_initialized)
    {
        initialize_constrained_characters();
        m_constrained_characters_are_initialized = true;
    }

    const auto begin_pos_ = begin_pos() + offset;

    const auto constrained_characters_before = m_constrained_characters;

    const auto length = length_of_current_value();
    for (size_t i = 0; i != length; ++i)
    {
        // To understand why 'm_constrained_characters' is updated,
        // see the implementation comment in
        // CharacterBlockRegex::do_constrain_once_with_current_value().
        m_constrained_characters[i] &= constraint[begin_pos_ + i];
        constraint[begin_pos_ + i] = m_constrained_characters[i];
    }

    const auto constraint_comes_from_backreference = (offset != 0);

    if (constraint_comes_from_backreference)
    {
        m_characters_were_constrained_by_backreference =
            (m_constrained_characters != constrained_characters_before);
    }

    return all_of(m_constrained_characters.cbegin(),
                  m_constrained_characters.cend(),
                  [](const SetOfCharacters& constrained_characters)
                  {
                      return constrained_characters.not_empty();
                  });
}

string
StringRegex::do_explicit_characters() const
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

size_t
StringRegex::do_length_of_current_value() const
{
    return m_character_blocks.size();
}

void
StringRegex::initialize_characters() const
{
    transform(m_character_blocks.cbegin(),
              m_character_blocks.cend(),
              back_inserter(m_characters),
              [](const unique_ptr<CharacterBlock>& character_block)
              {
                  return character_block->characters();
              });
}

void
StringRegex::initialize_constrained_characters()
{
    m_constrained_characters = characters();
}

// querying

bool
StringRegex::do_can_be_concatenated() const
{
    return true;
}

bool
StringRegex::do_characters_were_constrained_by_backreference() const
{
    return m_characters_were_constrained_by_backreference;
}

bool
StringRegex::do_is_string() const
{
    return true;
}

// converting

string
StringRegex::do_to_string() const
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
    return '"' + inside + '"';
}

// modifying

void
StringRegex::do_reset_after_constrain()
{
    initialize_constrained_characters();
}

void
StringRegex::do_reset_characters_were_constrained_by_backreference()
{
    m_characters_were_constrained_by_backreference = false;
}


// BackreferenceRegex
// ------------------

// instance creation and deletion

BackreferenceRegex::BackreferenceRegex(
                      const GroupNumber& referenced_group_number) :
  m_referenced_group_number(referenced_group_number)
{
    assert(class_invariant());
}

// copying

unique_ptr<Regex>
BackreferenceRegex::do_clone() const
{
    return Utils::make_unique<BackreferenceRegex>(m_referenced_group_number);
}

// accessing

vector<const BackreferenceRegex*>
BackreferenceRegex::backreferences_to(const GroupNumber& group_number) const
{
    if (group_number == m_referenced_group_number)
    {
        return { this };
    }
    else
    {
        return {};
    }
}

// See Regex::constrain_once_with_current_value().
bool
BackreferenceRegex::do_constrain_once_with_current_value(
                      Constraint& constraint, size_t offset)
{
    const auto referenced_group_ = referenced_group();
    assert(referenced_group_ != nullptr);

    const auto referenced_group_begin_pos = begin_pos(*referenced_group_);
    assert(referenced_group_begin_pos <= begin_pos());

    const auto offset_from_referenced_group =
        begin_pos() + offset - referenced_group_begin_pos;
    return constrain_once_with_current_value(*referenced_group_,
                                             constraint,
                                             offset_from_referenced_group);
}

void
BackreferenceRegex::do_get_used_backreference_numbers(
                      BackreferenceNumbers& used_backreference_numbers) const
{
    used_backreference_numbers.add(m_referenced_group_number);
}

size_t
BackreferenceRegex::do_length_of_current_value() const
{
    return length_of_current_value(*referenced_group());
}

GroupRegex*
BackreferenceRegex::referenced_group() const
{
    assert(parent() != nullptr);
    return rightmost_group(*parent(), m_referenced_group_number, this);
}

// querying

bool
BackreferenceRegex::do_has_a_value() const
{
    if (at_end())
    {
        return false;
    }

    const auto referenced_group_ = referenced_group();

    if (referenced_group_ == nullptr)
    {
        return false;
    }

    return has_a_value(*referenced_group_);
}

// converting

string
BackreferenceRegex::do_to_string() const
{
    return '\\' + m_referenced_group_number.to_string();
}

// error handling

void
BackreferenceRegex::do_check_no_self_references() const
{
    const GroupRegex* enclosing_group_ = enclosing_group();

    while (enclosing_group_ != nullptr)
    {
        if (enclosing_group_->has_number(m_referenced_group_number))
        {
            const string message = "in "                             +
                                   Utils::quoted(root().to_string()) +
                                   ", "                              +
                                   Utils::quoted(to_string())        +
                                   " refers to a group that encloses it";
            throw RegexStructureException(message);
        }
        else
        {
            enclosing_group_ = enclosing_group(*enclosing_group_);
        }
    }
}


// AbstractGroupRegex
// ------------------

// instance creation and deletion

AbstractGroupRegex::AbstractGroupRegex(unique_ptr<Regex> child) :
  m_child(move(child))
{
    set_parent(*m_child, this);
}

// accessing

vector<const BackreferenceRegex*>
AbstractGroupRegex::backreferences_to(const GroupNumber& group_number) const
{
    return Regex::backreferences_to(*m_child, group_number);
}

Regex&
AbstractGroupRegex::child() const
{
    return *m_child;
}

// See Regex::constrain_once_with_current_value().
bool
AbstractGroupRegex::do_constrain_once_with_current_value(Constraint& constraint,
                                                         size_t      offset)
{
    return constrain_once_with_current_value(*m_child, constraint, offset);
}

// See Regex::constrain_word_boundaries_with_current_value().
bool
AbstractGroupRegex::do_constrain_word_boundaries_with_current_value(
                      Constraint& constraint)
{
    return constrain_word_boundaries_with_current_value(*m_child, constraint);
}

string
AbstractGroupRegex::do_explicit_characters() const
{
    return m_child->explicit_characters();
}

void
AbstractGroupRegex::do_get_used_backreference_numbers(
                      BackreferenceNumbers& used_backreference_numbers) const
{
    get_used_backreference_numbers(*m_child, used_backreference_numbers);
}

size_t
AbstractGroupRegex::do_length_of_current_value() const
{
    return length_of_current_value(*m_child);
}

// See Regex::rightmost_group(Regex&, const GroupNumber&, const Regex*).
GroupRegex*
AbstractGroupRegex::do_rightmost_group(const GroupNumber& group_number,
                                       const Regex*       from_child)
{
    assert(from_child == m_child.get());

    static_cast<void>(from_child);
    return rightmost_group_from_parent(group_number);
}

// querying

bool
AbstractGroupRegex::do_at_end() const
{
    return at_end(*m_child);
}

bool
AbstractGroupRegex::do_characters_were_constrained_by_backreference() const
{
    return characters_were_constrained_by_backreference(*m_child);
}

bool
AbstractGroupRegex::do_parents_are_correctly_setup() const
{
    if (parent(*m_child) != this)
    {
        return false;
    }

    return parents_are_correctly_setup(*m_child);
}

// modifying

void
AbstractGroupRegex::do_increment()
{
    increment(*m_child);
}

Regex*
AbstractGroupRegex::do_optimize_concatenations()
{
    m_child = optimize_concatenations(move(m_child));
    return this;
}

Regex*
AbstractGroupRegex::do_optimize_groups(
                      const BackreferenceNumbers& used_backreference_numbers)
{
    m_child = optimize_groups(move(m_child), used_backreference_numbers);

    if (number_belongs_to(used_backreference_numbers))
    {
        // This group cannot be optimized away, because it is
        // backreferenced.
        return this;
    }
    else
    {
        return m_child.release();
    }
}

Regex*
AbstractGroupRegex::do_optimize_unions()
{
    m_child = optimize_unions(move(m_child));
    return this;
}

void
AbstractGroupRegex::do_reset_after_constrain()
{
    reset_after_constrain(*m_child);
}

void
AbstractGroupRegex::do_reset_characters_were_constrained_by_backreference()
{
    reset_characters_were_constrained_by_backreference(*m_child);
}

void
AbstractGroupRegex::do_rewind()
{
    rewind(*m_child, begin_pos());
}

void
AbstractGroupRegex::set_constraint_size_of_children(size_t constraint_size)
{
    set_constraint_size(*m_child, constraint_size);
}

// error handling

void
AbstractGroupRegex::do_check_no_self_references() const
{
    check_no_self_references(*m_child);
}


// GroupRegex
// ----------

// instance creation and deletion

GroupRegex::GroupRegex(unique_ptr<Regex>  child,
                       const GroupNumber& group_number) :
  AbstractGroupRegex(move(child)),
  m_group_number(group_number)
{
    assert(class_invariant());
}

// copying

unique_ptr<Regex>
GroupRegex::do_clone() const
{
    return Utils::make_unique<GroupRegex>(child().clone(), m_group_number);
}

// accessing

// See Regex::rightmost_group(Regex&, const GroupNumber&).
GroupRegex*
GroupRegex::do_rightmost_group(const GroupNumber& group_number)
{
    if (group_number == m_group_number)
    {
        return this;
    }

    return rightmost_group(child(), group_number);
}

vector<const GroupRegex*>
GroupRegex::groups() const
{
    auto result = Regex::groups(child());
    result.push_back(this);
    return result;
}

GroupNumber
GroupRegex::number() const
{
    return m_group_number;
}

const GroupRegex*
GroupRegex::yourself_or_enclosing_group() const
{
    return this;
}

// querying

bool
GroupRegex::has_number(const GroupNumber& group_number) const
{
    return m_group_number == group_number;
}

bool
GroupRegex::number_belongs_to(
              const BackreferenceNumbers& backreference_numbers) const
{
    return backreference_numbers.contains(m_group_number);
}

// converting

string
GroupRegex::do_to_string() const
{
    return '(' + child().to_string() + ')';
}


// NonCapturingGroupRegex
// ----------------------

// instance creation and deletion

NonCapturingGroupRegex::NonCapturingGroupRegex(unique_ptr<Regex> child) :
  AbstractGroupRegex(move(child))
{
    assert(class_invariant());
}

// copying

unique_ptr<Regex>
NonCapturingGroupRegex::do_clone() const
{
    return Utils::make_unique<NonCapturingGroupRegex>(child().clone());
}

// accessing

// See Regex::rightmost_group(Regex&, const GroupNumber&).
GroupRegex*
NonCapturingGroupRegex::do_rightmost_group(const GroupNumber& group_number)
{
    return rightmost_group(child(), group_number);
}

vector<const GroupRegex*>
NonCapturingGroupRegex::groups() const
{
    return Regex::groups(child());
}

// querying

bool
NonCapturingGroupRegex::number_belongs_to(
    const BackreferenceNumbers& /*backreference_numbers*/) const
{
    // A NonCapturingGroupRegex has no number, so we unconditionally
    // return false;
    return false;
}

// converting

string
NonCapturingGroupRegex::do_to_string() const
{
    return "(?:" + child().to_string() + ')';
}


// BinaryRegex
// -----------

// instance creation and deletion

BinaryRegex::BinaryRegex(unique_ptr<Regex> left_child,
                         unique_ptr<Regex> right_child) :
  m_left_child(move(left_child)),
  m_right_child(move(right_child))
{
    set_parent(*m_left_child, this);
    set_parent(*m_right_child, this);
}

// accessing

vector<const BackreferenceRegex*>
BinaryRegex::backreferences_to(const GroupNumber& group_number) const
{
    auto result = Regex::backreferences_to(*m_left_child, group_number);
    auto right_backreferences =
             Regex::backreferences_to(*m_right_child, group_number);
    result.insert(end(result),
                  begin(right_backreferences),
                  end(right_backreferences));
    return result;
}

string
BinaryRegex::do_explicit_characters() const
{
    return m_left_child->explicit_characters() +
           m_right_child->explicit_characters();
}

void
BinaryRegex::do_get_used_backreference_numbers(
               BackreferenceNumbers& used_backreference_numbers) const
{
    get_used_backreference_numbers(*m_left_child, used_backreference_numbers);
    get_used_backreference_numbers(*m_right_child, used_backreference_numbers);
}

vector<const GroupRegex*>
BinaryRegex::groups() const
{
    auto result = Regex::groups(*m_left_child);
    auto right_groups = Regex::groups(*m_right_child);
    result.insert(end(result), begin(right_groups), end(right_groups));
    return result;
}

Regex&
BinaryRegex::left_child() const
{
    return *m_left_child;
}

Regex&
BinaryRegex::right_child() const
{
    return *m_right_child;
}

unique_ptr<Regex>
BinaryRegex::steal_left_child()
{
    return move(m_left_child);
}

unique_ptr<Regex>
BinaryRegex::steal_right_child()
{
    return move(m_right_child);
}

// querying

bool
BinaryRegex::do_parents_are_correctly_setup() const
{
    if (parent(*m_left_child) != this)
    {
        return false;
    }

    if (parent(*m_right_child) != this)
    {
        return false;
    }

    return parents_are_correctly_setup(*m_left_child) &&
           parents_are_correctly_setup(*m_right_child);
}

// converting

string
BinaryRegex::do_to_string() const
{
    return m_left_child->to_string() + operator_string() +
           m_right_child->to_string();
}

// modifying

Regex*
BinaryRegex::do_optimize_concatenations()
{
    m_left_child = optimize_concatenations(move(m_left_child));
    m_right_child = optimize_concatenations(move(m_right_child));
    return this;
}

Regex*
BinaryRegex::do_optimize_groups(
               const BackreferenceNumbers& used_backreference_numbers)
{
    m_left_child = optimize_groups(move(m_left_child),
                                   used_backreference_numbers);
    m_right_child = optimize_groups(move(m_right_child),
                                    used_backreference_numbers);
    return this;
}

Regex*
BinaryRegex::do_optimize_unions()
{
    m_left_child = optimize_unions(move(m_left_child));
    m_right_child = optimize_unions(move(m_right_child));
    return this;
}

void
BinaryRegex::do_reset_after_constrain()
{
    reset_after_constrain(*m_left_child);
    reset_after_constrain(*m_right_child);
}

void
BinaryRegex::do_reset_characters_were_constrained_by_backreference()
{
    reset_characters_were_constrained_by_backreference(*m_left_child);
    reset_characters_were_constrained_by_backreference(*m_right_child);
}

// Transform this BinaryRegex as follows:
/*
 *    - : some BinaryRegex (i.e., ConcatenationRegex or UnionRegex)
 *
 *        -                   -
 *       / \                 / \
 *      /   \               /   \
 *     A     -     ==>     -     C
 *          / \           / \
 *         /   \         /   \
 *        B     C       A     B
 */
// Precondition:
// * the right child of this BinaryRegex is a BinaryRegex of the same
//   kind as this BinaryRegex - that is, if this BinaryRegex is a
//   ConcatenationRegex, its right child is also a ConcatenationRegex,
//   and if this BinaryRegex is a UnionRegex, its right child is also a
//   UnionRegex
void
BinaryRegex::rotate_left()
{
    assert((is_concatenation(*this) && is_concatenation(right_child())) ||
           (is_union(*this)         && is_union(right_child())));

    auto right_binary = downcast<BinaryRegex>(move(m_right_child));
    set_children(create(move(m_left_child),
                        move(right_binary->m_left_child)),
                 move(right_binary->m_right_child));
}

// Transform this BinaryRegex as follows:
/*
 *    - : some BinaryRegex (i.e., ConcatenationRegex or UnionRegex)
 *
 *           -             -
 *          / \           / \
 *         /   \         /   \
 *        -     C  ==>  A     -
 *       / \                 / \
 *      /   \               /   \
 *     A     B             B     C
 */
// Precondition:
// * the left child of this BinaryRegex is a BinaryRegex of the same
//   kind as this BinaryRegex - that is, if this BinaryRegex is a
//   ConcatenationRegex, its left child is also a ConcatenationRegex,
//   and if this BinaryRegex is a UnionRegex, its left child is also a
//   UnionRegex
void
BinaryRegex::rotate_right()
{
    assert((is_concatenation(*this) && is_concatenation(left_child())) ||
           (is_union(*this)         && is_union(left_child())));

    auto left_binary = downcast<BinaryRegex>(move(m_left_child));
    set_children(move(left_binary->m_left_child),
                 create(move(left_binary->m_right_child),
                        move(m_right_child)));
}

void
BinaryRegex::set_children(unique_ptr<Regex> left_child,
                          unique_ptr<Regex> right_child)
{
    set_left_child(move(left_child));
    set_right_child(move(right_child));
}

void
BinaryRegex::set_constraint_size_of_children(size_t constraint_size)
{
    set_constraint_size(*m_left_child, constraint_size);
    set_constraint_size(*m_right_child, constraint_size);
}

void
BinaryRegex::set_left_child(unique_ptr<Regex> left_child)
{
    set_left_child(left_child.release());
}

void
BinaryRegex::set_left_child(Regex* left_child)
{
    m_left_child.reset(left_child);
    set_parent(*m_left_child, this);
}

void
BinaryRegex::set_right_child(unique_ptr<Regex> right_child)
{
    set_right_child(right_child.release());
}

void
BinaryRegex::set_right_child(Regex* right_child)
{
    m_right_child.reset(right_child);
    set_parent(*m_right_child, this);
}

void
BinaryRegex::swap_children()
{
    swap(m_left_child, m_right_child);
}

// error handling

void
BinaryRegex::do_check_no_self_references() const
{
    check_no_self_references(*m_left_child);
    check_no_self_references(*m_right_child);
}


// ConcatenationRegex
// ------------------

// instance creation and deletion

ConcatenationRegex::ConcatenationRegex(unique_ptr<Regex> left_child,
                                       unique_ptr<Regex> right_child) :
  BinaryRegex(move(left_child), move(right_child))
{
    assert(class_invariant());
}

unique_ptr<Regex>
ConcatenationRegex::create(unique_ptr<Regex> left_child,
                           unique_ptr<Regex> right_child) const
{
    return Utils::make_unique<ConcatenationRegex>(move(left_child),
                                                  move(right_child));
}

// copying

unique_ptr<Regex>
ConcatenationRegex::do_clone() const
{
    return Utils::make_unique<ConcatenationRegex>(left_child().clone(),
                                                  right_child().clone());
}

// accessing

// See Regex::constrain_once_with_current_value().
bool
ConcatenationRegex::do_constrain_once_with_current_value(
                      Constraint& constraint, size_t offset)
{
    if (!constrain_once_with_current_value(left_child(), constraint, offset))
    {
        // Since 'constraint' is impossible, there is no point
        // constraining it any further - it will not be used anyway.
        return false;
    }

    return constrain_once_with_current_value(right_child(), constraint, offset);
}

// See Regex::constrain_word_boundaries_with_current_value().
bool
ConcatenationRegex::do_constrain_word_boundaries_with_current_value(
                      Constraint& constraint)
{
    if (!constrain_word_boundaries_with_current_value(left_child(), constraint))
    {
        // Since 'constraint' is impossible, there is no point
        // constraining it any further - it will not be used anyway.
        return false;
    }

    return constrain_word_boundaries_with_current_value(
             right_child(), constraint);
}

size_t
ConcatenationRegex::do_length_of_current_value() const
{
    return length_of_current_value(left_child()) +
           length_of_current_value(right_child());
}

// See Regex::rightmost_group(Regex&, const GroupNumber&, const Regex*).
GroupRegex*
ConcatenationRegex::do_rightmost_group(const GroupNumber& group_number,
                                       const Regex*       from_child)
{
    assert(from_child == &left_child() || from_child == &right_child());

    if (from_child == &right_child())
    {
        const auto group = rightmost_group(left_child(), group_number);
        if (group != nullptr)
        {
            return group;
        }
    }

    return rightmost_group_from_parent(group_number);
}

// See Regex::rightmost_group(Regex&, const GroupNumber&).
GroupRegex*
ConcatenationRegex::do_rightmost_group(const GroupNumber& group_number)
{
    auto rightmost_group_ = rightmost_group(right_child(), group_number);
    if (rightmost_group_ != nullptr)
    {
        return rightmost_group_;
    }

    rightmost_group_ = rightmost_group(left_child(), group_number);
    if (rightmost_group_ != nullptr)
    {
        return rightmost_group_;
    }

    return nullptr;
}

string
ConcatenationRegex::operator_string() const
{
    // concatenation is implicit
    return "";
}

// querying

bool
ConcatenationRegex::do_at_end() const
{
    return at_end(left_child()) ||
           at_end(right_child());
}

bool
ConcatenationRegex::do_characters_were_constrained_by_backreference() const
{
    return characters_were_constrained_by_backreference(left_child()) ||
           characters_were_constrained_by_backreference(right_child());
}

bool
ConcatenationRegex::do_is_concatenation() const
{
    return true;
}

// modifying

// Precondition:
// * both children can be concatenated
StringRegex*
ConcatenationRegex::concatenate()
{
    assert(can_be_concatenated(left_child()));
    assert(can_be_concatenated(right_child()));

    return new StringRegex(steal_left_child(), steal_right_child());
}

void
ConcatenationRegex::do_increment()
{
    increment(right_child());
    while_right_at_end_increment();
}

// Postconditions:
// * the returned regex is either a StringRegex or this
//   ConcatenationRegex
// * no further concatenation optimizations are possible on this regex
Regex*
ConcatenationRegex::do_optimize_concatenations()
{
    set_left_child(optimize_concatenations_on_left(steal_left_child()));
    set_right_child(optimize_concatenations_on_right(steal_right_child()));

    auto optimized_regex = optimize_concatenations_left_and_right();
    if (optimized_regex != nullptr)
    {
        return optimized_regex;
    }

    optimized_regex = optimize_concatenations_left_and_right_left();
    if (optimized_regex != nullptr)
    {
        return optimized_regex;
    }

    optimized_regex = optimize_concatenations_left_right_and_right();
    if (optimized_regex != nullptr)
    {
        return optimized_regex;
    }

    optimized_regex = optimize_concatenations_left_right_and_right_left();
    if (optimized_regex != nullptr)
    {
        return optimized_regex;
    }

    return this;
}

// Postcondition:
// * if the rightmost child regex which was reachable from this regex
//   only through concatenations when this method was called can be
//   concatenated, it is now the immediate right child of this
//   concatenation regex
Regex*
ConcatenationRegex::do_optimize_concatenations_on_left()
{
    auto optimized_regex = do_optimize_concatenations();

    if (!is_concatenation(*optimized_regex))
    {
        return optimized_regex;
    }

    assert(optimized_regex == this);

    if (!is_concatenation(right_child()))
    {
        return this;
    }

    auto& right_concatenation = downcast<ConcatenationRegex>(right_child());

    if (can_be_concatenated(right_concatenation.right_child()))
    {
        rotate_left();
    }

    return this;
}

// Postcondition:
// * if the leftmost child regex which was reachable from this regex
//   only through concatenations when this method was called can be
//   concatenated, it is now the immediate left child of this
//   concatenation regex
Regex*
ConcatenationRegex::do_optimize_concatenations_on_right()
{
    auto optimized_regex = do_optimize_concatenations();

    if (!is_concatenation(*optimized_regex))
    {
        return optimized_regex;
    }

    assert(optimized_regex == this);

    if (!is_concatenation(left_child()))
    {
        return this;
    }

    auto& left_concatenation = downcast<ConcatenationRegex>(left_child());

    if (can_be_concatenated(left_concatenation.left_child()))
    {
        rotate_right();
    }

    return this;
}

void
ConcatenationRegex::do_rewind()
{
    rewind(left_child(), begin_pos());

    if (at_end(left_child()))
    {
        return;
    }

    rewind(right_child(), end_pos(left_child()));

    while_right_at_end_increment();
}

// Attempt to concatenate:
// * the left child of this regex, with:
// * the right child of this regex
//
// If this optimization is possible, return its result. Return 'nullptr'
// otherwise.
//
// Example of a successful such optimization:
/*
 *    .     : a ConcatenationRegex
 *    "..." : a StringRegex
 *
 *        .
 *       / \    ==> "AB"
 *      /   \
 *     A     B
 */
Regex*
ConcatenationRegex::optimize_concatenations_left_and_right()
{
    if (!can_be_concatenated(left_child()))
    {
        return nullptr;
    }

    if (!can_be_concatenated(right_child()))
    {
        return nullptr;
    }

    return concatenate();
}

// Attempt to concatenate:
// * the left child this regex, with:
// * the left child of the right concatenation child of this regex
//
// If this optimization is possible, return its result. Return 'nullptr'
// otherwise.
//
// Example of a successful such optimization:
/*
 *    .     : a ConcatenationRegex
 *    "..." : a StringRegex
 *
 *        .                 .
 *       / \               / \
 *      /   \             /   \
 *     A     .     ==>  "AB"   C*
 *          / \
 *         /   \
 *        B    C*
 */
Regex*
ConcatenationRegex::optimize_concatenations_left_and_right_left()
{
    if (!can_be_concatenated(left_child()))
    {
        return nullptr;
    }

    if (!is_concatenation(right_child()))
    {
        return nullptr;
    }

    auto& right_concatenation = downcast<ConcatenationRegex>(right_child());

    if (!can_be_concatenated(right_concatenation.left_child()))
    {
        return nullptr;
    }

    rotate_left();

    assert(is_concatenation(left_child()));
    auto& left_concatenation = downcast<ConcatenationRegex>(left_child());

    set_left_child(left_concatenation.concatenate());
    set_parent(left_child(), this);

    return this;
}

// Attempt to concatenate:
// * the right child of the left concatenation child of this regex, with:
// * the right child this regex
//
// If this optimization is possible, return its result. Return 'nullptr'
// otherwise.
//
// Example of a successful such optimization:
/*
 *    .     : a ConcatenationRegex
 *    "..." : a StringRegex
 *
 *           .             .
 *          / \           / \
 *         /   \         /   \
 *        .     C  ==>  A*  "BC"
 *       / \
 *      /   \
 *     A*    B
 */
Regex*
ConcatenationRegex::optimize_concatenations_left_right_and_right()
{
    if (!can_be_concatenated(right_child()))
    {
        return nullptr;
    }

    if (!is_concatenation(left_child()))
    {
        return nullptr;
    }

    auto& left_concatenation = downcast<ConcatenationRegex>(left_child());

    if (!can_be_concatenated(left_concatenation.right_child()))
    {
        return nullptr;
    }

    rotate_right();

    assert(is_concatenation(right_child()));
    auto& right_concatenation = downcast<ConcatenationRegex>(right_child());

    set_right_child(right_concatenation.concatenate());
    set_parent(right_child(), this);

    return this;
}

// Attempt to concatenate:
// * the right child of the left concatenation child of this regex, with:
// * the left child of the right concatenation child of this regex
//
// If this optimization is possible, return its result. Return 'nullptr'
// otherwise.
//
// Example of a successful such optimization:
/*
 *    .     : a ConcatenationRegex
 *    "..." : a StringRegex
 *
 *            .                  .
 *           / \                / \
 *          /   \              /   \
 *         /     \            /     \
 *        .       .  ==>     .      D*
 *       / \     / \        / \
 *      /   \   /   \      /   \
 *     A*    B C    D*    A*  "BC"
 */
Regex*
ConcatenationRegex::optimize_concatenations_left_right_and_right_left()
{
    {
        if (!is_concatenation(left_child()))
        {
            return nullptr;
        }

        auto& left_concatenation = downcast<ConcatenationRegex>(left_child());

        if (!can_be_concatenated(left_concatenation.right_child()))
        {
            return nullptr;
        }

        if (!is_concatenation(right_child()))
        {
            return nullptr;
        }

        auto& right_concatenation = downcast<ConcatenationRegex>(right_child());

        if (!can_be_concatenated(right_concatenation.left_child()))
        {
            return nullptr;
        }
    }

    {
        // We rotate this concatenation to the left, then rotate the
        // resulting left regex (which is a concatenation) to the right,
        // and last we concatenate the right part of the resulting right
        // regex. This choice is arbitrary - we could do it the other
        // way around instead.

        rotate_left();

        assert(is_concatenation(left_child()));
        auto& left_concatenation = downcast<ConcatenationRegex>(left_child());

        left_concatenation.rotate_right();

        assert(is_concatenation(left_concatenation.right_child()));
        auto& left_right_concatenation =
            downcast<ConcatenationRegex>(left_concatenation.right_child());

        left_concatenation.set_right_child(
                             left_right_concatenation.concatenate());

        return this;
    }
}

void
ConcatenationRegex::while_right_at_end_increment()
{
    while (at_end(right_child()))
    {
        increment(left_child());

        if (at_end(left_child()))
        {
            break;
        }

        rewind(right_child(), end_pos(left_child()));
    }
}


// UnionRegex
// ----------

// instance creation and deletion

UnionRegex::UnionRegex(unique_ptr<Regex> left_child,
                       unique_ptr<Regex> right_child) :
  BinaryRegex(move(left_child), move(right_child))
{
    assert(class_invariant());
}

unique_ptr<Regex>
UnionRegex::create(unique_ptr<Regex> left_child,
                   unique_ptr<Regex> right_child) const
{
    return Utils::make_unique<UnionRegex>(move(left_child),
                                          move(right_child));
}

// copying

unique_ptr<Regex>
UnionRegex::do_clone() const
{
    return Utils::make_unique<UnionRegex>(left_child().clone(),
                                          right_child().clone());
}

// accessing

Regex&
UnionRegex::active_child() const
{
    if (not_at_end(left_child()))
    {
        return left_child();
    }
    else
    {
        assert(not_at_end(right_child()));
        return right_child();
    }
}

// See Regex::constrain_once_with_current_value().
bool
UnionRegex::do_constrain_once_with_current_value(Constraint& constraint,
                                                 size_t      offset)
{
    return constrain_once_with_current_value(
             active_child(), constraint, offset);
}

// See Regex::constrain_word_boundaries_with_current_value().
bool
UnionRegex::do_constrain_word_boundaries_with_current_value(
              Constraint& constraint)
{
    return constrain_word_boundaries_with_current_value(
             active_child(), constraint);
}

size_t
UnionRegex::do_length_of_current_value() const
{
    return length_of_current_value(active_child());
}

// See Regex::rightmost_group(Regex&, const GroupNumber&, const Regex*).
GroupRegex*
UnionRegex::do_rightmost_group(const GroupNumber& group_number,
                               const Regex*       /*from_child*/)
{
    return rightmost_group_from_parent(group_number);
}

// See Regex::rightmost_group(Regex&, const GroupNumber&).
GroupRegex*
UnionRegex::do_rightmost_group(const GroupNumber& group_number)
{
    return rightmost_group(active_child(), group_number);
}

Regex&
UnionRegex::first_child_with_a_value() const
{
    assert(has_a_value());

    if (has_a_value(left_child()))
    {
        return left_child();
    }
    else
    {
        assert(has_a_value(right_child()));
        return right_child();
    }
}

string
UnionRegex::operator_string() const
{
    return Utils::char_to_string('|');
}

// querying

bool
UnionRegex::do_at_end() const
{
    return at_end(left_child()) &&
           at_end(right_child());
}

bool
UnionRegex::do_characters_were_constrained_by_backreference() const
{
    return characters_were_constrained_by_backreference(active_child());
}

bool
UnionRegex::do_is_union() const
{
    return true;
}

void
UnionRegex::do_rewind()
{
    rewind(left_child(), begin_pos());

    if (at_end(left_child()))
    {
        rewind(right_child(), begin_pos());
    }
}

bool
UnionRegex::either_child_can_be_unified() const
{
    return can_be_unified(left_child()) ||
           can_be_unified(right_child());
}

// modifying

void
UnionRegex::do_increment()
{
    auto& first_child_with_a_value_before_increment =
        first_child_with_a_value();

    increment(first_child_with_a_value_before_increment);

    if (&first_child_with_a_value_before_increment == &left_child() &&
        at_end(left_child()))
    {
        rewind(right_child(), begin_pos());
    }
}

// Postconditions:
// * the returned regex is either a CharacterBlockRegex or this
//   UnionRegex
// * no further union optimizations are possible on this regex
Regex*
UnionRegex::do_optimize_unions()
{
    set_left_child(optimize_unions(steal_left_child()));
    set_right_child(optimize_unions(steal_right_child()));

    auto optimized_regex = optimize_unions_left_and_right();
    if (optimized_regex != nullptr)
    {
        return optimized_regex;
    }

    optimized_regex = optimize_unions_left_and_right_either();
    if (optimized_regex != nullptr)
    {
        return optimized_regex;
    }

    optimized_regex = optimize_unions_left_either_and_right();
    if (optimized_regex != nullptr)
    {
        return optimized_regex;
    }

    optimized_regex = optimize_unions_left_either_and_right_either();
    if (optimized_regex != nullptr)
    {
        return optimized_regex;
    }

    return this;
}

// If this optimization is possible, return its result. Return 'nullptr'
// otherwise.
//
// Example of a successful such optimization:
/*
 *    |     : a UnionRegex
 *    {...} : a CharacterBlockRegex with a CompositeCharacterBlock
 *
 *        |
 *       / \    ==> {AB}
 *      /   \
 *     A     B
 */
Regex*
UnionRegex::optimize_unions_left_and_right()
{
    if (!can_be_unified(left_child()))
    {
        return nullptr;
    }

    if (!can_be_unified(right_child()))
    {
        return nullptr;
    }

    return unify();
}

// If this optimization is possible, return its result. Return 'nullptr'
// otherwise.
//
// Example of a successful such optimization:
/*
 *    |     : a UnionRegex
 *    {...} : a CharacterBlockRegex with a CompositeCharacterBlock
 *
 *        |                 |
 *       / \               / \
 *      /   \             /   \
 *     A     |     ==>  {AB}   C*
 *          / \
 *         /   \
 *        C*    B
 */
Regex*
UnionRegex::optimize_unions_left_and_right_either()
{
    if (!can_be_unified(left_child()))
    {
        return nullptr;
    }

    if (!is_union(right_child()))
    {
        return nullptr;
    }

    auto& right_union = downcast<UnionRegex>(right_child());

    if (!right_union.either_child_can_be_unified())
    {
        return nullptr;
    }

    right_union.place_child_which_can_be_unified_to_left();

    rotate_left();

    assert(is_union(left_child()));
    auto& left_union = downcast<UnionRegex>(left_child());

    set_left_child(left_union.unify());
    set_parent(left_child(), this);

    return this;
}

// If this optimization is possible, return its result. Return 'nullptr'
// otherwise.
//
// Example of a successful such optimization:
/*
 *    |     : a UnionRegex
 *    {...} : a CharacterBlockRegex with a CompositeCharacterBlock
 *
 *           |             |
 *          / \           / \
 *         /   \         /   \
 *        |     C  ==>  A*  {BC}
 *       / \
 *      /   \
 *     B    A*
 */
Regex*
UnionRegex::optimize_unions_left_either_and_right()
{
    if (!can_be_unified(right_child()))
    {
        return nullptr;
    }

    if (!is_union(left_child()))
    {
        return nullptr;
    }

    auto& left_union = downcast<UnionRegex>(left_child());

    if (!left_union.either_child_can_be_unified())
    {
        return nullptr;
    }

    left_union.place_child_which_can_be_unified_to_right();

    rotate_right();

    assert(is_union(right_child()));
    auto& right_union = downcast<UnionRegex>(right_child());

    set_right_child(right_union.unify());
    set_parent(right_child(), this);

    return this;
}

// If this optimization is possible, return its result. Return 'nullptr'
// otherwise.
//
// Example of a successful such optimization:
/*
 *    |     : a UnionRegex
 *    {...} : a CharacterBlockRegex with a CompositeCharacterBlock
 *
 *            |                  |
 *           / \                / \
 *          /   \              /   \
 *         /     \            /     \
 *        |       |  ==>     |      D*
 *       / \     / \        / \
 *      /   \   /   \      /   \
 *     B    A* D*    C    A*  {BC}
 */
Regex*
UnionRegex::optimize_unions_left_either_and_right_either()
{
    {
        if (!is_union(left_child()))
        {
            return nullptr;
        }

        if (!is_union(right_child()))
        {
            return nullptr;
        }

        auto& left_union = downcast<UnionRegex>(left_child());
        if (!left_union.either_child_can_be_unified())
        {
            return nullptr;
        }

        auto& right_union = downcast<UnionRegex>(right_child());
        if (!right_union.either_child_can_be_unified())
        {
            return nullptr;
        }

        left_union.place_child_which_can_be_unified_to_right();
        right_union.place_child_which_can_be_unified_to_left();
    }

    {
        // We rotate this union to the left, then rotate the resulting
        // left regex (which is a union) to the right, and last we unify
        // the right part of the resulting right regex. This choice is
        // arbitrary - we could do it the other way around instead.

        rotate_left();

        assert(is_union(left_child()));
        auto& left_union = downcast<UnionRegex>(left_child());

        left_union.rotate_right();

        assert(is_union(left_union.right_child()));
        auto& left_right_union = downcast<UnionRegex>(left_union.right_child());

        left_union.set_right_child(left_right_union.unify());

        return this;
    }
}

// Precondition:
// * either child can be unified
void
UnionRegex::place_child_which_can_be_unified_to_left()
{
    if (can_be_unified(left_child()))
    {
        return;
    }

    assert(can_be_unified(right_child()));
    swap_children();
}

// Precondition:
// * either child can be unified
void
UnionRegex::place_child_which_can_be_unified_to_right()
{
    if (can_be_unified(right_child()))
    {
        return;
    }

    assert(can_be_unified(left_child()));
    swap_children();
}

// Precondition:
// * both children can be unified
Regex*
UnionRegex::unify()
{
    assert(can_be_unified(left_child()));
    assert(can_be_unified(right_child()));

    return new CharacterBlockRegex(steal_left_child(), steal_right_child());
}


// RepetitionRegex
// ---------------

// instance creation and deletion

RepetitionRegex::RepetitionRegex(unique_ptr<Regex>      child,
                                 const RepetitionCount& min_count,
                                 const RepetitionCount& max_count) :
  m_child_to_repeat(move(child)),
  m_min_count(min_count),
  m_max_count(max_count),
  m_variable_children_max_count(max_count - min_count),
  m_fixed_children_at_beginning(true),
  m_variable_children_at_beginning(true),
  m_num_used_variable_children(0)
{
    assert(min_count.is_not_infinite());
    assert(min_count <= max_count);

    set_parent(*m_child_to_repeat, this);
    build_fixed_children();
}

void
RepetitionRegex::build_fixed_children()
{
    for (size_t i = 0; i != m_min_count; ++i)
    {
        append_fixed_child();
    }
}

void
RepetitionRegex::rebuild_after_optimization()
{
    m_fixed_children.clear();
    m_all_children.clear();

    build_fixed_children();

    assert(m_num_used_variable_children == 0);
}

// accessing

vector<const BackreferenceRegex*>
RepetitionRegex::backreferences_to(const GroupNumber& group_number) const
{
    return Regex::backreferences_to(*m_child_to_repeat, group_number);
}

Regex&
RepetitionRegex::child_to_repeat() const
{
    return *m_child_to_repeat;
}

// See Regex::constrain_once_with_current_value().
bool
RepetitionRegex::do_constrain_once_with_current_value(
                   Constraint& constraint, size_t offset)
{
    // The lambda function used in all_of() needs to capture
    // 'constraint'. If 'constraint' were captured by value,
    // 'constraint' would be copied, which would be inefficient. And I
    // am not entirely sure that capturing a reference by reference is
    // allowed. Therefore, we capture by value a pointer to
    // 'constraint'.
    const auto constraint_ptr = &constraint;
    return all_of(m_all_children.cbegin(),
                  m_all_children.cend(),
                  [this, constraint_ptr, offset](Regex* child)
                  {
                      return constrain_once_with_current_value(
                               *child, *constraint_ptr, offset);
                  });
}

// See Regex::constrain_word_boundaries_with_current_value().
bool
RepetitionRegex::do_constrain_word_boundaries_with_current_value(
                   Constraint& constraint)
{
    // The lambda function used in all_of() needs to capture
    // 'constraint'. If 'constraint' were captured by value,
    // 'constraint' would be copied, which would be inefficient. And I
    // am not entirely sure that capturing a reference by reference is
    // allowed. Therefore, we capture by value a pointer to
    // 'constraint'.
    const auto constraint_ptr = &constraint;
    return all_of(m_all_children.cbegin(),
                  m_all_children.cend(),
                  [this, constraint_ptr](Regex* child)
                  {
                      return constrain_word_boundaries_with_current_value(
                               *child, *constraint_ptr);
                  });
}

string
RepetitionRegex::do_explicit_characters() const
{
    return m_child_to_repeat->explicit_characters();
}

void
RepetitionRegex::do_get_used_backreference_numbers(
                   BackreferenceNumbers& used_backreference_numbers) const
{
    get_used_backreference_numbers(*m_child_to_repeat,
                                   used_backreference_numbers);
}

size_t
RepetitionRegex::do_length_of_current_value() const
{
    return accumulate(m_all_children.cbegin(),
                      m_all_children.cend(),
                      static_cast<decltype(do_length_of_current_value())>(0),
                      [this](size_t sum, const Regex* child)
                      {
                          return sum + length_of_current_value(*child);
                      });
}

// See Regex::rightmost_group(Regex&, const GroupNumber&, const Regex*).
GroupRegex*
RepetitionRegex::do_rightmost_group(const GroupNumber& group_number,
                                    const Regex*       from_child)
{
    const auto active_child = find_if(m_all_children.crbegin(),
                                      m_all_children.crend(),
                                      [from_child](const Regex* child)
                                      {
                                          return child == from_child;
                                      });
    assert(active_child != m_all_children.crend());

    for (auto it = next(active_child); it != m_all_children.crend(); ++it)
    {
        const auto child = *it;
        const auto group = rightmost_group(*child, group_number);

        if (group != nullptr)
        {
            return group;
        }
    }

    return rightmost_group_from_parent(group_number);
}

// See Regex::rightmost_group(Regex&, const GroupNumber&).
GroupRegex*
RepetitionRegex::do_rightmost_group(const GroupNumber& group_number)
{
    for (auto rit = m_all_children.crbegin();
         rit != m_all_children.crend();
         ++rit)
    {
        const auto child = *rit;
        const auto rightmost_group_ = rightmost_group(*child, group_number);

        if (rightmost_group_ != nullptr)
        {
            return rightmost_group_;
        }
    }

    return nullptr;
}

vector<const GroupRegex*>
RepetitionRegex::groups() const
{
    return Regex::groups(*m_child_to_repeat);
}

// Precondition:
// * there is at least one variable child
unique_ptr<Regex>&
RepetitionRegex::last_variable_child()
{
    assert(m_num_used_variable_children != 0);

    return m_variable_children[m_num_used_variable_children - 1];
}

RepetitionCount
RepetitionRegex::max_count() const
{
    return m_max_count;
}

RepetitionCount
RepetitionRegex::min_count() const
{
    return m_min_count;
}

// querying

bool
RepetitionRegex::do_at_end() const
{
    return fixed_children_at_end() || variable_children_at_end();
}

bool
RepetitionRegex::do_characters_were_constrained_by_backreference() const
{
    return any_of(m_all_children.cbegin(),
                  m_all_children.cend(),
                  [this](const Regex* child)
                  {
                      return characters_were_constrained_by_backreference(
                               *child);
                  });
}

bool
RepetitionRegex::do_parents_are_correctly_setup() const
{
    if (parent(*m_child_to_repeat) != this)
    {
        return false;
    }

    if (!parents_are_correctly_setup(*m_child_to_repeat))
    {
        return false;
    }

    return all_of(m_all_children.cbegin(),
                  m_all_children.cend(),
                  [this](const Regex* child)
                  {
                      return parent(*child) == this &&
                             parents_are_correctly_setup(*child);
                  });
}

bool
RepetitionRegex::fixed_children_at_end() const
{
    if (m_fixed_children.empty())
    {
        return !m_fixed_children_at_beginning;
    }
    else
    {
        return any_of(m_fixed_children.cbegin(),
                      m_fixed_children.cend(),
                      [this](const unique_ptr<Regex>& child)
                      {
                          return at_end(*child);
                      });
    }
}

bool
RepetitionRegex::variable_children_at_end() const
{
    return m_num_used_variable_children == 0 &&
           !m_variable_children_at_beginning;
}

// converting

string
RepetitionRegex::do_to_string() const
{
    return m_child_to_repeat->to_string() + repetition_suffix();
}

// modifying

void
RepetitionRegex::append_fixed_child()
{
    auto new_fixed_child(clone(*m_child_to_repeat, this));

    m_all_children.push_back(new_fixed_child.get());
    m_fixed_children.push_back(move(new_fixed_child));
}

void
RepetitionRegex::append_variable_child()
{
    if (m_num_used_variable_children == m_variable_children.size())
    {
        // All the elements of m_variable_children are used, so a new
        // one must be created.

        auto new_variable_child(clone(*m_child_to_repeat, this));
        m_all_children.push_back(new_variable_child.get());
        set_constraint_size(*new_variable_child, constraint_size());
        m_variable_children.push_back(move(new_variable_child));
    }
    else
    {
        // Not all the elements of m_variable_children are used, so we
        // reuse the first available one.

        assert(m_num_used_variable_children < m_variable_children.size());

        auto new_variable_child =
                 m_variable_children[m_num_used_variable_children].get();
        m_all_children.push_back(new_variable_child);
        set_constraint_size(*new_variable_child, constraint_size());
    }

    ++m_num_used_variable_children;
}

void
RepetitionRegex::do_increment()
{
    increment_variable_children();
    while_variable_children_at_end_increment();
}

Regex*
RepetitionRegex::do_optimize_concatenations()
{
    m_child_to_repeat = optimize_concatenations(move(m_child_to_repeat));
    rebuild_after_optimization();
    return this;
}

Regex*
RepetitionRegex::do_optimize_groups(
                   const BackreferenceNumbers& used_backreference_numbers)
{
    m_child_to_repeat = optimize_groups(move(m_child_to_repeat),
                                        used_backreference_numbers);
    rebuild_after_optimization();
    return this;
}

Regex*
RepetitionRegex::do_optimize_unions()
{
    m_child_to_repeat = optimize_unions(move(m_child_to_repeat));
    rebuild_after_optimization();
    return this;
}

void
RepetitionRegex::do_reset_after_constrain()
{
    for (auto child : m_all_children)
    {
        reset_after_constrain(*child);
    }
}

void
RepetitionRegex::do_reset_characters_were_constrained_by_backreference()
{
    for (auto child : m_all_children)
    {
        reset_characters_were_constrained_by_backreference(*child);
    }
}

void
RepetitionRegex::do_rewind()
{
    rewind_fixed_children();

    if (fixed_children_at_end())
    {
        return;
    }

    rewind_variable_children();

    while_variable_children_at_end_increment();
}

void
RepetitionRegex::do_set_constraint_size(size_t constraint_size)
{
    set_constraint_size(*m_child_to_repeat, constraint_size);
    Regex::do_set_constraint_size(constraint_size);
}

void
RepetitionRegex::increment_fixed_children()
{
    assert(!fixed_children_at_end());

    m_fixed_children_at_beginning = false;

    if (m_fixed_children.empty())
    {
        return;
    }

    increment(*m_fixed_children.back());

    if (not_at_end(*m_fixed_children.back()))
    {
        return;
    }

    const auto first_fixed_child_at_end = prev(m_fixed_children.cend());
    while_fixed_child_at_end_increment_fixed(first_fixed_child_at_end);
}

void
RepetitionRegex::increment_variable_children()
{
    m_variable_children_at_beginning = false;

    const auto must_append_variable_child =
        m_num_used_variable_children < m_variable_children_max_count &&
        end_pos() < constraint_size();

    if (must_append_variable_child)
    {
        const auto end_pos_ = end_pos();
        append_variable_child();
        rewind(*last_variable_child(), end_pos_);
    }

    const auto last_child_was_just_appended = must_append_variable_child;
    while_variable_children_increment_them(last_child_was_just_appended);

    for (size_t i = 0; i != m_num_used_variable_children; ++i)
    {
        const auto& variable_child = m_variable_children[i];
        assert(not_at_end(*variable_child));
        static_cast<void>(variable_child);
    }
}

void
RepetitionRegex::remove_last_variable_child()
{
    m_all_children.pop_back();
    --m_num_used_variable_children;
}

void
RepetitionRegex::remove_variable_children()
{
    const auto num_children_to_remove =
        static_cast<int>(m_num_used_variable_children);
    m_all_children.erase(end(m_all_children) - num_children_to_remove,
                         end(m_all_children));

    m_num_used_variable_children = 0;
}

void
RepetitionRegex::rewind_fixed_children()
{
    m_fixed_children_at_beginning = true;

    if (m_fixed_children.empty())
    {
        return;
    }

    const auto first_fixed_child_at_end =
        rewind_fixed_children(m_fixed_children.cbegin(), begin_pos());
    while_fixed_child_at_end_increment_fixed(first_fixed_child_at_end);
}

// Rewind the fixed children, starting with 'first', which is to be
// positioned at 'first_begin_pos'.
//
// Return the first fixed child which is at the end, or
// m_fixed_children.cend() if none of fixed children is at the end.
vector<unique_ptr<Regex>>::const_iterator
RepetitionRegex::rewind_fixed_children(
                   vector<unique_ptr<Regex>>::const_iterator first,
                   size_t                                    first_begin_pos)
{
    auto begin_pos_of_next_child = first_begin_pos;

    auto it = first;

    const auto end = m_fixed_children.cend();

    while (it != end)
    {
        const auto& fixed_child = *it;

        rewind(*fixed_child, begin_pos_of_next_child);

        if (at_end(*fixed_child))
        {
            break;
        }

        begin_pos_of_next_child = end_pos(*fixed_child);
        ++it;
    }

    return it;
}

void
RepetitionRegex::rewind_variable_children()
{
    remove_variable_children();
    m_variable_children_at_beginning = true;
}

void
RepetitionRegex::set_constraint_size_of_children(size_t constraint_size)
{
    for (auto child : m_all_children)
    {
        set_constraint_size(*child, constraint_size);
    }
}

void
RepetitionRegex::while_fixed_child_at_end_increment_fixed(
    vector<unique_ptr<Regex>>::const_iterator first_fixed_child_at_end)
{
    const auto begin = m_fixed_children.cbegin();
    const auto end   = m_fixed_children.cend();

    while (first_fixed_child_at_end != end)
    {
        if (first_fixed_child_at_end == begin)
        {
            break;
        }

        auto last_fixed_child_not_at_end = prev(first_fixed_child_at_end);

        increment(**last_fixed_child_not_at_end);

        if (at_end(**last_fixed_child_not_at_end))
        {
            first_fixed_child_at_end = last_fixed_child_not_at_end;
            continue;
        }

        first_fixed_child_at_end =
            rewind_fixed_children(first_fixed_child_at_end,
                                  end_pos(**last_fixed_child_not_at_end));
    }
}

void
RepetitionRegex::while_variable_children_at_end_increment()
{
    while (variable_children_at_end())
    {
        increment_fixed_children();

        if (fixed_children_at_end())
        {
            break;
        }

        rewind_variable_children();
    }
}

void
RepetitionRegex::while_variable_children_increment_them(
                   bool last_child_was_just_appended)
{
    while (m_num_used_variable_children != 0)
    {
        const auto& last_child = last_variable_child();

        if (!last_child_was_just_appended)
        {
            increment(*last_child);
        }

        // Not skipping the epsilon values of '*last_child' would lead
        // to infinite recursion.
        while (not_at_end(*last_child) && value_is_epsilon(*last_child))
        {
            increment(*last_child);
        }

        if (at_end(*last_child))
        {
            remove_last_variable_child();
            last_child_was_just_appended = false;
            continue;
        }

        break;
    }
}

// error handling

void
RepetitionRegex::do_check_no_self_references() const
{
    check_no_self_references(*m_child_to_repeat);
}


// KleeneStarRegex
// ---------------

// instance creation and deletion

KleeneStarRegex::KleeneStarRegex(unique_ptr<Regex> child) :
  RepetitionRegex(move(child), 0, RepetitionCount::infinite())
{
    assert(class_invariant());
}

// copying

unique_ptr<Regex>
KleeneStarRegex::do_clone() const
{
    return Utils::make_unique<KleeneStarRegex>(child_to_repeat().clone());
}

// accessing

string
KleeneStarRegex::repetition_suffix() const
{
    return Utils::char_to_string('*');
}


// PlusRegex
// ---------

// instance creation and deletion

PlusRegex::PlusRegex(unique_ptr<Regex> child) :
  RepetitionRegex(move(child), 1, RepetitionCount::infinite())
{
    assert(class_invariant());
}

// copying

unique_ptr<Regex>
PlusRegex::do_clone() const
{
    return Utils::make_unique<PlusRegex>(child_to_repeat().clone());
}

// accessing

string
PlusRegex::repetition_suffix() const
{
    return Utils::char_to_string('+');
}


// QuestionMarkRegex
// -----------------

// instance creation and deletion

QuestionMarkRegex::QuestionMarkRegex(unique_ptr<Regex> child) :
  RepetitionRegex(move(child), 0, 1)
{
    assert(class_invariant());
}

// copying

unique_ptr<Regex>
QuestionMarkRegex::do_clone() const
{
    return Utils::make_unique<QuestionMarkRegex>(child_to_repeat().clone());
}

// accessing

string
QuestionMarkRegex::repetition_suffix() const
{
    return Utils::char_to_string('?');
}


// CountedRepetitionRegex
// ----------------------

// instance creation and deletion

CountedRepetitionRegex::CountedRepetitionRegex(
                          unique_ptr<Regex>      child,
                          const RepetitionCount& min_count,
                          const RepetitionCount& max_count) :
  RepetitionRegex(move(child), min_count, max_count)
{
    assert(min_count.is_not_infinite());
}


// FixedRepetitionRegex
// --------------------

// instance creation and deletion

FixedRepetitionRegex::FixedRepetitionRegex(
                        unique_ptr<Regex>      child,
                        const RepetitionCount& fixed_count) :
  CountedRepetitionRegex(move(child), fixed_count, fixed_count)
{
    assert(fixed_count.is_not_infinite());
    assert(class_invariant());
}

// copying

unique_ptr<Regex>
FixedRepetitionRegex::do_clone() const
{
    return Utils::make_unique<FixedRepetitionRegex>(
                    child_to_repeat().clone(), fixed_count());
}

// accessing

RepetitionCount
FixedRepetitionRegex::fixed_count() const
{
    assert(min_count() == max_count());
    return min_count();
}

string
FixedRepetitionRegex::repetition_suffix() const
{
    return '{' + fixed_count().to_string() + '}';
}


// RangeRepetitionRegex
// --------------------

// instance creation and deletion

RangeRepetitionRegex::RangeRepetitionRegex(unique_ptr<Regex>      child,
                                           const RepetitionCount& min_count,
                                           const RepetitionCount& max_count) :
  CountedRepetitionRegex(move(child), min_count, max_count)
{
    assert(min_count.is_not_infinite());
    assert(max_count.is_not_infinite());
    assert(class_invariant());
}

// copying

unique_ptr<Regex>
RangeRepetitionRegex::do_clone() const
{
    return Utils::make_unique<RangeRepetitionRegex>(
                    child_to_repeat().clone(), min_count(), max_count());
}

// accessing

string
RangeRepetitionRegex::repetition_suffix() const
{
    return '{' + min_count().to_string() + ',' + max_count().to_string() + '}';
}


// RangeRepetitionToInfinityRegex
// ------------------------------

// instance creation and deletion

RangeRepetitionToInfinityRegex::RangeRepetitionToInfinityRegex(
                                  unique_ptr<Regex>      child,
                                  const RepetitionCount& min_count) :
  CountedRepetitionRegex(move(child), min_count, RepetitionCount::infinite())
{
    assert(min_count.is_not_infinite());
    assert(class_invariant());
}

// copying

unique_ptr<Regex>
RangeRepetitionToInfinityRegex::do_clone() const
{
    return Utils::make_unique<RangeRepetitionToInfinityRegex>(
                    child_to_repeat().clone(), min_count());
}

// accessing

string
RangeRepetitionToInfinityRegex::repetition_suffix() const
{
    return '{' + min_count().to_string() + ",}";
}
