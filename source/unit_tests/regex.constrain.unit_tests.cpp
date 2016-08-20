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
#include "character_block.hpp"
#include "constraint.hpp"
#include "disable_warnings_from_gtest.hpp"
#include "regex.hpp"
#include "regex_crossword_solver_test.hpp"
#include "regex_optimizations.hpp"

#include <algorithm>
#include <numeric>

using namespace std;


class RegexConstrainTest : public RegexCrosswordSolverTest
{
};


namespace
{

void check_constraints(
       const Regex&                        regex,
       const Constraint&                   constraint,
       size_t                              begin_pos,
       const initializer_list<Constraint>& expected_constraints);
void check_constraints(const Regex&              regex,
                       const Constraint&         constraint,
                       size_t                    begin_pos,
                       const vector<Constraint>& sorted_expected_constraints,
                       const RegexOptimizations& optimizations);
Constraint combine(const vector<Constraint>& constraints,
                   size_t                    constraint_size);
string set_alphabet(const Regex& regex);

// Check that the constraints produced by 'regex' on 'constraint' are
// 'expected_constraints' (not necessarily in the same order).
void
check_constraints(const Regex&                        regex,
                  const Constraint&                   constraint,
                  size_t                              begin_pos,
                  const initializer_list<Constraint>& expected_constraints)
{
    vector<Constraint> sorted_expected_constraints(expected_constraints);
    sort(sorted_expected_constraints.begin(),
         sorted_expected_constraints.end());

    auto optimizations = RegexOptimizations::none();
    check_constraints(
      regex, constraint, begin_pos, sorted_expected_constraints, optimizations);

    optimizations.set(RegexOptimizations::Type::GROUPS, true);
    check_constraints(
      regex, constraint, begin_pos, sorted_expected_constraints, optimizations);

    optimizations.set(RegexOptimizations::Type::UNIONS, true);
    check_constraints(
      regex, constraint, begin_pos, sorted_expected_constraints, optimizations);

    optimizations.set(RegexOptimizations::Type::CONCATENATIONS, true);
    check_constraints(
      regex, constraint, begin_pos, sorted_expected_constraints, optimizations);
}

void
check_constraints(const Regex&              regex,
                  const Constraint&         constraint,
                  size_t                    begin_pos,
                  const vector<Constraint>& sorted_expected_constraints,
                  const RegexOptimizations& optimizations)
{
    auto cloned_regex = regex.clone();

    cloned_regex = Regex::optimize(move(cloned_regex), optimizations);

    auto constraints = cloned_regex->constraints(constraint, begin_pos);

    if (optimizations.optimize_unions())
    {
        // When unions are optimized, the elements of 'constraints' may
        // not match the elements of 'sorted_expected_constraints'.
        // However, the OR-combination of the elements of 'constraints'
        // does match the OR-combination of the elements of
        // 'sorted_expected_constraints'.
        //
        // For example, if 'regex' is 'A|B', 'constraint' is
        // { "ABC", "ABC" } and 'begin_pos' is 0, then
        // 'sorted_expected_constraints' contains two elements:
        //
        //     1. { "A", "ABC" }
        //     2. { "B", "ABC" }
        //
        // but if unions are optimized, thus resulting in regex '[AB]',
        // the resulting constraints contain a single element:
        // { "AB", "ABC" }. This element is the OR-combination of
        // constraints 1 and 2 above.

        const auto combined_expected_constraints =
            combine(sorted_expected_constraints, constraint.size());

        const auto combined_constraints =
            combine(constraints, constraint.size());

        EXPECT_EQ(combined_expected_constraints, combined_constraints);
    }
    else
    {
        // When unions are not optimized, the elements of 'constraints'
        // always match the elements of 'sorted_expected_constraints'.
        sort(constraints.begin(), constraints.end());
        EXPECT_EQ(sorted_expected_constraints, constraints);
    }
}

// Return the elements of 'constraints' ORed with each other.
//
// Precondition:
// * the elements of 'constraints' are all of the same size,
//   'constraint_size'
Constraint
combine(const vector<Constraint>& constraints, size_t constraint_size)
{
    return accumulate(constraints.cbegin(),
                      constraints.cend(),
                      Constraint::none(constraint_size),
                      [](Constraint& sum, const Constraint& constraint)
                      {
                          return sum | constraint;
                      });
}

// Set the alphabet to:
// * all the symbols that appear in 'regex', plus:
// * one additional character.
string
set_alphabet(const Regex& regex)
{
    const auto regex_characters = regex.explicit_characters();
    const set<char> characters_as_set(regex_characters.cbegin(),
                                      regex_characters.cend());
    const string characters_without_duplicates(characters_as_set.cbegin(),
                                               characters_as_set.cend());
    const char additional_character =
        characters_without_duplicates.empty() ?
        'A'                                   :
        static_cast<char>(characters_without_duplicates.back() + 1);
    const auto alphabet_characters = characters_without_duplicates +
                                     additional_character;
    Alphabet::set(alphabet_characters);
    return alphabet_characters;
}

} // unnamed namespace


TEST_F(RegexConstrainTest, empty)
{
    EmptyRegex regex;

    const auto all = set_alphabet(regex);

    const size_t constraint_size = 1;
    const auto constraint = Constraint::all(constraint_size);

    check_constraints(regex, constraint, 0, {});
}

TEST_F(RegexConstrainTest, epsilon)
{
    EpsilonRegex regex;

    const auto all = set_alphabet(regex);

    const size_t constraint_size = 1;
    const auto constraint = Constraint::all(constraint_size);

    check_constraints(regex, constraint, 0, { Constraint({ all }) });
    check_constraints(regex, constraint, 1, { Constraint({ all }) });
}

TEST_F(RegexConstrainTest, character)
{
    CharacterBlockRegex regex(SingleCharacter('A').clone());

    const auto all = set_alphabet(regex);

    const size_t constraint_size = 1;
    const auto constraint = Constraint::all(constraint_size);

    check_constraints(regex, constraint, 0, { Constraint({ "A" }) });
    check_constraints(regex, constraint, 1, {});
}

TEST_F(RegexConstrainTest, group)
{
    const auto regex = Regex::parse("(A)");

    const auto all = set_alphabet(*regex);

    const size_t constraint_size = 1;
    const auto constraint = Constraint::all(constraint_size);

    check_constraints(*regex, constraint, 0, { Constraint({ "A" }) });
    check_constraints(*regex, constraint, 1, {});
}

TEST_F(RegexConstrainTest, non_capturing_group)
{
    const auto regex = Regex::parse("(?:A)");

    const auto all = set_alphabet(*regex);

    const size_t constraint_size = 1;
    const auto constraint = Constraint::all(constraint_size);

    check_constraints(*regex, constraint, 0, { Constraint({ "A" }) });
    check_constraints(*regex, constraint, 1, {});
}

TEST_F(RegexConstrainTest, concatenation_1)
{
    const auto regex = Regex::parse("AB");

    const auto all = set_alphabet(*regex);

    const size_t constraint_size = 3;
    const auto constraint = Constraint::all(constraint_size);

    check_constraints(*regex, constraint, 0,
                      { Constraint({ "A", "B", all }) });
    check_constraints(*regex, constraint, 1,
                      { Constraint({ all, "A", "B" }) });

    for (size_t begin_pos = 2; begin_pos <= constraint_size; ++begin_pos)
    {
        check_constraints(*regex, constraint, begin_pos, {});
    }
}

