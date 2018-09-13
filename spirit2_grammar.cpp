// Example how to use Boost Spirit to parse simple arithmetic expressions such
// as "1 + 2 * 3".
//
// test1() parses and accepts "1"
// test2() parses "1" and returns it in an integer variable.
// test3() parses "1+2*3" but only accepts it without calculating.
// test4() parses "1 + 2 * 3"
//
// Evaluation of the expression is added in spirit3_arithmetic.cpp

#include <iostream>
#include <iomanip>
#include <stdexcept>

#include <boost/spirit/include/qi.hpp>

namespace qi = boost::spirit::qi;

/******************************************************************************/

// Helper to run a parser, check for errors, and capture the results.
template <typename Parser, typename ... Args>
void ParseOrDie(const std::string& input, const Parser& p, Args&& ... args)
{
    std::string::const_iterator begin = input.begin(), end = input.end();
    bool ok = qi::parse(begin, end, p, std::forward<Args>(args) ...);
    if (!ok || begin != end) {
        std::cout << "Unparseable: "
                  << std::quoted(std::string(begin, end)) << std::endl;
        throw std::runtime_error("Parse error");
    }
}

/******************************************************************************/
// First grammar example: parse a single integer

class ArithmeticGrammar1 : public qi::grammar<
    // the string iterator to parse: can also be const char* or templated.
    std::string::const_iterator>
{
public:
    // string iterator to parse
    using Iterator = std::string::const_iterator;

    ArithmeticGrammar1()
        // call base constructor and specify start symbol
        : ArithmeticGrammar1::base_type(start)
    {
        // construct the grammar: just set "start" for now.
        start = qi::int_;
    }

    // List of rule objects in the grammar. Templates just like qi::grammar.
    qi::rule<Iterator> start;
};

void test1()
{
    std::string input = "12345";

    ArithmeticGrammar1 g;
    ParseOrDie(input, g);
}

/******************************************************************************/
// Modify grammar to actually return an integer

class ArithmeticGrammar2 : public qi::grammar<
    // the string iterator to parse: can also be const char* or templated.
    std::string::const_iterator,
    // return value of the grammar, written in function syntax!
    int()>
{
public:
    using Iterator = std::string::const_iterator;

    ArithmeticGrammar2() : ArithmeticGrammar2::base_type(start)
    {
        start %= qi::int_;
    }

    // List of rule objects in the grammar. Each rule can have a return type.
    qi::rule<Iterator, int()> start;
};

void test2()
{
    std::string input = "12345";
    int out_int;

    // note that the grammar object does not contain any return values.
    ParseOrDie(input, ArithmeticGrammar2(), out_int);

    std::cout << "test2() parse result: "
              << out_int << std::endl;
}

/******************************************************************************/
// Let's make the grammar more interesting.

class ArithmeticGrammar3 : public qi::grammar<std::string::const_iterator, int()>
{
public:
    using Iterator = std::string::const_iterator;

    ArithmeticGrammar3() : ArithmeticGrammar3::base_type(start)
    {
        start   = product >> *('+' >> product);
        product = factor >> *('*' >> factor);
        factor  = qi::int_ | group;
        group   = '(' >> start >> ')';
    }

    // List of rule objects in the grammar. Now there are four rules and each
    // returns an integer value.
    qi::rule<Iterator, int()> start, group, product, factor;
};

void test3()
{
    std::string input = "1+2*3";
    int out_int;

    ParseOrDie(input, ArithmeticGrammar3(), out_int);

    std::cout << "test3() parse result: "
              << out_int << std::endl;
}

/******************************************************************************/
// Introduce error checking when running the arithmetic grammar and add a skip
// parser to jump over spaces.

// Helper to run a parser, check for errors, and capture the results.
template <typename Parser, typename Skipper, typename ... Args>
void PhraseParseOrDie(
    const std::string& input, const Parser& p, const Skipper& s,
    Args&& ... args)
{
    std::string::const_iterator begin = input.begin(), end = input.end();
    boost::spirit::qi::phrase_parse(
        begin, end, p, s, std::forward<Args>(args) ...);
    if (begin != end) {
        std::cout << "Unparseable: "
                  << std::quoted(std::string(begin, end)) << std::endl;
        throw std::runtime_error("Parse error");
    }
}

class ArithmeticGrammar4 : public qi::grammar<
    // the string iterator to parse: can also be const char* or templated.
    std::string::const_iterator,
    // return value of the grammar, written in function syntax!
    int(),
    // the _type_ of the skip parser
    qi::space_type>
{
public:
    using Iterator = std::string::const_iterator;

    ArithmeticGrammar4() : ArithmeticGrammar4::base_type(start)
    {
        start   = product >> *('+' >> product);
        product = factor >> *('*' >> factor);
        factor  = qi::int_ | group;
        group   = '(' >> start >> ')';
    }

    // as before, mirrors the template arguments of qi::grammar.
    qi::rule<Iterator, int(), qi::space_type> start, group, product, factor;
};

void test4(std::string input)
{
    int out_int;

    PhraseParseOrDie(
        // input string
        input,
        // grammar
        ArithmeticGrammar4(),
        // skip parser
        qi::space,
        // output variable
        out_int);

    std::cout << "test4() parse result: "
              << out_int << std::endl;
}

/******************************************************************************/

int main(int argc, char* argv[])
{
    test1();
    test2();
    test3();
    test4(argc >= 2 ? argv[1] : "1 + 2 * 3");

    return 0;
}

/******************************************************************************/
