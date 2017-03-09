# Copyright (c) 2016 Antoine Trux
#
# The original version is available at
# http://solving-regular-expression-crosswords.blogspot.com
#
# Permission is hereby granted, free of charge, to any person obtaining
# a copy of this software and associated documentation files (the
# "Software"), to deal in the Software without restriction, including
# without limitation the rights to use, copy, modify, merge, publish,
# distribute, sublicense, and/or sell copies of the Software, and to
# permit persons to whom the Software is furnished to do so, subject to
# the following conditions:
#
# The above copyright notice, the above original version notice, and
# this permission notice shall be included in all copies or substantial
# portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
# IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
# CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
# TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
# SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.


# I chose to implement a traditional Makefile instead of using a
# Makefile generator (such as CMake) in order not to require the end
# user to have the same Makefile generator installed. I concede that
# this is a debatable decision.
#
# Usage:
#
#     $ make help


###############################
# general settings and checks #
###############################

ifeq ($(findstring Linux,$(shell uname)),Linux)
    OS=linux
else
ifeq ($(findstring CYGWIN,$(shell uname)),CYGWIN)
    OS=cygwin
else
ifeq ($(findstring Darwin,$(shell uname)),Darwin)
    OS=darwin
else
    $(error unsupported environment)
endif
endif
endif

# This function takes two string parameters.
#
# If the strings are equal, the string is returned, otherwise an empty
# string is returned.
str_eq = $(and $(findstring $(1),$(2)),$(findstring $(2),$(1)))

# used in shell scripts
EXIT_ON_ERROR = set -e


#################################
# settings and checks for BUILD #
#################################

# default build - can be overridden in command line
BUILD = release

# This function takes 1, 2, 3 or 4 parameters.
#
# If 'BUILD' is equal to one of the arguments, return 'T', otherwise
# return 'F'.
is_build = $(if $(or $(call str_eq,$(BUILD),$(1)), \
                     $(call str_eq,$(BUILD),$(2)), \
                     $(call str_eq,$(BUILD),$(3)), \
                     $(call str_eq,$(BUILD),$(4))),T,F)

ifeq ($(call is_build,coverage,debug,profile,release),F)
    $(error invalid value for BUILD: '$(BUILD)')
endif


###############################
# settings and checks for CXX #
###############################

# default compiler / linker
CXX = g++

# If 'CXX' is [path/]g++[<version>], return 'T', otherwise return 'F'.
is_gcc = $(if $(and $(findstring g++,$(CXX)), \
                    $(if $(findstring clang++,$(CXX)),,not_clang)),T,F)

# If 'CXX' is [path/]clang++[<version>], return 'T', otherwise return 'F'.
is_clang = $(if $(findstring clang++,$(CXX)),T,F)

# If 'CXX' has a valid value, return 'T', otherwise return 'F'.
is_valid_cxx = $(if $(or $(call str_eq,$(call is_gcc),T), \
                         $(call str_eq,$(call is_clang),T)),T,F)

ifeq ($(call is_valid_cxx),F)
    $(error invalid value for CXX: '$(CXX)')
endif


#################################
# settings and checks for DEVEL #
#################################

# default
DEVEL = 0

ifneq ($(DEVEL),0)
ifneq ($(DEVEL),1)
    $(error invalid value for DEVEL: '$(DEVEL)')
endif
endif

# default verbosity - can be overridden in command line
V = 0

# If 'V' has a valid value, return 'T', otherwise return 'F'.
is_valid_v = $(if $(or $(call str_eq,$(V),0), \
                       $(call str_eq,$(V),1)),T,F)

ifeq ($(call is_valid_v),F)
    $(error invalid value for V: '$(V)')
endif

# When $(Q) is prepended to shell scripts:
# * if 'V' is 0, the script is not echoed (non verbose mode),
# * if 'V' is 1, the script is echoed (verbose mode).
Q_0 = @
Q_1 =
Q = $(Q_$(V))


#####################################
# other general settings and checks #
#####################################

# LTO = Link-Time Optimization
#
# I was unable to get LTO to work in Cygwin, or with clang.
USE_LTO = F
ifeq ($(call is_build,release),T)
ifeq ($(call is_gcc),T)
ifeq ($(OS),linux)
    USE_LTO = T
else
ifeq ($(OS),darwin)
    USE_LTO = T
endif
endif
endif
endif

# We set up directories with absolute paths, so this Makefile can be
# invoked from another directory than the one in which it resides.
# For example:
#
#     $ make -f source/Makefile

THIS_MAKEFILE := $(abspath $(lastword $(MAKEFILE_LIST)))
# This 'patsubst' command removes any trailing slash.
SOURCE_DIR := $(patsubst %/,%,$(dir $(THIS_MAKEFILE)))

MAIN_DIR  := $(abspath $(SOURCE_DIR)/..)
BUILD_DIR := $(abspath $(MAIN_DIR)/build.$(CXX).$(BUILD))
GTEST_DIR := $(abspath $(MAIN_DIR)/3rd_party/gtest-1.7.0)


