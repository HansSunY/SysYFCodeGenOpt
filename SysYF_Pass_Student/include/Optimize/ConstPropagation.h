#ifndef SYSYF_CONSTPROPAGATION_H
#define SYSYF_CONSTPROPAGATION_H
#include "Pass.h"
#include "Constant.h"
#include "Instruction.h"
#include "Module.h"
#include "Value.h"
#include "IRStmtBuilder.h"

// 用来判断value是否为ConstantFloat/ConstantInt
ConstantInt *cast_to_const_int(Value *value);
ConstantFloat *cast_to_const_float(Value *value);

class ConstFolder
{
public:
    ConstFolder(Module *module) : module_(module) {}

    ConstantInt *compute(Instruction::OpID op, ConstantInt *value1, ConstantInt *value2);
    ConstantFloat *compute(Instruction::OpID op, ConstantFloat *value1, ConstantFloat *value2);
    ConstantFloat *compute(Instruction::OpID op, ConstantInt *value);
    ConstantInt *compute(Instruction::OpID op, ConstantFloat *value);
    ConstantInt *compute(CmpInst::CmpOp op, ConstantInt *value1, ConstantInt *value2);
    ConstantInt *compute(ConstantInt *value);
    ConstantInt *compute(FCmpInst::CmpOp op, ConstantFloat *value1, ConstantFloat *value2);
    // ...
private:
    Module *module_;
};

class ConstPropagation : public Pass
{
private:
    Function *func_;
    bool change = true;
    const std::string name = "ConstPropagation";

public:
    ConstPropagation(Module *module) : Pass(module) {}
    void execute() final;
    const std::string get_name() const override {return name;}
    void Propagation();
    void value_change(Instruction* inst,Value* value);
    void useless_bb_delete();
    void phi_check();
};

#endif  // SYSYF_CONSTPROPAGATION_H
