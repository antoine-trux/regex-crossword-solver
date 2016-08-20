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


#include "regex_crossword_solver_test.hpp"

#include "alphabet.hpp"
#include "command_line.hpp"
#include "utils.hpp"


// test fixture

void
RegexCrosswordSolverTest::SetUp()
{
    // Some unit tests change the alphabet, so we reset the alphabet
    // here in order always to start from a known state.
    Alphabet::reset();

    // Some unit tests exercise code that calls CommandLine getters.
    // These functions would trigger assertions if CommandLine::parse()
    // had not been called before, hence this call to
    // CommandLine::parse() (with fake parameters).
    const char* const argv[] = { "program", "--help", nullptr };
    const auto argc = Utils::array_size(argv) - 1;
    CommandLine::parse(argc, argv);
}
