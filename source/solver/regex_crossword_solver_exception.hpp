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


#ifndef REGEX_CROSSWORD_SOLVER_EXCEPTION_HPP
#define REGEX_CROSSWORD_SOLVER_EXCEPTION_HPP

#include <stdexcept>
#include <string>


// class hierarchy
// ---------------
// std::runtime_error
//     RegexCrosswordSolverException
//         AlphabetException
//         CommandLineException
//         GridStructureException
//         InputFileException
//         RegexParseException
//         RegexStructureException


// All the exceptions explicitly thrown by this program are instances
// of a class derived from this abstract class.
class RegexCrosswordSolverException : public std::runtime_error
{
protected:
    // instance creation and deletion
    explicit RegexCrosswordSolverException(const std::string& message);
};


class AlphabetException final : public RegexCrosswordSolverException
{
public:
    // instance creation and deletion
    explicit AlphabetException(const std::string& message);
};


class CommandLineException final : public RegexCrosswordSolverException
{
public:
    // instance creation and deletion
    explicit CommandLineException(const std::string& message);

private:
    // accessing
    static std::string format(const std::string& message);
    static std::string meta_usage();
};


class GridStructureException final : public RegexCrosswordSolverException
{
public:
    // instance creation and deletion
    explicit GridStructureException(const std::string& message);
};


class InputFileException final : public RegexCrosswordSolverException
{
public:
    // instance creation and deletion
    explicit InputFileException(const std::string& message);
    InputFileException(const std::string& filepath,
                       size_t             line_number,
                       const std::string& line,
                       const std::string& message);

private:
    // accessing
    static std::string format(const std::string& filepath,
                              size_t             line_number,
                              const std::string& line,
                              const std::string& message);
};


class RegexParseException final : public RegexCrosswordSolverException
{
public:
    // instance creation and deletion
    RegexParseException(const std::string& message,
                        const std::string& regex_as_string,
                        size_t             error_position);

private:
    // accessing
    static std::string format(const std::string& message,
                              const std::string& regex_as_string,
                              size_t             error_position);
};


class RegexStructureException final : public RegexCrosswordSolverException
{
public:
    // instance creation and deletion
    explicit RegexStructureException(const std::string& message);
};


#endif // REGEX_CROSSWORD_SOLVER_EXCEPTION_HPP
