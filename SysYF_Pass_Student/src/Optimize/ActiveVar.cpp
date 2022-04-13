#include "ActiveVar.h"

#include <algorithm>

void ActiveVar::execute()
{
    for (auto &func : this->module->get_functions())
    {
        if (func->get_basic_blocks().empty())
            continue;
        else
        {
            func_ = func;
            /*you need to finish this function*/
            std::set<Value *> value_list;
            for(auto bb: func_->get_basic_blocks()){
                bb->get_live_in().clear();
                bb->get_live_out().clear();
                for(auto inst: bb->get_instructions()){
                    value_list.insert(inst);
                }
            }
            for(auto value:value_list){
                for(auto bb: func_->get_basic_blocks()){
                    if(has_used(value,bb)){
                        set_live(value,bb);
                    }
                    has_phi_used(value,bb);
                }
            }
            
            for(auto bb: func_->get_basic_blocks()){
                print(bb);
            }
        }
    }
    return;
}
void ActiveVar::print(BasicBlock *bb){
    std::string out;
    out += "\n";
    out += bb->get_name();
    out += ":\n";
    out += "  in = {";
    for(auto lin: bb->get_live_in()){
        out += lin->get_name();
        out += ", ";
    }
    out += "}";
    out += "\n  out = {";
    for(auto lout: bb->get_live_out()){
        out += lout->get_name();
        out += ", ";
    }
    out += "}";
    std::cout<<out<<std::endl;
}
void ActiveVar::set_live(Value *v, BasicBlock *bb)
{   
    bb->get_live_in().insert(v);
    for (auto pre_bb : bb->get_pre_basic_blocks())
    {
        pre_bb->get_live_out().insert(v);
        bool checked = false;
        for(auto value: pre_bb->get_live_in()){
            if(value == v)
                checked = true;
        }
        if (!has_defined(v, pre_bb) && !checked)
        {
            set_live(v, pre_bb);
        }
    }
}
bool ActiveVar::has_defined(Value *v, BasicBlock *bb)
{
    for (auto inst : bb->get_instructions())
        if (v == inst)return true;
    return false;
}
void ActiveVar::has_phi_used(Value *v, BasicBlock *bb){
    for(auto inst: bb->get_instructions()){
        if(inst->get_instr_type()==Instruction::phi){
            for(int i=0;i<inst->get_operands().size();i+=2){
                if(v == inst->get_operand(i)){
                    auto pre_bb = dynamic_cast<BasicBlock *>(inst->get_operand(i+1));
                    pre_bb->get_live_out().insert(v);
                }
            }
        }
    }
}
bool ActiveVar::has_used(Value *v, BasicBlock *bb)
{
    for (auto inst : bb->get_instructions()){
        if(inst == v){
            return false;
        }
        if(inst->is_binary()||inst->is_cmp()||inst->is_fcmp()){
            for(auto opr: inst->get_operands()){
                if(v == opr) return true;
            }
        }else if(inst->is_call()){
            for(int i=1;i<inst->get_operands().size();i++){
                auto opr = inst->get_operand(i);
                if(v == opr) return true;
            }
        }else if(inst->is_br()){
            BranchInst *br_inst = static_cast<BranchInst *>(inst);
            if(br_inst->is_cond_br()){
                auto opr = br_inst->get_operand(0);
                if(v == opr) return true;
            }
        }else if(inst->is_ret()){
            ReturnInst *ret_inst = static_cast<ReturnInst *>(inst);
            if(!ret_inst->is_void_ret()){
                auto opr = ret_inst->get_operand(0);
                if(v == opr) return true;
            }
        }else if(inst->is_gep()||inst->is_load()||inst->is_zext()||inst->is_fptosi()||inst->is_sitofp()){//一个操作数
            auto opr = inst->get_operand(0);
            if(v == opr) return true;
        }else if(inst->is_store()){
            for(auto opr:inst->get_operands()){
                if(v == opr) return true;
            }
        }/*else if(inst->is_phi()){
            for(int i = 0;i < inst->get_operands().size();i++){
                if(i % 2 == 0){
                    auto opr = inst->get_operand(i);
                    if(v == opr) return true;
                }
            }
        }*/
    }
    return false;
}