TEST_F(RegexConstrainTest, concatenation_2)
{
    const auto regex = Regex::parse("(A|B)C");

    const auto all = set_alphabet(*regex);

    const size_t constraint_size = 3;
    const auto constraint = Constraint::all(constraint_size);

    check_constraints(*regex, constraint, 0,
                      { Constraint({ "A", "C", all }),
                        Constraint({ "B", "C", all }) });
    check_constraints(*regex, constraint, 1,
                      { Constraint({ all, "A", "C" }),
                        Constraint({ all, "B", "C" }) });

    for (size_t begin_pos = 2; begin_pos <= constraint_size; ++begin_pos)
    {
        check_constraints(*regex, constraint, begin_pos, {});
    }
}

TEST_F(RegexConstrainTest, concatenation_3)
{
    const auto regex = Regex::parse("(A|BB)C");

    const auto all = set_alphabet(*regex);

    const size_t constraint_size = 4;
    const auto constraint = Constraint::all(constraint_size);

    check_constraints(*regex, constraint, 0,
                      { Constraint({ "A", "C",  all, all }),
                        Constraint({ "B", "B", "C",  all }) });
    check_constraints(*regex, constraint, 1,
                      { Constraint({ all, "A", "C",  all }),
                        Constraint({ all, "B", "B", "C"  }) });
    check_constraints(*regex, constraint, 2,
                      { Constraint({ all, all, "A", "C" }) });

    for (size_t begin_pos = 3; begin_pos <= constraint_size; ++begin_pos)
    {
        check_constraints(*regex, constraint, begin_pos, {});
    }
}

TEST_F(RegexConstrainTest, concatenation_4)
{
    const auto regex = Regex::parse("(AA|B)C");

    const auto all = set_alphabet(*regex);

    const size_t constraint_size = 2;
    const auto constraint = Constraint::all(constraint_size);

    check_constraints(*regex, constraint, 0, { Constraint({ "B", "C" }) });

    for (size_t begin_pos = 1; begin_pos <= constraint_size; ++begin_pos)
    {
        check_constraints(*regex, constraint, begin_pos, {});
    }
}

TEST_F(RegexConstrainTest, union)
{
    const auto regex = Regex::parse("A|BB");

    const auto all = set_alphabet(*regex);

    const size_t constraint_size = 3;
    const auto constraint = Constraint::all(constraint_size);

    check_constraints(*regex, constraint, 0,
                      { Constraint({ "A",  all, all }),
                        Constraint({ "B", "B",  all }) });
    check_constraints(*regex, constraint, 1,
                      { Constraint({ all, "A", all }),
                        Constraint({ all, "B", "B" }) });
    check_constraints(*regex, constraint, 2,
                      { Constraint({ all, all, "A" }) });
    check_constraints(*regex, constraint, 3, {});
}

TEST_F(RegexConstrainTest, kleene_star_1)
{
    const auto regex = Regex::parse("A*");

    const auto all = set_alphabet(*regex);

    const size_t constraint_size = 3;
    const auto constraint = Constraint::all(constraint_size);

    check_constraints(*regex, constraint, 0,
                      { Constraint({ all, all, all }),
                        Constraint({ "A", all, all }),
                        Constraint({ "A", "A", all }),
                        Constraint({ "A", "A", "A" }) });
    check_constraints(*regex, constraint, 1,
                      { Constraint({ all, all, all }),
                        Constraint({ all, "A", all }),
                        Constraint({ all, "A", "A" }) });
    check_constraints(*regex, constraint, 2,
                      { Constraint({ all, all, all }),
                        Constraint({ all, all, "A" }) });
    check_constraints(*regex, constraint, 3,
                      { Constraint({ all, all, all }) });
}

TEST_F(RegexConstrainTest, kleene_star_2)
{
    const auto regex = Regex::parse("(A|BB)*");

    const auto all = set_alphabet(*regex);

    const size_t constraint_size = 3;
    const auto constraint = Constraint::all(constraint_size);

    check_constraints(*regex, constraint, 0,
                      { Constraint({ all, all, all }),
                        Constraint({ "A", all, all }),
                        Constraint({ "A", "A", all }),
                        Constraint({ "A", "A", "A" }),
                        Constraint({ "A", "B", "B" }),
                        Constraint({ "B", "B", "A" }),
                        Constraint({ "B", "B", all }) });
    check_constraints(*regex, constraint, 1,
                      { Constraint({ all, all, all }),
                        Constraint({ all, "A", all }),
                        Constraint({ all, "A", "A" }),
                        Constraint({ all, "B", "B" }) });
    check_constraints(*regex, constraint, 2,
                      { Constraint({ all, all, all }),
                        Constraint({ all, all, "A" }) });
    check_constraints(*regex, constraint, 3,
                      { Constraint({ all, all, all }) });
}

TEST_F(RegexConstrainTest, kleene_star_3)
{
    const auto regex = Regex::parse("(AA|B)*");

    const auto all = set_alphabet(*regex);

    const size_t constraint_size = 3;
    const auto constraint = Constraint::all(constraint_size);

    check_constraints(*regex, constraint, 0,
                      { Constraint({ all, all, all }),
                        Constraint({ "B", all, all }),
                        Constraint({ "B", "B", all }),
                        Constraint({ "B", "B", "B" }),
                        Constraint({ "B", "A", "A" }),
                        Constraint({ "A", "A", "B" }),
                        Constraint({ "A", "A", all }) });
    check_constraints(*regex, constraint, 1,
                      { Constraint({ all, all, all }),
                        Constraint({ all, "B", all }),
                        Constraint({ all, "B", "B" }),
                        Constraint({ all, "A", "A" }) });
    check_constraints(*regex, constraint, 2,
                      { Constraint({ all, all, all }),
                        Constraint({ all, all, "B" }) });
    check_constraints(*regex, constraint, 3,
                      { Constraint({ all, all, all }) });
}

TEST_F(RegexConstrainTest, kleene_star_4)
{
    const auto regex = Regex::parse("(|A)*");

    const auto all = set_alphabet(*regex);

    const size_t constraint_size = 3;
    const auto constraint = Constraint::all(constraint_size);

    check_constraints(*regex, constraint, 0,
                      { Constraint({ all, all, all }),
                        Constraint({ "A", all, all }),
                        Constraint({ "A", "A", all }),
                        Constraint({ "A", "A", "A" }) });
    check_constraints(*regex, constraint, 1,
                      { Constraint({ all, all, all }),
                        Constraint({ all, "A", all }),
                        Constraint({ all, "A", "A" }) });
    check_constraints(*regex, constraint, 2,
                      { Constraint({ all, all, all }),
                        Constraint({ all, all, "A" }) });
    check_constraints(*regex, constraint, 3,
                      { Constraint({ all, all, all }) });
}

