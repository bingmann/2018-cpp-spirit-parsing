// Example how to use Boost Spirit to parse a HTML-like markup language with
// Markdown elements and enable additional instructions. This example was
// extracted from a HTML template engine, but only the AST printer is included.
//
// This example is designed to read "example.html".

#include <fstream>
#include <iostream>
#include <iomanip>
#include <stdexcept>
#include <memory>

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/variant/recursive_variant.hpp>
#include <boost/fusion/include/adapt_struct.hpp>

namespace qi = boost::spirit::qi;
namespace phx = boost::phoenix;
namespace ascii = boost::spirit::ascii;

/******************************************************************************/
// AST node structs

struct ast_null;
struct ast_comment;
struct ast_nodelist;

struct ast_func_variable;
struct ast_func_string;
struct ast_func_integer;
struct ast_func_double;
struct ast_func_call;
struct ast_func_filter;
struct ast_func_set;
struct ast_func_if;
struct ast_func_for;
struct ast_func_expr;
struct ast_func_template;

struct ast_tagged_node;
struct ast_html_node;
struct ast_html_selfnode;
struct ast_highlight;

// boost variant representing an AST node

typedef boost::variant<
    ast_null,
    std::string,
    ast_comment,
    boost::recursive_wrapper<ast_nodelist>,
    ast_func_variable,
    ast_func_string,
    ast_func_integer,
    ast_func_double,
    ast_func_template,
    boost::recursive_wrapper<ast_func_call>,
    boost::recursive_wrapper<ast_func_filter>,
    boost::recursive_wrapper<ast_func_set>,
    boost::recursive_wrapper<ast_func_if>,
    boost::recursive_wrapper<ast_func_for>,
    boost::recursive_wrapper<ast_func_expr>,
    boost::recursive_wrapper<ast_tagged_node>,
    boost::recursive_wrapper<ast_html_node>,
    boost::recursive_wrapper<ast_html_selfnode>,
    ast_highlight
    >
ast_node;

// *** Individual AST node structs

//! represent null or undefined
struct ast_null
{
};

//! a comment <# clause #>
struct ast_comment : public std::string
{
};

//! a sequence of multiple AST nodes
struct ast_nodelist : public std::vector<ast_node>
{
};

//! MyFunc node representing a variable
struct ast_func_variable : public std::string
{
};

//! MyFunc node representing a literal string
struct ast_func_string : public std::string
{
};

//! MyFunc node representing a template name
struct ast_func_template : public std::string
{
};

//! MyFunc node representing a literal integer
struct ast_func_integer
{
    long long   value;

    explicit inline ast_func_integer(const long long& v=0)
        : value(v) {}
};

BOOST_FUSION_ADAPT_STRUCT(
    ast_func_integer,
    (long long, value)
)

//! MyFunc node representing a literal double
struct ast_func_double
{
    double      value;
};

BOOST_FUSION_ADAPT_STRUCT(
    ast_func_double,
    (double, value)
)

//! tagged sequence of multiple AST nodes with HTML attributes, like <p [attr]> [nodes] </p>
struct ast_highlight
{
    std::string                 language;
    std::string                 content;
};

BOOST_FUSION_ADAPT_STRUCT(
    ast_highlight,
    (std::string, language)
    (std::string, content)
)

//! MyFunc node representing a function call with argument list
struct ast_func_call
{
    std::string         funcname;
    ast_nodelist        args;
};

BOOST_FUSION_ADAPT_STRUCT(
    ast_func_call,
    (std::string, funcname)
    (ast_nodelist, args)
)

//! MyFunc node representing a conditional clause
struct ast_func_filter
{
    ast_node            node;
    std::string         content;
};

BOOST_FUSION_ADAPT_STRUCT(
    ast_func_filter,
    (ast_node, node)
    (std::string, content)
)

//! MyFunc node representing a function call with argument list and filter content
struct ast_func_set
{
    std::string         varname;
    ast_node            value;
};

BOOST_FUSION_ADAPT_STRUCT(
    ast_func_set,
    (std::string, varname)
    (ast_node, value)
)

//! MyFunc node representing a function call with argument list and filter content
struct ast_func_if
{
    ast_node            condition, iftrue, iffalse;
};

BOOST_FUSION_ADAPT_STRUCT(
    ast_func_if,
    (ast_node, condition)
    (ast_node, iftrue)
    (ast_node, iffalse)
)