################
# default rule #
################

# The first rule which appears in a Makefile is the default one.
.PHONY: build_all
build_all: build_solver build_unit_tests build_fuzz_tests


#########
# usage #
#########

.PHONY: help
help:
	@echo
	@echo Targets:
	@echo
	@echo "    build_all (default) = next three targets"
	@echo
	@echo "    build_solver"
	@echo
	@echo "    build_unit_tests"
	@echo
	@echo "    build_fuzz_tests"
	@echo
	@echo "    unit_tests"
	@echo "        runs the unit tests"
	@echo
	@echo "    unit_tests_valgrind"
	@echo "        runs the unit tests with Valgrind"
	@echo
	@echo "    grid_tests"
	@echo "        runs all the grid tests"
	@echo
	@echo "    grid_tests_valgrind"
	@echo "        runs all the grid tests with Valgrind"
	@echo
	@echo "    <grid test name>"
	@echo "        runs test <grid test name>"
	@echo "        to list the grid tests: 'ls ../grid_tests/*.input.txt',"
	@echo -n "        and remove '.input.txt' from file names to get "
	@echo         "<grid test name>"
	@echo "        example: 'make MIT'"
	@echo
	@echo "    <grid test name>_valgrind"
	@echo "        runs test <grid test name> with Valgrind"
	@echo "        example: 'make MIT_valgrind'"
	@echo
	@echo "    check"
	@echo "        executes all the test targets, without and with Valgrind"
	@echo
	@echo "    check_without_valgrind"
	@echo "        executes all the test targets, without Valgrind"
	@echo
	@echo "    check_with_valgrind"
	@echo "        executes all the test targets, with Valgrind"
	@echo
	@echo "    coverage_report"
	@echo "        generates coverage report for unit tests and grid tests"
	@echo "        this target is only available with 'BUILD=coverage' and"
	@echo "        with gcc"
	@echo "        example: 'make BUILD=coverage coverage_report'"
	@echo
	@echo "    clean"
	@echo "        removes all the files generated by this Makefile"
	@echo
	@echo Variables:
	@echo
	@echo "    BUILD - one of: 'release' (default), 'debug'"
	@echo "        example: 'make BUILD=debug'"
	@echo
	@echo -n "    CXX - one of: '[path/]g++[<version>]' (default), "
	@echo        "'[path/]clang++[<version>]'"
	@echo
	@echo "    DEVEL - one of: '0' (default), '1'"
	@echo "        if DEVEL is 1, warnings are treated as errors"
	@echo "        example: 'make DEVEL=1 build_solver'"
	@echo
	@echo "    V - one of: '0' (default, non verbose), '1' (verbose)"
	@echo "        example: 'make V=1 build_solver'"
	@echo


###############
# compilation #
###############

SOLVER_SOURCE_DIR     = $(SOURCE_DIR)/solver
UNIT_TESTS_SOURCE_DIR = $(SOURCE_DIR)/unit_tests
FUZZ_TESTS_SOURCE_DIR = $(SOURCE_DIR)/fuzz_tests

SOLVER_SOURCES_NOT_MAIN  =
SOLVER_SOURCES_NOT_MAIN += alphabet.cpp
SOLVER_SOURCES_NOT_MAIN += backreference_numbers.cpp
SOLVER_SOURCES_NOT_MAIN += character_block.cpp
SOLVER_SOURCES_NOT_MAIN += command_line.cpp
SOLVER_SOURCES_NOT_MAIN += constraint.cpp
SOLVER_SOURCES_NOT_MAIN += grid.cpp
SOLVER_SOURCES_NOT_MAIN += grid_cell.cpp
SOLVER_SOURCES_NOT_MAIN += grid_line.cpp
SOLVER_SOURCES_NOT_MAIN += grid_line_regex.cpp
SOLVER_SOURCES_NOT_MAIN += grid_printer.cpp
SOLVER_SOURCES_NOT_MAIN += grid_reader.cpp
SOLVER_SOURCES_NOT_MAIN += group_number.cpp
SOLVER_SOURCES_NOT_MAIN += hexagonal_grid.cpp
SOLVER_SOURCES_NOT_MAIN += hexagonal_grid_printer.cpp
SOLVER_SOURCES_NOT_MAIN += logger.cpp
SOLVER_SOURCES_NOT_MAIN += rectangular_grid.cpp
SOLVER_SOURCES_NOT_MAIN += rectangular_grid_printer.cpp
SOLVER_SOURCES_NOT_MAIN += regex.cpp
SOLVER_SOURCES_NOT_MAIN += regex_crossword_solver_exception.cpp
SOLVER_SOURCES_NOT_MAIN += regex_optimizations.cpp
SOLVER_SOURCES_NOT_MAIN += regex_parser.cpp
SOLVER_SOURCES_NOT_MAIN += regex_token.cpp
SOLVER_SOURCES_NOT_MAIN += regex_tokenizer.cpp
SOLVER_SOURCES_NOT_MAIN += repetition_count.cpp
SOLVER_SOURCES_NOT_MAIN += set_of_characters.cpp
SOLVER_SOURCES_NOT_MAIN += utils.cpp

