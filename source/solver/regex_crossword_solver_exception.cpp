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


#include "regex_crossword_solver_exception.hpp"

#include "command_line.hpp"
#include "utils.hpp"

#include <algorithm>
#include <cassert>
#include <iterator>
#include <numeric>

using namespace std;


namespace
{

string         combine(vector<string> lines);
vector<string> indent(const vector<string>& lines);
string         indent_and_combine(const vector<string>& lines);
string         indent_and_combine(const string& message);
const string&  indentation();
string         without_trailing_newline(const string& s);

// Return 'lines' combined into a single string, with each line
// terminated by a newline character.
//
// For example, if 'lines' is { "line 1", "line 2" }, return
// "line 1\nline 2\n".
string
combine(vector<string> lines)
{
    return accumulate(lines.cbegin(),
                      lines.cend(),
                      string(),
                      [](const string& sum, const string& line)
                      {
                          return sum + line + '\n';
                      });
}

// Return 'lines', with each line indented.
vector<string>
indent(const vector<string>& lines)
{
    vector<string> result;

    transform(lines.cbegin(),
              lines.cend(),
              back_inserter(result),
              [](const string& line)
              {
                  return indentation() + line;
              });

    return result;
}

string
indent_and_combine(const vector<string>& lines)
{
    return combine(indent(lines));
}

string
indent_and_combine(const string& message)
{
    return indent_and_combine(Utils::split_into_lines(message));
}

const string&
indentation()
{
    static const string result(4, ' ');
    return result;
}

string
without_trailing_newline(const string& s)
{
    auto result = s;

    if (!result.empty() && result.back() == '\n')
    {
        result.pop_back();
    }

    return result;
}

} // unnamed namespace


// RegexCrosswordSolverException
// -----------------------------

// instance creation and deletion

RegexCrosswordSolverException::RegexCrosswordSolverException(
                                 const string& message) :
  runtime_error(without_trailing_newline("ERROR:\n" + message))
{
}


// AlphabetException
// -----------------

// instance creation and deletion

AlphabetException::AlphabetException(const string& message) :
  RegexCrosswordSolverException(indent_and_combine(message))
{
}


// CommandLineException
// --------------------

// instance creation and deletion

CommandLineException::CommandLineException(const string& message) :
  RegexCrosswordSolverException(format(message))
{
}

// accessing

string
CommandLineException::format(const string& message)
{
    auto result = indent(Utils::split_into_lines(message));

    result.push_back("");

    // 'meta_usage_lines' are not indented.
    const auto meta_usage_lines = Utils::split_into_lines(meta_usage());
    result.insert(result.end(),
                  meta_usage_lines.begin(),
                  meta_usage_lines.end());

    return combine(result);
}

string
CommandLineException::meta_usage()
{
    ostringstream oss;
    CommandLine::print_meta_usage(oss);
    return oss.str();
}


// GridStructureException
// ----------------------

// instance creation and deletion

GridStructureException::GridStructureException(const string& message) :
  RegexCrosswordSolverException(indent_and_combine(message))
{
}


// InputFileException
// ------------------

// instance creation and deletion

InputFileException::InputFileException(const string& message) :
  RegexCrosswordSolverException(indent_and_combine(message))
{
}

InputFileException::InputFileException(const string& filepath,
                                       size_t        line_number,
                                       const string& line,
                                       const string& message) :
  RegexCrosswordSolverException(format(filepath, line_number, line, message))
{
}

// accessing

string
InputFileException::format(const string& filepath,
                           size_t        line_number,
                           const string& line,
                           const string& message)
{
    const string line_1("in " + Utils::quoted(filepath) + ", line " +
                        Utils::to_string(line_number) + ':');
    const string line_2(indentation() + Utils::quoted(line));
    const string line_3(message);

    return indent_and_combine({ line_1, line_2, line_3 });
}


// RegexParseException
// -------------------

// instance creation and deletion

// Precondition:
// * error_position <= regex_as_string.size()
RegexParseException::RegexParseException(const string& message,
                                         const string& regex_as_string,
                                         size_t        error_position) :
  RegexCrosswordSolverException(format(
                                  message, regex_as_string, error_position))
{
}

// accessing

string
RegexParseException::format(const string& message,
                            const string& regex_as_string,
                            size_t        error_position)
{
    assert(error_position <= regex_as_string.size());

    const string line_1(message + ':');
    const string line_2(indentation() + Utils::quoted(regex_as_string));
    const string line_3(indentation() + ' ' +
                        string(error_position, ' ') + '^');

    return indent_and_combine({ line_1, line_2, line_3 });
}


// RegexStructureException
// -----------------------

// instance creation and deletion

RegexStructureException::RegexStructureException(const string& message) :
  RegexCrosswordSolverException(indent_and_combine(message))
{
}