//! MyFunc node representing a function call with argument list and filter content
struct ast_func_for
{
    std::string         varname;
    ast_node            arg;
    ast_node            subtree;
};

BOOST_FUSION_ADAPT_STRUCT(
    ast_func_for,
    (std::string, varname)
    (ast_node, arg)
    (ast_node, subtree)
)

//! MyFunc node representing a sequence of expressions with operators intermingled
struct ast_func_expr : public ast_nodelist
{
};

//! tagged sequence of multiple AST nodes like <p> [nodes] </p>
struct ast_tagged_node
{
    std::string         tag;
    ast_node            subtree;
};

BOOST_FUSION_ADAPT_STRUCT(
    ast_tagged_node,
    (std::string, tag)
    (ast_node, subtree)
)

//! key-value attributes for HTML like name=value
struct ast_html_attr
{
    std::string         name;
    ast_node            value;
};

BOOST_FUSION_ADAPT_STRUCT(
    ast_html_attr,
    (std::string, name)
    (ast_node, value)
)

//! a sequence of multiple key-value attributes for HTML
struct ast_html_attrlist : public std::vector<ast_html_attr>
{
    const ast_html_attr& find(const std::string& key) const
    {
        for (const_iterator it = begin(); it != end(); ++it)
        {
            if (it->name != key) continue;
            return *it;
        }
        std::cout << "{ERROR cannot find HTML attribute " << key << "}"
                  << std::endl;
        abort();
    }
};

//! tagged sequence of multiple AST nodes with HTML attributes, like <p [attr]> [nodes] </p>
struct ast_html_node
{
    std::string         tag;
    ast_html_attrlist   attrlist;
    ast_node            subtree;

    ast_html_node() {}

    ast_html_node(const std::string& _tag, const ast_html_attr& attr, const ast_node& _subtree)
        : tag(_tag), subtree(_subtree)
    {
        attrlist.push_back(attr);
    }
};

BOOST_FUSION_ADAPT_STRUCT(
    ast_html_node,
    (std::string, tag)
    (ast_html_attrlist, attrlist)
    (ast_node, subtree)
)

//! tagged sequence of multiple AST nodes with HTML attributes, like <img [attr] />
struct ast_html_selfnode
{
    std::string         tag;
    ast_html_attrlist   attrlist;

    ast_html_selfnode() {}

    ast_html_selfnode(const std::string& _tag, const ast_html_attr& attr1)
        : tag(_tag)
    {
        attrlist.push_back(attr1);
    }

    ast_html_selfnode(const std::string& _tag, const ast_html_attr& attr1,
                      const ast_html_attr& attr2)
        : tag(_tag)
    {
        attrlist.push_back(attr1);
        attrlist.push_back(attr2);
    }
};

BOOST_FUSION_ADAPT_STRUCT(
    ast_html_selfnode,
    (std::string, tag)
    (ast_html_attrlist, attrlist)
)

/******************************************************************************/
// MyMarkup parser

struct MyMarkupParser : qi::grammar<std::string::const_iterator, ast_node()>
{
    typedef std::string::const_iterator Iterator;

    // *** General Base Character Parsers

    qi::rule<Iterator, std::string()> HtmlText, SpecialChar, PlainText;

    qi::rule<Iterator> BlankLine, Indent;

    // *** Inline Blocks with Special Formatting

    qi::rule<Iterator, ast_node()> Inline, InlinePlain;

    qi::rule<Iterator, ast_comment()> Comment, CommentBlock;

    qi::rule<Iterator, ast_tagged_node()> Code, Emph, Strong;
    qi::rule<Iterator, ast_nodelist()> CodeBlock, EmphBlock, StrongBlock;

    qi::rule<Iterator, ast_html_node()> MarkLink;
    qi::rule<Iterator, ast_nodelist()> MarkLinkText, MarkLinkRefList;
    qi::rule<Iterator, ast_html_attr()> MarkLinkRef;

    qi::rule<Iterator, ast_html_selfnode()> MarkImage;
    qi::rule<Iterator, ast_html_attr()> MarkImageAlt, MarkImageSrc;

    qi::rule<Iterator, ast_html_selfnode()> MarkDownload;
    qi::rule<Iterator, ast_html_attr()> MarkDownloadRef;

    qi::rule<Iterator, std::string()> HttpLink, SelfLink;

