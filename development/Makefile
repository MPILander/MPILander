#CXX		 = g++-12
CXX		 = clang++
CXXFLAGS	 = -std=c++20
CXXFLAGS	+= -g -Wall -Wextra -Wno-unused
CXXFLAGS	+= -I../include

all: message-queue-test

%: %.cc
	${CXX} ${CXXFLAGS} $< -o $@

clean:
	rm -f message-queue-test
	rm -f *.o
	rm -rf *.dSYM
