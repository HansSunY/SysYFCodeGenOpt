#include "ConstPropagation.h"

// 给出了返回整型值的常数折叠实现，大家可以参考，在此基础上拓展
// 当然如果同学们有更好的方式，不强求使用下面这种方式

ConstantInt *ConstFolder::compute(Instruction::OpID op, ConstantInt *value1, ConstantInt *value2) {
    int const_value1 = value1->get_value();
    int const_value2 = value2->get_value();
    switch (op) {
    case Instruction::add:
        return ConstantInt::get(const_value1 + const_value2, module_);
        break;
    case Instruction::sub:
        return ConstantInt::get(const_value1 - const_value2, module_);
        break;
    case Instruction::mul:
        return ConstantInt::get(const_value1 * const_value2, module_);
        break;
    case Instruction::sdiv:
        return ConstantInt::get((int)(const_value1 / const_value2), module_);
        break;
    case Instruction::srem:
        return ConstantInt::get(const_value1 % const_value2, module_);
        break;
    default:
        return nullptr;
        break;
    }
}

ConstantFloat *ConstFolder::compute(Instruction::OpID op, ConstantFloat *value1, ConstantFloat *value2){
    float const_value1 = value1->get_value();
    float const_value2 = value2->get_value();
    switch (op) {
    case Instruction::fadd:
        return ConstantFloat::get(const_value1 + const_value2, module_);
        break;
    case Instruction::fsub:
        return ConstantFloat::get(const_value1 - const_value2, module_);
        break;
    case Instruction::fmul:
        return ConstantFloat::get(const_value1 * const_value2, module_);
        break;
    case Instruction::fdiv:
        return ConstantFloat::get((const_value1 / const_value2), module_);
        break;
    default:
        return nullptr;
        break;
    }
}
//用于计算sitofp指令
ConstantFloat *ConstFolder::compute(Instruction::OpID op, ConstantInt *value){
    int const_value = value->get_value();
    if(op == Instruction::sitofp){
        return ConstantFloat::get((float)const_value,module_);
    }
}
// 用于计算fptosi指令
ConstantInt *ConstFolder::compute(Instruction::OpID op, ConstantFloat *value){
    float const_value = value->get_value();
    if(op == Instruction::fptosi){
        return ConstantInt::get((int)const_value,module_);
    }
}
// 用于计算cmp指令
ConstantInt *ConstFolder::compute(CmpInst::CmpOp op, ConstantInt *value1,ConstantInt *value2){
    int const_value1 = value1->get_value();
    int const_value2 = value2->get_value();
    switch (op) {
    case CmpInst::CmpOp::EQ:
        return ConstantInt::get(const_value1==const_value2,module_);
        break;
    case CmpInst::CmpOp::NE:
        return ConstantInt::get(const_value1!=const_value2,module_);
        break;
    case CmpInst::CmpOp::GT:
        return ConstantInt::get(const_value1>const_value2,module_);
        break;
    case CmpInst::CmpOp::GE:
        return ConstantInt::get(const_value1>=const_value2,module_);
        break;
    case CmpInst::CmpOp::LT:
        return ConstantInt::get(const_value1<const_value2,module_);
        break;
    case CmpInst::CmpOp::LE:
        return ConstantInt::get(const_value1<=const_value2,module_);
        break;
    default:
        return nullptr;
        break;
    }
}
// 用来计算fcmp指令
ConstantInt *ConstFolder::compute(FCmpInst::CmpOp op, ConstantFloat *value1,ConstantFloat *value2){
    float const_value1 = value1->get_value();
    float const_value2 = value2->get_value();
    switch (op) {
    case FCmpInst::CmpOp::EQ:
        return ConstantInt::get(const_value1==const_value2,module_);
        break;
    case FCmpInst::CmpOp::NE:
        return ConstantInt::get(const_value1!=const_value2,module_);
        break;
    case FCmpInst::CmpOp::GT:
        return ConstantInt::get(const_value1>const_value2,module_);
        break;
    case FCmpInst::CmpOp::GE:
        return ConstantInt::get(const_value1>=const_value2,module_);
        break;
    case FCmpInst::CmpOp::LT:
        return ConstantInt::get(const_value1<const_value2,module_);
        break;
    case FCmpInst::CmpOp::LE:
        return ConstantInt::get(const_value1<=const_value2,module_);
        break;
    default:
        return nullptr;
        break;
    }
}
// 用于计算zext指令
ConstantInt *ConstFolder::compute(ConstantInt *value){
    int const_value = value->get_value();
    return ConstantInt::get(const_value,module_);
}