    qi::rule<Iterator, ast_node()> FuncBlock, FuncInline;
    qi::rule<Iterator, ast_func_filter()> FilterBlock, FilterInline;
    qi::rule<Iterator, std::string()> VerbatimBlock, VerbatimInline;

    // *** Inline HTML blocks

    qi::rule<Iterator, std::string()> HtmlTagName, HtmlComment;

    qi::rule<Iterator, ast_node()> HtmlPhrase;
    qi::rule<Iterator, ast_html_node()> HtmlTagBlock;
    qi::rule<Iterator, ast_html_selfnode()> HtmlTagSelfClose;
    qi::rule<Iterator, ast_nodelist()> HtmlInline;

    qi::rule<Iterator, ast_html_attr()> HtmlAttribute;
    qi::rule<Iterator, ast_nodelist()> HtmlQuoted;
    qi::rule<Iterator, std::string()> HtmlQuotedText;

    // *** Paragraph Blocks: Enumerations

    qi::rule<Iterator> Bullet, Enumet;

    qi::rule<Iterator, ast_tagged_node()> BulletList0, BulletList1, BulletList2;
    qi::rule<Iterator, ast_tagged_node()> OrderedList0, OrderedList1, OrderedList2;

    qi::rule<Iterator, ast_nodelist()> List0, List1, List2;

    qi::rule<Iterator, ast_tagged_node()> ListItem0, ListItem1, ListItem2;

    qi::rule<Iterator, ast_nodelist()> ListBlock0, ListBlock1, ListBlock2;
    qi::rule<Iterator, ast_nodelist()> ListBlockLine0, ListBlockLine1, ListBlockLine2;

    qi::rule<Iterator, ast_nodelist()> Line;

    // *** Paragraph Blocks: Headers

    qi::rule<Iterator, ast_tagged_node()> Header, Header1, Header2, Header3, Header4, Header5, Header6;

    qi::rule<Iterator, ast_nodelist()> HeaderA;
    qi::rule<Iterator, std::string()> HeaderAnchor;

    qi::rule<Iterator, ast_tagged_node()> Header1A, Header2A, Header3A, Header4A, Header5A, Header6A;

    // *** Source Highlighting Code Blocks

    qi::rule<Iterator, ast_highlight()> HighlightBlock;

    // *** Paragraph Blocks: Paragraphs and Plain

    qi::rule<Iterator, ast_tagged_node()> Paragraph;
    qi::rule<Iterator, ast_nodelist()> ParagraphBlock;

    qi::rule<Iterator, ast_nodelist()> InlineList;

    qi::rule<Iterator, ast_node()> Block;

    qi::rule<Iterator, ast_nodelist()> BlockList;

    qi::rule<Iterator, ast_node()> Start;

    // *** Inline Procedural Language

    typedef qi::space_type Skip;

    qi::rule<Iterator, std::string()> FIdentifier;
    qi::rule<Iterator, ast_func_variable(), Skip> FVariable;
    qi::rule<Iterator, ast_func_string()> FString;
    qi::rule<Iterator, ast_func_double(), Skip> FDouble;
    qi::rule<Iterator, ast_func_integer(), Skip> FInteger;
    qi::rule<Iterator, ast_func_call(), Skip> FCall;
    qi::rule<Iterator, ast_node(), Skip> FBracket;

    qi::rule<Iterator, ast_node(), Skip> FAtomic;
    qi::rule<Iterator, ast_func_expr(), Skip> FExpr;

    qi::rule<Iterator, ast_func_set(), Skip> FSetClause;
    qi::rule<Iterator, ast_func_if(), Skip> FIfClause, FEvalIfClause;
    qi::rule<Iterator, ast_func_for(), Skip> FForClause;
    qi::rule<Iterator, ast_func_call(), Skip> FInclude;

    qi::rule<Iterator, ast_node(), Skip> FClause;

    qi::rule<Iterator, ast_func_variable(), Skip> FFilterSetClause;
    qi::rule<Iterator, ast_func_template(), Skip> FFilterTemplateClause;
    qi::rule<Iterator, ast_node(), Skip> FFilterClause;

    // *** Construction

    MyMarkupParser();

    static const MyMarkupParser& get(); // get singleton
};

