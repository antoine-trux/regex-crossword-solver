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


#include "command_line.hpp"

#include "logger.hpp"
#include "regex_crossword_solver_exception.hpp"
#include "regex_optimizations.hpp"
#include "utils.hpp"

#include <cassert>
#include <iostream>
#include <limits>
#include <vector>

using namespace std;


namespace
{

// accessing
void   parse_help_option(vector<string>::const_iterator& args_it);
void   parse_log_option(const string& log_option);
void   parse_normal_option(vector<string>::const_iterator& args_it);
void   parse_options(vector<string>::const_iterator& args_it,
                     const vector<string>&           args);
void   parse_stop_after_option(const string& stop_after_option);
string parse_value_option(const string& option, const string& option_specifier);
void   parse_version_option(vector<string>::const_iterator& args_it);

// querying
bool is_help_option(const string& arg);
bool is_option(const string& arg);
bool is_version_option(const string& arg);

// error handling
void check_log_option();
void check_more_arguments_follow(vector<string>::const_iterator args_it,
                                 const vector<string>&          args);
void check_no_more_arguments_follow(vector<string>::const_iterator args_it,
                                    const vector<string>&          args);

// data

const bool         g_help_is_requested_default = false;
const string       g_input_filepath_default = "";
const bool         g_is_verbose_default = false;
const string       g_log_filepath_default = "";
// Reasons for setting the default value of
// 'g_num_solutions_to_find_default' to 2:
// * Usually, grids have one single solution. Having the default equal
//   to more than 1 allows the program to determine that the grid has
//   no other solutions (see reporting of solutions, in 'grid.cpp').
// * Limiting the number of solutions to 2 prevents the program from
//   running for a long time in case there would be many solutions.
const unsigned int g_num_solutions_to_find_default = 2;
const bool         g_optimize_concatenations_default = true;
const bool         g_optimize_groups_default = true;
const bool         g_optimize_unions_default = true;
const string       g_program_path_default = "";
const bool         g_version_is_requested_default = false;

bool         g_help_is_requested = g_help_is_requested_default;
string       g_input_filepath = g_input_filepath_default;
bool         g_is_verbose = g_is_verbose_default;
string       g_log_filepath = g_log_filepath_default;
unsigned int g_num_solutions_to_find = g_num_solutions_to_find_default;
bool         g_optimize_concatenations = g_optimize_concatenations_default;
bool         g_optimize_groups = g_optimize_groups_default;
bool         g_optimize_unions = g_optimize_unions_default;
string       g_program_path = g_program_path_default;
bool         g_version_is_requested = g_version_is_requested_default;

bool         g_command_line_was_parsed = false;

// accessing

// When this function is called, 'args_it' points to the '--help'
// option.
//
// Upon exit from this function, 'args_it' points past the '--help'
// option.
void
parse_help_option(vector<string>::const_iterator& args_it)
{
    assert(is_help_option(*args_it));

    ++args_it;

    g_help_is_requested = true;
}

// Parse '--log=<log_file>'.
void
parse_log_option(const string& log_option)
{
    g_log_filepath = parse_value_option(log_option, "--log");
    check_log_option();
}

// When this function is called, 'args_it' points to the next option
// (which is not '--help', nor '--version').
//
// Upon exit from this function, 'args_it' points past the option that
// this function parses.
void
parse_normal_option(vector<string>::const_iterator& args_it)
{
    const auto option = *(args_it++);
    assert(!is_help_option(option));
    assert(!is_version_option(option));

    if (Utils::starts_with(option, "--log"))
    {
        parse_log_option(option);
    }
    else if (option == "--no-concat-optim")
    {
        g_optimize_concatenations = false;
    }
    else if (option == "--no-group-optim")
    {
        g_optimize_groups = false;
    }
    else if (option == "--no-optim")
    {
        g_optimize_concatenations = false;
        g_optimize_groups = false;
        g_optimize_unions = false;
    }
    else if (option == "--no-union-optim")
    {
        g_optimize_unions = false;
    }
    else if (Utils::starts_with(option, "--stop-after"))
    {
        parse_stop_after_option(option);
    }
    else if (option == "--verbose" || option == "-v")
    {
        g_is_verbose = true;
    }
    else
    {
        throw CommandLineException("unrecognized option: " +
                                   Utils::quoted(option));
    }
}

// When this function is called, 'args_it' points to the first command
// line argument after the program name.
//
// Upon exit from this function, 'args_it' points to the first command
// line argument after the last option (if any option).
void
parse_options(vector<string>::const_iterator& args_it,
              const vector<string>&           args)
{
    if (args_it != args.cend())
    {
        if (is_help_option(*args_it))
        {
            parse_help_option(args_it);
            return;
        }

        if (is_version_option(*args_it))
        {
            parse_version_option(args_it);
            return;
        }
    }

    while (args_it != args.cend() && is_option(*args_it))
    {
        parse_normal_option(args_it);
    }
}

// Parse '--stop-after=<n>'.
void
parse_stop_after_option(const string& stop_after_option)
{
    const string stop_after_option_specifier = "--stop-after";

    const auto value = parse_value_option(stop_after_option,
                                          stop_after_option_specifier);

    if (value == "-1")
    {
        g_num_solutions_to_find = numeric_limits<unsigned int>::max();
        return;
    }

    if (!Utils::string_to_unsigned(value, &g_num_solutions_to_find))
    {
        throw CommandLineException("invalid value for " +
                                   Utils::quoted(stop_after_option_specifier));
    }

    if (g_num_solutions_to_find == 0)
    {
        throw CommandLineException("value for "                               +
                                   Utils::quoted(stop_after_option_specifier) +
                                   " must not be 0");
    }
}

// 'option' is of the form '--xxx=yyy', where '--xxx' is the option
// specifier and 'yyy' is the option value. Return the option value.
//
// Precondition:
// * 'option' starts with 'option_specifier'
string
parse_value_option(const string& option, const string& option_specifier)
{
    assert(Utils::starts_with(option, option_specifier));

    if (option.size() == option_specifier.size() ||
        option[option_specifier.size()] != '=')
    {
        throw CommandLineException("missing '=' after " +
                                   Utils::quoted(option_specifier));
    }

    const auto value = option.substr(option_specifier.size() + 1);

    if (value.empty())
    {
        throw CommandLineException("missing value after " +
                                   Utils::quoted(option_specifier + '='));
    }

    return value;
}

// When this function is called, 'args_it' points to the '--version'
// option.
//
// Upon exit from this function, 'args_it' points past the '--version'
// option.
void
parse_version_option(vector<string>::const_iterator& args_it)
{
    assert(is_version_option(*args_it));

    ++args_it;

    g_version_is_requested = true;
}

// querying

bool
is_help_option(const string& arg)
{
    return arg == "--help" || arg == "-h";
}

bool
is_option(const string& arg)
{
    return Utils::starts_with(arg, "-");
}

bool
is_version_option(const string& arg)
{
    return arg == "--version";
}

// error handling

void
check_log_option()
{
#if ENABLE_LOGGING

    assert(!g_log_filepath.empty());

    if (g_log_filepath != "-" &&
        Utils::filesystem_entity_exists(g_log_filepath))
    {
        throw CommandLineException("log file "                   +
                                   Utils::quoted(g_log_filepath) +
                                   " already exists");
    }

#else // ENABLE_LOGGING

    throw CommandLineException(
            "Logging is not enabled.\n"
            "Rebuild the program with 'ENABLE_LOGGING' set to 1 in logger.hpp\n"
            "in order to use option '--log'.\n");

#endif // ENABLE_LOGGING
}

void
check_more_arguments_follow(vector<string>::const_iterator args_it,
                            const vector<string>&          args)
{
    if (args_it == args.cend())
    {
        throw CommandLineException("missing arguments");
    }
}

void
check_no_more_arguments_follow(vector<string>::const_iterator args_it,
                               const vector<string>&          args)
{
    if (args_it != args.cend())
    {
        throw CommandLineException("extra arguments");
    }
}

} // unnamed namespace