void ConstPropagation::Propagation(){
    std::vector<Instruction*> delete_list;
    for(auto bb:func_->get_basic_blocks()){
        for(auto inst:bb->get_instructions()){
            Constant *const_tmp;
            if(inst->is_int_binary()){
                auto opr0 = cast_to_const_int(inst->get_operand(0));
                auto opr1 = cast_to_const_int(inst->get_operand(1));
                if(opr0 && opr1){
                    const_tmp = ConstFolder(module).compute(inst->get_instr_type(),opr0,opr1);
                    value_change(inst,const_tmp);
                    delete_list.push_back(inst);
                }
            }else if(inst->is_float_binary()){
                auto opr0 = cast_to_const_float(inst->get_operand(0));
                auto opr1 = cast_to_const_float(inst->get_operand(1));
                if(opr0 && opr1){
                    const_tmp = ConstFolder(module).compute(inst->get_instr_type(),opr0,opr1);
                    value_change(inst,const_tmp);
                    delete_list.push_back(inst);
                }
            }else if(inst->is_sitofp()){
                auto opr0 = cast_to_const_int(inst->get_operand(0));
                if(opr0){
                    const_tmp = ConstFolder(module).compute(inst->get_instr_type(),opr0);
                    value_change(inst,const_tmp);
                    delete_list.push_back(inst);
                }
            }else if(inst->is_fptosi()){
                auto opr0 = cast_to_const_float(inst->get_operand(0));
                if(opr0){
                    const_tmp = ConstFolder(module).compute(inst->get_instr_type(),opr0);
                    value_change(inst,const_tmp);
                    delete_list.push_back(inst);
                }
            }else if(inst->is_cmp()){
                auto opr0 = cast_to_const_int(inst->get_operand(0));
                auto opr1 = cast_to_const_int(inst->get_operand(1));
                if(opr0 && opr1){
                    const_tmp = ConstFolder(module).compute(static_cast<CmpInst *>(inst)->get_cmp_op(),opr0,opr1);
                    value_change(inst,const_tmp);
                    delete_list.push_back(inst);
                }
            }else if(inst->is_fcmp()){
                auto opr0 = cast_to_const_float(inst->get_operand(0));
                auto opr1 = cast_to_const_float(inst->get_operand(1));
                if(opr0 && opr1){
                    const_tmp = ConstFolder(module).compute(static_cast<FCmpInst *>(inst)->get_cmp_op(),opr0,opr1);
                    value_change(inst,const_tmp);
                    delete_list.push_back(inst);
                    std::cout<<"fcmp"<<const_tmp<<std::endl;
                }
            }else if(inst->is_zext()){
                auto opr = cast_to_const_int(inst->get_operand(0));
                if(opr){
                    const_tmp = ConstFolder(module).compute(opr);
                    value_change(inst,const_tmp);
                    delete_list.push_back(inst);
                }
            }else if(inst->is_phi()){
                if(inst->get_operands().size()==2){
                    auto tmp = inst->get_operand(0);
                    value_change(inst,tmp);
                    delete_list.push_back(inst);
                }
            }
        }
        for(auto inst:delete_list){
            bb->delete_instr(inst);
        }
        delete_list.clear();
    }
}
// 常量替代
void ConstPropagation::value_change(Instruction* source_inst,Value* value){
    for(auto bb:func_->get_basic_blocks()){
        for(auto inst:bb->get_instructions()){
            if(inst!=source_inst){
                for(int i=0;i<inst->get_operands().size();i++){
                    if(inst->get_operand(i)==source_inst){
                        inst->set_operand(i,value);
                    }
                }
            }
        }
    }
}

