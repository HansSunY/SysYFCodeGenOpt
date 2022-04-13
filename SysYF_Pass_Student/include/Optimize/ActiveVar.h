#ifndef SYSYF_ACTIVEVAR_H
#define SYSYF_ACTIVEVAR_H

#include "Pass.h"
#include "Module.h"

class ActiveVar : public Pass
{
public:
    ActiveVar(Module *module) : Pass(module) {}
    void execute() final;
    void set_live(Value *v,BasicBlock *bb);
    bool has_defined(Value *v,BasicBlock *bb);
    bool has_used(Value *v,BasicBlock *bb);
    void has_phi_used(Value *v,BasicBlock *bb);
    void print(BasicBlock *bb);
    const std::string get_name() const override {return name;}
private:
    Function *func_;
    const std::string name = "ActiveVar";
};

#endif  // SYSYF_ACTIVEVAR_H

