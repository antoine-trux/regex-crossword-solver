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


#ifndef REGEX_HPP
#define REGEX_HPP

#include "group_number.hpp"
#include "repetition_count.hpp"
#include "set_of_characters.hpp"

#include <iosfwd>
#include <memory>
#include <string>
#include <vector>

class BackreferenceNumbers;
class BackreferenceRegex;
class CharacterBlock;
class Constraint;
class GroupRegex;
class PositiveLookaheadRegex;
class RegexOptimizations;


// class hierarchy
// ---------------
// Regex
//     NullaryRegex
//         EmptyRegex
//         EpsilonRegex
//         EpsilonAtStartRegex
//         EpsilonAtEndRegex
//         EpsilonAtWordBoundaryRegex
//         EpsilonNotAtWordBoundaryRegex
//         PositiveLookaheadRegex
//         CharacterBlockRegex
//         StringRegex
//         BackreferenceRegex
//     AbstractGroupRegex
//         GroupRegex
//         NonCapturingGroupRegex
//     BinaryRegex
//         ConcatenationRegex
//         UnionRegex
//     RepetitionRegex
//         KleeneStarRegex
//         PlusRegex
//         QuestionMarkRegex
//         CountedRepetitionRegex
//             FixedRepetitionRegex
//             RangeRepetitionRegex
//             RangeRepetitionToInfinityRegex


// An abstract class for all the regex subclasses.
//
// Apart from their constructors, the subclasses of this class have no
// public methods. Therefore, the public interface of this class
// hierarchy consists of:
// 1. the public methods of this class,
// 2. the constructors of the concrete subclasses of this class.
class Regex
{
public:
    // instance creation and deletion
    virtual ~Regex() = 0;

    // copying
    std::unique_ptr<Regex> clone() const;

    // accessing
    Constraint constrain(const Constraint& constraint);
    std::vector<Constraint> constraints(const Constraint& constraint,
                                        size_t            begin_pos);
    std::string explicit_characters() const;
    static std::unique_ptr<Regex> parse(const std::string& regex_as_string);

    // converting
    std::string to_string() const;

    // modifying
    static std::unique_ptr<Regex> optimize(
                                    std::unique_ptr<Regex>    root,
                                    const RegexOptimizations& optimizations);

protected:
    // instance creation and deletion
    Regex();

    // copying
    static std::unique_ptr<Regex> clone(const Regex& regex, Regex* parent);

    // accessing
    static std::vector<const BackreferenceRegex*>
               backreferences_to(const Regex&       regex,
                                 const GroupNumber& group_number);
    size_t begin_pos() const;
    static size_t begin_pos(const Regex& regex);
    static bool constrain_as_positive_lookahead(Regex&      regex,
                                                Constraint& constraint,
                                                size_t      begin_pos);
    static bool constrain_once_with_current_value(Regex&      regex,
                                                  Constraint& constraint,
                                                  size_t      offset);
    static bool constrain_word_boundaries_with_current_value(
                  Regex& regex, Constraint& constraint);
    size_t constraint_size() const;
    static const GroupRegex* enclosing_group(const Regex& regex);
    const GroupRegex* enclosing_group() const;
    static size_t end_pos(const Regex& regex);
    size_t end_pos() const;
    static void get_used_backreference_numbers(
                   const Regex&          regex,
                   BackreferenceNumbers& used_backreference_numbers);
    static std::vector<const GroupRegex*> groups(const Regex& regex);
    static size_t length_of_current_value(const Regex& regex);
    size_t length_of_current_value() const;
    static Regex* parent(const Regex& regex);
    Regex* parent() const;
    static GroupRegex* rightmost_group(Regex&             regex,
                                       const GroupNumber& group_number,
                                       const Regex*       from_child);
    static GroupRegex* rightmost_group(Regex&             regex,
                                       const GroupNumber& group_number);
    GroupRegex* rightmost_group_from_parent(
                  const GroupNumber& group_number) const;
    const Regex& root() const;

    // querying
    static bool at_end(const Regex& regex);
    bool        at_end() const;
    static bool can_be_concatenated(const Regex& regex);
    static bool can_be_unified(const Regex& regex);
    static bool characters_were_constrained_by_backreference(
                  const Regex& regex);
    bool        class_invariant() const;
    static bool has_a_value(const Regex& regex);
    bool        has_a_value() const;
    static bool has_a_value_which_fits(const Regex& regex);
    static bool is_character_block(const Regex& regex);
    static bool is_concatenation(const Regex& regex);
    static bool is_empty(const Regex& regex);
    static bool is_string(const Regex& regex);
    static bool is_union(const Regex& regex);
    static bool not_at_end(const Regex& regex);
    bool        not_at_end() const;
    static bool parents_are_correctly_setup(const Regex& regex);
    static bool value_is_epsilon(const Regex& regex);

    // converting
    virtual std::string do_to_string() const = 0;
    template<typename TargetType> static TargetType& downcast(Regex& regex);
    template<typename TargetType> static std::unique_ptr<TargetType>
        downcast(std::unique_ptr<Regex> regex);

