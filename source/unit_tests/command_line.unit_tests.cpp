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
#include "disable_warnings_from_gtest.hpp"
#include "logger.hpp"
#include "regex_crossword_solver_exception.hpp"
#include "regex_crossword_solver_test.hpp"
#include "regex_optimizations.hpp"
#include "utils.hpp"

using namespace std;


class CommandLineTest : public RegexCrosswordSolverTest
{
protected:
    // test fixture
    void SetUp() override
    {
        RegexCrosswordSolverTest::SetUp();
        CommandLine::reset_to_defaults();
    }
};


TEST_F(CommandLineTest, no_arguments)
{
    const char* const argv[] = { "program", nullptr };
    const int argc = Utils::array_size(argv) - 1;
    EXPECT_THROW(CommandLine::parse(argc, argv), CommandLineException);
}

TEST_F(CommandLineTest, help_and_extra_argument)
{
    const char* const argv[] = { "program", "--help", "foo", nullptr };
    const int argc = Utils::array_size(argv) - 1;
    EXPECT_THROW(CommandLine::parse(argc, argv), CommandLineException);
}

TEST_F(CommandLineTest, version_and_extra_argument)
{
    const char* const argv[] = { "program", "--version", "foo", nullptr };
    const int argc = Utils::array_size(argv) - 1;
    EXPECT_THROW(CommandLine::parse(argc, argv), CommandLineException);
}

TEST_F(CommandLineTest, help)
{
    const char* const argv[] = { "program", "--help", nullptr };
    const int argc = Utils::array_size(argv) - 1;
    CommandLine::parse(argc, argv);
    EXPECT_TRUE(CommandLine::help_is_requested());
}

TEST_F(CommandLineTest, version)
{
    const char* const argv[] = { "program", "--version", nullptr };
    const int argc = Utils::array_size(argv) - 1;
    CommandLine::parse(argc, argv);
    EXPECT_TRUE(CommandLine::version_is_requested());
}

TEST_F(CommandLineTest, help_abbreviated)
{
    const char* const argv[] = { "program", "-h", nullptr };
    const int argc = Utils::array_size(argv) - 1;
    CommandLine::parse(argc, argv);
    EXPECT_TRUE(CommandLine::help_is_requested());
}

TEST_F(CommandLineTest, help_and_version_are_not_default)
{
    const char* const argv[] = { "program", "input_file", nullptr };
    const int argc = Utils::array_size(argv) - 1;
    CommandLine::parse(argc, argv);
    EXPECT_FALSE(CommandLine::help_is_requested());
    EXPECT_FALSE(CommandLine::version_is_requested());
}

TEST_F(CommandLineTest, verbose_and_missing_input_file)
{
    const char* const argv[] = { "program", "--verbose", nullptr };
    const int argc = Utils::array_size(argv) - 1;
    EXPECT_THROW(CommandLine::parse(argc, argv), CommandLineException);
}

TEST_F(CommandLineTest, verbose)
{
    const char* const argv[] =
        { "program", "--verbose", "input_file", nullptr };
    const int argc = Utils::array_size(argv) - 1;
    CommandLine::parse(argc, argv);
    EXPECT_TRUE(CommandLine::is_verbose());
}

TEST_F(CommandLineTest, verbose_abbreviated)
{
    const char* const argv[] = { "program", "-v", "input_file", nullptr };
    const int argc = Utils::array_size(argv) - 1;
    CommandLine::parse(argc, argv);
    EXPECT_TRUE(CommandLine::is_verbose());
}

TEST_F(CommandLineTest, invalid_option)
{
    const char* const argv[] = { "program", "--foo", "input_file", nullptr };
    const int argc = Utils::array_size(argv) - 1;
    EXPECT_THROW(CommandLine::parse(argc, argv), CommandLineException);
}

TEST_F(CommandLineTest, all_solutions_and_missing_input_file)
{
    const char* const argv[] = { "program", "--all-solutions", nullptr };
    const int argc = Utils::array_size(argv) - 1;
    EXPECT_THROW(CommandLine::parse(argc, argv), CommandLineException);
}

TEST_F(CommandLineTest, num_solutions_to_find_all)
{
    const char* const argv[] =
        { "program", "--stop-after=-1", "input_file", nullptr };
    const int argc = Utils::array_size(argv) - 1;
    CommandLine::parse(argc, argv);
    EXPECT_EQ(numeric_limits<unsigned int>::max(),
              CommandLine::num_solutions_to_find());
}