MyMarkupParser::MyMarkupParser() : base_type(Start, "MyMarkupParser")
{
    using namespace boost::spirit::ascii;
    using namespace qi::labels;

    using qi::lit;
    using qi::eoi;
    using qi::eol;
    using qi::attr;
    using qi::omit;
    using qi::as_string;

    // ********************************************************************
    // *** General Base Character Parsers

    // text is composed of non-special characters
    HtmlText =      +( char_("A-Za-z0-9~@$^.,:;_=+({}|?/-") [ _val += _1 ]
                       | lit('&')     [ _val += "&amp;" ]
                       | lit('"')     [ _val += "&quot;" ]
                       | lit('\'')    [ _val += "&apos;" ]
                       | lit('>')     [ _val += "&gt;" ]
                       | lit('\304')  [ _val += "&Auml;" ]
                       | lit('\326')  [ _val += "&Ouml;" ]
                       | lit('\334')  [ _val += "&Uuml;" ]
                       | lit('\337')  [ _val += "&szlig;" ]
                       | lit('\344')  [ _val += "&auml;" ]
                       | lit('\350')  [ _val += "&egrave;" ]
                       | lit('\351')  [ _val += "&eacute;" ]
                       | lit('\366')  [ _val += "&ouml;" ]
                       | lit('\374')  [ _val += "&uuml;" ]
                       | (+blank >> -(eol >> *blank >> !eol)) [ _val += " " ]
                       | (eol >> *blank >> !eol) [ _val += " " ]
        );

    // special characters, accepted if no special meaning
    SpecialChar =   ( char_("*`#[])!") [ _val += _1 ]
                      //| (lit('<') >> !lit("%")) [ _val += "&lt;" ]
                      | lit("\\\\")   [ _val += '\\' ]
                      | lit("\\\"")   [ _val += '"' ]
                      | lit("\\&")    [ _val += '&' ]
                      | lit("\\*")    [ _val += '*' ]
                      | lit("\\#")    [ _val += '#' ]
                      | lit("\\`")    [ _val += '`' ]
                      | lit("\\[")    [ _val += '[' ]
                      | lit("\\<")    [ _val += "&lt;" ]
                      | (lit('%') >> !lit('%')) [ _val += '%' ]
        );

    // a blank file
    BlankLine =     *blank >> eol;

    // identation for lists
    Indent =        lit('\t') | lit("  ");

    // ********************************************************************
    // *** Inline Blocks with Special Formatting

    Inline %=       Comment | VerbatimInline | FilterInline | FuncInline | Code | Strong | Emph | SelfLink | MarkDownload | MarkLink | MarkImage | HtmlPhrase | HtmlText | SpecialChar;

    InlinePlain %=  Comment | VerbatimInline | FilterInline | FuncInline | PlainText;

    // inline comments

    Comment %=      "<%#" >> *(!lit("%>") >> char_) >> "%>";

    CommentBlock %= "<%#" >> *(!lit("%>") >> char_) >> "%>" >> omit[*eol];

    // inline styling blocks

    Code %=         '`' >> attr("code") >> CodeBlock >> '`';
    CodeBlock %=    +(!lit('`') >> Inline);

    Emph %=         '*' >> attr("i") >> EmphBlock >> '*';
    EmphBlock %=    +(!lit('*') >> Inline);

    Strong %=       "**" >> attr("b") >> StrongBlock >> "**";
    StrongBlock %=  +(!lit("**") >> Inline);

    // markdown inline links

    MarkLink =      ('[' >> MarkLinkText >> "](" >> MarkLinkRef >> ')')
        [ _val = phx::construct<ast_html_node>(std::string("markdown-a"), _2, _1) ];

    MarkLinkText %= +(!lit(']') >> Inline);
    MarkLinkRef %=  attr("href") >> MarkLinkRefList;
    MarkLinkRefList %= +(!lit(')') >> Inline);

    // markdown inline images

    MarkImage =     ("![" >> MarkImageAlt >> "](" >> MarkImageSrc >> ')')
        [ _val = phx::construct<ast_html_selfnode>(std::string("markdown-img"), _1, _2) ];

    MarkImageAlt %= attr("alt") >> MarkLinkText;
    MarkImageSrc %= attr("src") >> MarkLinkRefList;

    // markdown download/view links

    MarkDownload =  "[[" >>
        MarkDownloadRef [ _val = phx::construct<ast_html_selfnode>(std::string("markdown-download"), _1) ] >>
        "]]";

    MarkDownloadRef %= attr("href") >> as_string[ +~char_(']') ];

    // self-link inline

    HttpLink %=     string("http") >> +~char_('>');

    SelfLink =      &lit("<http") >> '<' >> HttpLink
        [ _val = "<a href=\"" + _1 + "\">" + _1 + "</a>" ]  >> '>';

    // inline functional language

    FuncBlock %=    "<%" >> qi::skip(qi::space)[FClause] >> omit[*space] >> "%>" >> omit[eol];

    FuncInline %=   "<%" >> qi::skip(qi::space)[FClause] >> omit[*space] >> "%>";

    FilterBlock %=  "<%|" >> qi::skip(qi::space)[FFilterClause] >> omit[*space] >> "%>" >> omit[eol]
                          >> *(!(eol >> "<%|%>") >> char_)
                          >> omit[eol] >> "<%|%>";

    FilterInline %= "<%|" >> qi::skip(qi::space)[FFilterClause] >> omit[*space] >> "%>" >> omit[-eol]
                          >> *(!(-eol >> "<%|%>") >> char_)
                          >> omit[-eol] >> "<%|%>";

    VerbatimBlock %= "<%$" >> omit[eol] >> *(!lit("%>") >> char_) >> "%>" >> omit[eol];

    VerbatimInline %= "<%$" >> *(!lit("%>") >> char_) >> "%>";

    // ********************************************************************
    // *** Inline HTML blocks

    HtmlTagName =
        string("big") |
        string("br") |
        string("button") |
        string("caption") |
        string("code") |
        string("col") |
        string("dd") |
        string("div") |
        string("dl") |
        string("dt") |
        string("em") |
        string("form") |
        string("h1") | string("h2") | string("h3") | string("h4") | string("h5") | string("h6") |
        string("hr") |
        string("iframe") |
        string("img") |
        string("input") |
        string("li") |
        string("longversion") |
        string("object") |
        string("ol") |
        string("option") |
        string("param") |
        string("pre") |
        string("select") |
        string("script") |
        string("span") |
        string("strong") |
        string("sup") |
        string("table") |
        string("tbody") |
        string("td") |
        string("textarea") |
        string("tfoot") |
        string("thead") |
        string("tr") |
        string("tt") |
        string("ul") |
        // two letter overlap
        string("th") |
        // one letter matches
        string("a") | string("b") | string("i") | string("p")
        ;

    HtmlPhrase %=   &lit('<') >> ( HtmlTagBlock | HtmlComment | HtmlTagSelfClose );

    HtmlTagBlock %= '<' >> HtmlTagName [phx::at_c<0>(_val) = qi::_1]
                        >> *HtmlAttribute >> omit[*space] >> '>' >> omit[*eol]
                        >> HtmlInline
                        >> omit["</" > string(phx::at_c<0>(_val)) >> '>']
                        >> omit[*eol];

    HtmlInline %=   *( Inline >> omit[*eol] );

    HtmlTagSelfClose %= '<' >> HtmlTagName
                            >> *HtmlAttribute >> omit[*space] >> "/>"
                            >> omit[*eol];

    HtmlComment %=  string("<!--") >> *(!lit("-->") >> char_) >> string("-->") >> omit[*eol];

    HtmlAttribute %= omit[+space] >> +(alnum | char_('-')) >> omit[*space >> '=' >> *space] >> HtmlQuoted;

    HtmlQuotedText = +( char_("A-Za-z0-9~!@#$%^.,:;_=+*()[]{}>'|?/ -") [ _val += _1 ]
                        | (lit('<') >> !lit('%')) [ _val += '<' ]
                        | lit('&')      [ _val += "&amp;" ]
                        | lit('\304')   [ _val += "&Auml;" ]
                        | lit('\326')   [ _val += "&Ouml;" ]
                        | lit('\334')   [ _val += "&Uuml;" ]
                        | lit('\337')   [ _val += "&szlig;" ]
                        | lit('\344')   [ _val += "&auml;" ]
                        | lit('\350')   [ _val += "&egrave;" ]
                        | lit('\351')   [ _val += "&eacute;" ]
                        | lit('\366')   [ _val += "&ouml;" ]
                        | lit('\374')   [ _val += "&uuml;" ]
                        | lit("\\\"")   [ _val += '"' ]
        );

    HtmlQuoted %=   '"' >> *(!lit('"') >> (Comment | FuncInline | HtmlQuotedText)) >> '"';

    qi::on_error<qi::fail>(
        HtmlTagBlock,
        std::cout << phx::val("{debug error expecting ") << _4 << phx::val(" here: \"")
        << phx::construct<std::string>(_3, _2)   // iterators to error-pos, end
        << phx::val("\"}") << std::endl
        );

    // ********************************************************************
    // *** Paragraph Blocks: Enumerations

    Bullet =          char_("+*-") >> +blank;
    Enumet =          +digit >> '.' >> +blank;

    BulletList0 %=    &Bullet >> attr("ul") >> List0;
    OrderedList0 %=   &Enumet >> attr("ol") >> List0;

    BulletList1 %=    &(Indent >> Bullet) >> attr("ul") >> List1;
    OrderedList1 %=   &(Indent >> Enumet) >> attr("ol") >> List1;

    BulletList2 %=    &(Indent >> Indent >> Bullet) >> attr("ul") >> List2;
    OrderedList2 %=   &(Indent >> Indent >> Enumet) >> attr("ol") >> List2;

    List0 %=          +ListItem0;
    List1 %=          +ListItem1;
    List2 %=          +ListItem2;

    ListItem0 %=      omit[(Bullet | Enumet)] >> attr("li") >> ListBlock0;
    ListItem1 %=      omit[Indent >> (Bullet | Enumet)] >> attr("li") >> ListBlock1;
    ListItem2 %=      omit[Indent >> Indent >> (Bullet | Enumet)] >> attr("li") >> ListBlock2;

    ListBlock0 %=     !BlankLine >> Line >> *( BulletList1 | OrderedList1 | ListBlockLine0 );
    ListBlock1 %=     !BlankLine >> Line >> *( BulletList2 | OrderedList2 | ListBlockLine1 );
    ListBlock2 %=     !BlankLine >> Line >> *( ListBlockLine2 );

    ListBlockLine0 %= !BlankLine >> !( *Indent >> (Bullet | Enumet) )
                                 >> Indent >> attr(" ") >> Line;

    ListBlockLine1 %= !BlankLine >> !( *Indent >> (Bullet | Enumet) )
                                 >> Indent >> Indent >> attr(" ") >> Line;

    ListBlockLine2 %= !BlankLine >> !( *Indent >> (Bullet | Enumet) )
                                 >> Indent >> Indent >> Indent >> attr(" ") >> Line;

    // inline will gobble single eols, but stop at double eols.
    Line %=           +Inline >> omit[(eol >> BlankLine) | (*eol >> eoi)];

    // ********************************************************************
    // *** Paragraph Blocks: Headers

    Header6 %=        "###### " >> attr("h6") >> InlineList;
    Header5 %=        "##### "  >> attr("h5") >> InlineList;
    Header4 %=        "#### "   >> attr("h4") >> InlineList;
    Header3 %=        "### "    >> attr("h3") >> InlineList;
    Header2 %=        "## "     >> attr("h2") >> InlineList;
    Header1 %=        "# "      >> attr("h1") >> InlineList;

    HeaderAnchor =    as_string[ +~char_(')') ]
        [ _val = "<a id=\"" + _1 + "\"></a>" ];

    HeaderA %=        HeaderAnchor >> lit(") ") >> InlineList;

    Header6A %=       "######(" >> attr("h6") >> HeaderA;
    Header5A %=       "#####("  >> attr("h5") >> HeaderA;
    Header4A %=       "####("   >> attr("h4") >> HeaderA;
    Header3A %=       "###("    >> attr("h3") >> HeaderA;
    Header2A %=       "##("     >> attr("h2") >> HeaderA;
    Header1A %=       "#("      >> attr("h1") >> HeaderA;

    Header %=         &lit('#') >> ( Header6A | Header5A | Header4A | Header3A | Header2A | Header1A |
                                     Header6 | Header5 | Header4 | Header3 | Header2 | Header1 );

    // ********************************************************************
    // *** Source Highlighting Code Blocks

    HighlightBlock %= "```" >> omit[*blank] >> *print >> omit[eol]
                            >> *(!(eol >> "```") >> char_)
                            >> omit[eol] >> "```" >> omit[*blank >> eol];

    // ********************************************************************
    // *** Paragraph Blocks: Paragraphs and Plain

    Paragraph %=    attr("p") >> ParagraphBlock >> omit[+(eol | blank >> eoi)];
    ParagraphBlock %= InlineList;

    InlineList %=   +Inline;

    Block %=        omit[*BlankLine] >> (
        CommentBlock |
        VerbatimBlock | FilterBlock | FuncBlock |
        HighlightBlock |
        Header |
        BulletList0 | OrderedList0 |
        HtmlPhrase |
        Paragraph | InlineList );

    BlockList %=    *Block;

    Start %=        BlockList;

    // ********************************************************************
    // *** Inline Procedural Language

    FIdentifier %=  char_("A-Za-z_") >> *char_("A-Za-z0-9_");

    FVariable %=    FIdentifier;

    FString %=      '"' >> *(!lit('"') >> ((lit("\\\"") >> attr('"')) | char_)) >> '"';

    FDouble %=      qi::real_parser< double, qi::strict_real_policies<double> >();

    FInteger %=     qi::long_long;

    FCall %=        FIdentifier >> '(' >> -(FExpr % ',') >> ')';

    FBracket %=     '(' >> FExpr >> ')';

    FAtomic %=      FBracket | FCall | FString | FDouble | FInteger | FVariable;

    FExpr %=        FAtomic % as_string[char_("+")] [ phx::push_back(_val,_1) ];

    FSetClause %=   -lit("SET") >> FIdentifier >> '=' >> FExpr;

    FIfClause %=    "IF" >> FExpr >> "%%" >> Start >> "%%"
                         >> -(lit("ELSE") >> "%%" >> Start >> "%%")
                         >> "ENDIF";

    FEvalIfClause %= "EVALIF" >> FExpr >> "%%" >> FClause >> "%%"
                              >> -(lit("ELSE") >> "%%" >> FClause >> "%%")
                              >> "ENDIF";

    FForClause %=   "FOR" >> FIdentifier >> '=' >> FExpr >> "%%" >> Start >> "%%"
                          >> "ENDFOR";

    FInclude %=     "INCLUDE" >> attr("include") >> FIdentifier;

    FClause %=      FSetClause | FEvalIfClause | FIfClause | FForClause | FInclude | FExpr;

    FFilterSetClause %= "SET" >> FIdentifier;

    FFilterTemplateClause %= "TEMPLATE" >> FIdentifier;

    FFilterClause %= FFilterSetClause | FFilterTemplateClause | FCall;

    // ********************************************************************
}

