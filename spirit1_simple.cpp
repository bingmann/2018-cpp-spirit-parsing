// Example how to use Boost Spirit to parse plain integers and lists of integers
//
// test1() parses "5",
// test2() parses "76131 Karlsruhe",
// test3() parses "[12345,42,5,]"
// test4() parses "[12345,42,5]"
// test5() parses "[12345, 42, 5 ]"

#include <iostream>
#include <iomanip>
#include <stdexcept>

#include <boost/spirit/include/qi.hpp>

namespace qi = boost::spirit::qi;

/******************************************************************************/
// First Example: parse a single integer

void test1()
{
    std::string input = "12345";
    int out_int;

    qi::parse(
        // input string (iterators)
        input.begin(), input.end(),
        // parser grammar
        qi::int_,
        // output fields
        out_int);

    std::cout << "test1() parse result: "
              << out_int << std::endl;
}

/******************************************************************************/
// Parse an integer followed by a space and a string

void test2()
{
    std::string input = "76131 Karlsruhe";
    int out_int;
    std::string out_string;

    qi::parse(
        // input string (iterators)
        input.begin(), input.end(),
        // parser grammar
        qi::int_ >> ' ' >> *qi::char_,
        // output fields
        out_int, out_string);

    std::cout << "test2() parse result: "
              << out_int << " " << out_string << std::endl;
}

/******************************************************************************/
// Parse a bracketed list of integers

void test3()
{
    std::string input = "[12345,42,5,]";
    std::vector<int> out_int_list;

    qi::parse(
        // input string (iterators)
        input.begin(), input.end(),
        // parser grammar
        '[' >> *(qi::int_ >> ',') >> ']',
        // output list
        out_int_list);

    std::cout << "test3() parse result: size "
              << out_int_list.size() << std::endl;
    for (const size_t &i : out_int_list)
        std::cout << i << std::endl;
}

/******************************************************************************/
// Parse a bracketed list of integers without last comma

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

void test4(std::string input)
{
    std::vector<int> out_int_list;

    ParseOrDie(
        // input string
        input,
        // parser grammar with '%' operator
        '[' >> (qi::int_ % ',') >> ']',
        // output list
        out_int_list);

    std::cout << "test4() parse result: size "
              << out_int_list.size() << std::endl;
    for (const size_t &i : out_int_list)
        std::cout << i << std::endl;
}

/******************************************************************************/
// Parse a bracketed list of integers with spaces between symbols

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

void test5(std::string input)
{
    std::vector<int> out_int_list;

    PhraseParseOrDie(
        // input string
        input,
        // parser grammar
        '[' >> (qi::int_ % ',') >> ']',
        // skip parser
        qi::space,
        // output list
        out_int_list);

    std::cout << "test5() parse result: size "
              << out_int_list.size() << std::endl;
    for (const size_t &i : out_int_list)
        std::cout << i << std::endl;
}

/******************************************************************************/

int main(int argc, char* argv[])
{
    test1();
    test2();
    test3();
    test4(argc >= 2 ? argv[1] : "[12345,42,5]");
    test5(argc >= 3 ? argv[2] : "[12345, 42, 5]");

    return 0;
}

/******************************************************************************/
