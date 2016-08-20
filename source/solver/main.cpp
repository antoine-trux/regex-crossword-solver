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
#include "grid.hpp"
#include "grid_reader.hpp"
#include "logger.hpp"
#include "regex_optimizations.hpp"
#include "utils.hpp"

#include <cassert>
#include <chrono>
#include <cstdlib>

using namespace std;


namespace
{

// Return the time, in milliseconds, between 'time_at_start' and
// 'time_at_end'.
double
duration_ms(chrono::time_point<chrono::high_resolution_clock> time_at_start,
            chrono::time_point<chrono::high_resolution_clock> time_at_end)
{
    assert(time_at_start <= time_at_end);

    // We compute the duration first in microseconds, then convert it
    // into milliseconds, in order not to lose precision.
    const auto duration_us =
        chrono::duration_cast<chrono::microseconds>(
                  time_at_end - time_at_start).count();
    return static_cast<double>(duration_us) / 1000.0;
}

unique_ptr<Grid>
read_grid()
{
    const auto log_filepath = CommandLine::log_filepath();
    SET_LOG_FILEPATH(log_filepath);

    const auto input_filepath = CommandLine::input_filepath();
    return GridReader::read(input_filepath);
}

void
report_time_to_solve(double time_to_solve_ms)
{
    Utils::print_verbose_message(cout, "\n");
    Utils::print_verbose_message(cout,
                                 "solution(s) found in "            +
                                 Utils::to_string(time_to_solve_ms) +
                                 " milliseconds");
}

void
throwing_main(int argc, const char* const* argv)
{
    CommandLine::parse(argc, argv);

    if (CommandLine::help_is_requested())
    {
        CommandLine::print_usage(cout);
        return;
    }

    if (CommandLine::version_is_requested())
    {
        CommandLine::print_version(cout);
        return;
    }

    const auto num_solutions_to_find = CommandLine::num_solutions_to_find();

    const auto time_at_start = chrono::high_resolution_clock::now();

    const auto grid = read_grid();
    grid->optimize(CommandLine::regex_optimizations());
    const auto solutions = grid->solve(num_solutions_to_find);

    const auto time_at_end = chrono::high_resolution_clock::now();

    Grid::report_solutions(solutions, num_solutions_to_find);
    report_time_to_solve(duration_ms(time_at_start, time_at_end));
}

} // unnamed namespace


int
main(int argc, char* argv[])
{
    try
    {
        throwing_main(argc, argv);
    }
    // All the exception classes defined in this program - see module
    // 'regex_crossword_solver_exception' - derive from
    // 'std::runtime_error', which itself derives from 'std::exception'.
    // Therefore, this single catch clause is enough to catch all the
    // exceptions that might be thrown.
    catch (const exception& e)
    {
        cerr << e.what() << endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