/******************************************************************************/
// Interpret a boost::variant<> object by recursively visiting the nodes inside.

struct ast_debug : boost::static_visitor<>
{
    int depth;

    std::ostringstream oss;

    ast_debug(const ast_node& ast) : depth(0)
    {
        boost::apply_visitor(*this, ast);
    }

    inline std::string tab()
    {
        return std::string(2 * depth, ' ');
    }

    void operator()(const ast_null& )
    {
        oss << tab() << "NULL" << std::endl;
    }

    void operator()(const std::string& text)
    {
        oss << tab() << "text: \"" << text << '"' << std::endl;
    }

    void operator()(const ast_comment& text)
    {
        oss << tab() << "comment: \"" << text << '"' << std::endl;
    }

    void recurse(const ast_node& node)
    {
        ++depth;
        boost::apply_visitor(*this, node);
        --depth;
    }

    void recurse_list(const ast_nodelist& nodelist)
    {
        oss << tab() << '{' << std::endl;

        for (const ast_node& n : nodelist)
            recurse(n);

        oss << tab() << '}' << std::endl;
    }

    void operator()(const ast_nodelist& ast)
    {
        recurse_list(ast);
    }

    void operator()(const ast_tagged_node& ast)
    {
        oss << tab() << '<' << ast.tag << '>' << std::endl;
        recurse(ast.subtree);
    }

