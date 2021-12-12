#ifndef NODE_NODE
#define NODE_NODE

#include "../Token/Token.hpp"
#include <vector>

namespace node
{
    enum NodeType
    {
        PROGRAM,
        USE,
        GENERIC,
        FROM,
        BLOCK,
        MODULE,
        FUNCTION,
        EXTERN_FUNCTION,
        STRUCT,
        ENUM,
        DEF,
        VAR,
        CONST,
        IF,
        ELSEIF,
        ELSE,
        WHILE,
        DO_WHILE,
        FOR,
        LOOP,
        CAST,
        PTR_CAST,
        PTR_ARRAY_CAST,
        BREAK,
        CONTINUE,
        ASSIGNMENT,
        RETURN,
        FUNCCALL,
        METHODCALL,
        PARAM,
        MUT_PARAM,
        EXPRESSION,
        STR_LITERAL,
        MULTILINE_STR_LITERAL,
        LITERAL,
        IDENTIFIER,
        ASSIGN_OPERATOR,
        OPERATOR,
        UNARY_OPR,
        SUBSCRIPT,
        POINTER_STAR,
        EMPTY_SUBSCRIPT,
        STRUCT_FIELD,
        INITLIST,
        TERM,
        SIZEOF,
        TYPEOF,
        PRINT,
        PRINTLN,
        SPACE,
        INPUT
    };
}

struct Node
{
    node::NodeType type;
    Token tok;
    std::vector<Node> children;

    Node(node::NodeType nodeType, Token token);
    Node();
    bool isNthChild(node::NodeType nodeType, size_t index) const;
    bool isNthChildFromLast(node::NodeType nodeType, size_t N) const;
    Token getNthChildToken(size_t N) const;
    Token getNthChildTokenFromLast(size_t N) const;
    bool isConditionalExpression() const;
};

#endif