// accessing

string
CommandLine::input_filepath()
{
    assert(g_command_line_was_parsed);
    return g_input_filepath;
}

string
CommandLine::log_filepath()
{
    assert(g_command_line_was_parsed);
    return g_log_filepath;
}

unsigned int
CommandLine::num_solutions_to_find()
{
    assert(g_command_line_was_parsed);
    return g_num_solutions_to_find;
}

// 'argc' and 'argv' are the same arguments as those passed to main().
void
CommandLine::parse(int argc, const char* const* argv)
{
    // getopt() is not available in Windows, so we resort to our own
    // parsing code.

    const vector<string> args(argv, argv + argc);

    auto args_it = args.cbegin();

    assert(args_it != args.cend());
    g_program_path = *(args_it++);

    parse_options(args_it, args);

    if (!g_help_is_requested && !g_version_is_requested)
    {
        check_more_arguments_follow(args_it, args);
        g_input_filepath = *(args_it++);
    }

    check_no_more_arguments_follow(args_it, args);

    g_command_line_was_parsed = true;
}

RegexOptimizations
CommandLine::regex_optimizations()
{
    assert(g_command_line_was_parsed);

    auto optimizations = RegexOptimizations::all();

    using ROT = RegexOptimizations::Type;

    optimizations.set(ROT::CONCATENATIONS, g_optimize_concatenations);
    optimizations.set(ROT::GROUPS, g_optimize_groups);
    optimizations.set(ROT::UNIONS, g_optimize_unions);

    return optimizations;
}