TEST_F(RegexConstrainTest, kleene_star_5)
{
    const auto regex = Regex::parse("(AB|C)*");

    const auto all = set_alphabet(*regex);

    const size_t constraint_size = 3;
    const auto constraint = Constraint::all(constraint_size);

    check_constraints(*regex, constraint, 0,
                      { Constraint({ all, all, all }),
                        Constraint({ "A", "B", all }),
                        Constraint({ "C", all, all }),
                        Constraint({ "A", "B", "C" }),
                        Constraint({ "C", "A", "B" }),
                        Constraint({ "C", "C", all }),
                        Constraint({ "C", "C", "C" }) });
    check_constraints(*regex, constraint, 1,
                      { Constraint({ all, all, all }),
                        Constraint({ all, "A", "B" }),
                        Constraint({ all, "C", all }),
                        Constraint({ all, "C", "C" }) });
    check_constraints(*regex, constraint, 2,
                      { Constraint({ all, all, all }),
                        Constraint({ all, all, "C" }) });
    check_constraints(*regex, constraint, 3,
                      { Constraint({ all, all, all }) });
}

TEST_F(RegexConstrainTest, kleene_star_6)
{
    const auto regex = Regex::parse("()*A");

    const auto all = set_alphabet(*regex);

    const size_t constraint_size = 2;
    const auto constraint = Constraint::all(constraint_size);

    check_constraints(*regex, constraint, 0, { Constraint({ "A", all }) });
    check_constraints(*regex, constraint, 1, { Constraint({ all, "A" }) });
    check_constraints(*regex, constraint, 2, {});
}

TEST_F(RegexConstrainTest, plus_1)
{
    const auto regex = Regex::parse("A+");

    const auto all = set_alphabet(*regex);

    const size_t constraint_size = 3;
    const auto constraint = Constraint::all(constraint_size);

    check_constraints(*regex, constraint, 0,
                      { Constraint({ "A", all, all }),
                        Constraint({ "A", "A", all }),
                        Constraint({ "A", "A", "A" }) });
    check_constraints(*regex, constraint, 1,
                      { Constraint({ all, "A", all }),
                        Constraint({ all, "A", "A" }) });
    check_constraints(*regex, constraint, 2,
                      { Constraint({ all, all, "A" }) });
    check_constraints(*regex, constraint, 3, {});
}

TEST_F(RegexConstrainTest, plus_2)
{
    const auto regex = Regex::parse("(A|BB)+");

    const auto all = set_alphabet(*regex);

    const size_t constraint_size = 3;
    const auto constraint = Constraint::all(constraint_size);

    check_constraints(*regex, constraint, 0,
                      { Constraint({ "A", all, all }),
                        Constraint({ "A", "A", all }),
                        Constraint({ "A", "A", "A" }),
                        Constraint({ "A", "B", "B" }),
                        Constraint({ "B", "B", "A" }),
                        Constraint({ "B", "B", all }) });
    check_constraints(*regex, constraint, 1,
                      { Constraint({ all, "A", all }),
                        Constraint({ all, "A", "A" }),
                        Constraint({ all, "B", "B" }) });
    check_constraints(*regex, constraint, 2,
                      { Constraint({ all, all, "A" }) });
    check_constraints(*regex, constraint, 3, {});
}

TEST_F(RegexConstrainTest, plus_3)
{
    const auto regex = Regex::parse("(AA|B)+");

    const auto all = set_alphabet(*regex);

    const size_t constraint_size = 3;
    const auto constraint = Constraint::all(constraint_size);

    check_constraints(*regex, constraint, 0,
                      { Constraint({ "B", all, all }),
                        Constraint({ "B", "B", all }),
                        Constraint({ "B", "B", "B" }),
                        Constraint({ "B", "A", "A" }),
                        Constraint({ "A", "A", "B" }),
                        Constraint({ "A", "A", all }) });
    check_constraints(*regex, constraint, 1,
                      { Constraint({ all, "B", all }),
                        Constraint({ all, "B", "B" }),
                        Constraint({ all, "A", "A" }) });
    check_constraints(*regex, constraint, 2,
                      { Constraint({ all, all, "B" }) });
    check_constraints(*regex, constraint, 3, {});
}

TEST_F(RegexConstrainTest, question_mark_1)
{
    const auto regex = Regex::parse("A?");

    const auto all = set_alphabet(*regex);

    const size_t constraint_size = 3;
    const auto constraint = Constraint::all(constraint_size);

    check_constraints(*regex, constraint, 0,
                      { Constraint({ all, all, all }),
                        Constraint({ "A", all, all }) });
    check_constraints(*regex, constraint, 1,
                      { Constraint({ all, all, all }),
                        Constraint({ all, "A", all }) });
    check_constraints(*regex, constraint, 2,
                      { Constraint({ all, all, all }),
                        Constraint({ all, all, "A" }) });
    check_constraints(*regex, constraint, 3,
                      { Constraint({ all, all, all }) });
}

TEST_F(RegexConstrainTest, question_mark_2)
{
    const auto regex = Regex::parse("(A|BB)?");

    const auto all = set_alphabet(*regex);

    const size_t constraint_size = 3;
    const auto constraint = Constraint::all(constraint_size);

    check_constraints(*regex, constraint, 0,
                      { Constraint({ all, all, all }),
                        Constraint({ "A", all, all }),
                        Constraint({ "B", "B", all }) });
    check_constraints(*regex, constraint, 1,
                      { Constraint({ all, all, all }),
                        Constraint({ all, "A", all }),
                        Constraint({ all, "B", "B" }) });
    check_constraints(*regex, constraint, 2,
                      { Constraint({ all, all, all }),
                        Constraint({ all, all, "A" }) });
    check_constraints(*regex, constraint, 3,
                      { Constraint({ all, all, all }) });
}

TEST_F(RegexConstrainTest, fixed_repetition_1)
{
    const auto regex = Regex::parse("AA{2}");

    const auto all = set_alphabet(*regex);

    const size_t constraint_size = 3;
    const auto constraint = Constraint::all(constraint_size);

    check_constraints(*regex, constraint, 0,
                      { Constraint({ "A", "A", "A" }) });

    for (size_t begin_pos = 1; begin_pos <= constraint_size; ++begin_pos)
    {
        check_constraints(*regex, constraint, begin_pos, {});
    }
}

TEST_F(RegexConstrainTest, fixed_repetition_2)
{
    const auto regex = Regex::parse("A{0}");

    const auto all = set_alphabet(*regex);

    const size_t constraint_size = 1;
    const auto constraint = Constraint::all(constraint_size);

    for (size_t begin_pos = 0; begin_pos <= constraint_size; ++begin_pos)
    {
        check_constraints(*regex, constraint, begin_pos,
                          { Constraint({ all }) });
    }
}

TEST_F(RegexConstrainTest, fixed_repetition_3)
{
    const auto regex = Regex::parse("(AA|B){2}");

    const auto all = set_alphabet(*regex);

    const size_t constraint_size = 2;
    const auto constraint = Constraint::all(constraint_size);

    check_constraints(*regex, constraint, 0, { Constraint({ "B", "B" }) });

    for (size_t begin_pos = 1; begin_pos <= constraint_size; ++begin_pos)
    {
        check_constraints(*regex, constraint, begin_pos, {});
    }
}

TEST_F(RegexConstrainTest, range_repetition_1)
{
    const auto regex = Regex::parse("(A|B){2,}");

    const auto all = set_alphabet(*regex);

    const size_t constraint_size = 2;
    const auto constraint = Constraint::all(constraint_size);

    check_constraints(*regex, constraint, 0,
                      { Constraint({ "A", "A" }),
                        Constraint({ "A", "B" }),
                        Constraint({ "B", "A" }),
                        Constraint({ "B", "B" }) });

    for (size_t begin_pos = 1; begin_pos <= constraint_size; ++begin_pos)
    {
        check_constraints(*regex, constraint, begin_pos, {});
    }
}