TEST_F(CommandLineTest, num_solutions_to_find_one)
{
    const char* const argv[] =
        { "program", "--stop-after=1", "input_file", nullptr };
    const int argc = Utils::array_size(argv) - 1;
    CommandLine::parse(argc, argv);
    EXPECT_EQ(1, CommandLine::num_solutions_to_find());
}

TEST_F(CommandLineTest, num_solutions_to_find_all_is_default)
{
    const char* const argv[] = { "program", "input_file", nullptr };
    const int argc = Utils::array_size(argv) - 1;
    CommandLine::parse(argc, argv);
    EXPECT_EQ(2, CommandLine::num_solutions_to_find());
}

#if ENABLE_LOGGING

TEST_F(CommandLineTest, log_and_no_log_file)
{
    const char* const argv[] = { "program", "--log", nullptr };
    const int argc = Utils::array_size(argv) - 1;
    EXPECT_THROW(CommandLine::parse(argc, argv), CommandLineException);
}

TEST_F(CommandLineTest, log_file_exists)
{
    const char* const argv[] = { "program", "--log=.", nullptr };
    const int argc = Utils::array_size(argv) - 1;
    EXPECT_THROW(CommandLine::parse(argc, argv), CommandLineException);
}

TEST_F(CommandLineTest, log_and_missing_input_file)
{
    const char* const argv[] = { "program", "--log=log_file", nullptr };
    const int argc = Utils::array_size(argv) - 1;
    EXPECT_THROW(CommandLine::parse(argc, argv), CommandLineException);
}

TEST_F(CommandLineTest, log)
{
    const char* const argv[] =
        { "program", "--log=log_file", "input_file", nullptr };
    const int argc = Utils::array_size(argv) - 1;
    CommandLine::parse(argc, argv);
    EXPECT_EQ("log_file", CommandLine::log_filepath());
}

#else // ENABLE_LOGGING

TEST_F(CommandLineTest, log)
{
    const char* const argv[] =
        { "program", "--log=log_file", "input_file", nullptr };
    const int argc = Utils::array_size(argv) - 1;
    EXPECT_THROW(CommandLine::parse(argc, argv), CommandLineException);
}

#endif // ENABLE_LOGGING

TEST_F(CommandLineTest, no_optim)
{
    const char* const argv[] =
        { "program", "--no-optim", "input_file", nullptr };
    const int argc = Utils::array_size(argv) - 1;
    CommandLine::parse(argc, argv);
    const auto optimizations = CommandLine::regex_optimizations();
    EXPECT_FALSE(optimizations.optimize_concatenations());
    EXPECT_FALSE(optimizations.optimize_groups());
    EXPECT_FALSE(optimizations.optimize_unions());
}

TEST_F(CommandLineTest, input_file)
{
    const char* const input_file = "input_file";
    const char* const argv[] = { "program", input_file, nullptr };
    const int argc = Utils::array_size(argv) - 1;
    CommandLine::parse(argc, argv);
    EXPECT_EQ(input_file, CommandLine::input_filepath());
}

TEST_F(CommandLineTest, missing_value)
{
    const char* const argv[] =
        { "program", "--stop-after=", "input_file", nullptr };
    const int argc = Utils::array_size(argv) - 1;
    EXPECT_THROW(CommandLine::parse(argc, argv), CommandLineException);
}

TEST_F(CommandLineTest, invalid_stop_after_value)
{
    const char* const argv[] =
        { "program", "--stop-after=x", "input_file", nullptr };
    const int argc = Utils::array_size(argv) - 1;
    EXPECT_THROW(CommandLine::parse(argc, argv), CommandLineException);
}

TEST_F(CommandLineTest, stop_after_zero)
{
    const char* const argv[] =
        { "program", "--stop-after=0", "input_file", nullptr };
    const int argc = Utils::array_size(argv) - 1;
    EXPECT_THROW(CommandLine::parse(argc, argv), CommandLineException);
}

TEST_F(CommandLineTest, input_file_and_extra_argument)
{
    const char* const argv[] = { "program", "input_file", "foo", nullptr };
    const int argc = Utils::array_size(argv) - 1;
    EXPECT_THROW(CommandLine::parse(argc, argv), CommandLineException);
}

TEST_F(CommandLineTest, print_usage)
{
    // The output is not checked.
    ostringstream oss;
    CommandLine::print_usage(oss);
}
