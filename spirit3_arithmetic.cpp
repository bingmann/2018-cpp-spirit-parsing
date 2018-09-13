// Example how to use Boost Spirit to parse and _evaluate_ a simple arithmetic
// grammar. Evaluation is added by amending rules with semantic actions.

#include <iostream>
#include <iomanip>
#include <stdexcept>

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>

namespace qi = boost::spirit::qi;

/******************************************************************************/
// Arithmetic parser with semantic actions which calculate the arithmetic
// expression's result

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

class ArithmeticGrammar1 : public qi::grammar<
    std::string::const_iterator,
    // define grammar to return an integer ... which we will calculate from the
    // expression
    int(), qi::space_type>
{
public:
    using Iterator = std::string::const_iterator;

    ArithmeticGrammar1() : ArithmeticGrammar1::base_type(start)
    {
        start   =
            // first component: product, and transfer the result of product
            // (qi::_1) to the result of this rule (start, qi::_val).
            product [qi::_val = qi::_1]
            // zero or more components: add result of product (qi::_1) to the
            // result of this rule (qi::_val).
            >> *('+' >> product [qi::_val += qi::_1]);

        // product is defined in same way as start, but with multiplication
        product = factor [qi::_val = qi::_1]
            >> *('*' >> factor [qi::_val *= qi::_1]);

        // factor is either option, both return an int, and with "%=" is
        // equivalent to [qi::_val = qi::_1] in both cases.
        factor  %= qi::int_ | group;

        // group's result is identical to start. again "%=" is a shortcut
        group   %= '(' >> start >> ')';
    }

    // each rule also returns an integer
    qi::rule<Iterator, int(), qi::space_type> start, group, product, factor;
};

void test1(std::string input)
{
    int out_int;

    PhraseParseOrDie(input, ArithmeticGrammar1(), qi::space, out_int);

    std::cout << "test1() parse result: "
              << out_int << std::endl;
}

/******************************************************************************/

int main(int argc, char* argv[])
{
    test1(argc >= 2 ? argv[1] : "1 + 2 * 3");

    return 0;
}

/******************************************************************************/