    // modifying
    virtual void do_increment() = 0;
    virtual void do_set_constraint_size(size_t constraint_size);
    static void increment(Regex& regex);
    static std::unique_ptr<Regex> optimize_concatenations(
                                    std::unique_ptr<Regex> regex);
    static std::unique_ptr<Regex> optimize_concatenations_of_root(
                                    std::unique_ptr<Regex> root);
    static std::unique_ptr<Regex> optimize_concatenations_on_left(
                                    std::unique_ptr<Regex> regex);
    static std::unique_ptr<Regex> optimize_concatenations_on_right(
                                    std::unique_ptr<Regex> regex);
    static std::unique_ptr<Regex> optimize_groups(
        std::unique_ptr<Regex>      regex,
        const BackreferenceNumbers& used_backreference_numbers);
    static std::unique_ptr<Regex> optimize_groups_of_root(
                                    std::unique_ptr<Regex> root);
    static std::unique_ptr<Regex> optimize_unions(
                                    std::unique_ptr<Regex> regex);
    static std::unique_ptr<Regex> optimize_unions_of_root(
                                    std::unique_ptr<Regex> root);
    static void reset_after_constrain(Regex& regex);
    static void reset_characters_were_constrained_by_backreference(
                  Regex& regex);
    static void rewind(Regex& regex, size_t begin_pos);
    static void set_constraint_size(Regex& regex, size_t constraint_size);
    static void set_parent(Regex& regex, Regex* parent);

    // error handling
    static void check_no_self_references(const Regex& regex);

private:
    // instance creation and deletion
    static std::unique_ptr<Regex> optimized(
                                    std::unique_ptr<Regex> regex,
                                    Regex*                 optimized_regex,
                                    Regex*                 parent);

    // copying
    virtual std::unique_ptr<Regex> do_clone() const = 0;

    // accessing
    virtual std::vector<const BackreferenceRegex*>
                backreferences_to(const GroupNumber& group_number) const = 0;
    bool constrain_as_positive_lookahead(Constraint& constraint,
                                         size_t      begin_pos);
    bool constrain_once_with_current_value(Constraint& constraint);
    bool constrain_once_with_current_value(Constraint& constraint,
                                           size_t      offset);
    bool constrain_with_current_value(Constraint& constraint);
    bool constrain_word_boundaries_with_current_value(Constraint& constraint);
    virtual bool do_constrain_once_with_current_value(
                   Constraint& constraint, size_t offset) = 0;
    virtual bool do_constrain_word_boundaries_with_current_value(
                   Constraint& constraint) = 0;
    virtual std::string do_explicit_characters() const = 0;
    virtual void do_get_used_backreference_numbers(
                   BackreferenceNumbers& used_backreference_numbers) const = 0;
    virtual size_t do_length_of_current_value() const = 0;
    virtual GroupRegex* do_rightmost_group(const GroupNumber& group_number,
                                           const Regex*       from_child) = 0;
    virtual GroupRegex* do_rightmost_group(const GroupNumber& group_number) = 0;
    const PositiveLookaheadRegex* enclosing_lookahead() const;
    virtual std::vector<const GroupRegex*> groups() const = 0;
    static size_t invalid_begin_pos(size_t constraint_size);
    virtual const GroupRegex* yourself_or_enclosing_group() const;
    virtual const PositiveLookaheadRegex*
        yourself_or_enclosing_lookahead() const;

    // querying
    bool         begin_pos_is_not_set() const;
    bool         constraint_size_is_not_set() const;
    virtual bool do_at_end() const = 0;
    virtual bool do_can_be_concatenated() const;
    virtual bool do_can_be_unified() const;
    virtual bool do_characters_were_constrained_by_backreference() const = 0;
    virtual bool do_has_a_value() const;
    virtual bool do_is_character_block() const;
    virtual bool do_is_concatenation() const;
    virtual bool do_is_empty() const;
    virtual bool do_is_string() const;
    virtual bool do_is_union() const;
    virtual bool do_parents_are_correctly_setup() const = 0;
    bool         has_a_value_which_fits() const;
    bool         has_ancestor(const Regex* regex) const;
    bool         has_no_value() const;
    bool         has_parent() const;
    bool         is_root() const;
    bool         parents_are_correctly_setup() const;
    bool         value_fits() const;
    bool         value_fits_exactly() const;

    // modifying
    virtual Regex* do_optimize_concatenations() = 0;
    virtual Regex* do_optimize_concatenations_on_left();
    virtual Regex* do_optimize_concatenations_on_right();
    virtual Regex* do_optimize_groups(
                    const BackreferenceNumbers& used_backreference_numbers) = 0;
    virtual Regex* do_optimize_unions() = 0;
    virtual void do_reset_after_constrain() = 0;
    virtual void do_reset_characters_were_constrained_by_backreference() = 0;
    virtual void do_rewind() = 0;
    void increment();
    void rewind();
    void rewind(size_t begin_pos);
    void set_begin_pos(size_t begin_pos);
    void set_constraint_size(size_t constraint_size);
    virtual void set_constraint_size_of_children(size_t constraint_size) = 0;
    void set_parent(Regex* parent);

    // error handling
    void check_lookaheads_are_not_referenced_from_outside() const;
    void check_no_self_references() const;
    virtual void do_check_no_self_references() const = 0;

    // data members

    // The parent of this regex in its parse tree, or 'nullptr' if this
    // regex is the root of its parse tree.
    Regex* m_parent;

    // The size of the constraint that this regex is to handle.
    size_t m_constraint_size;

