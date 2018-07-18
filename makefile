# Copyright 2017 Nest Labs, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# Configure the compiler.
CPPSTD ?= -std=c++11
# Other options: -std=c++0x -stdlib=libstdc++

# Sometimes it's worth trying -nostdinc++ to see what leaked.
LITE_CONFIG ?= -DBUILD_FEATURE_DETECTORGRAPH_CONFIG_LITE -fno-exceptions -fno-rtti -Os
FULL_CONFIG ?=

# Enables std::static_asserts for checking library usage patterns.
LITE_CONFIG += -DBUILD_FEATURE_DETECTORGRAPH_CONFIG_STATIC_ASSERTS -DBUILD_FEATURE_DETECTORGRAPH_CONFIG_PERFECT_FORWARDING -DBUILD_FEATURE_DETECTORGRAPH_CONFIG_NO_64BIT_REMAINDER

# To use the lite version of the library in the examples swap the config below
CONFIG ?= $(FULL_CONFIG)
# CONFIG=$(LITE_CONFIG)

# Enables a bunch of debug logs that help understand Graph and TimeoutPublisherService resource usage.
# CONFIG += -DBUILD_FEATURE_DETECTORGRAPH_CONFIG_INSTRUMENT_RESOURCE_USAGE

FLAGS=-Wall -Werror -Wno-error=deprecated -Werror=sign-compare

# The core library will work fine without C++11 but some examples rely on it
# CPPSTD=-stdlib=libstdc++

# Core Library Include folder and sources
CORE_INCLUDE=./include
CORE_SRCS=src/graph.cpp \
     src/detector.cpp \
     src/timeoutpublisherservice.cpp \
     $(NULL)

# TODO(DGRAPH-10): TimeoutPublisherService on lite.
FULL_SRCS=$(CORE_SRCS) \
	src/statesnapshot.cpp \
	src/graphstatestore.cpp \
	$(NULL)


# Platform-specific headers and implementations
PLATFORM=./platform_standalone
PLATFORM_SRCS=$(PLATFORM)/dglogging.cpp

# Graph Analysis and Tools
UTIL=./util
UTIL_SRCS=$(UTIL)/graphanalyzer.cpp \
          $(UTIL)/nodenameutils.cpp \
          $(NULL)

# Test Utilities
TEST_UTIL=./test-util
TEST_UTIL_SRCS=$(TEST_UTIL)/testtimeoutpublisherservice.cpp \
               $(TEST_UTIL)/graphtestutils.cpp \
               $(NULL)

# Unit Test Framework
NLUNITTEST=./third_party/nltest/repo/src/
NLUNITTEST_SRCS=$(NLUNITTEST)/nltest.c