// querying

bool
CommandLine::help_is_requested()
{
    assert(g_command_line_was_parsed);
    return g_help_is_requested;
}

bool
CommandLine::is_verbose()
{
    assert(g_command_line_was_parsed);
    return g_is_verbose;
}

bool
CommandLine::version_is_requested()
{
    assert(g_command_line_was_parsed);
    return g_version_is_requested;
}

// printing

void
CommandLine::print_meta_usage(ostream& os)
{
    os << "For information on command line usage:" << endl;
    os << "    " << g_program_path << " --help" << endl;
}

void
CommandLine::print_usage(ostream& os)
{
    const string indentation(4, ' ');

    os
    << endl

    << "USAGE:" << endl

    << indentation
    << g_program_path << " <option>* <input file>" << endl

    << "or:" << endl

    << indentation
    << g_program_path << " --help     Print this screen and exit." << endl

    << indentation
    << g_program_path << " -h         Same as '--help'." << endl

    << "or:" << endl

    << indentation
    << g_program_path << " --version  Print the version and exit." << endl

    << endl
    << "with <option> one of:" << endl
    << endl

    << indentation
    << "--log=<log file>   For this option to work, the program must be built"
    << endl

    << indentation
    << "                   with 'ENABLE_LOGGING' set to 1 in logger.hpp,"
    << endl

    << indentation
    << "                   and <log file> must not already exist." << endl

    << indentation
    << "                   If <log file> is '-', the log is printed "
    << "to the console." << endl

    << indentation
    << "--no-concat-optim  Disable concatenation optimization" << endl

    << indentation
    << "                   (concatenations are optimized by default)."
    << endl

    << indentation
    << "--no-group-optim   Disable group optimization" << endl

    << indentation
    << "                   (groups are optimized by default)." << endl

    << indentation
    << "--no-union-optim   Disable union optimization" << endl

    << indentation
    << "                   (unions are optimized by default)." << endl

    << indentation
    << "--no-optim         Same as" << endl

    << indentation
    << "                   '--no-concat-optim --no-group-optim "
       "--no-union-optim'." << endl

    << indentation
    << "--stop-after=<n>   Stop after <n> solutions have been found."
    << endl

    << indentation
    << "                   If <n> is -1, all solutions are to be found."
    << endl

    << indentation
    << "                   Default is " << g_num_solutions_to_find_default
    << '.' << endl

    << indentation
    << "-v                 Same as '--verbose'." << endl

    << indentation
    << "--verbose          Verbose information includes timing results."
    << endl

    << endl

    << "EXAMPLES:" << endl

    << indentation
    << g_program_path << " MIT.input.txt" << endl

    << indentation
    << g_program_path << " --log=MIT.log --no-concat-optim MIT.input.txt"
    << endl << endl

    << indentation
    << "See directory 'grid_tests' for example input files." << endl

    << endl;
}

void
CommandLine::print_version(ostream& os)
{
    const string indentation(4, ' ');

    os << endl
       << indentation << "Regex Crossword Solver, version 1.0.0" << endl
       << indentation << "Copyright (c) 2016 Antoine Trux" << endl
       << endl;
}

// modifying

// Used by unit tests.
void
CommandLine::reset_to_defaults()
{
    g_help_is_requested = g_help_is_requested_default;
    g_input_filepath = g_input_filepath_default;
    g_is_verbose = g_is_verbose_default;
    g_log_filepath = g_log_filepath_default;
    g_num_solutions_to_find = g_num_solutions_to_find_default;
    g_optimize_concatenations = g_optimize_concatenations_default;
    g_optimize_groups = g_optimize_groups_default;
    g_optimize_unions = g_optimize_unions_default;
    g_program_path = g_program_path_default;
    g_version_is_requested = g_version_is_requested_default;

    g_command_line_was_parsed = false;
}