    // The position in the constraint where the value of this regex
    // starts to apply.
    //
    // For example, in regex 'A[BC]' (= concatenation of sub-regexes 'A'
    // and '[BC]'):
    // * m_begin_pos of 'A[BC]' is set to 0
    // * m_begin_pos of 'A' is set to 0
    // * m_begin_pos of '[BC]' is set to 1
    //
    // If the regex contains repetitions, sub-regexes can have different
    // m_begin_pos values when the regex is iterated.
    //
    // For example, in regex 'A*B' (= concatenation of sub-regexes 'A*'
    // and 'B'):
    // * m_begin_pos of 'B' is set to 0 when 'A' is repeated 0 times
    // * m_begin_pos of 'B' is set to 1 when 'A' is repeated once
    // * etc.
    size_t m_begin_pos;
};


// An abstract class for regex classes that have no children in their
// parse tree.
class NullaryRegex : public Regex
{
protected:
    // instance creation and deletion
    NullaryRegex();

    // querying
    bool do_at_end() const override;

private:
    // accessing
    std::vector<const BackreferenceRegex*>
        backreferences_to(const GroupNumber& group_number) const override;
    bool do_constrain_once_with_current_value(
           Constraint& constraint, size_t offset) override;
    bool do_constrain_word_boundaries_with_current_value(
           Constraint& constraint) override;
    std::string do_explicit_characters() const override;
    void do_get_used_backreference_numbers(
           BackreferenceNumbers& used_backreference_numbers) const override;
    size_t do_length_of_current_value() const override;
    GroupRegex* do_rightmost_group(const GroupNumber& group_number,
                                   const Regex*       from_child) override;
    GroupRegex* do_rightmost_group(const GroupNumber& group_number) override;
    std::vector<const GroupRegex*> groups() const override;

    // querying
    bool do_characters_were_constrained_by_backreference() const override;
    bool do_parents_are_correctly_setup() const override;

    // modifying
    void do_increment() override;
    Regex* do_optimize_concatenations() override;
    Regex* do_optimize_groups(
             const BackreferenceNumbers& used_backreference_numbers) override;
    Regex* do_optimize_unions() override;
    void do_reset_after_constrain() override;
    void do_reset_characters_were_constrained_by_backreference() override;
    void do_rewind() override;
    void set_constraint_size_of_children(size_t constraint_size) override;

    // error handling
    void do_check_no_self_references() const override;

    // data members

    // Whether this regex has been iterated to its end.
    bool m_at_end;
};


// An instance of this class represents the "empty regex", i.e., a
// regex that has no possible values.
//
// Instances of this class are never created by this program. This class
// is included only for completeness.
class EmptyRegex final : public NullaryRegex
{
public:
    // instance creation and deletion
    EmptyRegex();

private:
    // copying
    std::unique_ptr<Regex> do_clone() const override;

    // querying
    bool do_at_end() const override;
    bool do_can_be_unified() const override;
    bool do_is_empty() const override;

    // converting
    std::string do_to_string() const override;
};


// An instance of this class represents the empty string.
//
// For example, 'A|' is the UnionRegex of:
// * CharacterBlockRegex 'A'
// * EpsilonRegex
class EpsilonRegex final : public NullaryRegex
{
public:
    // instance creation and deletion
    EpsilonRegex();

private:
    // copying
    std::unique_ptr<Regex> do_clone() const override;

    // querying
    bool do_can_be_concatenated() const override;

    // converting
    std::string do_to_string() const override;
};


// An instance of this class represents an empty string regex that is
// matched only at the beginning of the string (noted '^' or '\A').
//
// For example, '^A' is the ConcatenationRegex of:
// * EpsilonAtStartRegex
// * CharacterBlockRegex 'A'
class EpsilonAtStartRegex final : public NullaryRegex
{
public:
    // instance creation and deletion
    EpsilonAtStartRegex();

private:
    // copying
    std::unique_ptr<Regex> do_clone() const override;

    // accessing
    bool do_constrain_once_with_current_value(
           Constraint& constraint, size_t offset) override;

    // converting
    std::string do_to_string() const override;
};


// An instance of this class represents an empty string regex that is
// matched only at the end of the string (noted '$' or '\Z').
//
// For example, 'A$' is the ConcatenationRegex of:
// * CharacterBlockRegex 'A'
// * EpsilonAtEndRegex
class EpsilonAtEndRegex final : public NullaryRegex
{
public:
    // instance creation and deletion
    EpsilonAtEndRegex();

private:
    // copying
    std::unique_ptr<Regex> do_clone() const override;

    // accessing
    bool do_constrain_once_with_current_value(
           Constraint& constraint, size_t offset) override;

    // converting
    std::string do_to_string() const override;
};


// An instance of this class represents an empty string regex that is
// matched only at a word boundary (noted '\b').
//
// For example, 'A\b' is the ConcatenationRegex of:
// * CharacterBlockRegex 'A'
// * EpsilonAtWordBoundaryRegex
class EpsilonAtWordBoundaryRegex final : public NullaryRegex
{
public:
    // instance creation and deletion
    EpsilonAtWordBoundaryRegex();

private:
    // copying
    std::unique_ptr<Regex> do_clone() const override;

    // accessing
    bool do_constrain_word_boundaries_with_current_value(
           Constraint& constraint) override;