    void operator()(const ast_html_node& ast)
    {
        oss << tab() << '<' << ast.tag << '>';
        if (ast.attrlist.size())
        {
            ++depth;
            oss << " [" << std::endl;
            for (const ast_html_attr& attr : ast.attrlist)
            {
                oss << tab() << attr.name << '=' << std::endl;
                recurse(attr.value);
            }
            oss << tab() << ']';
            --depth;
        }
        oss << std::endl;

        recurse(ast.subtree);
    }

    void operator()(const ast_html_selfnode& ast)
    {
        oss << tab() << '<' << ast.tag << '>';
        if (ast.attrlist.size())
        {
            ++depth;
            oss << " [" << std::endl;
            for (const ast_html_attr& attr : ast.attrlist)
            {
                oss << tab() << attr.name << '=' << std::endl;
                recurse(attr.value);
            }
            oss << tab() << ']';
            --depth;
        }
        oss << std::endl;
    }

    void operator()(const ast_func_variable& ast)
    {
        oss << tab() << "var: " << ast << std::endl;
    }

    void operator()(const ast_func_string& ast)
    {
        oss << tab() << "string: " << ast << std::endl;
    }

    void operator()(const ast_func_integer& ast)
    {
        oss << tab() << "integer: " << ast.value << std::endl;
    }