void ConstPropagation::useless_bb_delete(){
    // 前者为前驱基本块，后者为后驱，无效基本块
    std::vector<Instruction *>delete_inst;
    std::vector<BasicBlock *>delete_bb;
    for(auto bb:func_->get_basic_blocks()){
        for(auto inst:bb->get_instructions()){
            if(inst->is_br()){
                auto br = static_cast<BranchInst *>(inst);
                if(br->is_cond_br()){
                    auto opr = cast_to_const_int(inst->get_operand(0));
                    if(opr){
                        BasicBlock * useless_bb,* useful_bb;
                        if(opr->get_value() == 1){
                            useless_bb = dynamic_cast<BasicBlock *>(br->get_operand(2));
                            useful_bb = dynamic_cast<BasicBlock *>(br->get_operand(1));
                        }else if(opr->get_value() == 0){
                            useless_bb = dynamic_cast<BasicBlock *>(br->get_operand(1));
                            useful_bb = dynamic_cast<BasicBlock *>(br->get_operand(2));
                        }
                        delete_inst.push_back(inst);
                        bb->remove_succ_basic_block(useful_bb);
                        useful_bb->remove_pre_basic_block(bb);
                        BranchInst::create_br(useful_bb,bb);
                        bb->remove_succ_basic_block(useless_bb);
                        useless_bb->remove_pre_basic_block(bb);
                    }   
                }
            }
        }
        for(auto inst:delete_inst)
            bb->delete_instr(inst);
        delete_inst.clear();
    }
    bool has_changed = true;
    while(has_changed){
        has_changed = false;
        for(auto bb:func_->get_basic_blocks()){
            if(bb->get_name()!="label_entry" && bb->get_pre_basic_blocks().empty()){
                delete_bb.push_back(bb);
                has_changed = true;
                for(auto succ_bb:bb->get_succ_basic_blocks()){
                    succ_bb->remove_pre_basic_block(bb);
                }
            }
        }
        for(auto bb:delete_bb){
            bb->erase_from_parent();
        }
        delete_bb.clear();
    }
}

void ConstPropagation::phi_check(){
    for(auto bb: func_->get_basic_blocks()){
        std::vector<Instruction *>delete_phi;
        for(auto inst: bb->get_instructions()){
            if(inst->get_instr_type()==Instruction::phi){
                std::vector<Value *>vals;
                std::vector<BasicBlock *>pre_bbs;
                for(int i = 1;i < inst->get_operands().size();i+=2){
                    BasicBlock *phi_bb = dynamic_cast<BasicBlock *>(inst->get_operand(i));
                    Value *val = inst->get_operand(i-1);
                    for(auto pre: bb->get_pre_basic_blocks()){
                        if(phi_bb == pre){
                            pre_bbs.push_back(phi_bb);
                            vals.push_back(val);
                        }
                    }
                }
                if(vals.size()*2 < inst->get_operands().size()){
                    change = true;
                    delete_phi.push_back(inst);
                    auto new_phi = PhiInst::create_phi(inst->get_type(),bb);
                    PhiInst *phi = static_cast<PhiInst *>(inst);
                    new_phi->set_lval(phi->get_lval());
                    bb->add_instr_begin(new_phi);
                    for(int i=0;i < vals.size();i++){
                        new_phi->add_phi_pair_operand(vals[i],pre_bbs[i]);              
                    }
                    value_change(inst,new_phi);
                }
            }
        }
        for(auto inst:delete_phi){
            bb->delete_instr(inst);
        }
    }
}

// 用来判断value是否为ConstantInt，如果不是则会返回nullptr
ConstantInt *cast_to_const_int(Value *value) {
    auto const_int_ptr = dynamic_cast<ConstantInt *>(value);
    if (const_int_ptr) {
        return const_int_ptr;
    } else {
        return nullptr;
    }
}

ConstantFloat *cast_to_const_float(Value *value) {
    auto const_float_ptr = dynamic_cast<ConstantFloat *>(value);
    if (const_float_ptr) {
        return const_float_ptr;
    } else {
        return nullptr;
    }
}

void ConstPropagation::execute() {
    // Start from here!
    while(change){
        change = false;
        for(auto func:module->get_functions()){
            if(func->get_basic_blocks().empty())continue;
            func_=func;
            Propagation();
            useless_bb_delete();
            phi_check();
        }
    }
}

