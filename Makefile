CXX			?= g++
STD			?= gnu++26
CXXFLAGS 	?= -std=$(STD) -Wall -Wextra -Wpedantic -Iinclude

.PHONY: all check clean

all: check

check:
	$(CXX) $(CXXFLAGS) -fsyntax-only test-cases/compile.cpp

clean:
	@true