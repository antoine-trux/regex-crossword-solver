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


#ifndef GRID_UNIT_TESTS_UTILS_HPP
#define GRID_UNIT_TESTS_UTILS_HPP

#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>

class Grid;


// Facilities used by grid unit tests.
namespace GridUnitTestsUtils
{

// Return 'grid' cast to 'GridType'.
template<typename GridType>
std::unique_ptr<GridType>
downcast_grid(std::unique_ptr<Grid> grid)
{
    const auto raw_pointer = dynamic_cast<GridType*>(grid.release());
    EXPECT_NE(nullptr, raw_pointer);
    return std::unique_ptr<GridType>(raw_pointer);
}

std::unique_ptr<Grid> read_grid(const std::string& grid_contents);
void solve_and_check(
       const std::string&                           grid_contents,
       const std::vector<std::vector<std::string>>& expected_solutions);

} // namespace GridUnitTestsUtils


#endif // GRID_UNIT_TESTS_UTILS_HPP
