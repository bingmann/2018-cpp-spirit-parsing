# really simple Makefile

CXX=g++
CXXFLAGS=-W -Wall -pedantic -std=c++14

PROGRAMS= \
    regex \
    spirit1_simple \
    spirit2_grammar \
    spirit3_arithmetic \
    spirit4_struct \
    spirit5_ast \
    spirit6_ast \
    spirit7_html

all: $(PROGRAMS)

clean:
	rm -f *.o $(PROGRAMS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

regex: regex.o
	$(CXX) $(CXXFLAGS) -o $@ $^ -lboost_regex

spirit1_simple: spirit1_simple.o
	$(CXX) $(CXXFLAGS) -o $@ $^

spirit2_grammar: spirit2_grammar.o
	$(CXX) $(CXXFLAGS) -o $@ $^

spirit3_arithmetic: spirit3_arithmetic.o
	$(CXX) $(CXXFLAGS) -o $@ $^

spirit4_struct: spirit4_struct.o
	$(CXX) $(CXXFLAGS) -o $@ $^

spirit5_ast: spirit5_ast.o
	$(CXX) $(CXXFLAGS) -o $@ $^

spirit6_ast: spirit6_ast.o
	$(CXX) $(CXXFLAGS) -o $@ $^

spirit7_html: spirit7_html.o
	$(CXX) $(CXXFLAGS) -o $@ $^