    // converting
    std::string do_to_string() const override;
};


// An instance of this class represents an empty string regex that is
// matched anywhere except at a word boundary (noted '\B').
//
// For example, 'A\B' is the ConcatenationRegex of:
// * CharacterBlockRegex 'A'
// * EpsilonNotAtWordBoundaryRegex
class EpsilonNotAtWordBoundaryRegex final : public NullaryRegex
{
public:
    // instance creation and deletion
    EpsilonNotAtWordBoundaryRegex();

private:
    // copying
    std::unique_ptr<Regex> do_clone() const override;

    // accessing
    bool do_constrain_word_boundaries_with_current_value(
           Constraint& constraint) override;

    // converting
    std::string do_to_string() const override;
};


// An instance of this class represents a positive lookahead regex
// (noted '(?=...)').
class PositiveLookaheadRegex final : public NullaryRegex
{
public:
    // instance creation and deletion
    explicit PositiveLookaheadRegex(std::unique_ptr<Regex> regex);

private:
    // copying
    std::unique_ptr<Regex> do_clone() const override;

    // accessing
    std::vector<const BackreferenceRegex*>
        backreferences_to(const GroupNumber& group_number) const override;
    bool do_constrain_once_with_current_value(
           Constraint& constraint, size_t offset) override;
    std::string do_explicit_characters() const override;
    void do_get_used_backreference_numbers(
           BackreferenceNumbers& used_backreference_numbers) const override;
    std::vector<const GroupRegex*> groups() const override;
    const PositiveLookaheadRegex*
        yourself_or_enclosing_lookahead() const override;

    // converting
    std::string do_to_string() const override;

    // modifying
    Regex* do_optimize_concatenations() override;
    Regex* do_optimize_groups(
             const BackreferenceNumbers& used_backreference_numbers) override;
    Regex* do_optimize_unions() override;
    void set_constraint_size_of_children(size_t constraint_size) override;

    // error handling
    void do_check_no_self_references() const override;

    // The regex which this lookahead regex asserts. For example, if
    // this lookahead regex is '(?=abc)', m_regex is 'abc'.
    std::unique_ptr<Regex> m_regex;
};


// An instance of this class represents a character block regex. See
// class CharacterBlock and its subclasses for further details.
//
// For example, 'A[B-E]' is the ConcatenationRegex of:
// * CharacterBlockRegex 'A'
// * CharacterBlockRegex '[B-E]'
class CharacterBlockRegex final : public NullaryRegex
{
public:
    // instance creation and deletion
    explicit CharacterBlockRegex(
               std::unique_ptr<CharacterBlock> character_block);
    CharacterBlockRegex(std::unique_ptr<Regex> regex_1,
                        std::unique_ptr<Regex> regex_2);

    // accessing
    std::unique_ptr<CharacterBlock> steal_character_block();

private:
    // instance creation and deletion
    void append_character_block_of_regex(std::unique_ptr<Regex> regex);

    // copying
    std::unique_ptr<Regex> do_clone() const override;

    // accessing
    const SetOfCharacters& characters() const;
    SetOfCharacters compute_characters() const;
    bool do_constrain_once_with_current_value(
           Constraint& constraint, size_t offset) override;
    std::string do_explicit_characters() const override;
    size_t do_length_of_current_value() const override;

    // querying
    bool do_can_be_concatenated() const override;
    bool do_can_be_unified() const override;
    bool do_characters_were_constrained_by_backreference() const override;
    bool do_is_character_block() const override;

    // converting
    std::string do_to_string() const override;

    // modifying
    void do_reset_after_constrain() override;
    void do_reset_characters_were_constrained_by_backreference() override;

    // data members

    // The CharacterBlock associated to this CharacterBlockRegex.
    std::unique_ptr<CharacterBlock> m_character_block;

    // If 'm_characters_are_initialized' is true, 'm_characters'
    // contains the characters of 'm_character_block'. 'm_characters' is
    // undefined otherwise.
    mutable SetOfCharacters m_characters;
    mutable bool            m_characters_are_initialized;

    // If 'm_constrained_characters_are_initialized' is true,
    // 'm_constrained_characters' contains the characters of the current
    // constraint. 'm_constrained_characters' is undefined otherwise.
    //
    // Before a constraint is applied, 'm_constrained_characters' is
    // initialized to the characters of 'm_character_block'.
    SetOfCharacters m_constrained_characters;
    bool            m_constrained_characters_are_initialized;

    // Whether 'm_characters' were constrained by a backreference.
    bool m_characters_were_constrained_by_backreference;
};


// An instance of this class represents a sequence of
// CharacterBlockRegex'es. It never appears in the original parse tree,
// only when optimizing concatenations.
//
// For example, 'A[B-E]' may be optimized into the StringRegex
// '"A[B-E]"'.
class StringRegex final : public NullaryRegex
{
public:
    // instance creation and deletion
    explicit StringRegex(
               std::vector<std::unique_ptr<CharacterBlock>>&& character_blocks);
    StringRegex(std::unique_ptr<Regex> regex_1,
                std::unique_ptr<Regex> regex_2);

private:
    // instance creation and deletion
    void append_character_blocks_of_regex(std::unique_ptr<Regex> regex);

    // copying
    std::unique_ptr<Regex> do_clone() const override;