TEST_F(RegexConstrainTest, range_repetition_2)
{
    const auto regex = Regex::parse("(A|B){3,}");

    const auto all = set_alphabet(*regex);

    const size_t constraint_size = 1;
    const auto constraint = Constraint::all(constraint_size);

    for (size_t begin_pos = 0; begin_pos <= constraint_size; ++begin_pos)
    {
        check_constraints(*regex, constraint, begin_pos, {});
    }
}

TEST_F(RegexConstrainTest, backreference_01)
{
    const auto regex = Regex::parse(R"((A)\1)");

    const auto all = set_alphabet(*regex);

    const size_t constraint_size = 3;
    const auto constraint = Constraint::all(constraint_size);

    check_constraints(*regex, constraint, 0, { Constraint({ "A", "A", all }) });
    check_constraints(*regex, constraint, 1, { Constraint({ all, "A", "A" }) });

    for (size_t begin_pos = 2; begin_pos <= constraint_size; ++begin_pos)
    {
        check_constraints(*regex, constraint, begin_pos, {});
    }
}

TEST_F(RegexConstrainTest, backreference_02)
{
    const auto regex = Regex::parse(R"((A)(?:)\1)");

    const auto all = set_alphabet(*regex);

    const size_t constraint_size = 3;
    const auto constraint = Constraint::all(constraint_size);

    check_constraints(*regex, constraint, 0, { Constraint({ "A", "A", all }) });
    check_constraints(*regex, constraint, 1, { Constraint({ all, "A", "A" }) });

    for (size_t begin_pos = 2; begin_pos <= constraint_size; ++begin_pos)
    {
        check_constraints(*regex, constraint, begin_pos, {});
    }
}

TEST_F(RegexConstrainTest, backreference_03)
{
    const auto regex = Regex::parse(R"(()\1)");

    const auto all = set_alphabet(*regex);

    const size_t constraint_size = 1;
    const auto constraint = Constraint::all(constraint_size);

    check_constraints(*regex, constraint, 0, { Constraint({ all }) });
    check_constraints(*regex, constraint, 1, { Constraint({ all }) });
}

TEST_F(RegexConstrainTest, backreference_04)
{
    const auto regex = Regex::parse(R"((A)*B\1)");

    const auto all = set_alphabet(*regex);

    const size_t constraint_size = 4;
    const auto constraint = Constraint::all(constraint_size);

    check_constraints(*regex, constraint, 0,
                      { Constraint({ "A", "B", "A", all }),
                        Constraint({ "A", "A", "B", "A" }) });
    check_constraints(*regex, constraint, 1,
                      { Constraint({ all, "A", "B", "A" }) });

    for (size_t begin_pos = 2; begin_pos <= constraint_size; ++begin_pos)
    {
        check_constraints(*regex, constraint, begin_pos, {});
    }
}

TEST_F(RegexConstrainTest, backreference_05)
{
    const auto regex = Regex::parse(R"((A)?\1)");

    const auto all = set_alphabet(*regex);

    const size_t constraint_size = 3;
    const auto constraint = Constraint::all(constraint_size);

    check_constraints(*regex, constraint, 0,
                      { Constraint({ "A", "A", all }) });
    check_constraints(*regex, constraint, 1,
                      { Constraint({ all, "A", "A" }) });

    for (size_t begin_pos = 2; begin_pos <= constraint_size; ++begin_pos)
    {
        check_constraints(*regex, constraint, begin_pos, {});
    }
}

TEST_F(RegexConstrainTest, backreference_06)
{
    const auto regex = Regex::parse(R"((A)(B)\2\1)");

    const auto all = set_alphabet(*regex);

    const size_t constraint_size = 4;
    const auto constraint = Constraint::all(constraint_size);

    check_constraints(*regex, constraint, 0,
                      { Constraint({ "A", "B", "B", "A" }) });

    for (size_t begin_pos = 1; begin_pos <= constraint_size; ++begin_pos)
    {
        check_constraints(*regex, constraint, begin_pos, {});
    }
}

TEST_F(RegexConstrainTest, backreference_07)
{
    const auto regex = Regex::parse(R"((A)(B)\1\2)");

    const auto all = set_alphabet(*regex);

    const size_t constraint_size = 4;
    const auto constraint = Constraint::all(constraint_size);

    check_constraints(*regex, constraint, 0,
                      { Constraint({ "A", "B", "A", "B" }) });

    for (size_t begin_pos = 1; begin_pos <= constraint_size; ++begin_pos)
    {
        check_constraints(*regex, constraint, begin_pos, {});
    }
}

TEST_F(RegexConstrainTest, backreference_08)
{
    const auto regex = Regex::parse(R"((.)(.)\2\1)");

    const auto all = set_alphabet(*regex);

    const size_t constraint_size = 4;
    const auto constraint = Constraint::all(constraint_size);

    check_constraints(*regex, constraint, 0,
                      { Constraint({ all, all, all, all }) });

    for (size_t begin_pos = 1; begin_pos <= constraint_size; ++begin_pos)
    {
        check_constraints(*regex, constraint, begin_pos, {});
    }
}

TEST_F(RegexConstrainTest, backreference_09)
{
    const auto regex = Regex::parse(R"((.)(.)(.)\3\2\1)");

    const auto all = set_alphabet(*regex);

    const size_t constraint_size = 6;
    const auto constraint = Constraint::all(constraint_size);

    check_constraints(*regex, constraint, 0,
                      { Constraint({ all, all, all, all, all, all }) });

    for (size_t begin_pos = 1; begin_pos <= constraint_size; ++begin_pos)
    {
        check_constraints(*regex, constraint, begin_pos, {});
    }
}

TEST_F(RegexConstrainTest, backreference_10)
{
    const auto regex = Regex::parse(R"((.)(.)(.)\1\2\3)");

    const auto all = set_alphabet(*regex);

    const size_t constraint_size = 6;
    const auto constraint = Constraint::all(constraint_size);

    check_constraints(*regex, constraint, 0,
                      { Constraint({ all, all, all, all, all, all }) });

    for (size_t begin_pos = 1; begin_pos <= constraint_size; ++begin_pos)
    {
        check_constraints(*regex, constraint, begin_pos, {});
    }
}

TEST_F(RegexConstrainTest, backreference_11)
{
    const auto regex = Regex::parse(R"(((B)*|C)(A|\2)(D|E\2))");

    const auto all = set_alphabet(*regex);

    const size_t constraint_size = 4;
    const auto constraint = Constraint::all(constraint_size);

    check_constraints(*regex, constraint, 0,
                      { Constraint({ "A", "D", all, all }),
                        Constraint({ "B", "A", "D", all }),
                        Constraint({ "B", "A", "E", "B" }),
                        Constraint({ "B", "B", "D", all }),
                        Constraint({ "B", "B", "E", "B" }),
                        Constraint({ "B", "B", "A", "D" }),
                        Constraint({ "B", "B", "B", "D" }),
                        Constraint({ "C", "A", "D", all }) });
    check_constraints(*regex, constraint, 1,
                      { Constraint({ all, "A", "D", all }),
                        Constraint({ all, "B", "A", "D" }),
                        Constraint({ all, "B", "B", "D" }),
                        Constraint({ all, "C", "A", "D" }) });
    check_constraints(*regex, constraint, 2,
                      { Constraint({ all, all, "A", "D" }) });

    for (size_t begin_pos = 3; begin_pos <= constraint_size; ++begin_pos)
    {
        check_constraints(*regex, constraint, begin_pos, {});
    }
}

