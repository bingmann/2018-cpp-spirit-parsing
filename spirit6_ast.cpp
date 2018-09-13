// Example how to use Boost Spirit to construct an abstract syntax tree (AST)
// for a simple arithmetic grammar and to evaluate expressions _with_ variables!
//
// The grammar accepts expressions like "y = 1 + 2 * x", constructs an AST and
// evaluates it correctly. Non-assignment expression are also evaluated.

#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <stdexcept>

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>

namespace qi = boost::spirit::qi;
namespace phx = boost::phoenix;

/******************************************************************************/

// Utility to run a parser, check for errors, and capture the results.
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

/******************************************************************************/

// the variable value map
std::map<std::string, double> variable_map;

class ASTNode
{
public:
    virtual double evaluate() = 0;
    virtual ~ASTNode() { }
};

using ASTNodePtr = ASTNode*;

template <char Operator>
class OperatorNode : public ASTNode
{
public:
    OperatorNode(const ASTNodePtr& left, const ASTNodePtr& right)
        : left(left), right(right) { }

    double evaluate() {
        if (Operator == '+')
            return left->evaluate() + right->evaluate();
        else if (Operator == '*')
            return left->evaluate() * right->evaluate();
    }

    ~OperatorNode() {
        delete left;
        delete right;
    }

private:
    ASTNodePtr left, right;
};

class ConstantNode : public ASTNode
{
public:
    ConstantNode(double value)
        : value(value) { }

    double evaluate() {
        return value;
    }

private:
    double value;
};

class VariableNode : public ASTNode
{
public:
    VariableNode(std::string identifier)
        : identifier(identifier) { }

    double evaluate() {
        return variable_map[identifier];
    }

private:
    std::string identifier;
};

class AssignmentNode : public ASTNode
{
public:
    AssignmentNode(std::string identifier, const ASTNodePtr& value)
        : identifier(identifier), value(value) { }

    double evaluate() {
        double v = value->evaluate();
        variable_map[identifier] = v;
        return v;
    }

private:
    std::string identifier;
    ASTNodePtr value;
};

/******************************************************************************/

class ArithmeticGrammar1
    : public qi::grammar<std::string::const_iterator, ASTNodePtr(), qi::space_type>
{
public:
    using Iterator = std::string::const_iterator;

    ArithmeticGrammar1() : ArithmeticGrammar1::base_type(start)
    {
        varname %= qi::alpha >> *qi::alnum;

        start = (varname >> '=' >> term)
            [qi::_val = phx::new_<AssignmentNode>(qi::_1, qi::_2) ] |
            term [qi::_val = qi::_1];

        term = (product >> '+' >> term)
            [qi::_val = phx::new_<OperatorNode<'+'> >(qi::_1, qi::_2) ] |
            product [qi::_val = qi::_1];
        product = (factor >> '*' >> product)
            [qi::_val = phx::new_<OperatorNode<'*'> >(qi::_1, qi::_2) ] |
            factor [qi::_val = qi::_1];
        factor  = group [qi::_val = qi::_1] |
            varname [qi::_val = phx::new_<VariableNode>(qi::_1) ] |
            qi::int_ [qi::_val = phx::new_<ConstantNode>(qi::_1) ];
        group   %= '(' >> term >> ')';
    }

    qi::rule<Iterator, std::string(), qi::space_type> varname;
    qi::rule<Iterator, ASTNodePtr(), qi::space_type> start, term, group, product, factor;
};

void test1(std::string input)
{
    try {
        ASTNode* out_node;
        PhraseParseOrDie(input, ArithmeticGrammar1(), qi::space, out_node);

        std::cout << "evaluate() = " << out_node->evaluate() << std::endl;
        delete out_node;
    }
    catch (std::exception& e) {
        std::cout << "EXCEPTION: " << e.what() << std::endl;
    }
}

/******************************************************************************/

int main()
{
    // important variables
    variable_map["x"] = 42;

    std::cout << "Reading stdin" << std::endl;

    std::string line;
    while (std::getline(std::cin, line)) {
        test1(line);
    }

    return 0;
}

/******************************************************************************/