    // accessing
    std::vector<SetOfCharacters>& characters() const;
    bool do_constrain_once_with_current_value(
           Constraint& constraint, size_t offset) override;
    std::string do_explicit_characters() const override;
    size_t do_length_of_current_value() const override;
    void initialize_characters() const;
    void initialize_constrained_characters();

    // querying
    bool do_can_be_concatenated() const override;
    bool do_characters_were_constrained_by_backreference() const override;
    bool do_is_string() const override;

    // converting
    std::string do_to_string() const override;

    // modifying
    void do_reset_after_constrain() override;
    void do_reset_characters_were_constrained_by_backreference() override;

    // data members

    // The CharacterBlock's associated to this StringRegex, for each
    // index of the string.
    std::vector<std::unique_ptr<CharacterBlock>> m_character_blocks;

    // If 'm_characters_are_initialized' is true, each element of
    // 'm_characters' contains the corresponding characters of
    // 'm_character_blocks'. 'm_characters' is undefined otherwise.
    mutable std::vector<SetOfCharacters> m_characters;
    mutable bool                         m_characters_are_initialized;

    // If 'm_constrained_characters_are_initialized' is true, each
    // element of 'm_constrained_characters' contains the characters of
    // the corresponding element of the current constraint.
    // 'm_constrained_characters' is undefined otherwise.
    //
    // Before a constraint is applied, each element of
    // 'm_constrained_characters' is initialized to the characters of
    // the corresponding element of 'm_character_blocks'.
    std::vector<SetOfCharacters> m_constrained_characters;
    bool                         m_constrained_characters_are_initialized;

    // Whether any of the elements of 'm_characters' was constrained by
    // a backreference.
    bool m_characters_were_constrained_by_backreference;
};


// An instance of this class represents a backreference regex.
//
// For example, '(A)\1' is the ConcatenationRegex of:
// * GroupRegex '(A)'
// * BackreferenceRegex '\1' - which references '(A)'
class BackreferenceRegex final : public NullaryRegex
{
public:
    // instance creation and deletion
    explicit BackreferenceRegex(const GroupNumber& referenced_group_number);

private:
    // copying
    std::unique_ptr<Regex> do_clone() const override;

    // accessing
    std::vector<const BackreferenceRegex*>
        backreferences_to(const GroupNumber& group_number) const override;
    bool do_constrain_once_with_current_value(
           Constraint& constraint, size_t offset) override;
    void do_get_used_backreference_numbers(
           BackreferenceNumbers& used_backreference_numbers) const override;
    size_t do_length_of_current_value() const override;
    GroupRegex* referenced_group() const;

    // querying
    bool do_has_a_value() const override;

    // converting
    std::string do_to_string() const override;

    // error handling
    void do_check_no_self_references() const override;

    // data members

    GroupNumber m_referenced_group_number;
};


// An abstract class for the group subclasses.
class AbstractGroupRegex : public Regex
{
protected:
    // instance creation and deletion
    explicit AbstractGroupRegex(std::unique_ptr<Regex> child);

    // accessing
    Regex& child() const;
    GroupRegex* do_rightmost_group(const GroupNumber& group_number,
                                   const Regex*       from_child) override;

private:
    // accessing
    std::vector<const BackreferenceRegex*>
        backreferences_to(const GroupNumber& group_number) const override;
    bool do_constrain_once_with_current_value(
           Constraint& constraint, size_t offset) override;
    bool do_constrain_word_boundaries_with_current_value(
           Constraint& constraint) override;
    std::string do_explicit_characters() const override;
    void do_get_used_backreference_numbers(
           BackreferenceNumbers& used_backreference_numbers) const override;
    size_t do_length_of_current_value() const override;

    // querying
    bool do_at_end() const override;
    bool do_characters_were_constrained_by_backreference() const override;
    bool do_parents_are_correctly_setup() const override;
    virtual bool number_belongs_to(
                   const BackreferenceNumbers& backreference_numbers) const = 0;

    // modifying
    void do_increment() override;
    Regex* do_optimize_concatenations() override;
    Regex* do_optimize_groups(
             const BackreferenceNumbers& used_backreference_numbers) override;
    Regex* do_optimize_unions() override;
    void do_reset_after_constrain() override;
    void do_reset_characters_were_constrained_by_backreference() override;
    void do_rewind() override;
    void set_constraint_size_of_children(size_t constraint_size) override;

    // error handling
    void do_check_no_self_references() const override;

    // data members

    // For example, if this GroupRegex is '(A)', 'm_child' is the
    // CharacterBlockRegex 'A'.
    std::unique_ptr<Regex> m_child;
};


// An instance of this class represents a group regex.
//
// For example, '(A)B' is the ConcatenationRegex of:
// * GroupRegex '(A)'
// * CharacterBlockRegex 'B'
class GroupRegex final : public AbstractGroupRegex
{
public:
    // instance creation and deletion
    GroupRegex(std::unique_ptr<Regex> child, const GroupNumber& group_number);

    // accessing
    GroupNumber number() const;

    // querying
    bool has_number(const GroupNumber& group_number) const;

private:
    // copying
    std::unique_ptr<Regex> do_clone() const override;

    // accessing
    using AbstractGroupRegex::do_rightmost_group;
    GroupRegex* do_rightmost_group(const GroupNumber& group_number) override;
    std::vector<const GroupRegex*> groups() const override;
    const GroupRegex* yourself_or_enclosing_group() const override;

