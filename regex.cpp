// Example how to use C++11 <regex> and Boost.Regex

#include <iostream>

/******************************************************************************/
// use std::regex to find a date in a string

#include <regex>

void std_regex() {
    std::string str = "C++ Meetup on 2018-09-12 about String Parsing";

    // simple regex match: "on ####-##-##"
    std::regex re1("on ([0-9]{4}-[0-9]{2}-[0-9]{2})");

    if (std::regex_search(str, re1)) {
        std::cout << "std::regex_search() with re1: matched!" << std::endl;
    }
    else {
        std::cout << "std::regex_search() with re1: no match!" << std::endl;
    }

    if (std::regex_match(str, re1)) {
        std::cout << "std::regex_match() with re1: matched!" << std::endl;
    }
    else {
        std::cout << "std::regex_match() with re1: no match!" << std::endl;
    }

    // regex match and std::string captures
    std::smatch match;

    if (std::regex_search(str, match, re1)) {
        std::cout << "std::regex_search() with re1: matched!" << std::endl
                  << "  match.size() = " << match.size() << std::endl
                  << "  match[0] = " << match[0] << std::endl
                  << "  match[1] = " << match[1] << std::endl;
    }
    else {
        std::cout << "std::regex_search() with re1: no match!" << std::endl;
    }

    // Also: std::cmatch for const char* captures
    const char* cstr = "Hello on 2018-09-13";
    std::cmatch cmatch;

    if (std::regex_search(cstr, cmatch, re1)) {
        std::cout << "std::regex_search() with re1: matched!" << std::endl
                  << "  match.size() = " << cmatch.size() << std::endl
                  << "  match[0] = " << cmatch[0] << std::endl
                  << "  match[1] = " << cmatch[1] << std::endl;
    }
    else {
        std::cout << "std::regex_search() with re1: no match!" << std::endl;
    }

    // use regex_replace and construct a new string
    std::string result = std::regex_replace(str, re1, "TODAY");
    std::cout << "std::regex_replace() result = " << result << std::endl;
}

/******************************************************************************/
// alternative: use Boost.Regex

#include <boost/regex.hpp>

void boost_regex() {
    std::string str = "C++ Meetup on 2018-09-12 about String Parsing";

    // simple regex match
    boost::regex re1("on ([0-9]{4}-[0-9]{2}-[0-9]{2})");

    if (boost::regex_search(str, re1)) {
        std::cout << "boost::regex_search() with re1: matched!" << std::endl;
    }
    else {
        std::cout << "boost::regex_search() with re1: no match!" << std::endl;
    }

    if (boost::regex_match(str, re1)) {
        std::cout << "boost::regex_match() with re1: matched!" << std::endl;
    }
    else {
        std::cout << "boost::regex_match() with re1: no match!" << std::endl;
    }

    // regex match and std::string captures
    boost::smatch match;

    if (boost::regex_search(str, match, re1)) {
        std::cout << "boost::regex_search() with re1: matched!" << std::endl
                  << "  match.size() = " << match.size() << std::endl
                  << "  match[0] = " << match[0] << std::endl
                  << "  match[1] = " << match[1] << std::endl;
    }
    else {
        std::cout << "boost::regex_search() with re1: no match!" << std::endl;
    }

    // also: boost::cmatch for const char* captures, and regex_replace.
}

/******************************************************************************/
// Note: I usually make a "compatibility" include which defines

#if (__cplusplus >= 201103L)

using std::regex_search;
// ... and other symbols

#else

// or import from Boost
using boost::regex_search;

#endif

/******************************************************************************/

int main()
{
    std_regex();
    boost_regex();

    return 0;
}

/******************************************************************************/
