                       Regex Crossword Solver
                           version 1.0.0


Contents
--------
01. What is it?
02. Where to get it?
03. Requirements
04. How to build it?
04.a. How to build it with gcc? (Linux, Cygwin)
04.b. How to build it with clang? (Linux, Cygwin)
04.c. How to build it with Visual Studio? (Windows)
04.d. Difference of speed between Release and Debug builds
05. How to install it?
06. How to use it?
07. How to follow the program's progress?
08. How does it work?
09. Known bugs
10. Limitations
10.a. Supported crossword geometries
10.b. Supported regular expressions
11. License
12. Author
13. Contacts


01. What is it?
---------------
Regex Crossword Solver is a program which can solve regular expression
crosswords, such as the MIT puzzle
http://www.mit.edu/~puzzle/2013/coinheist.com/rubik/a_regular_crossword/grid.pdf
and some of the ones available at https://regexcrossword.com.


02. Where to get it?
--------------------
Regex Crossword Solver is available at
http://solving-regular-expression-crosswords.blogspot.com.


03. Requirements
----------------
Regex Crossword Solver is available only as source code, no binaries
are provided, so you need a compiler to build it.

Regex Crossword Solver can be built with these compilers:
* gcc - in Linux and Cygwin
* clang - in Linux and Cygwin
* Visual Studio - in Windows

I have built and tested Regex Crossword Solver with these combinations:
* Linux (64-bit Kubuntu): gcc 4.7.4, gcc 5.2.1, clang 3.6.2
* Cygwin (32-bit): gcc 4.9.3, clang 3.5.2
* Windows 7 (64-bit): Visual Studio Community 2015 (Update 2)

Clang has some limitations compared to gcc:
* One unit test had to be disabled - search contents of directory
  'source/unit_tests' for "__clang__" for details.
* I was unable to get Link-Time Optimization to work with clang. As a
  result, the program is about 7.3% slower than when built with gcc.
* Coverage reports are not available.

Cygwin and Visual Studio also have limitations:
* Valgrind is not available for Cygwin and Windows, so the Valgrind
  checks do not apply in those environments.
* Coverage reports are not available for Visual Studio.

That said, the program built with clang or Visual Studio can solve the
same crosswords as when built with gcc.

The program is written in standard C++11, so *in principle* any
compiler which complies with that standard should work.

Note that I was unable to build the program in Visual Studio 2013.
For example, that version does not support the C++11 construct
'constexpr' (used in a few places).


04. How to build it?
--------------------
Unzip the downloaded file. The instructions then depend on your
compiler:

04.a. How to build it with gcc? (Linux, Cygwin)
-----------------------------------------------
$ cd /path_to_unzipped_package/source
$ make

The program should now be:
/path_to_unzipped_package/build.g++.release/regex_crossword_solver

Then, to check the build:
$ make check_without_valgrind

If you are patient, you can also check the build with Valgrind
(Linux only):
$ make check_with_valgrind

For more options:
$ make help

04.b. How to build it with clang? (Linux, Cygwin)
-------------------------------------------------
$ cd /path_to_unzipped_package/source
$ make CXX=clang++

The program should now be:
/path_to_unzipped_package/build.clang++.release/regex_crossword_solver

Then, to check the build:
$ make CXX=clang++ check_without_valgrind

If you are patient, you can also check the build with Valgrind
(Linux only):
$ make CXX=clang++ check_with_valgrind

For more options:
$ make help

04.c. How to build it with Visual Studio? (Windows)
---------------------------------------------------
Open Visual Studio solution
\path_to_unzipped_package\VisualStudio2015\regex_crossword_solver.sln

In Visual Studio:
* change the configuration from "Debug" to "Release",
* in menu "Build", select "Build Solution".

The program should now be:
\path_to_unzipped_package\VisualStudio2015\regex_crossword_solver\release\regex_crossword_solver.exe

Then, to check the build:
* open a Command Prompt
* in the Command Prompt:
  cd \path_to_unzipped_package\VisualStudio2015
  move run_grid_tests.rename_extension_to_cmd run_grid_tests.cmd
  run_grid_tests.cmd

Note that the unit tests are automatically run when project
'regex_crossword_solver_unit_tests' is built in Visual Studio.

04.d. Difference of speed between Release and Debug builds
----------------------------------------------------------
The Release build is much faster than the Debug build. For example,
when built with gcc in Linux, the program solves the MIT puzzle 21
times faster (as measured by Callgrind) when built in Release mode
than in Debug mode.

Such a large difference can be explained by the following factors:
* the Release build uses full optimizations, the Debug build uses
  none;
* the program makes liberal use of assertions - assertions are enabled
  in the Debug build, disabled in the Release build;
* the program's functions are small on average - the Release build
  inlines small functions, the Debug build does not.


05. How to install it?
----------------------
No installation is needed. The program can be used as such, for
example in the directory in which it was built.


06. How to use it?
------------------
Regex Crossword Solver has no GUI. Only a command line interface is
provided.

Basic usage:

[Linux, Cygwin]
$ cd /path_to_unzipped_package/build.<compiler>.release
$ ./regex_crossword_solver <input file>

[Visual Studio]
* open a Command Prompt
* in the Command Prompt:
  cd \path_to_unzipped_package\VisualStudio2015\regex_crossword_solver\release
  regex_crossword_solver.exe <input file>

