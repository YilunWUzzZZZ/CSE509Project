#include "IR_lifter.h"
#include "IR.h"
#include "ast.h"


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

void IRLifter::__LiftInst(Instruction * inst) {
  string true_target;
  string false_target;

  if (isa<BranchInst>(inst)) {
    string cond;
    BranchInst * b = (BranchInst*)inst;
    bool is_cond_branch = !b->IsUncondBranch();
    if (is_cond_branch) {
      cond += "if (";
      true_target = b->GetTrueTarget();
      false_target = b->GetFalseTarget();
      cond += LiftExpr(b->GetOperand(0)) + opInfo[b->GetJmpType()].name_ + LiftExpr(b->GetOperand(1));
      cond += ")";
      C_code_.push_back(cond);
      C_code_.push_back("goto " + true_target +";");
      C_code_.push_back("else");
      C_code_.push_back("goto " + false_target + ";");
    } else {
      C_code_.push_back("goto " + true_target + ";");
    }
  } else if (isa<CallInst>(inst)) {
    C_code_.push_back(LiftExpr(inst));
  } else if (isa<ReturnInst>(inst)) {
    IRValue * ret_val = inst->GetOperand(0);
    Immediate<int> * rval = (Immediate<int> *)ret_val;
    int ival = rval->GetValue();
    if (ival == PermNode::ALLOW) {
      C_code_.push_back("ALLOW();");
    } else if (ival == PermNode::DENY) {
      C_code_.push_back("DENY();");
    }
  } else if (isa<StoreInst>(inst)) {
    IRValue * expr = inst->GetOperand(0);
    IRValue * var = inst->GetOperand(1);
    C_code_.push_back(var->String() + " = " + LiftExpr(expr) + ";");
  }
}

void IRLifter::LiftInst(Instruction * inst, const string & label) {
  if (isa<BranchInst>(inst) || isa<StoreInst>(inst) || isa<ReturnInst>(inst)) {
    if (!label.empty()) {
      AddMapping(label);
      C_code_.push_back(label + ":");
    }
    __LiftInst(inst);
  }
}

void IRLifter::LiftBasicBlock(BasicBlock * bb) {
  C_code_.push_back(bb->GetLabel() + ":");
  AddMapping(bb->GetLabel());
  for (Instruction *inst : bb->Insts()) {
    // We only lift branch, store, return here, other immediate stmts are redundent for C
    if (isa<BranchInst>(inst) || isa<StoreInst>(inst) || isa<ReturnInst>(inst))
      __LiftInst(inst); 
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