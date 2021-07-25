#include <map>

#include "DataType.hpp"

namespace icode
{
    bool isSignedInteger(DataType dtype)
    {
        return (dtype == I8 || dtype == I16 || dtype == I32 || dtype == I64 || dtype == AUTO_INT);
    }

    bool isUnsignedInteger(DataType dtype)
    {
        return (dtype == UI8 || dtype == UI16 || dtype == UI32 || dtype == UI64);
    }

    bool isInteger(DataType dtype)
    {
        return (isSignedInteger(dtype) || isUnsignedInteger(dtype));
    }

    bool isFloat(DataType dtype)
    {
        return (dtype == F32 || dtype == F64 || dtype == AUTO_FLOAT);
    }

    std::string dataTypeToString(const DataType dtype)
    {
        static std::string dataTypeStringsArray[] = { "byte",    "ubyte",     "short",  "ushort", "int",
                                                      "uint",    "long",      "ulong",  "float",  "double",
                                                      "autoInt", "autoFloat", "struct", "void" };

        return dataTypeStringsArray[dtype];
    }

    DataType stringToDataType(const std::string& dtypeName)
    {
        static std::map<std::string, DataType> dataTypeNames = {
            { "byte", icode::I8 },          { "ubyte", icode::UI8 },
            { "short", icode::I16 },        { "ushort", icode::UI16 },
            { "int", icode::I32 },          { "uint", icode::UI32 },
            { "long", icode::I64 },         { "ulong", icode::UI64 },
            { "float", icode::F32 },        { "double", icode::F64 },
            { "char", icode::UI8 },         { "bool", icode::UI8 },
            { "autoInt", icode::AUTO_INT }, { "autoFloat", icode::AUTO_FLOAT },
            { "struct", icode::STRUCT },    { "void", icode::VOID }

        };

        if (dataTypeNames.find(dtypeName) != dataTypeNames.end())
            return dataTypeNames[dtypeName];

        return STRUCT;
    }

    int getDataTypeSize(const DataType dtype)
    {
        const int dataTypeSizesArray[] = { 1, 1, 2, 2, 4, 4, 8, 8, 4, 8, 8, 8, 0, 0 };
        return dataTypeSizesArray[dtype];
    }

    bool dataTypeIsEqual(DataType dtype1, DataType dtype2)
    {
        return dtype1 == dtype2 || (dtype1 == AUTO_INT && isInteger(dtype2)) ||
               (isInteger(dtype1) && dtype2 == AUTO_INT) || (dtype1 == AUTO_FLOAT && isFloat(dtype2)) ||
               (isFloat(dtype1) && dtype2 == AUTO_FLOAT);
    }
}