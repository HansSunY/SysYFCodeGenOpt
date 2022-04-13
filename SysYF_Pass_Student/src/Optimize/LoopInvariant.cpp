#include "LoopInvariant.h"
#define max_block_num 100
BasicBlock* blocktable[max_block_num];
int stack[max_block_num];
int top=0;
bool InStack[max_block_num];
int dfn[max_block_num]={-1,-1,-1,-1,-1,-1,-1,-1,-1};

int low[max_block_num];
int index=0;
std::vector<int> edge[max_block_num];
int InComponent[max_block_num];

bool be_adjusted=false;
void init_data(){
    for(int i=0;i<max_block_num;i++){
        blocktable[i]=NULL;
        stack[i]=-1;
        InStack[i]=false;
        dfn[i]=-1;
        low[i]=-1;
        edge[i].clear();
        InComponent[i]=-1;
    }   
    top=0;
    index=0;

}
void LoopInvariant::execute(){
    
for(auto fun: module->get_functions()){
    if(fun->get_basic_blocks().empty())continue;
    func_ = fun;
    int ComponentNumber=0;
    std::vector<int> component[max_block_num];
    init_data();
    create_table(-1);
    get_loop(0,&ComponentNumber,component);
    do{
    be_adjusted=false;
    loop_motion(ComponentNumber,component);
    adjust_br();}while(be_adjusted==true);

}
    printf("Loop Invariant over\n");
}
int get_num(BasicBlock* block,std::list<BasicBlock*>list){
    int i=0;
    for(auto block2:list)
    {

        if(block2==block)
            return i;
        i++;
    }
}
void LoopInvariant::create_table(int j){
    int i=0;    
    for(auto block:func_->get_basic_blocks()){
        blocktable[i]=block;
        if(i!=j){
        for(auto sblock:block->get_succ_basic_blocks()){
            edge[i].push_back(get_num(sblock,func_->get_basic_blocks()));
        }
        }
        i++;
    }

}
void LoopInvariant::get_loop(int i,int* ComponentNumber,std::vector<int>* component){
    int j;
    dfn[i]=low[i]=index++;
    InStack[i]=true;stack[++top]=i;
    for(int e=0;e<edge[i].size();e++){
        j=edge[i][e];
        if(dfn[j]==-1)
        {
            get_loop(j,ComponentNumber,component);
            low[i]=std::min(low[i],low[j]);
        }
        else
        if(InStack[j])
        low[i]=std::min(low[i],dfn[j]);
    }
    if(dfn[i]==low[i])
    {
        (*ComponentNumber)++;
        do{
            j=stack[top--];
            InStack[j]=false;
            component[*ComponentNumber].push_back(j);
            InComponent[j]=*ComponentNumber;
        }
        while(j!=i);
    }
}
bool LoopInvariant::is_valid_expr(Instruction *inst) {
    return !(inst->is_void()||inst->is_call()||inst->is_phi()||inst->is_alloca()||inst->is_cmp()||inst->is_zext()||inst->is_div());//TODO:CHECK VALID INST
}

