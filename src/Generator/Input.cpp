#include "Expression.hpp"

#include "Input.hpp"

void input(generator::GeneratorContext& ctx, const Node& root)
{
    Unit unit = ordinaryExpression(ctx, root.children[0]);

    if (!unit.isValidForPointerAssignment())
        ctx.console.compileErrorOnToken("Invalid term for INPUT", root.children[0].tok);

    if (unit.isStruct())
        ctx.console.compileErrorOnToken("Cannot INPUT STRUCT", root.children[0].tok);

    if (unit.isMultiDimArray())
        ctx.console.compileErrorOnToken("Cannot INPUT more than 1D ARRAY", root.children[0].tok);

    if (unit.isArray() && !unit.isChar())
        ctx.console.compileErrorOnToken("String input requires 1D CHAR ARRAY", root.children[0].tok);

    ctx.ir.functionBuilder.createInput(unit);
}