TEST_F(RegexConstrainTest, backreference_12)
{
    const auto regex = Regex::parse(R"((A)\1*)");

    const auto all = set_alphabet(*regex);

    const size_t constraint_size = 2;
    const auto constraint = Constraint::all(constraint_size);

    check_constraints(*regex, constraint, 0,
                      { Constraint({ "A", all }),
                        Constraint({ "A", "A" }) });
    check_constraints(*regex, constraint, 1,
                      { Constraint({ all, "A" }) });
    check_constraints(*regex, constraint, 2, {});
}

TEST_F(RegexConstrainTest, backreference_13)
{
    const auto regex = Regex::parse(R"(A+(..)\1*)");

    const auto all = set_alphabet(*regex);

    const size_t constraint_size = 6;
    const auto constraint = Constraint::all(constraint_size);

    check_constraints(*regex, constraint, 0,
                      { Constraint({ "A", all, all, all, all, all }),
                        Constraint({ "A", all, all, all, all, all }),
                        Constraint({ "A", "A", all, all, all, all }),
                        Constraint({ "A", "A", all, all, all, all }),
                        Constraint({ "A", "A", "A", all, all, all }),
                        Constraint({ "A", "A", "A", "A", all, all }) });
    check_constraints(*regex, constraint, 1,
                      { Constraint({ all, "A", all, all, all, all }),
                        Constraint({ all, "A", all, all, all, all }),
                        Constraint({ all, "A", "A", all, all, all }),
                        Constraint({ all, "A", "A", "A", all, all }) });
    check_constraints(*regex, constraint, 2,
                      { Constraint({ all, all, "A", all, all, all }),
                        Constraint({ all, all, "A", "A", all, all }) });
    check_constraints(*regex, constraint, 3,
                      { Constraint({ all, all, all, "A", all, all }) });

    for (size_t begin_pos = 4; begin_pos <= constraint_size; ++begin_pos)
    {
        check_constraints(*regex, constraint, begin_pos, {});
    }
}

TEST_F(RegexConstrainTest, backreference_14)
{
    const auto regex = Regex::parse(R"((A|B)\1)");

    const auto all = set_alphabet(*regex);

    const size_t constraint_size = 2;
    const auto constraint = Constraint::all(constraint_size);

    check_constraints(*regex, constraint, 0, { Constraint({ "A", "A" }),
                                               Constraint({ "B", "B" }) });

    for (size_t begin_pos = 1; begin_pos <= constraint_size; ++begin_pos)
    {
        check_constraints(*regex, constraint, begin_pos, {});
    }
}

TEST_F(RegexConstrainTest, backreference_15)
{
    const auto regex = Regex::parse(R"((A|B)(A|B)\1\2)");

    const auto all = set_alphabet(*regex);

    const size_t constraint_size = 4;
    const auto constraint = Constraint::all(constraint_size);

    check_constraints(*regex, constraint, 0,
                      { Constraint({ "A", "A", "A", "A" }),
                        Constraint({ "A", "B", "A", "B" }),
                        Constraint({ "B", "A", "B", "A" }),
                        Constraint({ "B", "B", "B", "B" }) });

    for (size_t begin_pos = 1; begin_pos <= constraint_size; ++begin_pos)
    {
        check_constraints(*regex, constraint, begin_pos, {});
    }
}

TEST_F(RegexConstrainTest, backreference_16)
{
    const auto regex = Regex::parse(R"((A)|B\1)");

    const auto all = set_alphabet(*regex);

    const size_t constraint_size = 3;
    const auto constraint = Constraint::all(constraint_size);

    check_constraints(*regex, constraint, 0,
                      { Constraint({ "A", all, all }) });
    check_constraints(*regex, constraint, 1,
                      { Constraint({ all, "A", all }) });
    check_constraints(*regex, constraint, 2,
                      { Constraint({ all, all, "A" }) });
}

TEST_F(RegexConstrainTest, backreference_17)
{
    const auto regex = Regex::parse(R"((A)(B)|(\1|\2))");

    const auto all = set_alphabet(*regex);

    const size_t constraint_size = 3;
    const auto constraint = Constraint::all(constraint_size);

    check_constraints(*regex, constraint, 0,
                      { Constraint({ "A", "B", all }) });
    check_constraints(*regex, constraint, 1,
                      { Constraint({ all, "A", "B" }) });

    for (size_t begin_pos = 2; begin_pos <= constraint_size; ++begin_pos)
    {
        check_constraints(*regex, constraint, begin_pos, {});
    }
}

TEST_F(RegexConstrainTest, backreference_18)
{
    const auto regex = Regex::parse(R"((((A)(B))|\4)\3)");

    const auto all = set_alphabet(*regex);

    const size_t constraint_size = 3;
    const auto constraint = Constraint::all(constraint_size);

    check_constraints(*regex, constraint, 0,
                      { Constraint({ "A", "B", "A" }) });

    for (size_t begin_pos = 1; begin_pos <= constraint_size; ++begin_pos)
    {
        check_constraints(*regex, constraint, begin_pos, {});
    }
}

TEST_F(RegexConstrainTest, backreference_19)
{
    const auto regex = Regex::parse(R"(((A)|(B))(\2|\3))");

    const auto all = set_alphabet(*regex);

    const size_t constraint_size = 2;
    const auto constraint = Constraint::all(constraint_size);

    check_constraints(*regex, constraint, 0, { Constraint({ "A", "A" }),
                                               Constraint({ "B", "B" }) });

    for (size_t begin_pos = 1; begin_pos <= constraint_size; ++begin_pos)
    {
        check_constraints(*regex, constraint, begin_pos, {});
    }
}

TEST_F(RegexConstrainTest, backreference_20)
{
    const auto regex = Regex::parse(R"(((A)|(B))(\2\3))");

    const auto all = set_alphabet(*regex);

    const size_t constraint_size = 3;
    const auto constraint = Constraint::all(constraint_size);

    for (size_t begin_pos = 0; begin_pos <= constraint_size; ++begin_pos)
    {
        check_constraints(*regex, constraint, begin_pos, {});
    }
}

TEST_F(RegexConstrainTest, backreference_21)
{
    const auto regex = Regex::parse(R"(((A)|B)(C|\2))");

    const auto all = set_alphabet(*regex);

    const size_t constraint_size = 2;
    const auto constraint = Constraint::all(constraint_size);

    check_constraints(*regex, constraint, 0, { Constraint({ "A", "A" }),
                                               Constraint({ "A", "C" }),
                                               Constraint({ "B", "C" }) });

    for (size_t begin_pos = 1; begin_pos <= constraint_size; ++begin_pos)
    {
        check_constraints(*regex, constraint, begin_pos, {});
    }
}

TEST_F(RegexConstrainTest, backreference_22)
{
    const auto regex = Regex::parse(R"((A)(\1)\2)");

    const auto all = set_alphabet(*regex);

    const size_t constraint_size = 3;
    const auto constraint = Constraint::all(constraint_size);

    check_constraints(*regex, constraint, 0,
                      { Constraint({ "A", "A", "A" }) });

    for (size_t begin_pos = 1; begin_pos <= constraint_size; ++begin_pos)
    {
        check_constraints(*regex, constraint, begin_pos, {});
    }
}