bool is_in_loop(int i,std::vector<int> v){
    for(auto j:v){
        if(i==j)
        return true;
    }
    return false;
}
bool is_parent_b(BasicBlock* child,BasicBlock* parent){
    if(child==parent)
        return true;
    if(child->get_pre_basic_blocks().size()==0)
        return false;
    for(auto block:child->get_pre_basic_blocks()){
        if(block==parent)
        return true;
        else{
            is_parent_b(block,parent);
        }
    }
}
BasicBlock* LoopInvariant::find_co_dom(std::list<BasicBlock*>list,std::vector<int>loop){
    std::list<BasicBlock*> tmp;
    for(auto block:list){
        if(is_in_loop(get_num(block,func_->get_basic_blocks()),loop)){
            tmp.push_back(block);
        }
    }
    for(auto block:tmp){
        list.remove(block);
    }
    bool is_co_dom;
    if(list.size()==1)
    return list.front();
    for(auto block:list){

        is_co_dom=true;
        for(auto block2:list){
            if(block!=block2){
                if(!is_parent_b(block2,block)){
                            printf("child:");
        std::cout<<block2->get_name()<<std::endl;
                    is_co_dom=false;
                    break;
                }
            }
        }
        if(is_co_dom==true)
        return block;
    }
    std::list<BasicBlock*>list2;
    for(auto b:list){
        list2.splice(list2.begin(),b->get_pre_basic_blocks());
    }
    find_co_dom(list2,loop);
}
void LoopInvariant::loop_motion(int ComponentNumber,std::vector<int>* component){
    for(int i=1;i<=ComponentNumber;i++){
        std::vector<int> loop=component[i];
        if(loop.size()<=1)
        continue;
        else{
            init_data();
            int ComponentNumber_next=0;
            std::vector<int>component_next[max_block_num];
            create_table(loop.back());
            
            for(auto bl:blocktable[loop.back()]->get_succ_basic_blocks()){
                if(is_in_loop(get_num(bl,func_->get_basic_blocks()),loop)){
                    get_loop(get_num(bl,func_->get_basic_blocks()),&ComponentNumber_next,component_next);
                    loop_motion(ComponentNumber_next,component_next);
                }
            }
            for(auto i:loop){
                BasicBlock* block=blocktable[i];
                std::vector<Instruction*> inst2;
                BasicBlock* parent;
                int mark=0;
                for(auto inst:block->get_instructions()){
                    if(is_valid_expr(inst)){
                        mark=0;
                        for(auto op:inst->get_operands()){
                            if(dynamic_cast<ConstantInt*>(op)==NULL&&is_in_loop(get_num(dynamic_cast<Instruction*>(op)->get_parent(),func_->get_basic_blocks()),loop)){
                                mark=1;
                            }
                        }
                        if(mark==0){
                            inst2.push_back(inst);
                            BasicBlock* b=find_co_dom((blocktable[loop.back()])->get_pre_basic_blocks(),loop);
                            b->add_instruction(inst);
                            be_adjusted=true;  
                            inst->set_parent(b);
                        }
                        /*
                        if(dynamic_cast<ConstantInt*>(inst->get_operand(0))!=NULL&&dynamic_cast<ConstantInt*>(inst->get_operand(1))!=NULL){
                            inst2.push_back(inst);
                            BasicBlock* b=find_co_dom((blocktable[loop.back()])->get_pre_basic_blocks(),loop);
                            b->add_instruction(inst);
                            be_adjusted=true;  
                            inst->set_parent(b);

                        }
                        else if((dynamic_cast<Instruction*>(inst->get_operand(0))!=NULL&&!is_in_loop(get_num(dynamic_cast<Instruction*>(inst->get_operand(0))->get_parent(),func_->get_basic_blocks()),loop)&&dynamic_cast<ConstantInt*>(inst->get_operand(1))!=NULL)||(dynamic_cast<Instruction*>(inst->get_operand(1))!=NULL&&!is_in_loop(get_num(dynamic_cast<Instruction*>(inst->get_operand(1))->get_parent(),func_->get_basic_blocks()),loop)&&dynamic_cast<ConstantInt*>(inst->get_operand(0))!=NULL)){
                            inst2.push_back(inst);
                            BasicBlock* b=find_co_dom((blocktable[loop.back()])->get_pre_basic_blocks(),loop);
                            b->add_instruction(inst);
                            be_adjusted=true;  
                            inst->set_parent(b);                          
                        }
                        else if(!is_in_loop(get_num(dynamic_cast<Instruction*>(inst->get_operand(0))->get_parent(),func_->get_basic_blocks()),loop)&&!is_in_loop(get_num(dynamic_cast<Instruction*>(inst->get_operand(1))->get_parent(),func_->get_basic_blocks()),loop)){
                            inst2.push_back(inst);
                            BasicBlock* b=find_co_dom((blocktable[loop.back()])->get_pre_basic_blocks(),loop);
                            b->add_instruction(inst);
                            be_adjusted=true;  
                            inst->set_parent(b);   
                        }*/
                    }
                }
                
            for(auto ist:inst2)
            block->delete_instr(ist);
            }
        }
    }
}

void LoopInvariant::adjust_br(){
    Instruction* temp;
    for(auto block:func_->get_basic_blocks()){
        temp=NULL;
        for(auto inst:block->get_instructions())
            if(inst->is_br())
                temp=inst;
        if(temp!=NULL){
        block->delete_instr(temp);
        block->add_instruction(temp);
        }
    }
}