    void operator()(const ast_func_double& ast)
    {
        oss << tab() << "double: " << ast.value << std::endl;
    }

    void operator()(const ast_func_template& ast)
    {
        oss << tab() << "template: " << ast << std::endl;
    }

    void operator()(const ast_func_call& ast)
    {
        oss << tab() << "call: " << ast.funcname << " {" << std::endl;
        recurse_list(ast.args);
        oss << tab() << '}' << std::endl;
    }

    void operator()(const ast_func_filter& ast)
    {
        oss << tab() << "filter: [" << std::endl;
        recurse(ast.node);
        oss << tab() << "] on \"" << ast.content << "\"" << std::endl;
    }

    void operator()(const ast_func_expr& ast)
    {
        oss << tab() << "expr: {" << std::endl;
        recurse_list(ast);
        oss << tab() << '}' << std::endl;
    }

    void operator()(const ast_func_set& ast)
    {
        oss << tab() << "set: " << ast.varname << std::endl;
        oss << tab() << "value: " << std::endl;
        recurse(ast.value);
    }

    void operator()(const ast_func_if& ast)
    {
        oss << tab() << "if: [" << std::endl;
        recurse(ast.condition);
        oss << tab() << "]" << std::endl;
        oss << tab() << "true: " << std::endl;
        recurse(ast.iftrue);
        oss << tab() << "else: " << std::endl;
        recurse(ast.iffalse);
    }

