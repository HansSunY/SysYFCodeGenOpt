#include "Pass.h"
#include "ComSubExprEli.h"
#include "Instruction.h"
#include <set>
#include <algorithm>
#include <string>
#define CONST_INT(num) ConstantInt::get(num, module.get())
bool be_changed=false;
int mark=0;
void ComSubExprEli::execute() {
    for(auto fun: module->get_functions()){
        if(fun->get_basic_blocks().empty())continue;
        func_ = fun;
                merge_phi();
        do{
        be_changed=false;
        std::list<Instruction*> valid_inst;
        get_valid_table(&valid_inst);
        cse(&valid_inst);
        }while(be_changed==true);
    }
    //module->set_print_name();
    printf("ComSubExprEli over\n");
    return;
}
bool is_dom(BasicBlock* child,BasicBlock*parent){
    bool a=false;
    if(child==parent)
    return true;
    if(child->get_pre_basic_blocks().size()==0)
        return false;
    if(child->get_pre_basic_blocks().size()==1&&child->get_pre_basic_blocks().front()==parent)
        return true;
    for(auto block:child->get_pre_basic_blocks()){
        if(block==parent)
            a=true;
    }
    if(a==true&&child->get_pre_basic_blocks().size()==1)
    return true;
    else if(a==true&&child->get_pre_basic_blocks().size()>1)
    return false;
    else {
        for(auto b:child->get_pre_basic_blocks()){
            if(!is_dom(b,parent)){
                return false;
            }
        }
    }
}
Instruction* ComSubExprEli::all_path_have(BasicBlock* parent,Instruction* inst){
        mark=0;
        if(is_dom(inst->get_parent(),parent))
        return NULL;
        for(auto inst2:parent->get_instructions()){
            if(compare_inst(inst,inst2)){
                return inst2;
            }

        }
        for(auto block:parent->get_pre_basic_blocks())
            all_path_have(block,inst);
        //all_path_have(block->get_pre_basic_blocks(),inst);
        return NULL;
}
void ComSubExprEli::merge_phi(){
    for(auto block:func_->get_basic_blocks()){
        std::list<Instruction*> list_add;
        std::list<Instruction*> list_delete;
        for(auto inst:block->get_instructions()){
            if(!is_valid_expr(inst))
            break;
            mark=0;
            std::map<BasicBlock*,Instruction*> map;
            for(auto parent:block->get_pre_basic_blocks()){
                Instruction* inst2=all_path_have(parent,inst);
                if(inst2==NULL){
                 mark=1;
                break;
                }
                else{
                    map[parent]=inst2;
                }
            }
            if(mark==0){
                std::cout<<inst->get_name()<<std::endl;
                auto phi=PhiInst::create_phi(inst->get_type(),block);
                for(auto val:map){
                    phi->add_phi_pair_operand(val.second,val.first);
                }
                for(auto use: inst->get_use_list()){
                Instruction * use_inst = dynamic_cast<Instruction *>(use.val_);
                //printf("inst_type:%d",use_inst->get_instr_type());
                use_inst->set_operand(use.arg_no_, phi);
                }
                list_add.push_back(phi);
                list_delete.push_back(inst);
             // block->add_instr_begin(phi);
                printf("add\n");
            }
            
        }
        for(auto inst3:list_add){
            //if(inst==NULL)
            block->add_instr_begin(inst3);
            printf("%d\n",inst3->get_num_operand());
        }
        for(auto inst4:list_delete){
            block->delete_instr(inst4);
        }

    }
}

bool ComSubExprEli::is_valid_expr(Instruction *inst) {
    return !(inst->is_void()||inst->is_call()||inst->is_phi()||inst->is_alloca()||inst->is_load()||inst->is_cmp()||inst->is_zext());//TODO:CHECK VALID INST
}

void ComSubExprEli::get_valid_table(std::list<Instruction*> *valid_inst){
for(auto bb: func_->get_basic_blocks()){
    for(auto inst:bb->get_instructions()){
        if(is_valid_expr(inst)){
            (*valid_inst).push_back(inst);
           // printf("type:%d\n",(inst)->get_instr_type());
        }
        }
    }
}

void ComSubExprEli::cse(std::list<Instruction*> *valid_inst){
int flag=0;
    for(auto inst=(*valid_inst).rbegin();inst!=(*valid_inst).rend();inst++){
        for(auto inst2=(*valid_inst).rbegin();(inst2!=(*valid_inst).rend());inst2++){
            //std::cout<<"inst2"<<(*inst2)->get_name()<<std::endl;           
            if(inst2==inst)
            continue;
            flag=0;
            if(compare_inst(*inst,*inst2)&&is_dom((*inst)->get_parent(),(*inst2)->get_parent())){
                //if((*inst)->get_parent()!=NULL)
                flag=1;
                be_changed=true;
                //std::cout<<"inst1"<<(*inst)->get_name()<<std::endl;
                //std::cout<<"inst2"<<(*inst2)->get_name()<<std::endl;
                for(auto use: (*inst)->get_use_list()){
                Instruction * use_inst = dynamic_cast<Instruction *>(use.val_);
                //printf("inst_type:%d",use_inst->get_instr_type());
                use_inst->set_operand(use.arg_no_, *inst2);
            }
            }
            if(flag==1){
                BasicBlock* bb=(*inst)->get_parent();
                bb->delete_instr(*inst);
                (*inst)->set_parent(NULL);
                break;
            }
        }
    }
     
}
bool ComSubExprEli::compare_inst(Instruction* inst1,Instruction* inst2){
    if(inst1->get_instr_type()==inst2->get_instr_type()&&inst1->get_num_operand()==inst2->get_num_operand()){
        for(int i=0;i!=inst1->get_num_operand();i++){
            if(inst1->get_operand(i)==inst2->get_operand(i))
            continue;
            else{
                auto inst3=dynamic_cast<ConstantInt*>(inst1->get_operand(i));
                auto inst4=dynamic_cast<ConstantInt*>(inst2->get_operand(i));
                if(inst3==NULL||inst4==NULL||inst3->get_value()!=inst4->get_value())
                return false;
                else 
                continue;
            }
              // std::cout<<inst1->get_operand(0)->get_name()<<std::endl;
        }

        return true;
    }
    else return false;
    
}