    // querying
    bool number_belongs_to(
           const BackreferenceNumbers& backreference_numbers) const override;

    // converting
    std::string do_to_string() const override;

    // data members

    GroupNumber m_group_number;
};


// An instance of this class represents a non-capturing group regex.
//
// For example, '(?:A)B' is the ConcatenationRegex of:
// * NonCapturingGroupRegex '(?:A)'
// * CharacterBlockRegex 'B'
class NonCapturingGroupRegex final : public AbstractGroupRegex
{
public:
    // instance creation and deletion
    explicit NonCapturingGroupRegex(std::unique_ptr<Regex> child);

private:
    // copying
    std::unique_ptr<Regex> do_clone() const override;

    // accessing
    using AbstractGroupRegex::do_rightmost_group;
    GroupRegex* do_rightmost_group(const GroupNumber& group_number) override;
    std::vector<const GroupRegex*> groups() const override;

    // querying
    bool number_belongs_to(
           const BackreferenceNumbers& backreference_numbers) const override;

    // converting
    std::string do_to_string() const override;
};


// An abstract class for regex classes that have two children in their
// parse tree.
class BinaryRegex : public Regex
{
protected:
    // instance creation and deletion
    BinaryRegex(std::unique_ptr<Regex> left_child,
                std::unique_ptr<Regex> right_child);

    // accessing
    Regex& left_child() const;
    Regex& right_child() const;
    std::unique_ptr<Regex> steal_left_child();
    std::unique_ptr<Regex> steal_right_child();

    // modifying
    void rotate_left();
    void rotate_right();
    void set_children(std::unique_ptr<Regex> left_child,
                      std::unique_ptr<Regex> right_child);
    void set_left_child(std::unique_ptr<Regex> left_child);
    void set_left_child(Regex* left_child);
    void set_right_child(std::unique_ptr<Regex> right_child);
    void set_right_child(Regex* right_child);
    void swap_children();

private:
    // instance creation and deletion
    virtual std::unique_ptr<Regex>
        create(std::unique_ptr<Regex> left_child,
               std::unique_ptr<Regex> right_child) const = 0;

    // accessing
    std::vector<const BackreferenceRegex*>
        backreferences_to(const GroupNumber& group_number) const override;
    std::string do_explicit_characters() const override;
    void do_get_used_backreference_numbers(
           BackreferenceNumbers& used_backreference_numbers) const override;
    std::vector<const GroupRegex*> groups() const override;
    virtual std::string operator_string() const = 0;

    // querying
    bool do_parents_are_correctly_setup() const override;

    // converting
    std::string do_to_string() const override;

    // modifying
    Regex* do_optimize_concatenations() override;
    Regex* do_optimize_groups(
             const BackreferenceNumbers& used_backreference_numbers) override;
    Regex* do_optimize_unions() override;
    void do_reset_after_constrain() override;
    void do_reset_characters_were_constrained_by_backreference() override;
    void set_constraint_size_of_children(size_t constraint_size) override;

    // error handling
    void do_check_no_self_references() const override;

    // data members

    // For example, if this BinaryRegex is 'AB' (a ConcatenationRegex):
    // * m_left_child  = CharacterBlockRegex 'A'
    // * m_right_child = CharacterBlockRegex 'B'
    std::unique_ptr<Regex> m_left_child;
    std::unique_ptr<Regex> m_right_child;
};


// An instance of this class represents a concatenation regex.
//
// For example, 'AB' is the ConcatenationRegex of:
// * CharacterBlockRegex 'A'
// * CharacterBlockRegex 'B'
class ConcatenationRegex final : public BinaryRegex
{
public:
    // instance creation and deletion
    ConcatenationRegex(std::unique_ptr<Regex> left_child,
                       std::unique_ptr<Regex> right_child);

private:
    // instance creation and deletion
    std::unique_ptr<Regex>
        create(std::unique_ptr<Regex> left_child,
               std::unique_ptr<Regex> right_child) const override;

    // copying
    std::unique_ptr<Regex> do_clone() const override;

    // accessing
    bool do_constrain_once_with_current_value(
           Constraint& constraint, size_t offset) override;
    bool do_constrain_word_boundaries_with_current_value(
           Constraint& constraint) override;
    size_t do_length_of_current_value() const override;
    GroupRegex* do_rightmost_group(const GroupNumber& group_number,
                                   const Regex*       from_child) override;
    GroupRegex* do_rightmost_group(const GroupNumber& group_number) override;
    std::string operator_string() const override;

    // querying
    bool do_at_end() const override;
    bool do_characters_were_constrained_by_backreference() const override;
    bool do_is_concatenation() const override;

    // modifying
    StringRegex* concatenate();
    void   do_increment() override;
    Regex* do_optimize_concatenations() override;
    Regex* do_optimize_concatenations_on_left() override;
    Regex* do_optimize_concatenations_on_right() override;
    void   do_rewind() override;
    Regex* optimize_concatenations_left_and_right();
    Regex* optimize_concatenations_left_and_right_left();
    Regex* optimize_concatenations_left_right_and_right();
    Regex* optimize_concatenations_left_right_and_right_left();
    void   while_right_at_end_increment();
};


