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


#ifndef COMMAND_LINE_HPP
#define COMMAND_LINE_HPP

#include <iosfwd>
#include <string>

class RegexOptimizations;


// This module provides facilities for handling the command line.
//
// The command line is parsed with CommandLine::parse(), after which
// the other functions of this module can be called:
//
//     CommandLine::parse(argc, argv);
//     [...]
//     if (CommandLine::help_is_requested())
//     {
//         CommandLine::print_usage();
//     }
//     else
//     {
//         [...]
//     }
namespace CommandLine
{

// accessing
std::string        input_filepath();
std::string        log_filepath();
unsigned int       num_solutions_to_find();
void               parse(int argc, const char* const* argv);
RegexOptimizations regex_optimizations();

// querying
bool help_is_requested();
bool is_verbose();
bool version_is_requested();

// printing
void print_meta_usage(std::ostream& os);
void print_usage(std::ostream& os);
void print_version(std::ostream& os);

// modifying
void reset_to_defaults();

}


#endif // COMMAND_LINE_HPP
