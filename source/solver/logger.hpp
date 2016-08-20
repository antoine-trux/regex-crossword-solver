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


#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <fstream>
#include <iostream>
#include <string>
#include <vector>


#define ENABLE_LOGGING (0)

#if ENABLE_LOGGING

#define DECREMENT_LOGGING_INDENTATION_LEVEL() \
        Logger::decrement_indentation_level()
#define INCREMENT_LOGGING_INDENTATION_LEVEL() \
        Logger::increment_indentation_level()
#define LOG(message) Logger::log(message)
#define SET_LOG_FILEPATH(log_filepath) \
        Logger::set_log_filepath(log_filepath)

#else // ENABLE_LOGGING

#define DECREMENT_LOGGING_INDENTATION_LEVEL()
#define INCREMENT_LOGGING_INDENTATION_LEVEL()
#define LOG(message) do {} while (false)
#define SET_LOG_FILEPATH(log_filepath)

#endif // ENABLE_LOGGING

#define LOG_BLANK_LINE() LOG("\n")


#if ENABLE_LOGGING

// This class is not to be used directly, but only through the above
// macros.
class Logger final
{
public:
    // printing
    template<typename T>
    static void log(const T& message);

    // modifying
    static void decrement_indentation_level();
    static void increment_indentation_level();
    static void set_log_filepath(const std::string& log_filepath);

private:
    // accessing
    static std::string indentation();
    static std::ostream& output_stream();

    // querying
    static bool is_logging_requested();

    // printing
    static void log(std::ostream& os, const std::string& message);
    static void log(std::ostream& os, const std::vector<std::string>& message);
    static void log_line(std::ostream& os, const std::string& line);

    // data members

    static const size_t m_num_spaces_per_indentation_level = 2;

    static std::string m_log_filepath;
    static size_t m_indentation_level;
};

// printing

// 'T' can be 'std::string' or 'std::vector<std::string>'.
//
// If 'T' is 'std::string', 'message' may contain several lines,
// separated by '\n'.
template<typename T>
inline
void
Logger::log(const T& message)
{
    if (is_logging_requested())
    {
        log(output_stream(), message);
    }
}

#endif // ENABLE_LOGGING


#endif // LOGGER_HPP
