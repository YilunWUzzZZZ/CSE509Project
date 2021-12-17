#include "IR_lifter.h"
#include "IR.h"
#include "ast.h"
#include "syscall.h"


static string LiftExpr(IRValue * expr) {
  string code;
  if (isa<ArithmeticInst>(expr)) {
    ArithmeticInst * inst = (ArithmeticInst*)expr;
    IRValue *op1 = inst->GetOperand(0);
    IRValue *op2 = inst->GetOperand(1);
    code = LiftExpr(op1);
    if (inst->GetOpcode() == OpNode::UMINUS) {
      code = "(-" + code +")";
      return code;
    }
    code += opInfo[inst->GetOpcode()].name_ + LiftExpr(op2);
    code = "(" + code + ")";
    return code;
  }  else if (isa<Immediate<int>>(expr) || isa<Memory>(expr)) {
    return expr->String(); 
  } else if (isa<CallInst>(expr)) {
    CallInst * call = (CallInst*)expr;
    code = call->GetCalleeName() + "(";
    auto args = call->GetArgs();
    for (size_t i=0; i<args->size(); i++) {
      code += (*args)[i];
      if (i < args->size() - 1) {
        code += ", ";
      }
    }
    code += ")";
    return code;
  } else {
    return "<unknown>";
  }
  
  
}

void IRLifter::__LiftInst(Instruction * inst, list<PtraceCode*>* code) {
  string true_target;
  string false_target;

  PtraceCode *pcode = new PtraceCode();
  if (isa<BranchInst>(inst)) {
    string cond;
    BranchInst * b = (BranchInst*)inst;
    bool is_cond_branch = !b->IsUncondBranch();
    if (is_cond_branch) {
      pcode->type = PTRACE_IF;
      pcode->true_target = b->GetTrueTarget();
      pcode->false_target = b->GetFalseTarget();
      pcode->expr = LiftExpr(b->GetOperand(0)) + opInfo[b->GetJmpType()].name_ + LiftExpr(b->GetOperand(1));
      block_ref_cnt_[pcode->true_target]++;
      block_ref_cnt_[pcode->false_target]++;
    } else {
      pcode->type = PTRACE_GOTO;
      pcode->true_target = b->GetTrueTarget();
      block_ref_cnt_[pcode->true_target]++;
    }
  } else if (isa<CallInst>(inst)) {
    pcode->type = PTRACE_STMT;
    pcode->expr =  LiftExpr(inst);
  } else if (isa<ReturnInst>(inst)) {
    IRValue * ret_val = inst->GetOperand(0);
    Immediate<int> * rval = (Immediate<int> *)ret_val;
    int ival = rval->GetValue();
    if (ival == PermNode::ALLOW) {
      pcode->type = PTRACE_ALLOW;
    } else if (ival == PermNode::DENY) {
      pcode->type = PTRACE_DENY;
    }
  } else if (isa<StoreInst>(inst)) {
    pcode->type = PTRACE_ASSIGN;
    IRValue * expr = inst->GetOperand(0);
    IRValue * var = inst->GetOperand(1);
    pcode->expr = LiftExpr(expr);
    pcode->dst = var->String();
  }
  code->push_back(pcode);
}

void IRLifter::LiftInst(Instruction * inst, list<PtraceCode*>* code, const string & label) {
  (void)label;
  __LiftInst(inst, code);
}

void IRLifter::LiftBasicBlock(BasicBlock * bb) {
  auto * code = new list<PtraceCode*>();
  code_blocks_[bb->GetLabel()] = code;
  block_order_.push_back(bb->GetLabel());
  for (Instruction *inst : bb->Insts()) {
    // We only lift branch, store, return here, other immediate stmts are redundent for C
    if (isa<BranchInst>(inst) || isa<StoreInst>(inst) || isa<ReturnInst>(inst))
      __LiftInst(inst, code); 
  }
}



void IRLifter::GenCode() {
  for (auto bb_label : block_order_) {
    if (used_[bb_label]) {
      continue;
    }
    GenBasicBlockCode(code_blocks_[bb_label], bb_label, 0);
  }
}

void IRLifter::GenBasicBlockCode(list<PtraceCode*>* bb, const string & label, int indent) {
  used_[label] = true;
  if (interface_points_->count(label) || block_ref_cnt_[label] > 1) {
    C_code_.push_back(label + ":");
  }  
  for (auto pcode :  *bb) {
    if (pcode->type == PTRACE_IF) {
      string cond = string(indent, ' ') +  "if (" + pcode->expr + ") {";
      C_code_.push_back(cond);
      if (block_ref_cnt_[pcode->true_target] == 1) {
        GenBasicBlockCode(code_blocks_[pcode->true_target], pcode->true_target, indent+2);
      } else {
        C_code_.push_back(string(indent+2, ' ') + "goto " + pcode->true_target + ";");
      }
      C_code_.push_back(string(indent, ' ') + "} else {");
      if (block_ref_cnt_[pcode->false_target] == 1) {
        GenBasicBlockCode(code_blocks_[pcode->false_target], pcode->false_target, indent+2);
      } else {
        C_code_.push_back(string(indent+2, ' ') + "goto " + pcode->false_target + ";");
      }
      C_code_.push_back(string(indent, ' ') + "}");
    } else if (pcode->type == PTRACE_GOTO) {
      C_code_.push_back(string(indent, ' ') + "goto " + pcode->true_target + ";");
    } else if (pcode->type == PTRACE_ALLOW) {
      C_code_.push_back(string(indent, ' ') + "ALLOW();");
    } else if (pcode->type == PTRACE_DENY) {
      C_code_.push_back(string(indent, ' ') + "DENY();");
    } else if (pcode->type == PTRACE_ASSIGN) {
      C_code_.push_back(string(indent, ' ') + pcode->dst + " = " + pcode->expr + ";");
      if (string_args_->count(pcode->dst)) {
        C_code_.push_back(string(indent, ' ') + "ptrace_copy_back_string(child, " + pcode->dst + ", GET_ARG(" +to_string((*arg_index_)[pcode->dst])+"));");
      } else if (mem_args_size_->count(pcode->dst)) {
        C_code_.push_back(string(indent, ' ') +"ptrace_copy_back_mem(child,  GET_ARG(" + to_string((*arg_index_)[pcode->dst])+"), " 
                          + pcode->dst+ ","+(*mem_args_size_)[pcode->dst] +");");
      } else {
        C_code_.push_back(string(indent, ' ') + "SET_ARG(" + to_string((*arg_index_)[pcode->dst]) + ", " + pcode->expr + ");");
      }
    } else if (pcode->type == PTRACE_STMT) {
      C_code_.push_back(string(indent, ' ') + pcode->expr +";");
    }
  }
}


void PrintCCode(ostream & os, const vector<string> & code) {
  int indent = 0;
  for (auto & s: code) {
    if (s.find('}') != string::npos ){
      indent -= 2;
      os << s << "\n";
    } else if (s.find('{') != string::npos) {
      os << s << "\n";
      indent += 2;
    } else {
      os << s << "\n";
    }
  }
}

int IRLifter::ret_data = 0;