SOLVER_SOURCES_NOT_MAIN := \
    $(addprefix $(SOLVER_SOURCE_DIR)/,$(SOLVER_SOURCES_NOT_MAIN))

SOLVER_SOURCES_MAIN += $(SOLVER_SOURCE_DIR)/main.cpp

SOLVER_SOURCES = $(SOLVER_SOURCES_MAIN) $(SOLVER_SOURCES_NOT_MAIN)

SOLVER_OBJECTS  = $(subst $(SOLVER_SOURCE_DIR)/,$(BUILD_DIR)/,$(SOLVER_SOURCES))
SOLVER_OBJECTS := $(subst .cpp,.o,$(SOLVER_OBJECTS))

SOLVER_OBJECTS_NOT_MAIN = $(patsubst %main.o,,$(SOLVER_OBJECTS))

# The unit tests are executed in the order they are listed (at least
# with gcc).
UNIT_TESTS_SOURCES  =
UNIT_TESTS_SOURCES += regex_crossword_solver_test.cpp
UNIT_TESTS_SOURCES += grid.unit_tests.utils.cpp
UNIT_TESTS_SOURCES += utils.unit_tests.cpp
UNIT_TESTS_SOURCES += group_number.unit_tests.cpp
UNIT_TESTS_SOURCES += set_of_characters.unit_tests.cpp
UNIT_TESTS_SOURCES += alphabet.unit_tests.cpp
UNIT_TESTS_SOURCES += command_line.unit_tests.cpp
UNIT_TESTS_SOURCES += regex_crossword_solver_exception.unit_tests.cpp
UNIT_TESTS_SOURCES += repetition_count.unit_tests.cpp
UNIT_TESTS_SOURCES += constraint.unit_tests.cpp
UNIT_TESTS_SOURCES += regex_token.unit_tests.cpp
UNIT_TESTS_SOURCES += regex_tokenizer.unit_tests.cpp
UNIT_TESTS_SOURCES += character_block.unit_tests.cpp
UNIT_TESTS_SOURCES += regex_parser.unit_tests.cpp
UNIT_TESTS_SOURCES += regex.unit_tests.cpp
UNIT_TESTS_SOURCES += regex.constrain.unit_tests.cpp
UNIT_TESTS_SOURCES += rectangular_grid.unit_tests.cpp
UNIT_TESTS_SOURCES += hexagonal_grid.unit_tests.cpp
UNIT_TESTS_SOURCES += grid_reader.unit_tests.cpp
UNIT_TESTS_SOURCES += hexagonal_grid_printer.unit_tests.cpp
UNIT_TESTS_SOURCES += rectangular_grid_printer.unit_tests.cpp

UNIT_TESTS_SOURCES := \
    $(addprefix $(UNIT_TESTS_SOURCE_DIR)/,$(UNIT_TESTS_SOURCES))

UNIT_TESTS_OBJECTS = \
    $(subst $(UNIT_TESTS_SOURCE_DIR)/,$(BUILD_DIR)/,$(UNIT_TESTS_SOURCES))
UNIT_TESTS_OBJECTS := $(subst .cpp,.o,$(UNIT_TESTS_OBJECTS))

# These two modules are part of gtest.
#
# We follow the advice of
# https://github.com/google/googletest/blob/master/googletest/docs/FAQ.md#why-is-it-not-recommended-to-install-a-pre-compiled-copy-of-google-test-for-example-into-usrlocal
#
# That is, we do not use a precompiled version of gtest. Instead, we
# compile gtest with compilation flags as close as possible to those
# used for compiling the other modules - see below.
UNIT_TESTS_OBJECTS += $(BUILD_DIR)/gtest-all.o
UNIT_TESTS_OBJECTS += $(BUILD_DIR)/gtest_main.o

FUZZ_TESTS_OBJECTS = $(BUILD_DIR)/fuzz_tests.o


# preprocessor flags

CPPFLAGS =

# If the headers in '$(GTEST_DIR)/include' are not marked as system
# headers, g++ reports many warnings about them.
CPPFLAGS += -isystem $(GTEST_DIR)/include
CPPFLAGS += -I $(SOLVER_SOURCE_DIR)

ifeq ($(call is_build,profile,release),T)
    # Warning '-Wunused-result' is enabled if:
    # 1. macro '_FORTIFY_SOURCE' is (pre)defined (to 1 or higher)
    # and:
    # 2. optimizations are enabled
    HAS_FORTIFY := $(shell g++ -dM -E - < /dev/null | grep _FORTIFY_SOURCE)
ifndef HAS_FORTIFY
    CPPFLAGS += -D_FORTIFY_SOURCE=2
