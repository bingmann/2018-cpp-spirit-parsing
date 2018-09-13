# Tutorial on Parsing Structured Text with Boost Spirit in C++

This repository contains heavily commented source code showing and explaining how to use Boost.Spirit. They were presented for a C++ Meetup evening talk on 2018-09-13 in Karlsruhe, Germany.

- [regex.cpp](regex.cpp) - How to use std::regex for regular expressions.

- [spirit1_simple.cpp](spirit1_simple.cpp) - How to parse integers and lists of integers. Parses "`[12345, 5, 42 ]`" into a `std::vector<int>`.

- [spirit2_grammar.cpp](spirit2_grammar.cpp) - How to make larger grammars in Boost Spirit. Parses and accepts arithmetic expressions such as "`1 + 2 * 3`".

- [spirit3_arithmetic.cpp](spirit3_arithmetic.cpp) - How to parse arithmetic expressions using the grammar and **evaluate them with semantic actions** applied to the parser rules.

- [spirit4_struct.cpp](spirit4_struct.cpp) - How to parse data from CSV files directly into a C++ struct. This parser can read the [stock_list.txt](stock_list.txt) file.

- [spirit5_ast.cpp](spirit5_ast.cpp) - How to build an abstract syntax tree (AST) for arithmetic expressions. The AST can then be evaluated.

- [spirit6_ast.cpp](spirit6_ast.cpp) - Continues the AST example by adding variable names and assignment operations.

- [spirit7_html.cpp](spirit7_html.cpp) - Presents a stripped-down HTML Markup parser for HTML snippets which also accepts some Markdown syntax and includes template directives which can be used to call C++ functions and embed their output.

Written by Timo Bingmann (2018)
