CXX		 = g++-8
CXXFLAGS	 = -std=c++17
CXXFLAGS	+= -g -Wall -Wextra -Wno-unused

all: message-queue-test

%: %.cc
	${CXX} ${CXXFLAGS} $< -o $@

clean:
	rm -f message-queue-test
	rm -f *.o
	rm -rf *.dSYM