endif
endif

# Assertions skrew up profiling, so we disable assertions also in that
# build.
ifeq ($(call is_build,profile,release),T)
    CPPFLAGS += -DNDEBUG
endif

# Without '-D__STRICT_ANSI__', clang fails to compile the unit tests
# on Linux. With '-D__STRICT_ANSI__', clang fails to compile the unit
# tests on Cygwin.

USE_STRICT_ANSI = F
ifeq ($(call is_clang),T)
ifeq ($(OS),linux)
    USE_STRICT_ANSI = T
endif
endif

CPPFLAGS_UNIT_TESTS = $(CPPFLAGS)
ifeq ($(USE_STRICT_ANSI),T)
    CPPFLAGS_UNIT_TESTS += -D__STRICT_ANSI__
endif

CPPFLAGS_GTEST  = $(CPPFLAGS)
CPPFLAGS_GTEST += -I $(GTEST_DIR)
ifeq ($(USE_STRICT_ANSI),T)
    CPPFLAGS_GTEST += -D__STRICT_ANSI__
endif


# compiler flags

CXXFLAGS =
# dialect options
CXXFLAGS += -std=c++11
# C++ specific options
CXXFLAGS += -ffor-scope
CXXFLAGS += -Wctor-dtor-privacy
CXXFLAGS += -Wnon-virtual-dtor
ifeq ($(OS),darwin)
    # Don't add flags
else ifeq ($(call is_gcc),T)
    CXXFLAGS += -fno-nonansi-builtins
    CXXFLAGS += -Wnoexcept
    CXXFLAGS += -Wstrict-null-sentinel
endif
CXXFLAGS += -Wold-style-cast
# warning options
ifeq ($(DEVEL),1)
    CXXFLAGS += -Werror
endif
CXXFLAGS += -pedantic-errors
CXXFLAGS += -Wall
CXXFLAGS += -Wextra
CXXFLAGS += -Wformat=2
CXXFLAGS += -Winit-self
CXXFLAGS += -Wmissing-include-dirs
CXXFLAGS += -Wswitch-default
CXXFLAGS += -Wunused-parameter
CXXFLAGS += -Wunknown-pragmas
CXXFLAGS += -Wstrict-aliasing
CXXFLAGS += -Wundef
CXXFLAGS += -Wpointer-arith
CXXFLAGS += -Wcast-qual
CXXFLAGS += -Wcast-align
CXXFLAGS += -Wwrite-strings
CXXFLAGS += -Wconversion
CXXFLAGS += -Wsign-conversion
CXXFLAGS += -Wmissing-declarations
CXXFLAGS += -Wpacked
CXXFLAGS += -Wredundant-decls
CXXFLAGS += -Winvalid-pch
CXXFLAGS += -Wvariadic-macros
CXXFLAGS += -Wvla
CXXFLAGS += -Wdisabled-optimization
CXXFLAGS += -Woverlength-strings
ifeq ($(OS),darwin)
    CXXFLAGS += -Wdouble-promotion
    CXXFLAGS += -Wunused-local-typedefs
    CXXFLAGS += -Wno-sign-conversion
else ifeq ($(call is_gcc),T)
    CXXFLAGS += -Wdouble-promotion
    CXXFLAGS += -Wsync-nand
    CXXFLAGS += -Wunused-local-typedefs
    CXXFLAGS += -Wzero-as-null-pointer-constant
    CXXFLAGS += -Wlogical-op
endif
# Option '-Wshadow' is too aggressive in gcc, but not in clang, so we
# enable it only in clang. For details, see
# https://gcc.gnu.org/bugzilla/show_bug.cgi?id=57709
ifeq ($(call is_clang),T)
    CXXFLAGS += -Wshadow
endif

ifeq ($(call is_build,debug),T)
ifeq ($(call is_gcc),T)
    CXXFLAGS += -ftrapv
endif
endif

ifeq ($(call is_build,coverage,debug,profile),T)
    CXXFLAGS += -g
endif

ifeq ($(call is_build,profile,release),T)
    CXXFLAGS += -O3
else
    CXXFLAGS += -O0
endif

ifeq ($(USE_LTO),T)
    CXXFLAGS += -flto
endif

ifeq ($(call is_build,profile),T)
    # So we can examine the stack.
    CXXFLAGS += -fno-omit-frame-pointer
endif

ifeq ($(call is_build,coverage),T)
    CXXFLAGS += --coverage
endif

# CXXFLAGS_UNIT_TESTS and CXXFLAGS_GTEST use '-std=gnu++11' instead of
# '-std=c++11'. This is because some gtest headers cause compilation
# errors when compiled with option '-std=c++11' in Cygwin. Option
# '-std=gnu++11' fixes the problem.


CXXFLAGS_UNIT_TESTS = $(CXXFLAGS)
CXXFLAGS_UNIT_TESTS += -std=gnu++11