TEST_F(RegexConstrainTest, backreference_23)
{
    const auto regex = Regex::parse(R"((A|B)(\1)\2)");

    const auto all = set_alphabet(*regex);

    const size_t constraint_size = 3;
    const auto constraint = Constraint::all(constraint_size);

    check_constraints(*regex, constraint, 0,
                      { Constraint({ "A", "A", "A" }),
                        Constraint({ "B", "B", "B" }) });

    for (size_t begin_pos = 1; begin_pos <= constraint_size; ++begin_pos)
    {
        check_constraints(*regex, constraint, begin_pos, {});
    }
}

TEST_F(RegexConstrainTest, backreference_24)
{
    const auto regex = Regex::parse(R"((A|B)(\1)(\2)\3)");

    const auto all = set_alphabet(*regex);

    const size_t constraint_size = 4;
    const auto constraint = Constraint::all(constraint_size);

    check_constraints(*regex, constraint, 0,
                      { Constraint({ "A", "A", "A", "A" }),
                        Constraint({ "B", "B", "B", "B" }) });

    for (size_t begin_pos = 1; begin_pos <= constraint_size; ++begin_pos)
    {
        check_constraints(*regex, constraint, begin_pos, {});
    }
}

TEST_F(RegexConstrainTest, backreference_25)
{
    const auto regex = Regex::parse(R"(((A|B))((\2))\4)");

    const auto all = set_alphabet(*regex);

    const size_t constraint_size = 3;
    const auto constraint = Constraint::all(constraint_size);

    check_constraints(*regex, constraint, 0,
                      { Constraint({ "A", "A", "A" }),
                        Constraint({ "B", "B", "B" }) });

    for (size_t begin_pos = 1; begin_pos <= constraint_size; ++begin_pos)
    {
        check_constraints(*regex, constraint, begin_pos, {});
    }
}

TEST_F(RegexConstrainTest, backreference_26)
{
    const auto regex = Regex::parse(R"((A|B)C(\1)(\1)C\2\3)");

    const auto all = set_alphabet(*regex);

    const size_t constraint_size = 7;
    const auto constraint = Constraint::all(constraint_size);

    check_constraints(*regex, constraint, 0,
                      { Constraint({ "A", "C", "A", "A", "C", "A", "A" }),
                        Constraint({ "B", "C", "B", "B", "C", "B", "B" }) });

    for (size_t begin_pos = 1; begin_pos <= constraint_size; ++begin_pos)
    {
        check_constraints(*regex, constraint, begin_pos, {});
    }
}

TEST_F(RegexConstrainTest, backreference_27)
{
    const auto regex = Regex::parse(R"((A|B)C((\1)(\1))C\3\4C\2)");

    const auto all = set_alphabet(*regex);

    const size_t constraint_size = 10;
    const auto constraint = Constraint::all(constraint_size);

    check_constraints(*regex, constraint, 0,
        { Constraint({ "A", "C", "A", "A", "C", "A", "A", "C", "A", "A" }),
          Constraint({ "B", "C", "B", "B", "C", "B", "B", "C", "B", "B" }) });

    for (size_t begin_pos = 1; begin_pos <= constraint_size; ++begin_pos)
    {
        check_constraints(*regex, constraint, begin_pos, {});
    }
}

TEST_F(RegexConstrainTest, backreference_28)
{
    const auto regex = Regex::parse(R"((A|B)C(\1\1)C\2)");

    const auto all = set_alphabet(*regex);

    const size_t constraint_size = 7;
    const auto constraint = Constraint::all(constraint_size);

    check_constraints(*regex, constraint, 0,
                      { Constraint({ "A", "C", "A", "A", "C", "A", "A" }),
                        Constraint({ "B", "C", "B", "B", "C", "B", "B" }) });

    for (size_t begin_pos = 1; begin_pos <= constraint_size; ++begin_pos)
    {
        check_constraints(*regex, constraint, begin_pos, {});
    }
}

TEST_F(RegexConstrainTest, backreference_29)
{
    const auto regex = Regex::parse(R"(((A|B)\2)*)");

    const auto all = set_alphabet(*regex);

    const size_t constraint_size = 4;
    const auto constraint = Constraint::all(constraint_size);

    check_constraints(*regex, constraint, 0,
                      { Constraint({ all, all, all, all }),
                        Constraint({ "A", "A", all, all }),
                        Constraint({ "B", "B", all, all }),
                        Constraint({ "A", "A", "A", "A" }),
                        Constraint({ "A", "A", "B", "B" }),
                        Constraint({ "B", "B", "A", "A" }),
                        Constraint({ "B", "B", "B", "B" }) });
    check_constraints(*regex, constraint, 1,
                      { Constraint({ all, all, all, all }),
                        Constraint({ all, "A", "A", all }),
                        Constraint({ all, "B", "B", all }) });
    check_constraints(*regex, constraint, 2,
                      { Constraint({ all, all, all, all }),
                        Constraint({ all, all, "A", "A" }),
                        Constraint({ all, all, "B", "B" }) });
    check_constraints(*regex, constraint, 3,
                      { Constraint({ all, all, all, all }) });
    check_constraints(*regex, constraint, 4,
                      { Constraint({ all, all, all, all }) });
}

TEST_F(RegexConstrainTest, backreference_30)
{
    const auto regex = Regex::parse(R"(([AB])\1)");

    const auto all = set_alphabet(*regex);

    const auto constraint = Constraint({ "A", "AB" });
    const auto constraint_size = constraint.size();

    check_constraints(*regex, constraint, 0, { Constraint({ "A", "A" }) });

    for (size_t begin_pos = 1; begin_pos <= constraint_size; ++begin_pos)
    {
        check_constraints(*regex, constraint, begin_pos, {});
    }
}

TEST_F(RegexConstrainTest, backreference_31)
{
    const auto regex = Regex::parse(R"(([AB])\1)");

    const auto all = set_alphabet(*regex);

    const auto constraint = Constraint({ "AB", "A" });
    const auto constraint_size = constraint.size();

    check_constraints(*regex, constraint, 0, { Constraint({ "A", "A" }) });

    for (size_t begin_pos = 1; begin_pos <= constraint_size; ++begin_pos)
    {
        check_constraints(*regex, constraint, begin_pos, {});
    }
}

TEST_F(RegexConstrainTest, backreference_32)
{
    const auto regex = Regex::parse(R"((..?)\1)");

    const auto all = "ABC";
    Alphabet::set(all);

    const auto constraint = Constraint({ "A", "B", "AB", "AB" });
    const auto constraint_size = constraint.size();

    check_constraints(*regex, constraint, 0,
                      { Constraint({ "A", "B",  "A",  "B" }) });
    check_constraints(*regex, constraint, 1,
                      { Constraint({ "A", "B", "B", "AB" }) });
    check_constraints(*regex, constraint, 2,
                      { Constraint({ "A", "B", "AB", "AB" }) });

    for (size_t begin_pos = 3; begin_pos <= constraint_size; ++begin_pos)
    {
        check_constraints(*regex, constraint, begin_pos, {});
    }
}

