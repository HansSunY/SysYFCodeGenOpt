#ifndef MHSJ_LOOPINVARIANT_H
#define MHSJ_LOOPINVARIANT_H

#include "Module.h"
#include "Pass.h"
#include <vector>
#include <map>
#include <stack>
#include <set>
#include <memory>

class LoopInvariant : public Pass
{
public:
    explicit LoopInvariant(Module* module): Pass(module){}
    void execute() final;
    const std::string get_name() const override {return name;}
    void get_loop(int i,int* ComponentNumber,std::vector<int>* component);
    void loop_motion(int ComponentNumber,std::vector<int>* component);
    bool is_valid_expr(Instruction *inst);
    void create_table(int j);
    void adjust_br();
    BasicBlock* find_co_dom(std::list<BasicBlock*>list,std::vector<int>loop);
    Function* func_;
private:
    std::string name = "LoopInvariant";
};




#endif