CXXFLAGS_GTEST = $(CXXFLAGS)
# dialect options
CXXFLAGS_GTEST += -std=gnu++11
# C++ specific options
CXXFLAGS_GTEST += -Wno-old-style-cast
# warning options
CXXFLAGS_GTEST += -Wno-format-nonliteral
CXXFLAGS_GTEST += -Wno-undef
CXXFLAGS_GTEST += -Wno-conversion
CXXFLAGS_GTEST += -Wno-sign-conversion
CXXFLAGS_GTEST += -Wno-missing-declarations
CXXFLAGS_GTEST += -Wno-missing-field-initializers
ifeq ($(OS),darwin)
    # Don't add flags
else ifeq ($(call is_gcc),T)
    CXXFLAGS_GTEST += -Wno-zero-as-null-pointer-constant
endif


# For details on how header dependencies are handled, see
# http://make.mad-scientist.net/papers/advanced-auto-dependency-generation

TARGET_BASE_NAME = $(basename $(@F))
D_FILE = $(BUILD_DIR)/$(TARGET_BASE_NAME).d
P_FILE = $(BUILD_DIR)/$(TARGET_BASE_NAME).P

# For an explanation of 'ADD_EMPTY_RULES', see
# http://make.mad-scientist.net/papers/advanced-auto-dependency-generation/#norule
#
# Note that '\#' has to be used instead of '#', because otherwise GNU Make
# would interpret '#' as the start of a comment.
ADD_EMPTY_RULES = sed -e 's/\#.*//'      \
                      -e 's/^[^:]*: *//' \
                      -e 's/ *\\$$//'    \
                      -e '/^$$/ d'       \
                      -e 's/$$/ :/'      \
                      <  $(D_FILE)       \
                      >> $(P_FILE)

vpath %.cpp $(SOLVER_SOURCE_DIR)
vpath %.cpp $(FUZZ_TESTS_SOURCE_DIR)

$(BUILD_DIR)/%.o: %.cpp $(THIS_MAKEFILE) | $(BUILD_DIR)
	@echo "    compiling $<"
	$(Q)$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) -MMD $< -o $@
	@cp $(D_FILE) $(P_FILE)
	@$(ADD_EMPTY_RULES)
	@rm -f $(D_FILE)

$(BUILD_DIR)/%.o: $(UNIT_TESTS_SOURCE_DIR)/%.cpp $(THIS_MAKEFILE) | $(BUILD_DIR)
	@echo "    compiling $<"
	$(Q)$(CXX) -c $(CPPFLAGS_UNIT_TESTS) $(CXXFLAGS_UNIT_TESTS) -MMD $< \
                   -o $@
	@cp $(D_FILE) $(P_FILE)
	@$(ADD_EMPTY_RULES)
	@rm -f $(D_FILE)

-include $(SOLVER_OBJECTS:.o=.P)
-include $(UNIT_TESTS_OBJECTS:.o=.P)
-include $(FUZZ_TESTS_OBJECTS:.o=.P)

# Header dependencies are not handled for gtest modules, because they
# are external components.
$(BUILD_DIR)/%.o: $(GTEST_DIR)/src/%.cc $(THIS_MAKEFILE) | $(BUILD_DIR)
	@echo "    compiling $<"
	$(Q)$(CXX) -c $(CPPFLAGS_GTEST) $(CXXFLAGS_GTEST) $< -o $@

$(BUILD_DIR):
	@echo "    creating $@"
	$(Q)mkdir $(BUILD_DIR)


###########
# linking #
###########

# linker flags

ifeq ($(OS),darwin)
    LDFLAGS += -Wl
else
    LDFLAGS += -Wl,--fatal-warnings
endif

ifeq ($(call is_build,coverage,debug,profile),T)
    LDFLAGS += -g
endif

ifeq ($(call is_build,coverage),T)
    LDFLAGS += --coverage
endif

ifeq ($(USE_LTO),T)
    LDFLAGS += -O3
    LDFLAGS += -flto
endif


SOLVER     = $(BUILD_DIR)/regex_crossword_solver
UNIT_TESTS = $(BUILD_DIR)/regex_crossword_solver_unit_tests
FUZZ_TESTS = $(BUILD_DIR)/regex_crossword_solver_fuzz_tests

.PHONY: build_solver
build_solver: $(SOLVER)

$(SOLVER): $(SOLVER_OBJECTS) $(THIS_MAKEFILE) | $(BUILD_DIR)
	@echo "    linking -> $@"
	$(Q)$(CXX) $(LDFLAGS) -o $@ $(SOLVER_OBJECTS)

.PHONY: build_unit_tests
build_unit_tests: $(UNIT_TESTS)

$(UNIT_TESTS): $(SOLVER_OBJECTS_NOT_MAIN) $(UNIT_TESTS_OBJECTS) \
               $(THIS_MAKEFILE) | $(BUILD_DIR)
	@echo "    linking -> $@"
	$(Q)$(CXX) $(LDFLAGS) -o $@ \
                   $(SOLVER_OBJECTS_NOT_MAIN) $(UNIT_TESTS_OBJECTS) -lpthread

