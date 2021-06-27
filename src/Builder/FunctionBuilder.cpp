#include "TypeDescriptionUtil.hpp"

#include "FunctionBuilder.hpp"

using namespace icode;

FunctionBuilder::FunctionBuilder(StringModulesMap& modulesMap,
                                 OperandBuilder& opBuilder,
                                 UnitBuilder& unitBuilder,
                                 Console& console)
  : modulesMap(modulesMap)
  , opBuilder(opBuilder)
  , unitBuilder(unitBuilder)
  , console(console)
{
}

void FunctionBuilder::setWorkingFunction(FunctionDescription* functionDesc)
{
    workingFunction = functionDesc;
}

void FunctionBuilder::pushEntry(Entry entry)
{
    (*workingFunction).icodeTable.push_back(entry);
}

Operand FunctionBuilder::getCreatePointerDestinationOperand(const Operand& op,
                                                            const std::string& dtypeName,
                                                            ModuleDescription* workingModule)
{
    /* If not a struct, just copy the operand but change its type to a pointer */

    if (op.dtype != STRUCT)
        return opBuilder.createPointerOperand(op.dtype);

    /* If it a struct, create pointer to the first field */

    TypeDescription firstFieldDesc = workingModule->structures[dtypeName].structFields.begin()->second;

    return opBuilder.createPointerOperand(firstFieldDesc.dtype);
}

Operand FunctionBuilder::createPointer(const Operand& op,
                                       const std::string& dtypeName,
                                       ModuleDescription* workingModule)
{
    if (op.isPointer() && op.dtype != STRUCT)
        return op;

    /* Converted TEMP_PTR */
    Operand pointerOperand = getCreatePointerDestinationOperand(op, dtypeName, workingModule);

    /* Construct CREATE_PTR instruction */
    Entry createPointerEntry;

    createPointerEntry.opcode = CREATE_PTR;
    createPointerEntry.op1 = pointerOperand;
    createPointerEntry.op2 = op;

    pushEntry(createPointerEntry);

    return pointerOperand;
}

// TODO: merge this with createPointer
Operand FunctionBuilder::getPointerOperand(const Unit& unit)
{
    ModuleDescription* workingModule = &modulesMap.at(unit.second.moduleName);

    return createPointer(unit.first, unit.second.dtypeName, workingModule);
}

void FunctionBuilder::operandCopy(Operand op1, Operand op2)
{
    /* If op2 is a literal, change generic dtypes like INT and FLOAT
        to correct specific dtype */
    if (op2.operandType == LITERAL)
        op2.dtype = op1.dtype;

    /* Copy one operand value to another, use READ and WRITE instruction
        if pointers are involved */

    if (op1.isPointer() && op2.isPointer())
    {
        Operand temp = opBuilder.createTempOperand(op2.dtype);
        operandCopy(temp, op2);
        operandCopy(op1, temp);
    }
    else
    {
        Entry copyEntry;

        if (op1.isPointer() && !op2.isPointer())
            copyEntry.opcode = WRITE;
        else if (!op1.isPointer() && op2.isPointer())
            copyEntry.opcode = READ;
        else if (!op1.isPointer() && !op2.isPointer())
            copyEntry.opcode = EQUAL;

        copyEntry.op1 = op1;
        copyEntry.op2 = op2;

        pushEntry(copyEntry);
    }
}

void FunctionBuilder::memCopy(Operand op1, Operand op2, int numBytes)
{
    Entry memCpyEntry;

    memCpyEntry.op1 = op1;
    memCpyEntry.op2 = op2;
    memCpyEntry.op3 = opBuilder.createIntLiteralOperand(I64, numBytes);
    memCpyEntry.opcode = MEMCPY;

    pushEntry(memCpyEntry);
}

void FunctionBuilder::unitCopy(const Unit& dest, const Unit& src)
{
    if (dest.second.isArray() || dest.second.isStruct())
    {
        Operand destPointer = getPointerOperand(dest);
        Operand srcPointer = getPointerOperand(src);

        memCopy(destPointer, srcPointer, src.second.size);
    }
    else
        operandCopy(dest.first, src.first);
}

Operand FunctionBuilder::ensureNotPointer(Operand op)
{
    /* Make sure the operand is not a pointer, if it is a pointer,
        converts it to a temp using the READ instruction */

    if (!op.isPointer())
        return op;

    Operand temp = opBuilder.createTempOperand(op.dtype);
    operandCopy(temp, op);
    return temp;
}

Operand FunctionBuilder::pushEntryAndEnsureNoPointerWrite(Entry entry)
{
    /* Push an ir entry to the current function's icode table,
        but ensures entry.op1 is not a pointer */

    if (!entry.op1.isPointer())
    {
        pushEntry(entry);
        return entry.op1;
    }

    /* If entry.op1 is a pointer, replace it with a temp and
        write that temp to the pointer */

    /* Create corresponding TEMP to TEMP_PTR  */
    Operand pointerOperand = entry.op1;
    Operand temp = opBuilder.createTempOperand(pointerOperand.dtype);

    /* Replace TEMP_PTR with TEMP */
    Entry modifiedEntry = entry;
    modifiedEntry.op1 = temp;
    pushEntry(modifiedEntry);

    /* Create WRITE instruction to write the TEMP to TEMP_PTR */
    Entry writeEntry;

    writeEntry.op1 = pointerOperand;
    writeEntry.op2 = temp;
    writeEntry.opcode = WRITE;

    pushEntry(writeEntry);

    return temp;
}

