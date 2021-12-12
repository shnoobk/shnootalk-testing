#ifndef MONOMORPHIZER_MONOMORPHIZER
#define MONOMORPHIZER_MONOMORPHIZER

#include <map>
#include <string>
#include <vector>

#include "../../Console/Console.hpp"
#include "../../IntermediateRepresentation/TypeDescription.hpp"
#include "../../Node/Node.hpp"

namespace monomorphizer
{
    struct GenericASTIndex
    {
        Node ast;
        std::vector<std::string> genericIdentifiers;
        std::vector<std::string> genericStructs;
    };

    typedef std::map<std::string, GenericASTIndex> StringGenericASTMap;

    class Monomorphizer
    {
        StringGenericASTMap& genericsMap;
        std::map<std::string, std::string> genericUses;

        Console& console;

        std::vector<std::string> getGenericStructs(const Node& root,
                                                   const std::vector<std::string>& genericIdentifiers);

    public:
        Monomorphizer(StringGenericASTMap& genericsMap, Console& console);

        void indexAST(const std::string& genericModuleName, const Node& ast);

        void instantiateAST(const std::string& genericModuleName,
                            std::vector<std::string> typeModuleNames,
                            std::vector<Node> typeDescriptionNodes);

        icode::TypeDescription getMonomorphizedTypeDescription(const std::string& genericModuleName,
                                                               const std::string& genericStructName,
                                                               const std::vector<icode::TypeDescription>& types);
    };
};

#endif