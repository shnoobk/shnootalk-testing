#include "../Builder/TypeDescriptionUtil.hpp"
#include "Module.hpp"

#include "TypeDescriptionFromNode.hpp"

using namespace icode;

void generateModule(generator::GeneratorContext& ctx, const Node& root);

std::pair<std::string, std::string> generateIRUsingMonomorphizer(generator::GeneratorContext& ctx,
                                                                 const std::string& genericModuleName,
                                                                 const Token& genericStructNameToken,
                                                                 const std::vector<TypeDescription>& instantiationTypes,
                                                                 const std::vector<Node>& instantiationTypeNodes)
{
    const std::string& instantiationSuffix = constructInstantiationSuffix(instantiationTypes);

    const std::string& instantiatedModuleName = getInstantiatedModuleName(instantiationSuffix, genericModuleName);

    if (!ctx.moduleExists(instantiatedModuleName))
    {
        ctx.console.pushModule(genericModuleName);

        Node ast = ctx.mm.instantiateGeneric(genericModuleName,
                                             instantiationSuffix,
                                             genericStructNameToken,
                                             instantiationTypes,
                                             instantiationTypeNodes);

        generator::GeneratorContext generatorContext = ctx.clone(instantiatedModuleName);
        generateModule(generatorContext, ast);

        ctx.console.popModule();
    }

    std::string instantiatedStructName = getInstantiatedStructName(instantiationSuffix, genericStructNameToken);

    return std::pair<std::string, std::string>(instantiatedModuleName, instantiatedStructName);
}

TypeDescription instantiateGenericAndGetType(generator::GeneratorContext& ctx,
                                             const std::string& genericModuleName,
                                             const Token& genericStructNameToken,
                                             const std::vector<TypeDescription>& instantiationTypes,
                                             const std::vector<Node>& instantiationTypeNodes)
{
    ctx.ir.pushWorkingModule();

    std::pair<std::string, std::string> moduleNameStructNamePair =
        generateIRUsingMonomorphizer(ctx,
                                     genericModuleName,
                                     genericStructNameToken,
                                     instantiationTypes,
                                     instantiationTypeNodes);

    ctx.ir.setWorkingModule(&ctx.modulesMap.at(moduleNameStructNamePair.first));

    TypeDescription monomorphizedType =
        ctx.ir.moduleBuilder.createTypeDescriptionFromStructName(moduleNameStructNamePair.second);

    ctx.ir.popWorkingModule();

    return monomorphizedType;
}

TypeDescription getMonomorphizedTypeDescriptionFromNode(generator::GeneratorContext& ctx, const Node& root)
{
    ctx.ir.pushWorkingModule();
    ctx.ir.resetWorkingModule();

    size_t childNodeCounter = 1;

    std::string genericModuleName = "";

    while (root.isNthChild(node::PARAM, childNodeCounter) || root.isNthChild(node::MUT_PARAM, childNodeCounter))
        childNodeCounter++;

    while (root.isNthChild(node::MODULE, childNodeCounter))
    {
        const Token& aliasToken = root.getNthChildToken(childNodeCounter);

        if (childNodeCounter > 1)
            ctx.console.compileErrorOnToken("Invalid MODULE ACCESS from GENERIC", aliasToken);

        genericModuleName = ctx.mm.getGenericModuleNameFromAlias(aliasToken);

        childNodeCounter++;
    }

    const Token& genericStructNameToken = root.getNthChildToken(childNodeCounter);
    childNodeCounter++;

    if (genericModuleName == "")
        genericModuleName = ctx.mm.getGenericModuleNameFromStruct(genericStructNameToken);

    std::vector<TypeDescription> instantiationTypes;
    std::vector<Node> instantiationTypeNodes;

    while (root.isNthChild(node::GENERIC_TYPE_PARAM, childNodeCounter))
    {
        instantiationTypes.push_back(typeDescriptionFromNode(ctx, root.children[childNodeCounter]));
        instantiationTypeNodes.push_back(root.children[childNodeCounter]);

        childNodeCounter++;
    }

    TypeDescription monomorphizedType = instantiateGenericAndGetType(ctx,
                                                                     genericModuleName,
                                                                     genericStructNameToken,
                                                                     instantiationTypes,
                                                                     instantiationTypeNodes);

    ctx.ir.popWorkingModule();

    return monomorphizedType;
}

TypeDescription arrayTypeFromSubscript(const Node& root, const TypeDescription& typeDescription, size_t startIndex)
{
    std::vector<int> dimensions;

    size_t nodeCounter;

    for (nodeCounter = startIndex; root.isNthChild(node::SUBSCRIPT, nodeCounter); nodeCounter++)
    {
        const int subscriptInt = root.children[nodeCounter].children[0].tok.toInt();
        dimensions.push_back(subscriptInt);
    }

    return createArrayTypeDescription(typeDescription, dimensions, FIXED_DIM);
}

TypeDescription typeDescriptionFromNode(generator::GeneratorContext& ctx, const Node& root)
{
    ctx.ir.pushWorkingModule();
    ctx.ir.resetWorkingModule();

    if (root.isGenericTypeParamPresent())
        return getMonomorphizedTypeDescriptionFromNode(ctx, root);

    size_t childNodeCounter = 1;
    if (root.type == node::GENERIC_TYPE_PARAM)
        childNodeCounter = 0;

    while (root.isNthChild(node::PARAM, childNodeCounter) || root.isNthChild(node::MUT_PARAM, childNodeCounter))
        childNodeCounter++;

    if (root.isNthChild(node::MODULE, childNodeCounter))
        childNodeCounter = setWorkingModuleFromNode(ctx, root, childNodeCounter);

    if (!root.isNthChild(node::IDENTIFIER, childNodeCounter))
        return ctx.ir.moduleBuilder.createVoidTypeDescription();

    const Token& dataTypeToken = root.getNthChildToken(childNodeCounter);
    TypeDescription typeDescription = ctx.ir.moduleBuilder.createTypeDescription(dataTypeToken);

    childNodeCounter++;

    if (root.isNthChild(node::POINTER_STAR, childNodeCounter))
        typeDescription.becomePointer();

    if (root.isNthChild(node::EMPTY_SUBSCRIPT, childNodeCounter))
        typeDescription.becomeArrayPointer();

    if (root.isNthChild(node::SUBSCRIPT, childNodeCounter))
        typeDescription = arrayTypeFromSubscript(root, typeDescription, childNodeCounter);

    ctx.ir.popWorkingModule();

    return typeDescription;
}