Unit FunctionBuilder::binaryOperator(Instruction instruction, const Unit& LHS, const Unit& RHS)
{
    /* Construct icode instruction for binary operator instructions,
        ADD, SUB, MUL, DIV, MOD, RSH, LSH, BWA, BWO, BWX */

    DataType dtype = LHS.second.dtype;

    Entry entry;

    entry.opcode = instruction;
    entry.op1 = opBuilder.createTempOperand(dtype);
    entry.op2 = ensureNotPointer(LHS.first);
    entry.op3 = ensureNotPointer(RHS.first);

    Operand result = pushEntryAndEnsureNoPointerWrite(entry);

    return Unit(result, LHS.second);
}

Unit FunctionBuilder::unaryOperator(Instruction instruction, const Unit& unaryOperatorTerm)
{
    /* Construct icode for unary operator instructions,
        UNARY_MINUS and NOT  */

    const DataType dtype = unaryOperatorTerm.second.dtype;

    Entry entry;

    entry.opcode = instruction;
    entry.op1 = opBuilder.createTempOperand(dtype);
    entry.op2 = ensureNotPointer(unaryOperatorTerm.first);

    Operand result = pushEntryAndEnsureNoPointerWrite(entry);

    return Unit(result, unaryOperatorTerm.second);
}

Unit FunctionBuilder::castOperator(const Unit& unitToCast, DataType destinationDataType)
{
    /* Construct icode for CAST */

    Entry entry;

    entry.opcode = CAST;
    entry.op1 = opBuilder.createTempOperand(destinationDataType);
    entry.op2 = ensureNotPointer(unitToCast.first);

    Operand result = pushEntryAndEnsureNoPointerWrite(entry);

    return Unit(result, typeDescriptionFromDataType(destinationDataType));
}

void FunctionBuilder::compareOperator(Instruction instruction, const Unit& LHS, const Unit& RHS)
{
    Operand op1 = LHS.first;
    Operand op2 = RHS.first;

    /* If op2 is a literal, change generic dtypes like INT and FLOAT
        to correct specific dtype */
    if (op2.operandType == LITERAL)
        op2.dtype = op1.dtype;

    /* Construct icode for comparator operator instructions,
        EQ, NEQ, LT, LTE, GT, GTE  */

    Entry entry;

    entry.opcode = instruction;
    entry.op1 = ensureNotPointer(op1);
    entry.op2 = ensureNotPointer(op2);

    pushEntry(entry);
}

Operand FunctionBuilder::addressAddOperator(Operand op2, Operand op3)
{
    /* Construct icode for ADDR_ADD */

    Entry entry;

    entry.opcode = ADDR_ADD;
    entry.op1 = opBuilder.createPointerOperand(op2.dtype);
    entry.op2 = op2;
    entry.op3 = op3;

    pushEntry(entry);

    return entry.op1;
}

Operand FunctionBuilder::addressMultiplyOperator(Operand op2, Operand op3)
{
    /* Construct icode for ADDR_MUL */

    Entry entry;

    entry.opcode = ADDR_MUL;
    entry.op1 = opBuilder.createPointerOperand(VOID);
    entry.op2 = ensureNotPointer(op2);
    entry.op3 = op3;

    pushEntry(entry);

    return entry.op1;
}

Unit FunctionBuilder::getStructField(const Token& fieldName, const Unit& unit)
{
    StructDescription structDescription = modulesMap[unit.second.moduleName].structures[unit.second.dtypeName];

    TypeDescription fieldType;

    if (!structDescription.getField(fieldName.toString(), fieldType))
        console.compileErrorOnToken("Undefined STRUCT field", fieldName);

    if (unit.second.isMutable())
        fieldType.becomeMutable();

    Operand pointerOperand = getPointerOperand(unit);

    Operand fieldOperand = addressAddOperator(pointerOperand, opBuilder.createLiteralAddressOperand(fieldType.offset));

    fieldOperand.dtype = fieldType.dtype;

    return Unit(fieldOperand, fieldType);
}

Unit FunctionBuilder::getIndexedElement(const Unit& unit, const std::vector<Unit>& indices)
{
    unsigned int dimensionCount = 0;
    unsigned int elementWidth = unit.second.size / unit.second.dimensions[0];

    Operand elementOperand = getPointerOperand(unit);

    TypeDescription elementType = unit.second;

    for (const Unit indexUnit : indices)
    {
        Operand subscriptOperand =
          addressMultiplyOperator(indexUnit.first, opBuilder.createLiteralAddressOperand(elementWidth));

        if (dimensionCount + 1 != elementType.dimensions.size())
            elementWidth /= elementType.dimensions[dimensionCount + 1];

        elementOperand = addressAddOperator(elementOperand, subscriptOperand);

        dimensionCount++;
    }

    unsigned int remainingDimensionCount = elementType.dimensions.size() - dimensionCount;

    elementType.dimensions.erase(elementType.dimensions.begin(),
                                 elementType.dimensions.end() - remainingDimensionCount);

    return Unit(elementOperand, elementType);
}