// An instance of this class represents a union regex.
//
// For example, 'A|B' is the UnionRegex of:
// * CharacterBlockRegex 'A'
// * CharacterBlockRegex 'B'
class UnionRegex final : public BinaryRegex
{
public:
    // instance creation and deletion
    UnionRegex(std::unique_ptr<Regex> left_child,
               std::unique_ptr<Regex> right_child);

private:
    // instance creation and deletion
    std::unique_ptr<Regex>
        create(std::unique_ptr<Regex> left_child,
               std::unique_ptr<Regex> right_child) const override;

    // copying
    std::unique_ptr<Regex> do_clone() const override;

    // accessing
    Regex& active_child() const;
    bool do_constrain_once_with_current_value(
           Constraint& constraint, size_t offset) override;
    bool do_constrain_word_boundaries_with_current_value(
           Constraint& constraint) override;
    size_t do_length_of_current_value() const override;
    GroupRegex* do_rightmost_group(const GroupNumber& group_number,
                                   const Regex*       from_child) override;
    GroupRegex* do_rightmost_group(const GroupNumber& group_number) override;
    Regex& first_child_with_a_value() const;
    std::string operator_string() const override;

    // querying
    bool do_at_end() const override;
    bool do_characters_were_constrained_by_backreference() const override;
    bool do_is_union() const override;
    bool either_child_can_be_unified() const;

    // modifying
    void   do_increment() override;
    Regex* do_optimize_unions() override;
    void   do_rewind() override;
    Regex* optimize_unions_left_and_right();
    Regex* optimize_unions_left_and_right_either();
    Regex* optimize_unions_left_either_and_right();
    Regex* optimize_unions_left_either_and_right_either();
    void   place_child_which_can_be_unified_to_left();
    void   place_child_which_can_be_unified_to_right();
    Regex* unify();
};


// An abstract class for repetition regexes.
class RepetitionRegex : public Regex
{
protected:
    // instance creation and deletion
    RepetitionRegex(std::unique_ptr<Regex> child,
                    const RepetitionCount& min_count,
                    const RepetitionCount& max_count);

    // accessing
    Regex& child_to_repeat() const;
    RepetitionCount max_count() const;
    RepetitionCount min_count() const;

private:
    // instance creation and deletion
    void build_fixed_children();
    void rebuild_after_optimization();

    // accessing
    std::vector<const BackreferenceRegex*>
        backreferences_to(const GroupNumber& group_number) const override;
    bool do_constrain_once_with_current_value(
           Constraint& constraint, size_t offset) override;
    bool do_constrain_word_boundaries_with_current_value(
           Constraint& constraint) override;
    std::string do_explicit_characters() const override;
    void do_get_used_backreference_numbers(
           BackreferenceNumbers& used_backreference_numbers) const override;
    size_t do_length_of_current_value() const override;
    GroupRegex* do_rightmost_group(const GroupNumber& group_number,
                                   const Regex*       from_child) override;
    GroupRegex* do_rightmost_group(const GroupNumber& group_number) override;
    std::vector<const GroupRegex*> groups() const override;
    std::unique_ptr<Regex>& last_variable_child();
    virtual std::string repetition_suffix() const = 0;

    // querying
    bool do_at_end() const override;
    bool do_characters_were_constrained_by_backreference() const override;
    bool do_parents_are_correctly_setup() const override;
    bool fixed_children_at_end() const;
    bool variable_children_at_end() const;

    // converting
    std::string do_to_string() const override;

    // modifying
    void append_fixed_child();
    void append_variable_child();
    void do_increment() override;
    Regex* do_optimize_concatenations() override;
    Regex* do_optimize_groups(
             const BackreferenceNumbers& used_backreference_numbers) override;
    Regex* do_optimize_unions() override;
    void do_reset_after_constrain() override;
    void do_reset_characters_were_constrained_by_backreference() override;
    void do_rewind() override;
    void do_set_constraint_size(size_t constraint_size) override;
    void increment_fixed_children();
    void increment_variable_children();
    void remove_last_variable_child();
    void remove_variable_children();
    void rewind_fixed_children();
    std::vector<std::unique_ptr<Regex>>::const_iterator
         rewind_fixed_children(
          std::vector<std::unique_ptr<Regex>>::const_iterator first,
          size_t                                              first_begin_pos);
    void rewind_variable_children();
    void set_constraint_size_of_children(size_t constraint_size) override;
    void while_fixed_child_at_end_increment_fixed(
           std::vector<std::unique_ptr<Regex>>::const_iterator
               first_fixed_child_at_end);
    void while_variable_children_at_end_increment();
    void while_variable_children_increment_them(
           bool last_child_was_just_appended);

    // error handling
    void do_check_no_self_references() const override;

    // data members

    // For example, if this RepetitionRegex is 'A*'
    // (a KleeneStarRegex), 'm_child_to_repeat' is the
    // CharacterBlockRegex 'A'.
    std::unique_ptr<Regex> m_child_to_repeat;

    // Examples:
    // 'A{2,5} => m_min_count = 2, m_max_count = 5
    // 'A{2,}  => m_min_count = 2, m_max_count = infinity
    // 'A*     => m_min_count = 0, m_max_count = infinity
    RepetitionCount m_min_count;
    RepetitionCount m_max_count;