.PHONY: build_fuzz_tests
build_fuzz_tests: $(FUZZ_TESTS)

$(FUZZ_TESTS): $(SOLVER_OBJECTS_NOT_MAIN) $(FUZZ_TESTS_OBJECTS) \
               $(THIS_MAKEFILE) | $(BUILD_DIR)
	@echo "    linking -> $@"
	$(Q)$(CXX) $(LDFLAGS) -o $@ \
                   $(SOLVER_OBJECTS_NOT_MAIN) $(FUZZ_TESTS_OBJECTS)


##############
# unit tests #
##############

# unit tests without Valgrind

.PHONY: unit_tests
unit_tests: $(UNIT_TESTS)
	@echo "    executing $@"
	$(Q)$(UNIT_TESTS)


# unit tests with Valgrind

# A Valgrind report is considered clean if:
# 1. it contains VALGRIND_REPORT_EXPECTED_STRING_1
# and:
# 2. it contains VALGRIND_REPORT_EXPECTED_STRING_2, or it contains
#    all of VALGRIND_REPORT_EXPECTED_STRING_3_a, ..._3_d
#
# Ideally, a Valgrind report contains
# VALGRIND_REPORT_EXPECTED_STRING_1 and
# VALGRIND_REPORT_EXPECTED_STRING_2. However, the libstdc++ version I
# use generates "still reachable:..." Valgrind warnings, hence the
# VALGRIND_REPORT_EXPECTED_STRING_3_x checks.
#
# For details on the "still reachable:..." Valgrind warnings, see, for
# example:
# http://stackoverflow.com/questions/32933301/valgrind-and-stdunique-ptr-false-positive-or-not

VALGRIND_REPORT_EXPECTED_STRING_1 = \
    "ERROR SUMMARY: 0 errors from 0 contexts"
VALGRIND_REPORT_EXPECTED_STRING_2 = \
    "All heap blocks were freed -- no leaks are possible"
VALGRIND_REPORT_EXPECTED_STRING_3_a = "definitely lost: 0 bytes in 0 blocks"
VALGRIND_REPORT_EXPECTED_STRING_3_b = "indirectly lost: 0 bytes in 0 blocks"
VALGRIND_REPORT_EXPECTED_STRING_3_c = "possibly lost: 0 bytes in 0 blocks"
VALGRIND_REPORT_EXPECTED_STRING_3_d = "suppressed: 0 bytes in 0 blocks"

HAS_VALGRIND := $(shell which valgrind 2> /dev/null)

VALGRIND_NOT_FOUND_MESSAGE = \
    'valgrind' was not found, so skipping target '$@'

.PHONY: unit_tests_valgrind
ifdef HAS_VALGRIND
unit_tests_valgrind: $(UNIT_TESTS)
	@echo "    executing $@"
	$(Q)$(EXIT_ON_ERROR);                                                 \
        valgrind_report=$(BUILD_DIR)/valgrind_report.unit_tests.txt;          \
        valgrind --leak-check=full --num-callers=50 --show-reachable=yes      \
                 $(UNIT_TESTS) > /dev/null 2> $${valgrind_report};            \
        check_expected_string()                                               \
        {                                                                     \
            string="$$1";                                                     \
                                                                              \
            ok=1;                                                             \
            grep "$${string}" $${report} > /dev/null || ok=0;                 \
            if [ $${ok} != 1 ] && [ $${error_if_missing} = 1 ];               \
            then                                                              \
                echo                                               1>&2;      \
                echo "ERROR: '$${string}' missing from $${report}" 1>&2;      \
                echo                                               1>&2;      \
            fi                                                                \
        };                                                                    \
        check_valgrind_report()                                               \
        {                                                                     \
            report=$$1;                                                       \
                                                                              \
            error_if_missing=1;                                               \
            check_expected_string $(VALGRIND_REPORT_EXPECTED_STRING_1);       \
            if [ $${ok} != 1 ]; then exit 1; fi;                              \
                                                                              \
            error_if_missing=0;                                               \
            check_expected_string $(VALGRIND_REPORT_EXPECTED_STRING_2);       \
            if [ $${ok} != 1 ];                                               \
            then                                                              \
                error_if_missing=1;                                           \
                                                                              \
                check_expected_string $(VALGRIND_REPORT_EXPECTED_STRING_3_a); \
                if [ $${ok} != 1 ]; then exit 1; fi;                          \
                                                                              \
                check_expected_string $(VALGRIND_REPORT_EXPECTED_STRING_3_b); \
                if [ $${ok} != 1 ]; then exit 1; fi;                          \
                                                                              \
                check_expected_string $(VALGRIND_REPORT_EXPECTED_STRING_3_c); \
                if [ $${ok} != 1 ]; then exit 1; fi;                          \
                                                                              \
                check_expected_string $(VALGRIND_REPORT_EXPECTED_STRING_3_d); \
                if [ $${ok} != 1 ]; then exit 1; fi;                          \
            fi;                                                               \
        };                                                                    \
        check_valgrind_report $${valgrind_report};                            \
	rm $${valgrind_report}