Operand FunctionBuilder::createLabel(const Token& tok, bool isTrueLabel, std::string prefix)
{
    std::string label_name = tok.getLineColString();

    return opBuilder.createLabelOperand("_" + prefix + "_" + (isTrueLabel ? "true" : "false") + label_name);
}

void FunctionBuilder::insertLabel(Operand op)
{
    /* Construct CREATE_LABEL */

    Entry labelEntry;

    labelEntry.opcode = CREATE_LABEL;
    labelEntry.op1 = op;

    pushEntry(labelEntry);
}

void FunctionBuilder::createBranch(Instruction instruction, Operand op)
{
    /* Construct icode for GOTO, IF_TRUE_GOTO, IF_FALSE_GOTO */

    Entry branchEntry;

    branchEntry.opcode = instruction;
    branchEntry.op1 = op;

    pushEntry(branchEntry);
}

void FunctionBuilder::createPrint(const Unit& unit)
{
    /* Construct icode for PRINT, PRINT_STR */

    Entry printEntry;

    Instruction printInstruction = unit.second.isArray() ? PRINT_STR : PRINT;

    printEntry.opcode = printInstruction;

    if (printInstruction == PRINT)
        printEntry.op1 = ensureNotPointer(unit.first);
    else
        printEntry.op1 = unit.first;

    pushEntry(printEntry);
}

void FunctionBuilder::createInput(const Unit& unit)
{
    /* Construct icode for INPUT, INPUT_STR */

    Entry inputEntry;

    Instruction inputInstruction = INPUT;
    int size = 0;

    if (unit.second.isArray())
    {
        inputInstruction = INPUT_STR;
        size = unit.second.dimensions[0];
    }

    inputEntry.opcode = inputInstruction;
    inputEntry.op1 = unit.first;
    inputEntry.op2 = opBuilder.createIntLiteralOperand(INT, (int)size);

    pushEntry(inputEntry);
}

Unit FunctionBuilder::createLocal(const Token nameToken, TypeDescription& typeDescription)
{
    if (workingFunction->symbolExists(nameToken.toString()))
        console.compileErrorOnToken("Symbol already defined", nameToken);

    typeDescription.setProperty(IS_LOCAL);

    workingFunction->symbols[nameToken.toString()] = typeDescription;

    return unitBuilder.unitFromTypeDescription(typeDescription, nameToken.toString());
}

void FunctionBuilder::passParameter(const Token& calleeNameToken,
                                    FunctionDescription callee,
                                    const Unit& formalParam,
                                    const Unit& actualParam)
{
    /* Construct icode for PASS and PASS_ADDR instructions */

    DataType functionDataType = callee.functionReturnType.dtype;

    Entry entry;

    entry.opcode = PASS_ADDR;
    entry.op1 = actualParam.first;

    if (!(formalParam.second.isMutable() || formalParam.second.isStruct() || formalParam.second.isArray()))
    {
        entry.opcode = PASS;
        entry.op1 = ensureNotPointer(actualParam.first);
    }

    entry.op2 = opBuilder.createVarOperand(functionDataType, calleeNameToken.toString());
    entry.op3 = opBuilder.createModuleOperand(callee.moduleName);

    pushEntry(entry);
}

Unit FunctionBuilder::callFunction(const Token& calleeNameToken, FunctionDescription callee)
{
    /* Construct icode for CALL instruction */

    DataType functionDataType = callee.functionReturnType.dtype;

    Entry callEntry;

    callEntry.opcode = CALL;
    callEntry.op1 = opBuilder.createCalleeRetValOperand(functionDataType);
    callEntry.op2 = opBuilder.createVarOperand(functionDataType, calleeNameToken.toString());
    callEntry.op3 = opBuilder.createModuleOperand(callee.moduleName);

    pushEntry(callEntry);

    return Unit(callEntry.op1, callee.functionReturnType);
}

void FunctionBuilder::noArgumentEntry(Instruction instruction)
{
    /* Construct icode for instructions with no arguments,
        RET, SPACE, NEWLN, EXIT */

    Entry entry;

    entry.opcode = instruction;

    pushEntry(entry);
}

bool FunctionBuilder::doesFunctionTerminate()
{
    if (workingFunction->icodeTable.size() < 1)
        return false;

    icode::Instruction lastOpcode = workingFunction->icodeTable.back().opcode;
    return lastOpcode == icode::RET;
}

bool FunctionBuilder::terminateFunction()
{
    if (doesFunctionTerminate())
        return true;

    if (!workingFunction->isVoid())
        return false;

    noArgumentEntry(icode::RET);
    return true;
}
