// Example how to use Boost Spirit to parse CSV data directly into a C++ struct
//
// This example is designed to read the file "stock_list.txt"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/fusion/include/adapt_struct.hpp>

namespace qi = boost::spirit::qi;
namespace phx = boost::phoenix;

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
// Our simple stock struct: two strings and a double.

struct Stock
{
    std::string symbol;
    std::string name;
    double price;

    // constructors
    Stock() { }
    Stock(std::string symbol, std::string name, double price)
        : symbol(symbol), name(name), price(price) { }

    // and how to format it to cout
    friend std::ostream& operator << (std::ostream& os, const Stock& s)
    {
        return os << "[Stock"
                  << " symbol=" << std::quoted(s.symbol)
                  << " name=" << std::quoted(s.name)
                  << " price=" << s.price
                  << "]";
    }
};

/******************************************************************************/
// First Grammar: use Boost Phoenix in semantic action to construct a Stock
// object with parsed parameters

class StockGrammar1 : public qi::grammar<
    // new grammar, this time the result type is a "Stock" object!
    std::string::const_iterator, Stock()>
{
public:
    using Iterator = std::string::const_iterator;

    StockGrammar1() : StockGrammar1::base_type(start)
    {
        // define name rule: returns all characters up to ';' as a string.
        name %= *(~qi::char_(';'));

        // parse a CSV line, and construct Stock object using the three symbols
        // stored as qi::_1, .. qi::_3. Optionally allow a trailing ';'.
        start = (name >> ';' >> name >> ';' >> qi::double_ >> -(qi::lit(';')))
            [qi::_val = phx::construct<Stock>(qi::_1, qi::_2, qi::_3) ];
    }

    // a helper rule which parser a name
    qi::rule<Iterator, std::string()> name;
    // rule which actually parses a CSV line containing the information
    qi::rule<Iterator, Stock()> start;
};

void test1_stream(std::istream& input)
{
    // function to read each line of input and parse it.
    std::string line;
    StockGrammar1 g;
    while (std::getline(input, line)) {
        Stock stock;
        ParseOrDie(line, g, stock);
        std::cout << stock << std::endl;
    }
}

/******************************************************************************/
// First Grammar: use Boost Fusion to instrument the Stock class and enable
// automatic semantic actions

BOOST_FUSION_ADAPT_STRUCT(
    Stock,
    (std::string, symbol)
    (std::string, name)
    (double, price)
)

class StockGrammar2
    : public qi::grammar<std::string::const_iterator, Stock()>
{
public:
    using Iterator = std::string::const_iterator;

    StockGrammar2() : StockGrammar2::base_type(start)
    {
        name %= *(~qi::char_(';'));
        // parse CSV line, and let Boost Fusion automatically map results into
        // the Stock struct (this does not use the constructor).
        start %= name >> ';' >> name >> ';' >> qi::double_ >> -(qi::lit(';'));
    }

    qi::rule<Iterator, std::string()> name;
    qi::rule<Iterator, Stock()> start;
};

void test2_stream(std::istream& input)
{
    std::string line;
    while (std::getline(input, line)) {
        Stock stock;
        ParseOrDie(line, StockGrammar2(), stock);
        std::cout << stock << std::endl;
    }
}

/******************************************************************************/

int main(int argc, char* argv[])
{
    if (argc >= 2) {
        std::ifstream in(argv[1]);
        test1_stream(in);
    }
    else {
        std::cout << "Reading stdin" << std::endl;
        test2_stream(std::cin);
    }
    return 0;
}

/******************************************************************************/
