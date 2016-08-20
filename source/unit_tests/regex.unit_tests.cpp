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
#include "disable_warnings_from_gtest.hpp"
#include "regex.hpp"
#include "regex_crossword_solver_test.hpp"
#include "regex_optimizations.hpp"

using namespace std;


class RegexTest : public RegexCrosswordSolverTest
{
};


TEST_F(RegexTest, clone)
{
    {
        const EmptyRegex empty;
        const auto empty_copy(empty.clone());
        EXPECT_EQ(empty.to_string(), empty_copy->to_string());
    }

    {
        const auto plus = Regex::parse("A+");
        const auto plus_copy(plus->clone());
        EXPECT_EQ(plus->to_string(), plus_copy->to_string());
    }
}

TEST_F(RegexTest, explicit_characters)
{
    {
        const EmptyRegex empty;
        EXPECT_EQ("", empty.explicit_characters());
    }

    {
        const EpsilonRegex epsilon;
        EXPECT_EQ("", epsilon.explicit_characters());
    }

    {
        const auto regex = Regex::parse(R"(A([CE]+)\1[^EG]*)");
        Alphabet::set(regex->explicit_characters());
        EXPECT_EQ("ACEG", Alphabet::characters_as_string());
    }
}

TEST_F(RegexTest, optimize_groups)
{
    // for each pair of strings:
    // * first  = non optimized regex
    // * second = optimized regex
    const vector<pair<string, string>> regex_pairs =
        { { R"()",        R"()"       },
          { R"(A)",       R"(A)"      },
          { R"((A))",     R"(A)"      },
          { R"((A)\1)",   R"((A)\1)"  },
          { R"(((A))\1)", R"((A)\1)"  },
          { R"(((A))\2)", R"((A)\2)"  },
          { R"(A*)",      R"(A*)"     },
          { R"((A)*)",    R"(A*)"     },
          { R"((A)*\1)",  R"((A)*\1)" },
          { R"(AB)",      R"(AB)"     },
          { R"((A)B)",    R"(AB)"     },
          { R"(A(B))",    R"(AB)"     },
          { R"((A)B\1)",  R"((A)B\1)" },
          { R"(A(B)\1)",  R"(A(B)\1)" } };

    for (const auto& regex_pair : regex_pairs)
    {
        const auto& non_optimized_regex_as_string = regex_pair.first;
        const auto& optimized_regex_as_string     = regex_pair.second;

        auto regex = Regex::parse(non_optimized_regex_as_string);

        auto optimizations = RegexOptimizations::none();
        optimizations.set(RegexOptimizations::Type::GROUPS, true);
        regex = Regex::optimize(move(regex), optimizations);

        EXPECT_EQ(optimized_regex_as_string, regex->to_string());
    }
}

TEST_F(RegexTest, optimize_concatenations)
{
    // for each pair of strings:
    // * first  = non optimized regex
    // * second = optimized regex
    const vector<pair<string, string>> regex_pairs =
        { { R"()",         R"()"          },
          { R"(A)",        R"(A)"         },
          { R"(AB)",       R"("AB")"      },
          { R"((AB)*)",    R"("AB"*)"     },
          { R"(A*B)",      R"(A*B)"       },
          { R"(AB*)",      R"(AB*)"       },
          { R"(ABC)",      R"("ABC")"     },
          { R"((AB)C)",    R"("ABC")"     },
          { R"(ABC*)",     R"("AB"C*)"    },
          { R"((A*B)C)",   R"(A*"BC")"    },
          { R"((A*B)CD*)", R"(A*"BC"D*)"  },
          { R"((AB*C)D)",  R"(AB*"CD")"   },
          { R"(A(BC*)D)",  R"("AB"C*D)"   },
          { R"(AB|CD)",    R"("AB"|"CD")" }
        };

    for (const auto& regex_pair : regex_pairs)
    {
        const auto& non_optimized_regex_as_string = regex_pair.first;
        const auto& optimized_regex_as_string     = regex_pair.second;

        auto regex = Regex::parse(non_optimized_regex_as_string);

        auto optimizations = RegexOptimizations::none();
        optimizations.set(RegexOptimizations::Type::CONCATENATIONS, true);
        optimizations.set(RegexOptimizations::Type::GROUPS, true);
        regex = Regex::optimize(move(regex), optimizations);

        EXPECT_EQ(optimized_regex_as_string, regex->to_string());
    }
}

TEST_F(RegexTest, optimize_unions)
{
    // for each pair of strings:
    // * first  = non optimized regex
    // * second = optimized regex
    const vector<pair<string, string>> regex_pairs =
        { { R"()",            R"()"           },
          { R"(A)",           R"(A)"          },
          { R"(A|B)",         R"({AB})"       },
          { R"((A|B)*)",      R"({AB}*)"      },
          { R"(A*B)",         R"(A*B)"        },
          { R"(AB*)",         R"(AB*)"        },
          { R"(A|B|C)",       R"({ABC})"      },
          { R"((A|B)|C)",     R"({ABC})"      },
          { R"(A|B|C*)",      R"({AB}|C*)"    },
          { R"((A*|B)|C)",    R"(A*|{BC})"    },
          { R"((A*|B)|C|D*)", R"(A*|{BC}|D*)" },
          { R"((A|B*|C)|D)",  R"(B*|{ACD})"   },
          { R"(A|(B|C*)|D)",  R"({ABD}|C*)"   },
          { R"((A|B)(C|D))",  R"({AB}{CD})"   } };

    for (const auto& regex_pair : regex_pairs)
    {
        const auto& non_optimized_regex_as_string = regex_pair.first;
        const auto& optimized_regex_as_string     = regex_pair.second;

        auto regex = Regex::parse(non_optimized_regex_as_string);

        auto optimizations = RegexOptimizations::none();
        optimizations.set(RegexOptimizations::Type::GROUPS, true);
        optimizations.set(RegexOptimizations::Type::UNIONS, true);
        regex = Regex::optimize(move(regex), optimizations);

        EXPECT_EQ(optimized_regex_as_string, regex->to_string());
    }
}