TEST_F(RegexConstrainTest, backreference_33)
{
    const auto regex = Regex::parse(R"((A|BB)\1)");

    const auto all = set_alphabet(*regex);

    const size_t constraint_size = 4;
    const auto constraint = Constraint::all(constraint_size);

    check_constraints(*regex, constraint, 0,
                      { Constraint({ "A", "A", all, all }),
                        Constraint({ "B", "B", "B", "B" }) });
    check_constraints(*regex, constraint, 1,
                      { Constraint({ all, "A", "A", all }) });
    check_constraints(*regex, constraint, 2,
                      { Constraint({ all, all, "A", "A" }) });

    for (size_t begin_pos = 3; begin_pos <= constraint_size; ++begin_pos)
    {
        check_constraints(*regex, constraint, begin_pos, {});
    }
}

TEST_F(RegexConstrainTest, backreference_34)
{
    // Excerpt from http://www.regular-expressions.info/backref.html,
    // section "Repetition and Backreferences":
    //
    //     As I mentioned in the above inside look, the regex engine
    //     does not permanently substitute backreferences in the regular
    //     expression. It will use the last match saved into the
    //     backreference each time it needs to be used. If a new match
    //     is found by capturing parentheses, the previously saved match
    //     is overwritten.
    //
    // This behavior is tested here.

    const auto regex = Regex::parse(R"((A|B)*C\1)");

    const auto all = set_alphabet(*regex);

    const size_t constraint_size = 4;
    const auto constraint = Constraint::all(constraint_size);

    check_constraints(*regex, constraint, 0,
                      { Constraint({ "A", "C", "A", all }),
                        Constraint({ "B", "C", "B", all }),
                        Constraint({ "A", "A", "C", "A" }),
                        Constraint({ "A", "B", "C", "B" }),
                        Constraint({ "B", "A", "C", "A" }),
                        Constraint({ "B", "B", "C", "B" }) });
    check_constraints(*regex, constraint, 1,
                      { Constraint({ all, "A", "C", "A" }),
                        Constraint({ all, "B", "C", "B" }) });

    for (size_t begin_pos = 2; begin_pos <= constraint_size; ++begin_pos)
    {
        check_constraints(*regex, constraint, begin_pos, {});
    }
}

TEST_F(RegexConstrainTest, backreference_35)
{
    const auto regex = Regex::parse(R"(((\bA)| \2){2})");

    const auto all = set_alphabet(*regex);

    const size_t constraint_size = 3;
    const auto constraint = Constraint::all(constraint_size);

    check_constraints(*regex, constraint, 0,
                      { Constraint({ "A", " ", "A" }) });

    for (size_t begin_pos = 1; begin_pos <= constraint_size; ++begin_pos)
    {
        check_constraints(*regex, constraint, begin_pos, {});
    }
}

TEST_F(RegexConstrainTest, caret_1)
{
    const auto regex = Regex::parse("^A");

    const auto all = set_alphabet(*regex);

    const size_t constraint_size = 2;
    const auto constraint = Constraint::all(constraint_size);

    check_constraints(*regex, constraint, 0, { Constraint({ "A", all }) });
    check_constraints(*regex, constraint, 1, {});
}

TEST_F(RegexConstrainTest, caret_2)
{
    const auto regex = Regex::parse("A^B");

    const auto all = set_alphabet(*regex);

    const size_t constraint_size = 3;
    const auto constraint = Constraint::all(constraint_size);

    for (size_t begin_pos = 0; begin_pos <= constraint_size; ++begin_pos)
    {
        check_constraints(*regex, constraint, begin_pos, {});
    }
}

TEST_F(RegexConstrainTest, caret_3)
{
    const auto regex = Regex::parse("A*^B");

    const auto all = set_alphabet(*regex);

    const size_t constraint_size = 2;
    const auto constraint = Constraint::all(constraint_size);

    check_constraints(*regex, constraint, 0, { Constraint({ "B", all }) });

    for (size_t begin_pos = 1; begin_pos <= constraint_size; ++begin_pos)
    {
        check_constraints(*regex, constraint, begin_pos, {});
    }
}

TEST_F(RegexConstrainTest, dollar_1)
{
    const auto regex = Regex::parse("A$");

    const auto all = set_alphabet(*regex);

    const size_t constraint_size = 2;
    const auto constraint = Constraint::all(constraint_size);

    check_constraints(*regex, constraint, 0, {});
    check_constraints(*regex, constraint, 1, { Constraint({ all, "A" }) });
}

TEST_F(RegexConstrainTest, dollar_2)
{
    const auto regex = Regex::parse("A$B");

    const auto all = set_alphabet(*regex);

    const size_t constraint_size = 3;
    const auto constraint = Constraint::all(constraint_size);

    for (size_t begin_pos = 0; begin_pos <= constraint_size; ++begin_pos)
    {
        check_constraints(*regex, constraint, begin_pos, {});
    }
}

TEST_F(RegexConstrainTest, dollar_3)
{
    const auto regex = Regex::parse("A$B*");

    const auto all = set_alphabet(*regex);

    const size_t constraint_size = 2;
    const auto constraint = Constraint::all(constraint_size);

    check_constraints(*regex, constraint, 0, {});
    check_constraints(*regex, constraint, 1, { Constraint({ all, "A" }) });
    check_constraints(*regex, constraint, 2, {});
}

TEST_F(RegexConstrainTest, word_boundaries_01)
{
    const auto regex = Regex::parse(R"(\b[A=])");

    const auto all = "AB=&";
    Alphabet::set(all);

    const size_t constraint_size = 2;
    const auto constraint = Constraint::all(constraint_size);

    check_constraints(*regex, constraint, 0, { Constraint({ "A", all  }) });
    check_constraints(*regex, constraint, 1, { Constraint({ all, "A=" }) });
}

TEST_F(RegexConstrainTest, word_boundaries_02)
{
    const auto regex = Regex::parse(R"(\B[A=])");

    const auto all = "AB=&";
    Alphabet::set(all);

    const size_t constraint_size = 2;
    const auto constraint = Constraint::all(constraint_size);

    check_constraints(*regex, constraint, 0, { Constraint({ "=", all  }) });
    check_constraints(*regex, constraint, 1, { Constraint({ all, "A=" }) });
}

TEST_F(RegexConstrainTest, word_boundaries_03)
{
    const auto regex = Regex::parse(R"([A=]\b)");

    const auto all = "AB=&";
    Alphabet::set(all);

    const size_t constraint_size = 2;
    const auto constraint = Constraint::all(constraint_size);

    check_constraints(*regex, constraint, 0, { Constraint({ "A=", all }) });
    check_constraints(*regex, constraint, 1, { Constraint({ all,  "A" }) });
}

TEST_F(RegexConstrainTest, word_boundaries_04)
{
    const auto regex = Regex::parse(R"([A=]\B)");

    const auto all = "AB=&";
    Alphabet::set(all);

    const size_t constraint_size = 2;
    const auto constraint = Constraint::all(constraint_size);

    check_constraints(*regex, constraint, 0, { Constraint({ "A=", all }) });
    check_constraints(*regex, constraint, 1, { Constraint({ all,  "=" }) });
}

TEST_F(RegexConstrainTest, word_boundaries_05)
{
    const auto regex = Regex::parse(R"([=&]\B)");

    const auto all = "AB=&";
    Alphabet::set(all);

    const size_t constraint_size = 2;
    const auto constraint = Constraint::all(constraint_size);

    check_constraints(*regex, constraint, 0, { Constraint({ "=&", "=&" }) });
    check_constraints(*regex, constraint, 1, { Constraint({ all,  "=&" }) });
}

