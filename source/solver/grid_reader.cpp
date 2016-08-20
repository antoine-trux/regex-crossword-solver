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


#include "grid_reader.hpp"

#include "hexagonal_grid.hpp"
#include "rectangular_grid.hpp"
#include "regex_crossword_solver_exception.hpp"
#include "utils.hpp"

#include <cassert>
#include <cctype>
#include <fstream>
#include <iostream>
#include <sstream>

using namespace std;


namespace
{

const string g_hexagonal   = "hexagonal";
const string g_rectangular = "rectangular";

}


// instance creation and deletion

GridReader::GridReader(const string& filepath, istream& is) :
  m_filepath(filepath),
  m_is(is),
  m_line_number(0)
{
}

// accessing

unique_ptr<Grid>
GridReader::read(const string& filepath)
{
    ifstream ifs(filepath);
    if (!ifs)
    {
        throw InputFileException("input file " + Utils::quoted(filepath) +
                                 " could not be opened for reading");
    }
    return read(filepath, ifs);
}

unique_ptr<Grid>
GridReader::read(istream& is)
{
    const string filepath = "";
    return read(filepath, is);
}

unique_ptr<Grid>
GridReader::read(const string& filepath, istream& is)
{
    GridReader reader(filepath, is);
    return reader.read();
}

unique_ptr<Grid>
GridReader::read()
{
    const auto shape = read_shape();

    if (shape == g_hexagonal)
    {
        return read_hexagonal_grid();
    }
    else
    {
        assert(shape == g_rectangular);
        return read_rectangular_grid();
    }
}

// Read the next data line, store it in 'm_data_line', and return true.
// Return false if no data line is found.
bool
GridReader::read_data_line()
{
    string line;

    while (getline(m_is, line))
    {
        ++m_line_number;

        // Handle DOS format by removing the trailing carriage-return
        // character from 'line'.
        if (!line.empty() && line.back() == '\r')
        {
            line.pop_back();
        }

        if (is_data_line(line))
        {
            m_data_line = line;
            return true;
        }
    }

    return false;
}

unique_ptr<Grid>
GridReader::read_hexagonal_grid()
{
    const auto num_regexes_per_line = read_num_regexes_per_line();
    const auto regexes = read_regexes();
    return Utils::make_unique<HexagonalGrid>(regexes, num_regexes_per_line);
}

// Return the non-zero, numeric value that corresponds to 'key', which
// is to be next in the input.
size_t
GridReader::read_non_zero_num_value(const string& key)
{
    const auto string_value = read_string_value(key);

    size_t num_value;

    if (!Utils::string_to_unsigned(string_value, &num_value))
    {
        throw_input_file_exception("invalid value for "      +
                                   Utils::quoted(key) + ": " +
                                   Utils::quoted(string_value));
    }

    if (num_value == 0)
    {
        throw_input_file_exception("value for " +
                                   Utils::quoted(key) + " should not be 0");
    }

    return num_value;
}

size_t
GridReader::read_num_cols()
{
    return read_non_zero_num_value("num_cols");
}

size_t
GridReader::read_num_regexes_per_col()
{
    return read_non_zero_num_value("num_regexes_per_col");
}

size_t
GridReader::read_num_regexes_per_line()
{
    return read_non_zero_num_value("num_regexes_per_line");
}

size_t
GridReader::read_num_regexes_per_row()
{
    return read_non_zero_num_value("num_regexes_per_row");
}

size_t
GridReader::read_num_rows()
{
    return read_non_zero_num_value("num_rows");
}

unique_ptr<Grid>
GridReader::read_rectangular_grid()
{
    const auto num_rows = read_num_rows();
    const auto num_cols = read_num_cols();

    const auto num_regexes_per_row = read_num_regexes_per_row();
    const auto num_regexes_per_col = read_num_regexes_per_col();

    const auto regexes = read_regexes();

    return Utils::make_unique<RectangularGrid>(regexes,
                                               num_rows, num_regexes_per_row,
                                               num_cols, num_regexes_per_col);
}

// If the next data line contains something which can be a regex, read
// it, append it to 'regexes_as_strings' and return true. Return false
// otherwise.
bool
GridReader::read_regex(vector<string>& regexes_as_strings)
{
    if (!read_data_line())
    {
        return false;
    }

    auto regex_as_string = m_data_line;
    assert(!regex_as_string.empty());

    if (regex_as_string.front() != '\'' ||
        regex_as_string.back()  != '\'' ||
        regex_as_string.size()  == 1)
    {
        throw_input_file_exception(
          "regular expression is not surrounded by single quotes");
    }

    // Strip surrounding quotes from 'regex_as_string'.
    regex_as_string = regex_as_string.substr(1, regex_as_string.size() - 2);

    regexes_as_strings.push_back(regex_as_string);
    return true;
}

// Read and return the regexes.
vector<string>
GridReader::read_regexes()
{
    vector<string> regexes_as_strings;

    while (read_regex(regexes_as_strings))
    {
    }

    return regexes_as_strings;
}

// Read and return the shape.
string
GridReader::read_shape()
{
    const auto shape = read_string_value("shape");

    if (shape != g_hexagonal && shape != g_rectangular)
    {
        throw_input_file_exception("invalid grid shape");
    }

    return shape;
}

// Return the string value that corresponds to 'key', which is to be
// next in the input.
string
GridReader::read_string_value(const string& key)
{
    if (!read_data_line())
    {
        throw_input_file_exception("missing " + Utils::quoted(key));
    }

    istringstream line_iss(m_data_line);

    skip_key(line_iss, key);

    skip_spaces(line_iss);

    skip_equal_sign(line_iss, key);

    skip_spaces(line_iss);

    string string_value;
    getline(line_iss, string_value);

    if (string_value.empty())
    {
        throw_input_file_exception("missing value for " + Utils::quoted(key));
    }

    return string_value;
}

// Skip '=', which is to be next in 'line_is', knowing that '=' occurs
// after 'key'.
void
GridReader::skip_equal_sign(istream& line_is, const string& key)
{
    if (!Utils::skip(line_is, "="))
    {
        throw_input_file_exception("missing '=' after " + Utils::quoted(key));
    }
}

// Skip 'key', which is to be next in 'line_is'.
void
GridReader::skip_key(istream& line_is, const string& key)
{
    if (!Utils::skip(line_is, key))
    {
        throw_input_file_exception("expected " + Utils::quoted(key) +
                                   " at beginning of line");
    }
}

void
GridReader::skip_spaces(istream& is)
{
    char c;

    while (is.get(c))
    {
        if (!isspace(c))
        {
            is.unget();
            break;
        }
    }
}

// querying

bool
GridReader::is_data_line(const string& line)
{
    return !Utils::has_only_whitespace(line) && line.front() != '#';
}

// error handling

void
GridReader::throw_input_file_exception(const string& error_message) const
{
    throw InputFileException(m_filepath,
                             m_line_number,
                             m_data_line,
                             error_message);
}