COMMON_TESTS=./unit-test/common
FULL_TESTS=./unit-test/full
LITE_TESTS=./unit-test/lite
COMMON_TESTS_SRCS=$(wildcard $(COMMON_TESTS)/*.cpp)
FULL_TESTS_SRCS=$(wildcard $(FULL_TESTS)/*.cpp)
LITE_TESTS_SRCS=$(wildcard $(LITE_TESTS)/*.cpp)

.PHONY: docs
docs:
	doxygen ./doxygen/Doxyfile

unit-test/test_all: unit-test/test_full unit-test/test_lite
	@echo Ran unit tests for the Vanilla and Lite configs of the library

unit-test/test_full:
	g++ $(CPPSTD) $(FLAGS) $(FULL_CONFIG) -g -I$(CORE_INCLUDE) -I$(PLATFORM) -I$(UTIL) -I$(TEST_UTIL) -I$(NLUNITTEST) -I$(COMMON_TESTS) -I$(FULL_TESTS) $(FULL_SRCS) $(PLATFORM_SRCS) $(UTIL_SRCS) $(TEST_UTIL_SRCS) $(NLUNITTEST_SRCS) $(COMMON_TESTS_SRCS) $(FULL_TESTS_SRCS) -o test_full.out && ./test_full.out

unit-test/test_lite:
	g++ $(CPPSTD) $(FLAGS) $(LITE_CONFIG) -g -I$(CORE_INCLUDE) -I$(PLATFORM) -I$(TEST_UTIL) -I$(NLUNITTEST) -I$(COMMON_TESTS) -I$(LITE_TESTS) $(CORE_SRCS) $(PLATFORM_SRCS) $(TEST_UTIL_SRCS) $(NLUNITTEST_SRCS) $(COMMON_TESTS_SRCS) $(LITE_TESTS_SRCS) -o test_lite.out && ./test_lite.out

examples/helloworld:
	# Minimal Include & Sources dependencies
	g++ $(CPPSTD) $(FLAGS) $(CONFIG) -g -I$(CORE_INCLUDE) -I$(PLATFORM) $(FULL_SRCS) $(PLATFORM_SRCS) examples/helloworld.cpp -o helloworld.out && ./helloworld.out

examples/robotlocalization:
	# Note that this example depends on the Eigen library. For more info see
	# https://eigen.tuxfamily.org/dox/GettingStarted.html
	# If Eigen is not installed in your default include path you may need to
	# add it (e.g. -I/usr/include/eigen3 ).
	# Additionally, some versions of Eigen3 throw a bunch of warnings when
	# compiling so you may need to turn off -Werror in FLAGS at the top of this
	# file.
	g++ $(CPPSTD) $(FLAGS) $(CONFIG) -g -I$(CORE_INCLUDE) -I$(PLATFORM) -I$(UTIL) $(FULL_SRCS) $(PLATFORM_SRCS) $(UTIL_SRCS) examples/robotlocalization.cpp -o robotlocalization.out && ./robotlocalization.out

examples/%:
	# General Purpose Example building rule.
	g++ $(CPPSTD) $(FLAGS) $(CONFIG) -g -I$(CORE_INCLUDE) -I$(PLATFORM) -I$(UTIL) $(FULL_SRCS) $(PLATFORM_SRCS) $(UTIL_SRCS) $@.cpp -o $(@:examples/%=%.out) && ./$(@:examples/%=%.out)

examples/all: $(basename $(wildcard examples/*.cpp))
	@echo Built and Ran all Examples

unit-test/test_coverage: cleancoverage
	g++ $(CPPSTD) $(FLAGS) $(CONFIG) --coverage -g -I$(CORE_INCLUDE) -I$(PLATFORM) -I$(UTIL) -I$(TEST_UTIL) -I$(NLUNITTEST) -I$(COMMON_TESTS) -I$(FULL_TESTS) $(FULL_SRCS) $(PLATFORM_SRCS) $(UTIL_SRCS) $(TEST_UTIL_SRCS) $(NLUNITTEST_SRCS) $(COMMON_TESTS_SRCS) $(FULL_TESTS_SRCS) -o test_coverage && ./test_coverage
	mkdir -p coverage/
	lcov --capture --directory . --no-external \
         -q --output-file coverage/coverage.info
	lcov --remove coverage/coverage.info "*/third_party/*" \
         -q --output-file coverage/coverage.info
	lcov --extract coverage/coverage.info "*/src/*" \
         --extract coverage/coverage.info "*/include/*" \
         --extract coverage/coverage.info "*/unit-test/*" \
         --extract coverage/coverage.info "*/test-util/*" \
         -q --output-file coverage/coverage.info
	genhtml coverage/coverage.info --output-directory coverage/.

code_size_benchmark/%:
	g++ $(CPPSTD) $(FLAGS) -DNDEBUG $(LITE_CONFIG) -I$(CORE_INCLUDE) -I$(PLATFORM) -I$(dir $@) $(CORE_SRCS) $(PLATFORM_SRCS) $@.cpp -o $(@:code_size_benchmark/%/main=%.out) \
	&& ./$(@:code_size_benchmark/%/main=%.out) \
	&& size $(@:code_size_benchmark/%/main=%.out) \
	&& objdump -h $(@:code_size_benchmark/%/main=%.out)

code_size_benchmark/all: $(basename $(wildcard code_size_benchmark/*/main.cpp))
	@echo Built and Ran all Benchmarks

all: unit-test/test_all docs examples/all unit-test/test_coverage

cleandocs:
	rm -rf ./docs/html

cleancoverage:
	rm -fr *.gcda *.gcno coverage

clean: cleancoverage cleandocs
	rm -rf *.out* test_all* test_coverage* *.gcda *.gcno *.o coverage
