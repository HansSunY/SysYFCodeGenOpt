#ifndef SYSYF_COMSUBEXPRELI_H
#define SYSYF_COMSUBEXPRELI_H

#include "Pass.h"
#include <map>
#include <set>
#include "Instruction.h"



/*****************************CommonSubExprElimination**************************************/
/***************************This class is based on SSA form*********************************/
/***************************you need to finish this class***********************************/
class ComSubExprEli : public Pass {
public:
    explicit ComSubExprEli(Module* module):Pass(module){}
    const std::string get_name() const override {return name;}
    void execute() override;
    static bool is_valid_expr(Instruction* inst);
    void get_valid_table(std::list<Instruction*> *valid_inst);
    void cse(std::list<Instruction*> *valid_inst);
    bool compare_inst(Instruction* inst1,Instruction* inst2);
    void merge_phi();
    Instruction* all_path_have(BasicBlock* parent,Instruction* inst);
private:
    Function *func_;	
    const std::string name = "ComSubExprEli";
};

#endif // SYSYF_COMSUBEXPRELI_H