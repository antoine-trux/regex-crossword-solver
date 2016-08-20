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


#include "logger.hpp"

#include "utils.hpp"

#include <cassert>
#include <fstream>

using namespace std;


#if ENABLE_LOGGING

// static data members
string Logger::m_log_filepath = "";
size_t Logger::m_indentation_level = 0;

// accessing

string
Logger::indentation()
{
    const size_t num_spaces = m_indentation_level *
                              m_num_spaces_per_indentation_level;
    return string(num_spaces, ' ');
}

ostream&
Logger::output_stream()
{
    if (m_log_filepath == "-")
    {
        return clog;
    }
    else
    {
        static ofstream os;

        if (!os.is_open())
        {
            os.open(m_log_filepath);
        }

        return os;
    }
}

// querying

bool
Logger::is_logging_requested()
{
    return !m_log_filepath.empty();
}

// printing

void
Logger::log(ostream& os, const string& message)
{
    log(os, Utils::split_into_lines(message));
}

void
Logger::log(ostream& os, const vector<string>& message)
{
    for (const auto& line : message)
    {
        log_line(os, line);
    }
}

void
Logger::log_line(ostream& os, const string& line)
{
    os << indentation() << line << endl;
}

// modifying

void
Logger::decrement_indentation_level()
{
    assert(m_indentation_level > 0);
    --m_indentation_level;
}

void
Logger::increment_indentation_level()
{
    ++m_indentation_level;
}

void
Logger::set_log_filepath(const string& log_filepath)
{
    m_log_filepath = log_filepath;
}

#endif // ENABLE_LOGGING
