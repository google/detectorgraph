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

CPPSTD=-std=c++11
FLAGS=-Wall -Werror -Wno-error=deprecated -Werror=sign-compare

# The core library will work fine without C++11 but some examples rely on it
# CPPSTD=-stdlib=libstdc++

# Core Library Include folder and sources
CORE_INCLUDE=./include
CORE_SRCS=src/graph.cpp \
     src/detector.cpp \
     src/graphstatestore.cpp \
     src/statesnapshot.cpp \
     src/timeoutpublisherservice.cpp

# Platform-specific headers and implementations
PLATFORM=./platform_standalone
PLATFORM_SRCS=$(PLATFORM)/dglogging.cpp

# Graph Analysis and Tools
UTIL=./util
UTIL_SRCS=$(UTIL)/graphanalyzer.cpp \
          $(UTIL)/nodenameutils.cpp

# Test Utilities
TEST_UTIL=./test-util
TEST_UTIL_SRCS=$(TEST_UTIL)/testtimeoutpublisherservice.cpp \
               $(TEST_UTIL)/graphtestutils.cpp

# Unit Test Framework
NLUNITTEST=./third_party/nltest/repo/src/
NLUNITTEST_SRCS=$(NLUNITTEST)/nltest.c

.PHONY: docs
docs:
	doxygen ./doxygen/Doxyfile

unit-test/test_all:
	g++ $(CPPSTD) $(FLAGS) -g -I$(CORE_INCLUDE) -I$(PLATFORM) -I$(UTIL) -I$(TEST_UTIL) -I$(NLUNITTEST) $(CORE_SRCS) $(PLATFORM_SRCS) $(UTIL_SRCS) $(TEST_UTIL_SRCS) $(NLUNITTEST_SRCS) unit-test/main.cpp unit-test/test_*.cpp -o test_all && ./test_all

examples/helloworld:
	# Minimal Include & Sources dependencies
	g++ $(CPPSTD) $(FLAGS) -g -I$(CORE_INCLUDE) -I$(PLATFORM) $(CORE_SRCS) $(PLATFORM_SRCS) examples/helloworld.cpp -o helloworld.out && ./helloworld.out

examples/robotlocalization:
	# Note that this example depends on the Eigen library. For more info see
	# https://eigen.tuxfamily.org/dox/GettingStarted.html
	# If Eigen is not installed in your default include path you may need to
	# add it (e.g. -I/usr/include/eigen3 ).
	# Additionally, some versions of Eigen3 throw a bunch of warnings when
	# compiling so you may need to turn off -Werror in FLAGS at the top of this
	# file.
	g++ $(CPPSTD) $(FLAGS) -g -I$(CORE_INCLUDE) -I$(PLATFORM) -I$(UTIL) $(CORE_SRCS) $(PLATFORM_SRCS) $(UTIL_SRCS) examples/robotlocalization.cpp -o robotlocalization.out && ./robotlocalization.out

examples/%:
	# General Purpose Example building rule.
	g++ $(CPPSTD) $(FLAGS) -g -I$(CORE_INCLUDE) -I$(PLATFORM) -I$(UTIL) $(CORE_SRCS) $(PLATFORM_SRCS) $(UTIL_SRCS) $@.cpp -o $(@:examples/%=%.out) && ./$(@:examples/%=%.out)

examples/all: $(basename $(wildcard examples/*.cpp))
	@echo Built and Ran all Examples

unit-test/test_coverage: cleancoverage
	g++ $(CPPSTD) $(FLAGS) --coverage -g -I$(CORE_INCLUDE) -I$(PLATFORM) -I$(UTIL) -I$(TEST_UTIL) -I$(NLUNITTEST) $(CORE_SRCS) $(PLATFORM_SRCS) $(UTIL_SRCS) $(TEST_UTIL_SRCS) $(NLUNITTEST_SRCS) unit-test/main.cpp unit-test/test_*.cpp -o test_coverage && ./test_coverage
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

all: unit-test/test_all docs examples/all unit-test/test_coverage

cleandocs:
	rm -rf ./docs/html

cleancoverage:
	rm -fr *.gcda *.gcno coverage

clean: cleancoverage cleandocs
	rm -rf *.out* test_all* test_coverage* *.gcda *.gcno *.o coverage
