#include "../Builder/TypeCheck.hpp"
#include "Assignment.hpp"
#include "Expression.hpp"
#include "TypeDescriptionFromNode.hpp"

#include "Local.hpp"

void local(generator::GeneratorContext& ctx, const Node& root)
{
    const Token& nameToken = root.getNthChildToken(0);

    icode::TypeDescription localType = typeDescriptionFromNode(ctx, root);

    ctx.scope.putInCurrentScope(nameToken);

    if (root.type == node::VAR)
        localType.becomeMutable();

    Unit local = ctx.ir.functionBuilder.createLocal(nameToken, localType);

    Node lastNode = root.children.back();

    if (lastNode.type == node::EXPRESSION || lastNode.type == node::TERM ||
        lastNode.type == node::MULTILINE_STR_LITERAL || lastNode.type == node::INITLIST)
    {
        Unit RHS = expression(ctx, lastNode);
        assignmentFromTree(ctx, root, nameToken, lastNode.tok, local, RHS);
    }
}