TEST_F(RegexConstrainTest, word_boundaries_06)
{
    const auto regex = Regex::parse(R"([=&]\b)");

    const auto all = "AB=&";
    Alphabet::set(all);

    const size_t constraint_size = 2;
    const auto constraint = Constraint::all(constraint_size);

    check_constraints(*regex, constraint, 0, { Constraint({ "=&", "AB" }) });
    check_constraints(*regex, constraint, 1, {});
}

TEST_F(RegexConstrainTest, word_boundaries_07)
{
    const auto regex = Regex::parse(R"([AB]\b)");

    const auto all = "AB=&";
    Alphabet::set(all);

    const size_t constraint_size = 2;
    const auto constraint = Constraint::all(constraint_size);

    check_constraints(*regex, constraint, 0, { Constraint({ "AB", "=&" }) });
    check_constraints(*regex, constraint, 1, { Constraint({ all,  "AB" }) });
}

TEST_F(RegexConstrainTest, word_boundaries_08)
{
    const auto regex = Regex::parse(R"([AB]\B)");

    const auto all = "AB=&";
    Alphabet::set(all);

    const size_t constraint_size = 2;
    const auto constraint = Constraint::all(constraint_size);

    check_constraints(*regex, constraint, 0, { Constraint({ "AB", "AB" }) });
    check_constraints(*regex, constraint, 1, {});
}

TEST_F(RegexConstrainTest, word_boundaries_09)
{
    const auto regex = Regex::parse(R"(\b[AB])");

    const auto all = "AB=&";
    Alphabet::set(all);

    const size_t constraint_size = 2;
    const auto constraint = Constraint::all(constraint_size);

    check_constraints(*regex, constraint, 0, { Constraint({ "AB", all  }) });
    check_constraints(*regex, constraint, 1, { Constraint({ "=&", "AB" }) });
}

TEST_F(RegexConstrainTest, word_boundaries_10)
{
    const auto regex = Regex::parse(R"(\B[AB])");

    const auto all = "AB=&";
    Alphabet::set(all);

    const size_t constraint_size = 2;
    const auto constraint = Constraint::all(constraint_size);

    check_constraints(*regex, constraint, 0, {});
    check_constraints(*regex, constraint, 1, { Constraint({ "AB", "AB" }) });
}

TEST_F(RegexConstrainTest, word_boundaries_11)
{
    const auto regex = Regex::parse(R"(\b[=&])");

    const auto all = "AB=&";
    Alphabet::set(all);

    const size_t constraint_size = 2;
    const auto constraint = Constraint::all(constraint_size);

    check_constraints(*regex, constraint, 0, {});
    check_constraints(*regex, constraint, 1, { Constraint({ "AB", "=&" }) });
}

TEST_F(RegexConstrainTest, word_boundaries_12)
{
    const auto regex = Regex::parse(R"(\B[=&])");

    const auto all = "AB=&";
    Alphabet::set(all);

    const size_t constraint_size = 2;
    const auto constraint = Constraint::all(constraint_size);

    check_constraints(*regex, constraint, 0, { Constraint({ "=&", all  }) });
    check_constraints(*regex, constraint, 1, { Constraint({ "=&", "=&" }) });
}

TEST_F(RegexConstrainTest, word_boundaries_13)
{
    const auto regex = Regex::parse(R"(\b)");

    const auto all = "AB=&";
    Alphabet::set(all);

    const size_t constraint_size = 0;
    const auto constraint = Constraint::all(constraint_size);

    check_constraints(*regex, constraint, 0, {});
}

TEST_F(RegexConstrainTest, word_boundaries_14)
{
    const auto regex = Regex::parse(R"(\B)");

    const auto all = "AB=&";
    Alphabet::set(all);

    const size_t constraint_size = 0;
    const auto constraint = Constraint::all(constraint_size);

    check_constraints(*regex, constraint, 0, {});
}

TEST_F(RegexConstrainTest, positive_lookahead_1)
{
    const auto regex = Regex::parse("(?=A)");

    const auto all = set_alphabet(*regex);

    const size_t constraint_size = 2;
    const auto constraint = Constraint::all(constraint_size);

    check_constraints(*regex, constraint, 0, { Constraint({ "A", all }) });
    check_constraints(*regex, constraint, 1, { Constraint({ all, "A" }) });
    check_constraints(*regex, constraint, 2, {});
}

TEST_F(RegexConstrainTest, positive_lookahead_2)
{
    const auto regex = Regex::parse("(?=A|BC)");

    const auto all = set_alphabet(*regex);

    const size_t constraint_size = 3;
    const auto constraint = Constraint::all(constraint_size);

    check_constraints(*regex, constraint, 0,
                      { Constraint({ "AB", all, all }) });
    check_constraints(*regex, constraint, 1,
                      { Constraint({ all, "AB", all }) });
    check_constraints(*regex, constraint, 2,
                      { Constraint({ all, all, "A" }) });
    check_constraints(*regex, constraint, 3, {});
}

TEST_F(RegexConstrainTest, positive_lookahead_3)
{
    const auto regex = Regex::parse(R"((A(?=B)).\1)");

    const auto all = set_alphabet(*regex);

    const size_t constraint_size = 3;
    const auto constraint = Constraint::all(constraint_size);

    check_constraints(*regex, constraint, 0,
                      { Constraint({ "A", "B", "A" }) });

    for (size_t begin_pos = 1; begin_pos <= constraint_size; ++begin_pos)
    {
        check_constraints(*regex, constraint, begin_pos, {});
    }
}

TEST_F(RegexConstrainTest, positive_lookahead_4)
{
    const auto regex = Regex::parse("(?=A)(?=B)[ABC]");

    const auto all = set_alphabet(*regex);

    const size_t constraint_size = 1;
    const auto constraint = Constraint::all(constraint_size);

    for (size_t begin_pos = 0; begin_pos <= constraint_size; ++begin_pos)
    {
        check_constraints(*regex, constraint, 0, {});
    }
}

TEST_F(RegexConstrainTest, other_1)
{
    // This regex generates the following constraints of length 3:
    //   AEF, BCD
    const auto regex = Regex::parse("(A|BC)(D|EF)");

    set_alphabet(*regex);

    // The regex generates from this constraint the following possible
    // constraints:
    //   AEF
    const Constraint constraint({ "AB", "E", "DF" });

    const auto updated_constraint = regex->constrain(constraint);

    // When ORing the possible constraints above, we get:
    EXPECT_EQ(Constraint({ "A", "E", "F" }), updated_constraint);
}

TEST_F(RegexConstrainTest, other_2)
{
    // This regex generates the following constraints of length 3:
    //   [AB][AB][AB], [AB][AB]D, [AB]BC, [AB]DD, BC[AB], BCD, DDD
    const auto regex = Regex::parse("([AB]|BC)*D*");

    set_alphabet(*regex);

    // The regex generates from this constraint the following possible
    // constraints:
    //   [AB]B[AB], [AB]BD, [AB]BC
    const Constraint constraint({ "ABCD", "B", "ABCD" });

    const auto updated_constraint = regex->constrain(constraint);

    // When ORing the possible constraints above, we get:
    EXPECT_EQ(Constraint({ "AB", "B", "ABCD" }), updated_constraint);
}