For input file examples, see directory
/path_to_unzipped_package/grid_tests, which contains many examples.

For more advanced usage, start the program with option '--help'.


07. How to follow the program's progress?
-----------------------------------------
In order to see how the program works its way to a solution, you can
use option '--log'. In order to use this option:
* set ENABLE_LOGGING, defined in 'source/logger.hpp', to 1,
* rebuild the program.

Then, for example:

[Linux, Cygwin]
$ cd /path_to_unzipped_package/build.<compiler>.release
$ ./regex_crossword_solver --log MIT.log ../grid_tests/MIT.input.txt

[Visual Studio]
* open a Command Prompt
* in the Command Prompt:
  cd \path_to_unzipped_package\VisualStudio2015\regex_crossword_solver\release
  regex_crossword_solver.exe --log MIT.log ..\..\..\grid_tests\MIT.input.txt


08. How does it work?
---------------------
Here is a brief overview of how the program works:

1. The command line is parsed.
   => see module 'command_line'

2. The text input file provided in the command line is parsed.
   => see module 'grid_reader'

3. The grid is constructed. This involves:
   a. setting up the geometry of the grid
      => see modules 'grid', 'rectangular_grid', 'hexagonal_grid',
                     'grid_line', 'grid_cell'
   b. building the parsed regexes from the string regexes - for each regex:
      * the string regex input is split into tokens
        => see modules 'regex_token' and 'regex_tokenizer'
      * the tokens are parsed into a regex
        => see module 'regex_parser'
   c. setting the alphabet to the explicitly mentioned characters
   d. initializing the cells to universal constraints
      (i.e., constraints which contain all the alphabet characters)

4. The regexes are optimized.
   => see modules 'grid', 'grid_line', 'regex'

5. The grid is solved.
   => see modules 'grid', 'grid_line', 'regex'

The core of the program resides in method Regex::constrain() and its
callees.

Module 'regex' implements a regex engine specially designed for
solving regex crosswords, as I found no regex library suitable for
that purpose.


09. Known bugs
--------------
No *known* bugs at the time of this writing.


10. Limitations
---------------

10.a. Supported crossword geometries
------------------------------------
Regex Crossword Solver only supports:
* crossword grids of regular hexagonal shape (all sides of equal length),
* crossword grids of rectangular shape.

Regex Crossword Solver supports one or several regexes per line:
* for crossword grids of regular hexagonal shape, the number of
  regexes must be the same for all the lines,
* for crossword grids of rectangular shape, the number of regexes must
  be the same for all rows, and the same for all columns, but can be
  different for rows and columns.

Crosswords with different numbers of regexes for different lines in a
hexagonal grid, or for different rows/columns in a rectangular grid
are not supported as such. It is easy, however, to work around this
limitation, by adding '.*' (a "universal regex") to the crossword.

    For example, if the crossword has two rows, one with one regex,
    the other with two regexes:
    * row 1 -> regex_1
    * row 2 -> regex_1, regex_2
    You can add '.*' to row 1:
    * row 1 -> regex_1, .*
    * row 2 -> regex_1, regex_2
    after which both rows have the same number of regexes.

10.b. Supported regular expressions
-----------------------------------
There are *many* regular expression standards, and they differ in
small details. I have used Python 3's specification of regular
expressions as my reference, because Python is widely used, and
because Python's regular expression specification is fairly well
documented.

The latest version of Python at the time of this writing is
Python 3.5, and the regular expression specification for that version
is available at https://docs.python.org/3.5/library/re.html

In the rare cases where I found the specification was not clear
enough, I checked the implementation's behavior, with deprecation
warnings enabled:

$ python3.5
>>> import warnings
>>> warnings.simplefilter('always', DeprecationWarning)

and I treated warnings as errors.

Here is a (possibly non-exhaustive) list of regular expression
features supported by Python but not by Regex Crossword Solver:

* {,n} - treated the same as {0,n} by Python

* \uhhhh - 16-bit unicode character

* \Uhhhhhhhh - 32-bit unicode character

* all the (? constructs - many puzzles in
  'https://regexcrossword.com/playerpuzzles' use these constructs,
  and are thus not solvable by Regex Crossword Solver

Here is a (probably non-exhaustive) list of regular expression
features which behave differently in Python and in Regex Crossword
Solver:

* Python supports backreference numbers up to 99, Regex Crossword
  Solver supports backreference numbers only up to 9.

* In Python, '\s' means "any whitespace character", in Regex Crossword
  Solver it only means space. If Regex Crossword Solver treated '\s'
  as any whitespace character, some puzzles would have thousands of
  solutions instead of one.

* Python accepts constructs such as 'a{ 1,2}' (note the space after
  the opening brace), where this means string "a{ 1,2}", not "a repeated
  once or twice". Regex Crossword Solver rejects such constructs.

* Python does not accept nested repetitions - for example: 'a**'.
  Regex Crossword Solver does.

* Python does not accept regular expressions such as '*'. Regex
  Crossword Solver does (it interprets it as 'epsilon repeated any
  number of times'). In my opinion, this is consistent with the fact
  that regular expressions such as '|' are accepted (both by Python
  and by Regex Crossword Solver).


11. License
-----------
See /path_to_unzipped_package/LICENSE.txt.


12. Author
----------
Antoine Trux


13. Contacts
------------
http://solving-regular-expression-crosswords.blogspot.com
or:
author's email: firstname.lastname@gmail.com