else
unit_tests_valgrind:
	@echo "    $(VALGRIND_NOT_FOUND_MESSAGE)"
endif


##############
# grid tests #
##############

# grid tests without Valgrind

GRID_TESTS_DIR = $(abspath $(MAIN_DIR)/grid_tests)

GRID_TEST_INPUT_FILEPATHS = $(wildcard $(GRID_TESTS_DIR)/*.input.txt)
GRID_TEST_INPUT_FILENAMES = $(notdir $(GRID_TEST_INPUT_FILEPATHS))
GRID_TEST_NAMES = $(basename $(GRID_TEST_INPUT_FILENAMES))
GRID_TEST_NAMES := $(sort $(basename $(GRID_TEST_NAMES)))

define grid_test_template
.PHONY: $(1)
$(1): $(SOLVER)
	@echo "    testing $$@"
	$(Q)$(EXIT_ON_ERROR);                                       \
        input=$(GRID_TESTS_DIR)/$(1).input.txt;                     \
        output=$(BUILD_DIR)/$(1).output.txt;                        \
        output_expected=$(GRID_TESTS_DIR)/$(1).output_expected.txt; \
        $(SOLVER) $$$${input} > $$$${output};                       \
	diff $$$${output_expected} $$$${output};                    \
	rm $$$${output}
endef

$(foreach test_name,$(GRID_TEST_NAMES),\
          $(eval $(call grid_test_template,$(test_name))))

.PHONY: grid_tests
grid_tests: $(GRID_TEST_NAMES)


# grid tests with Valgrind

VALGRIND_NOT_FOUND_MESSAGE_in_template = \
    'valgrind' was not found, so skipping target '$$@'

define grid_test_valgrind_template
.PHONY: $(1)
$(1): $(SOLVER)
ifdef HAS_VALGRIND
	@echo "    testing $$@"
	$(Q)$(EXIT_ON_ERROR);                                                 \
        rule_name=$(1);                                                       \
        test_name=$$$${rule_name%_valgrind};                                  \
	valgrind_report=$(BUILD_DIR)/$$$${test_name}.valgrind_report.txt;     \
        valgrind --leak-check=full --num-callers=50 --show-reachable=yes      \
                 $(SOLVER) $(GRID_TESTS_DIR)/$$$${test_name}.input.txt >      \
                 /dev/null 2> $$$${valgrind_report};                          \
        check_expected_string()                                               \
        {                                                                     \
            string="$$$$1";                                                   \
                                                                              \
            ok=1;                                                             \
            grep "$$$${string}" $$$${report} > /dev/null || ok=0;             \
            if [ $$$${ok} != 1 ] && [ $$$${error_if_missing} = 1 ];           \
            then                                                              \
                echo                                                   1>&2;  \
                echo "ERROR: '$$$${string}' missing from $$$${report}" 1>&2;  \
                echo                                                   1>&2;  \
            fi                                                                \
        };                                                                    \
        check_valgrind_report()                                               \
        {                                                                     \
            report=$$$$1;                                                     \
                                                                              \
            error_if_missing=1;                                               \
            check_expected_string $(VALGRIND_REPORT_EXPECTED_STRING_1);       \
            if [ $$$${ok} != 1 ]; then exit 1; fi;                            \
                                                                              \
            error_if_missing=0;                                               \
            check_expected_string $(VALGRIND_REPORT_EXPECTED_STRING_2);       \
            if [ $$$${ok} != 1 ];                                             \
            then                                                              \
                error_if_missing=1;                                           \
                                                                              \
                check_expected_string $(VALGRIND_REPORT_EXPECTED_STRING_3_a); \
                if [ $$$${ok} != 1 ]; then exit 1; fi;                        \
                                                                              \
                check_expected_string $(VALGRIND_REPORT_EXPECTED_STRING_3_b); \
                if [ $$$${ok} != 1 ]; then exit 1; fi;                        \
                                                                              \
                check_expected_string $(VALGRIND_REPORT_EXPECTED_STRING_3_c); \
                if [ $$$${ok} != 1 ]; then exit 1; fi;                        \
                                                                              \
                check_expected_string $(VALGRIND_REPORT_EXPECTED_STRING_3_d); \
                if [ $$$${ok} != 1 ]; then exit 1; fi;                        \
            fi;                                                               \
        };                                                                    \
        check_valgrind_report $$$${valgrind_report};                          \
	rm $$$${valgrind_report}
else
	@echo "    $(VALGRIND_NOT_FOUND_MESSAGE_in_template)"
endif
endef

GRID_TEST_NAMES_VALGRIND = $(addsuffix _valgrind,$(GRID_TEST_NAMES))

$(foreach test_name,$(GRID_TEST_NAMES_VALGRIND),\
          $(eval $(call grid_test_valgrind_template,$(test_name))))

.PHONY: grid_tests_valgrind
ifdef HAS_VALGRIND
grid_tests_valgrind: $(GRID_TEST_NAMES_VALGRIND)
else
grid_tests_valgrind:
	@echo "    $(VALGRIND_NOT_FOUND_MESSAGE)"
endif


#############
# all tests #
#############

.PHONY: check
check: check_without_valgrind check_with_valgrind

.PHONY: check_without_valgrind
check_without_valgrind: unit_tests grid_tests

.PHONY: check_with_valgrind
check_with_valgrind: unit_tests_valgrind grid_tests_valgrind


#####################
# coverage analysis #
#####################

ifeq ($(call is_build,coverage),T)
ifeq ($(call is_clang),F)

GCOVR_PATH = $(abspath $(MAIN_DIR)/3rd_party/gcovr-3.2)

.PHONY: coverage_report
coverage_report: restore_logger_header
	@echo "    generating coverage report"
	$(Q)$(EXIT_ON_ERROR);                                      \
        cd $(MAIN_DIR);                                            \
        report=$(BUILD_DIR)/coverage.html;                         \
        $(GCOVR_PATH) --root=. --output=$${report}                 \
                      --exclude=.*gtest.* --exclude=.*unit_tests.* \
                      --html --html-details;                       \
        echo "    coverage report available at $${report}"

LOGGER_HEADER       = $(SOLVER_SOURCE_DIR)/logger.hpp
LOGGER_HEADER_TMP   = $(LOGGER_HEADER).tmp
LOGGER_HEADER_SAVED = $(LOGGER_HEADER).saved

# The following targets are executed in the order they are listed.

.PHONY: save_logger_header
save_logger_header:
	@echo "    $@"
	$(Q)cp -p $(LOGGER_HEADER) $(LOGGER_HEADER_SAVED)

# Some code is exercised only when ENABLE_LOGGING is switched on, so
# we set that macro to 1.
.PHONY: enable_logging
enable_logging: save_logger_header
	@echo "    $@"
	$(Q)cp -p $(LOGGER_HEADER) $(LOGGER_HEADER_SAVED);                 \
        sed -e 's/#define ENABLE_LOGGING (0)/#define ENABLE_LOGGING (1)/g' \
               $(LOGGER_HEADER) > $(LOGGER_HEADER_TMP);                    \
        diff $(LOGGER_HEADER) $(LOGGER_HEADER_TMP) > /dev/null;            \
        if [ $$? = 0 ];                                                    \
        then                                                               \
            rm -f $(LOGGER_HEADER_TMP);                                    \
        else                                                               \
            mv $(LOGGER_HEADER_TMP) $(LOGGER_HEADER);                      \
        fi

.PHONY: build_for_coverage
build_for_coverage: enable_logging
	@echo "    $@"
	$(Q)$(MAKE) -C $(SOURCE_DIR) build_solver build_unit_tests

.PHONY: clean_coverage_files
clean_coverage_files: build_for_coverage
	@echo "    $@"
	$(Q)rm -f $(BUILD_DIR)/*.gcda $(BUILD_DIR)/*.html

.PHONY: unit_tests_for_coverage
unit_tests_for_coverage: clean_coverage_files
	@echo "    executing $@"
	$(Q)$(UNIT_TESTS)

# Note that the tests must not be run in parallel here, because each
# test accesses the same set of files, where statistics are
# accumulated (these files have extension .gcda).
.PHONY: grid_tests_for_coverage
grid_tests_for_coverage: unit_tests_for_coverage
	$(Q)for grid_test_name in $(GRID_TEST_NAMES);                \
        do                                                           \
            input=$(GRID_TESTS_DIR)/$${grid_test_name}.input.txt;    \
            output=$(BUILD_DIR)/$${grid_test_name}.output.txt;       \
            output_expected=$(GRID_TESTS_DIR)/$${grid_test_name};    \
            output_expected=$${output_expected}.output_expected.txt; \
            echo "    testing $${grid_test_name}";                   \
            $(SOLVER) $${input} > $${output};                        \
	    diff $${output_expected} $${output};                     \
            if [ $$? != 0 ];                                         \
            then                                                     \
	        mv $(LOGGER_HEADER_SAVED) $(LOGGER_HEADER);          \
                exit 1;                                              \
            fi;                                                      \
            rm $${output};                                           \
        done

.PHONY: restore_logger_header
restore_logger_header: grid_tests_for_coverage
	@echo "    $@"
	$(Q)mv $(LOGGER_HEADER_SAVED) $(LOGGER_HEADER)

endif
endif


############
# cleaning #
############

.PHONY: clean
clean:
	@echo "    cleaning"
	$(Q)rm -fr $(BUILD_DIR)
