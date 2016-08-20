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


#ifndef UTILS_HPP
#define UTILS_HPP

#include <cctype>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <iosfwd>
#include <limits>
#include <memory>
#include <sstream>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

class SetOfCharacters;


// Utils::make_unique()
//
// std::make_unique() is part of C++14, but not of C++11, so we provide
// Utils::make_unique() as a replacement. Our implementation is copied
// from
// http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2013/n3656.htm
// with minor modifications.
namespace Utils
{

template<class T> struct Unique_if
{
    typedef std::unique_ptr<T> Single_object;
};

template<class T> struct Unique_if<T[]>
{
    typedef std::unique_ptr<T[]> Unknown_bound;
};

template<class T, size_t N> struct Unique_if<T[N]>
{
    typedef void Known_bound;
};

template<class T, class... Args>
typename Unique_if<T>::Single_object
make_unique(Args&&... args)
{
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

template<class T>
typename Unique_if<T>::Unknown_bound
make_unique(size_t n)
{
    typedef typename std::remove_extent<T>::type U;
    return std::unique_ptr<T>(new U[n]());
}

template<class T, class... Args>
typename Unique_if<T>::Known_bound
make_unique(Args&&...) = delete;

} // namespace Utils


// Various utilities.
namespace Utils
{

// accessing
template<typename T, int N>
constexpr int array_size(T(& array)[N]);

// querying
bool filesystem_entity_exists(const std::string& path);
bool has_only_whitespace(const std::string& s);
bool is_ascii_letter(char c);
bool is_octal_digit(char c);
bool starts_with(const std::string& s, const std::string& prefix);

// printing
void print_verbose_message(std::ostream& os, const std::string& message);

// converting
std::string char_to_string(char c);
int digit_to_int(char c);
int hex_digit_to_int(char c);
std::string quoted(const std::string& s);
std::vector<std::string> split_into_lines(const std::string& s);
template<typename UnsignedIntegralType>
bool string_to_unsigned(const std::string& s, UnsignedIntegralType* number);
template<typename UnsignedIntegralType>
bool string_to_unsigned(const char* s, UnsignedIntegralType* number);
template<typename T>
std::string to_string(T x);

// modifying
bool skip(std::istream& is, const std::string& s);

} // namespace Utils


// accessing

// Return the number of elements in array[].
//
// Example use:
//
//     int integers[] = { 0, 1, 2 };
//     ArraySize(integers); // returns 3
//
// Note that erroneously passing a pointer instead of an array to this
// function causes a compilation error. For example:
//
//     int* integers;
//     // allocate and populate 'integers'
//     ArraySize(integers); // compilation error
template<typename T, int N>
constexpr int
Utils::array_size(T(&/*array*/)[N])
{
    return N;
}

// converting

// If success, return in 'number' the number whose string representation
// is exactly 's'. If failure, the content of '*number' is undefined.
//
// Extra characters in 's' (either at the beginning or at the end of
// 's'), make this function fail.
//
// Return boolean success.
template<typename UnsignedIntegralType>
bool
Utils::string_to_unsigned(const std::string& s, UnsignedIntegralType* number)
{
    return string_to_unsigned(s.c_str(), number);
}

// See Utils::string_to_unsigned() with a 'const string&' parameter.
template<typename UnsignedIntegralType>
bool
Utils::string_to_unsigned(const char* s, UnsignedIntegralType* number)
{
    static_assert(std::is_unsigned<UnsignedIntegralType>::value,
                  "type is not unsigned");
    static_assert(std::is_integral<UnsignedIntegralType>::value,
                  "type is not integral");

    // The empty string must be handled separately, because strtoull()
    // does not flag this case as an error.
    if (s[0] == '\0')
    {
        return false;
    }

    if (std::isspace(s[0]))
    {
        // strtoull() allows initial white space (by skipping it), but
        // this function does not.
        return false;
    }

    // Contrary to how I understand the specification of strtoull(), the
    // glibc implementation of that function does not set 'errno' to
    // 'ERANGE' when 's' represents a negative number. Hence this
    // explicit check.
    if (std::strchr(s, '-') != nullptr)
    {
        return false;
    }

    errno = 0;

    char* ptr;
    const unsigned long long int i = strtoull(s, &ptr, 10);

    if (errno == ERANGE)
    {
        // The converted number is outside the representable range.
        return false;
    }

    if (*ptr != '\0')
    {
        // Extra input after the converted number.
        return false;
    }

    if (i > std::numeric_limits<UnsignedIntegralType>::max())
    {
        // The converted 'unsigned long long int' does not fit in an
        // 'UnsignedIntegralType'.
        return false;
    }

    *number = static_cast<UnsignedIntegralType>(i);
    return true;
}

// std::to_string() is not available in Cygwin:
//
//     https://sourceware.org/ml/cygwin/2015-01/msg00245.html
//
// so we provide Utils::to_string() as a replacement.
template<typename T>
std::string
Utils::to_string(T x)
{
    std::ostringstream oss;
    oss << x;
    return oss.str();
}


#endif // UTILS_HPP
