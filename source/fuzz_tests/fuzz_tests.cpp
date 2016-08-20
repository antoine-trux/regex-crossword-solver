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


// This program fuzz-tests critical parts of the Regex Crossword
// Solver, by repeatedly doing the following:
//
// 1. Generate a semi-random string.
// 2. Parse the string as a regex.
// 3. Optimize the regex.
// 4. Constrain the original regex and the optimized regex, and check
//    that the results are identical.
//
// The program goes on until it finds an error, or reaches a limit set
// on the command line.
//
// Usage:
//
//     regex_crossword_solver_fuzz_tests --help


#include "alphabet.hpp"
#include "constraint.hpp"
#include "regex.hpp"
#include "regex_crossword_solver_exception.hpp"
#include "regex_optimizations.hpp"
#include "utils.hpp"

#include <algorithm>
#include <cassert>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <random>

using namespace std;


namespace
{

default_random_engine g_random_engine;

string g_program_path;
bool g_is_help_requested = false;
bool g_randomize = false;
bool g_run_infinitely = true;
unsigned long long int g_num_tests_to_run = 0;

unsigned long long int g_num_regexes_which_cannot_be_parsed = 0;
unsigned long long int g_num_regexes_with_bad_structure = 0;
unsigned long long int g_num_good_regexes_not_constrained = 0;
unsigned long long int g_num_good_regexes_constrained = 0;

constexpr auto g_regex_filename = "fuzz_test_regex.txt";

void   check_constraints(Regex& regex, const string& regex_as_string);
Constraint combine(const vector<Constraint>& constraints,
                   size_t                    constraint_size);
void   handle_command_line_option_help();
void   handle_command_line_option_num_tests(const string& option);
void   handle_command_line_option_randomize();
int    num_digits(unsigned long long int n);
void   parse_command_line(int argc, const char* const* argv);
double percentage(unsigned long long int num_regexes);
void   print_statistics();
void   print_usage();
char   random_char();
size_t random_length();
string random_string();
double ratio(unsigned long long int num_regexes);
void   remove_regex_string_from_disk();
void   save_regex_string_to_disk(const string& s);
bool   starts_with(const string& s, const string& prefix);
void   test_string(const string& s);
unsigned int time_seed();
unsigned long long int total_num_regexes();
bool   would_take_too_long_to_constrain(const string& s);

void
check_constraints(Regex& regex, const string& regex_as_string)
{
    const auto explicit_characters = regex.explicit_characters();
    if (explicit_characters.empty())
    {
        return;
    }

    Alphabet::reset();
    Alphabet::set(explicit_characters);

    auto optimized_regex = regex.clone();
    optimized_regex = Regex::optimize(move(optimized_regex),
                                      RegexOptimizations::all());

    const size_t begin_pos = 0;

    const auto constraint = Constraint::all(regex_as_string.size());

    // regex.constrain.unit_tests.cc explains why we combine the
    // resulting constraints.

    const auto constraints = regex.constraints(constraint, begin_pos);
    const auto combined_constraints =
        combine(constraints, constraint.size());

    const auto constraints_optimized =
        optimized_regex->constraints(constraint, begin_pos);
    const auto combined_constraints_optimized =
        combine(constraints_optimized, constraint.size());

    if (combined_constraints != combined_constraints_optimized)
    {
        cerr << "different constraints for "
             << Utils::quoted(regex_as_string)
             << " without and with optimizations" << endl;
        exit(EXIT_FAILURE);
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

void
handle_command_line_option_help()
{
    g_is_help_requested = true;
}

void
handle_command_line_option_num_tests(const string& option)
{
    const string num_tests_option = "--num-tests";

    if (!starts_with(option, num_tests_option))
    {
        cerr << "unknown option: '" << option << "'" << endl;
        exit(EXIT_FAILURE);
    }

    if (option.size() == num_tests_option.size() ||
        option[num_tests_option.size()] != '=')
    {
        cerr << "missing '=' after '" + num_tests_option << "'" << endl;
        exit(EXIT_FAILURE);
    }

    const auto num_tests_as_string = option.substr(num_tests_option.size() + 1);
    const auto num_tests = atoll(num_tests_as_string.c_str());

    if (num_tests <= 0)
    {
        cerr << "invalid value after '" + num_tests_option << "'" << endl;
        exit(EXIT_FAILURE);
    }

    g_num_tests_to_run = static_cast<decltype(g_num_tests_to_run)>(num_tests);
    g_run_infinitely = false;
}

void
handle_command_line_option_randomize()
{
    g_randomize = true;
}

// Return the number of digits in the decimal representation of 'n'.
//
// Examples:
// * if n is   0, return 1
// * if n is   9, return 1
// * if n is 123, return 3
int
num_digits(unsigned long long int n)
{
    int num_digits = 1;

    while (n > 9)
    {
        ++num_digits;
        n /= 10;
    }

    return num_digits;
}

void
parse_command_line(int argc, const char* const* argv)
{
    const vector<string> args(argv, argv + argc);

    auto args_it = args.cbegin();

    assert(args_it != args.cend());
    g_program_path = *(args_it++);

    if (args_it == args.cend())
    {
        return;
    }

    string arg = *(args_it++);

    if (arg == "--help")
    {
        handle_command_line_option_help();

        if (args_it != args.cend())
        {
            cerr << "extraneous argument after '--help'" << endl;
            exit(EXIT_FAILURE);
        }

        return;
    }

    if (arg == "--randomize")
    {
        handle_command_line_option_randomize();

        if (args_it == args.cend())
        {
            // '--randomize' was the only argument.
            return;
        }

        arg = *(args_it++);
    }

    handle_command_line_option_num_tests(arg);

    if (args_it != args.cend())
    {
        cerr << "extraneous argument after '--num-tests'" << endl;
        exit(EXIT_FAILURE);
    }
}

// Return num_regexes / total number of regexes done so far, as percents.
double
percentage(unsigned long long int num_regexes)
{
    return ratio(num_regexes) * 100.0;
}

void
print_statistic(const string&          statistic_name,
                unsigned long long int num_regexes)
{
    const auto num_digts_in_total_num_regexes = num_digits(total_num_regexes());

    clog << statistic_name
         << " = " << setw(num_digts_in_total_num_regexes) << num_regexes
         << " = " << setw(6) << fixed << setprecision(2) << right
         << percentage(num_regexes) << '%' << endl;
}

void
print_statistics()
{
    print_statistic("regexes which cannot be parsed",
                    g_num_regexes_which_cannot_be_parsed);

    print_statistic("regexes with bad structure    ",
                    g_num_regexes_with_bad_structure);

    print_statistic("good regexes, not constrained ",
                    g_num_good_regexes_not_constrained);

    print_statistic("good regexes, constrained     ",
                    g_num_good_regexes_constrained);

    print_statistic("total number of regexes       ",
                    total_num_regexes());

    clog << endl;
}

void
print_usage()
{
    const string indentation(4, ' ');

    cout
    << endl

    << "USAGE:" << endl

    << indentation
    << g_program_path << " <option>*" << endl

    << "or:" << endl

    << indentation
    << g_program_path << " --help" << endl

    << endl

    << "with <option> one of:" << endl

    << endl

    << indentation
    << "--randomize      Use a non-deterministic sequence of random numbers."
    << endl

    << indentation
    << "                 Without this option, the sequence of tests is "
    << endl

    << indentation
    << "                 identical across executions on the same platform."
    << endl

    << endl

    << indentation
    << "--num-tests=<n>  Generate <n> tests; <n> must be strictly positive."
    << endl

    << indentation
    << "                 The default is to run infinitely."
    << endl

    << endl

    << "If both options are given, they must appear in the indicated order."
    << endl

    << endl;
}

char
random_char()
{
    static const string chars = R"( !"#$%&'()*+,-./)"
                                R"(0123456789:;<=>?)"
                                R"(0123456789:;<=>?)"
                                R"(@ABCDEFGHIJKLMNO)"
                                R"(PQRSTUVWXYZ[\]^_)"
                                R"(`abcdefghijklmno)"
                                R"(pqrstuvwxyz{|}~)";
    static const size_t num_chars = chars.size();

    static const size_t min_index = 0;
    static const size_t max_index = num_chars - 1;
    static uniform_int_distribution<size_t> distribution(min_index, max_index);

    const auto random_index = distribution(g_random_engine);
    return chars[random_index];
}

size_t
random_length()
{
    static const size_t min_length =  0;
    static const size_t max_length = 20;
    static uniform_int_distribution<size_t> distribution(
                                              min_length, max_length);

    return distribution(g_random_engine);
}

string
random_string()
{
    string regex_string;
    const auto length = random_length();
    regex_string.reserve(length);

    for (size_t i = 0; i != length; ++i)
    {
        regex_string.push_back(random_char());
    }

    return regex_string;
}

// Return num_regexes / total number of regexes done so far.
double
ratio(unsigned long long int num_regexes)
{
    return static_cast<double>(num_regexes) /
           static_cast<double>(total_num_regexes());
}

void
remove_regex_string_from_disk()
{
    if (remove(g_regex_filename) != 0)
    {
        cerr << "could not delete file "
             << Utils::quoted(g_regex_filename) << endl;
        exit(EXIT_FAILURE);
    }
}

void
save_regex_string_to_disk(const string& s)
{
    ofstream ofs(g_regex_filename, ofstream::trunc);
    ofs << s;
}

bool
starts_with(const string& s, const string& prefix)
{
    return s.compare(0, prefix.size(), prefix) == 0;
}

void
test_string(const string& s)
{
    unique_ptr<Regex> regex;

    try
    {
        regex = Regex::parse(s);
    }
    catch (const RegexParseException&)
    {
        ++g_num_regexes_which_cannot_be_parsed;
        return;
    }
    catch (const RegexStructureException&)
    {
        ++g_num_regexes_with_bad_structure;
        return;
    }

    if (would_take_too_long_to_constrain(s))
    {
        ++g_num_good_regexes_not_constrained;
        return;
    }

    ++g_num_good_regexes_constrained;
    check_constraints(*regex, s);
}

// Return a seed based on time (for use by a random generator).
unsigned int
time_seed()
{
    return static_cast<unsigned int>(time(nullptr));
}

unsigned long long int
total_num_regexes()
{
    return g_num_regexes_which_cannot_be_parsed +
           g_num_regexes_with_bad_structure     +
           g_num_good_regexes_not_constrained   +
           g_num_good_regexes_constrained;
}

bool
would_take_too_long_to_constrain(const string& s)
{
    const auto num_kleene_stars = count(s.begin(), s.end(), '*');
    const auto num_pluses = count(s.begin(), s.end(), '+');
    const auto num_question_marks = count(s.begin(), s.end(), '?');
    const auto has_open_brace  = find(s.begin(), s.end(), '{') != s.end();
    const auto has_close_brace = find(s.begin(), s.end(), '}') != s.end();
    const auto may_have_brace_repetitions = has_open_brace && has_close_brace;
    const auto num_brace_repetitions = may_have_brace_repetitions ? 1 : 0;
    const auto num_repetitions = num_kleene_stars + num_pluses +
                                 num_question_marks + num_brace_repetitions;

    return num_repetitions >= 2;
}

} // unnamed namespace


int
main(int argc, char* argv[])
{
    parse_command_line(argc, argv);

    if (g_is_help_requested)
    {
        print_usage();
        return EXIT_SUCCESS;
    }

    if (g_randomize)
    {
        g_random_engine.seed(time_seed());
    }

    unsigned long long int i = 0;

    while (true)
    {
        if (!g_run_infinitely && i == g_num_tests_to_run)
        {
            break;
        }

        if (i % 1000 == 0 && i != 0)
        {
            print_statistics();
        }

        ++i;

        const auto s = random_string();
        save_regex_string_to_disk(s);
        test_string(s);
    }

    print_statistics();
    remove_regex_string_from_disk();

    return EXIT_SUCCESS;
}