    void operator()(const ast_func_for& ast)
    {
        oss << tab() << "for: " << ast.varname << "[" << std::endl;
        recurse(ast.arg);
        oss << tab() << "]" << std::endl;
        oss << tab() << "subtree: " << std::endl;
        recurse(ast.subtree);
    }

    void operator()(const ast_highlight& ast)
    {
        oss << tab() << "highlight[" << ast.language << "]" << std::endl
            << tab() << "\"" << ast.content << "\"" << std::endl;
    }
};

/******************************************************************************/

ast_node parse_markup(const std::string& input, const std::string& name)
{
    std::string::const_iterator
        begin = input.begin(), end = input.end();

    static const MyMarkupParser p;
    ast_node ast;
    bool r = phrase_parse(begin, end, p, qi::space, ast);

    if (r && begin == end)
    {
        std::cout << std::string(80, '-') << std::endl;
        std::cout << "Parsing " << name << " succeeded." << std::endl;
        std::cout << std::string(80, '-') << std::endl;

        ast_debug prn(ast);
        std::cout << prn.oss.str();

        std::cout << std::string(80, '-') << std::endl;
    }
    else
    {
        std::cout << std::string(80, '-') << std::endl;
        std::cout << "Parsing " << name << " failed, stopped at" << std::endl;
        std::cout << std::string(80, '-') << std::endl;

        ast_debug prn(ast);
        std::cout << prn.oss.str();

        std::cout << std::string(80, '-') << std::endl;
        std::cout << "Remaining input" << std::endl;

        std::cout << std::string(begin,end) << std::endl;

        std::cout << std::string(80, '-') << std::endl;
        std::cout << "!!! " << name << " parsing FAILED!" << std::endl;
    }

    return ast;
}

/******************************************************************************/

int main(int argc, char* argv[])
{
    if (argc >= 2) {
        std::ifstream in(argv[1]);
        std::string input((std::istreambuf_iterator<char>(in)),
                          std::istreambuf_iterator<char>());
        parse_markup(input, argv[1]);
    }
    else {
        std::cout << "Reading stdin" << std::endl;
        std::string input((std::istreambuf_iterator<char>(std::cin)),
                          std::istreambuf_iterator<char>());
        parse_markup(input, "stdin");
    }
    return 0;
}

/******************************************************************************/
