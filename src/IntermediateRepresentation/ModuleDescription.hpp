#ifndef IR_MODULE_DESCRIPTION
#define IR_MODULE_DESCRIPTION

#include <map>
#include <string>
#include <vector>

#include "FunctionDescription.hpp"
#include "StructDescription.hpp"
#include "TargetDescription.hpp"

namespace icode
{
    struct ModuleDescription
    {
        std::string name;
        std::vector<std::string> uses;
        std::map<std::string, StructDescription> structures;
        std::map<std::string, FunctionDescription> functions;
        std::map<std::string, int> enumerations;
        std::map<std::string, int> intDefines;
        std::map<std::string, float> floatDefines;
        std::map<std::string, std::string> stringDefines;
        std::map<std::string, TypeDescription> globals;
        std::map<std::string, std::string> stringsData;
        std::map<std::string, int> stringsDataCharCounts;

        void initializeTarget(const TargetEnums& target);

        bool useExists(const std::string&);
        bool getStruct(const std::string& name, StructDescription& returnValue);
        bool getFunction(const std::string& name, FunctionDescription& returnValue);
        bool getEnum(const std::string& name, int& returnValue);
        bool getIntDefine(const std::string& name, int& returnValue);
        bool getFloatDefine(const std::string& name, float& returnValue);
        bool getStringDefine(const std::string& name, std::string& returnValue);
        bool getGlobal(const std::string& name, TypeDescription& returnValue);
        bool symbolExists(const std::string&);
    };

    typedef std::map<std::string, ModuleDescription> StringModulesMap;
}

#endif