    // The maximum number of elements of 'm_variable_children'.
    //
    // Examples:
    // 'A{2,5} => m_variable_children_max_count = 3
    // 'A*     => m_variable_children_max_count = infinity
    RepetitionCount m_variable_children_max_count;

    // Whether 'm_fixed_children' are at the beginning of their
    // iteration. This is necessary for the case where
    // 'm_fixed_children' is empty.
    bool m_fixed_children_at_beginning;

    // 'm_fixed_children' contains a fixed number of elements (the
    // minimum number of repetitions).
    //
    // Examples:
    // 'A{2,5} => m_fixed_children always contains 2 elements
    // 'A*     => m_fixed_children is always empty
    std::vector<std::unique_ptr<Regex>> m_fixed_children;

    // Whether 'm_variable_children' are at the beginning of their
    // iteration.
    bool m_variable_children_at_beginning;

    // 'm_variable_children' contains a number of elements which varies
    // in the interval [0, m_variable_children_max_count].
    //
    // Only the first 'm_num_used_variable_children' are meaningful; the
    // elements of 'm_variable_children' beyond the first
    // 'm_num_used_variable_children' ones are reused when iterating.
    //
    // Examples:
    // 1. 'A{2,5} => m_variable_children contains 0-3 elements
    // 2. 'A*     => m_variable_children may contain any number of
    //               elements
    // In both examples, the number of elements of m_variable_children
    // is limited by m_constraint_size - for example, if
    // m_constraint_size is 4 and m_begin_pos is 1, m_variable_children
    // may contain only 0 or 1 element in example 1, and [0, 3]
    // elements in example 2.
    std::vector<std::unique_ptr<Regex>> m_variable_children;

    // The number of elements of 'm_variable_children' which are
    // currently used.
    size_t m_num_used_variable_children;

    // The concatenation of 'm_fixed_children' and
    // 'm_variable_children'.
    std::vector<Regex*> m_all_children;
};


// An instance of this class represents a Kleene star regex.
//
// For example, 'A*' is the KleeneStarRegex of CharacterBlockRegex 'A'.
class KleeneStarRegex final : public RepetitionRegex
{
public:
    // instance creation and deletion
    explicit KleeneStarRegex(std::unique_ptr<Regex> child);

private:
    // copying
    std::unique_ptr<Regex> do_clone() const override;

    // accessing
    std::string repetition_suffix() const override;
};


// An instance of this class represents a plus regex.
//
// For example, 'A+' is the PlusRegex of CharacterBlockRegex 'A'.
class PlusRegex final : public RepetitionRegex
{
public:
    // instance creation and deletion
    explicit PlusRegex(std::unique_ptr<Regex> child);

private:
    // copying
    std::unique_ptr<Regex> do_clone() const override;

    // accessing
    std::string repetition_suffix() const override;
};


// An instance of this class represents a question mark regex.
//
// For example, 'A?' is the QuestionMarkRegex of CharacterBlockRegex 'A'.
class QuestionMarkRegex final : public RepetitionRegex
{
public:
    // instance creation and deletion
    explicit QuestionMarkRegex(std::unique_ptr<Regex> child);

private:
    // copying
    std::unique_ptr<Regex> do_clone() const override;

    // accessing
    std::string repetition_suffix() const override;
};


// An abstract class for repetition regexes which have one of their
// repetition counts explicitly mentioned.
class CountedRepetitionRegex : public RepetitionRegex
{
protected:
    // instance creation and deletion
    CountedRepetitionRegex(std::unique_ptr<Regex> child,
                           const RepetitionCount& min_count,
                           const RepetitionCount& max_count);
};


// An instance of this class represents a regex which is repeated a
// fixed number of times.
//
// For example, 'A{2}' is the CharacterBlockRegex 'A' repeated exactly
// twice.
class FixedRepetitionRegex final : public CountedRepetitionRegex
{
public:
    // instance creation and deletion
    FixedRepetitionRegex(std::unique_ptr<Regex> child,
                         const RepetitionCount& fixed_count);

private:
    // copying
    std::unique_ptr<Regex> do_clone() const override;

    // accessing
    RepetitionCount fixed_count() const;
    std::string repetition_suffix() const override;
};


// An instance of this class represents a regex which is repeated a
// range of times.
//
// For example, 'A{2-4}' is the CharacterBlockRegex 'A' repeated 2, 3
// or 4 times.
class RangeRepetitionRegex final : public CountedRepetitionRegex
{
public:
    // instance creation and deletion
    RangeRepetitionRegex(std::unique_ptr<Regex> child,
                         const RepetitionCount& min_count,
                         const RepetitionCount& max_count);

private:
    // copying
    std::unique_ptr<Regex> do_clone() const override;

    // accessing
    std::string repetition_suffix() const override;
};


// An instance of this class represents a regex which is repeated up to
// an infinite number of times.
//
// For example, 'A{2,}' is the CharacterBlockRegex 'A' repeated 2 or
// more times.
class RangeRepetitionToInfinityRegex final : public CountedRepetitionRegex
{
public:
    // instance creation and deletion
    RangeRepetitionToInfinityRegex(std::unique_ptr<Regex> child,
                                   const RepetitionCount& min_count);

private:
    // copying
    std::unique_ptr<Regex> do_clone() const override;

    // accessing
    std::string repetition_suffix() const override;
};


#endif // REGEX_HPP
