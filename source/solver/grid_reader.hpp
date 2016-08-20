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


#ifndef GRID_READER_HPP
#define GRID_READER_HPP

#include <iosfwd>
#include <memory>
#include <string>
#include <vector>

class Grid;


// This class reads and returns a grid from a file which contains a
// textual representation of a grid.
//
// For the format of a textual representation of a grid, see the unit
// tests for this module, as well as the input files for the grid tests.
class GridReader final
{
public:
    // accessing
    static std::unique_ptr<Grid> read(const std::string& filepath);
    static std::unique_ptr<Grid> read(std::istream& is);

private:
    // instance creation and deletion
    GridReader(const std::string& filepath, std::istream& is);

    // accessing
    static std::unique_ptr<Grid> read(const std::string& filepath,
                                      std::istream&      is);
    std::unique_ptr<Grid> read();
    bool read_data_line();
    std::unique_ptr<Grid> read_hexagonal_grid();
    size_t read_non_zero_num_value(const std::string& key);
    size_t read_num_cols();
    size_t read_num_regexes_per_col();
    size_t read_num_regexes_per_line();
    size_t read_num_regexes_per_row();
    size_t read_num_rows();
    std::unique_ptr<Grid> read_rectangular_grid();
    bool read_regex(std::vector<std::string>& regexes_as_strings);
    std::vector<std::string> read_regexes();
    std::string read_shape();
    std::string read_string_value(const std::string& key);
    void skip_equal_sign(std::istream& line_is, const std::string& key);
    void skip_key(std::istream& line_is, const std::string& key);
    void skip_spaces(std::istream& is);

    // querying
    static bool is_data_line(const std::string& line);

    // error handling
    void throw_input_file_exception(const std::string& error_message) const;

    // data members

    // The path of the file which contains the textual representation
    // of the grid.
    std::string m_filepath;

    // The input stream from which the textual representation of the
    // grid is read.
    std::istream& m_is;

    // The data line that was last read, or an empty string if no data
    // line was read yet.
    std::string m_data_line;

    // The number of the line that was last read, or 0 if no line was
    // read yet.
    size_t m_line_number;
};


#endif // GRID_READER_HPP
