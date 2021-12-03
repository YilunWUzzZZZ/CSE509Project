#include "ast.h"
#include "error.h"
#include "IR.h"
#include "IR_gen.h"
#include <map>

AstNode::AstNode(AstNodeType nt, int line, int column, string file):
  ProgramElem(line, column, file) {
  node_type_ = nt;
}


inline static void PrintIndent(ostream& os, int indent) {
  for (int i=0; i<indent; i++) {
    os << " ";
  }
}


const OpNode::OpInfo opInfo[] = {

  {OpNode::UMINUS, "-",  1, 0, OpNode::PREFIX, {Type::INT}, Type::INT,  },
  {OpNode::PLUS, "+",  2, 1, OpNode::INFIX, {Type::INT, Type::INT}, Type::INT,   },
  {OpNode::MINUS, "-",  2, 1, OpNode::INFIX, {Type::INT, Type::INT}, Type::INT,  },
  {OpNode::MULT, "*",  2, 0, OpNode::INFIX, {Type::INT, Type::INT}, Type::INT},
  {OpNode::DIV, "/",  2, 1, OpNode::INFIX, {Type::INT, Type::INT}, Type::INT,     },

  {OpNode::MOD, "%",  2, 1, OpNode::INFIX, {Type::INT, Type::INT}, Type::INT,    },

  {OpNode::EQ, "==", 2, 0, OpNode::INFIX, {Type::INT, Type::INT}, Type::BOOLEAN,     },
  {OpNode::NE, "!=", 2, 0, OpNode::INFIX, {Type::INT, Type::INT}, Type::BOOLEAN,       },
  {OpNode::GT, ">",  2, 0, OpNode::INFIX, {Type::INT, Type::INT}, Type::BOOLEAN,     },
  {OpNode::LT, "<",  2, 0, OpNode::INFIX, {Type::INT, Type::INT}, Type::BOOLEAN,    },
  {OpNode::GE, ">=", 2, 0, OpNode::INFIX, {Type::INT, Type::INT}, Type::BOOLEAN,    },
  {OpNode::LE, "<=", 2, 0, OpNode::INFIX, {Type::INT, Type::INT}, Type::BOOLEAN,  },


  {OpNode::AND, "&&",  2, 1, OpNode::INFIX, {Type::BOOLEAN, Type::BOOLEAN}, Type::BOOLEAN,               },
  {OpNode::OR, "||",  2, 1, OpNode::INFIX, {Type::BOOLEAN, Type::BOOLEAN}, Type::BOOLEAN,              },
  {OpNode::NOT, "!",  1, 0, OpNode::PREFIX, {Type::BOOLEAN}, Type::BOOLEAN,                          }, 

  {OpNode::BITNOT, "~",  1, 0, OpNode::PREFIX, {Type::INT}, Type::INT,                },
  {OpNode::BITAND, "&",  2, 1, OpNode::INFIX, {Type::INT, Type::INT}, Type::INT,  },
  {OpNode::BITOR, "|",  2, 1, OpNode::INFIX, {Type::INT, Type::INT}, Type::INT,  },
  {OpNode::BITXOR, "^",  2, 0, OpNode::INFIX, {Type::INT, Type::INT}, Type::INT, },

  {OpNode::SHL, "<<", 2, 1, OpNode::INFIX, {Type::INT, Type::INT}, Type::INT,  },
  {OpNode::SHR, ">>", 2, 1, OpNode::INFIX, {Type::INT, Type::INT}, Type::INT,    },


  {OpNode::INVALID, "invalid",            0, 0, OpNode::PREFIX, {}, Type::NONE,            }
};

void RefExprNode::Print(ostream& os, int indent) const {
  os << name_; 
}

void OpNode::Print(ostream& os, int indent) const {
  OpNode::OpInfo opinfo = opInfo[opcode_];
  int arity = opinfo.arity_;
  if (opinfo.prt_type_ == OpNode::PREFIX) {
    os << opinfo.name_;
    if (arity  > 0) {
      if (opinfo.need_paren_) 
        os << '(';
      for (unsigned i=0; i < arity  -1; i++) {
        if (arg_[i])
          arg_[i]->Print(os, 0);
        else os << "nullptr";
        os << ", ";
      }
      if (arg_[arity -1])
        arg_[arity -1]->Print(os, 0);
      else 
        os << "nullptr";
      if (opinfo.need_paren_) 
        os << ")";
    }
  }
  else if ((opinfo.prt_type_ == OpNode::INFIX) && (arity == 2)) {
    if (opinfo.need_paren_)
      os << "(";
    if(arg_[0])
      arg_[0]->Print(os, 0);
    else os << "nullptr";
    os << opinfo.name_; 
    if(arg_[1])
      arg_[1]->Print(os, 0);
    else os << "nullptr";
    if (opinfo.need_paren_) 
      os << ")";
  }
  else internalErr("Unhandled case in OpNode::print");
}


void ValueNode::Print(ostream& os, int indent) const {
  const Value * v = GetValue();
  Type t = v->GetType();
  if (t == Type::INT) {
    os << v->ival();
  } else if (t == Type::BOOLEAN) {
    os << v->bval();
  } else if ( t == Type::STRING) {
    os << v->cval();
  } else {
    os << "None";
  }
}

void CompoundStmtNode::Print(ostream& os, int indent) const {
  if (!stmts_) return;
  for (auto s : *stmts_) {
    if (s)
      s->Print(os, indent);
    else {
      PrintIndent(os, indent);
      os << "nullptr";
    }
  }
 
}

void InvocationNode::Print(ostream& os, int indent) const {
  os << func_name_ << "(";
  for (size_t i=0; i<params_->size(); i++) {
    os << (*params_)[i];
    if (i < params_->size() -1) {
      os << ",";
    }
  }
  os << ")";
}

void IfNode::Print(ostream& os, int indent) const {
  PrintIndent(os, indent);
  os << "if ";
  os << "(";
  if (cond_) {
    cond_->Print(os, 0);
  } 
  os << ")";

  if (then_) {
    os << "  {\n";
    then_->Print(os, indent+2);
    PrintIndent(os, indent);
    os << "}\n";
  } 

  if (else_) {
    PrintIndent(os, indent);
    os << "else {\n";
    else_->Print(os, indent+2);
    PrintIndent(os, indent);
    os << "}\n";
  
  }
  
}

void AssignmentNode::Print(ostream& os, int indent) const {
  PrintIndent(os, indent);
  os << arg_ << " = ";
  expr_->Print(os, 0);
  os << "\n";
}

void PermNode::Print(ostream& os, int indent) const {
  PrintIndent(os, indent);
  os << ((type_ == PermType::ALLOW)? "allow" : "deny") << endl; 
}


/******************* IR Gen ******************/
static void GenBranchCode(ExprNode * expr, list<Instruction*> & code, CodeGenMgr & mgr) {
  if (expr->GetExprNodeType() == ExprNodeType::OP_NODE) {
    OpNode * op_node = (OpNode*)expr;
    if (op_node->GetOpCode() >= OpNode::EQ && op_node->GetOpCode() <= OpNode::NOT) {
      expr->IRGen(code, mgr);
      return;
    } 
  } 
  // This is a stand-alone expression: No comparator, No &&, ||, ! 
  // Convert expr to (expr != 0)
  
  OpNode * expr_ne_zero = new OpNode(OpNode::NE, expr, new ValueNode(new Value((int)0)));
  expr_ne_zero->IRGen(code, mgr);
  delete expr_ne_zero;
}

IRValue *IfNode::IRGen(list<Instruction*> & code, CodeGenMgr & mgr) {
  string true_target =  mgr.NewLabel();
  string false_target = mgr.NewLabel();
 
  mgr.true_target = true_target;
  mgr.false_target = false_target;
  
  GenBranchCode(cond_, code, mgr);
  Instruction * true_label = new LabelInst(true_target);
  code.push_back(true_label);
  then_->IRGen(code, mgr);
  Instruction * false_label = new LabelInst(false_target);
  
  if (else_) {
    string out_target = mgr.NewLabel();
    Instruction * out_label = new LabelInst(out_target);
    Instruction * jmp_out = new BranchInst(OpNode::INVALID, nullptr, nullptr, out_target, "");
    code.push_back(jmp_out);
    code.push_back(false_label);
    else_->IRGen(code, mgr);
    code.push_back(out_label);

  } else {
    code.push_back(false_label);
  }
  return nullptr;
}

IRValue *CompoundStmtNode::IRGen(list<Instruction*> & code, CodeGenMgr & mgr) {
  for (size_t i=0; i<stmts_->size(); i++) {
    StmtNode *s = (*stmts_)[i];
    s->IRGen(code, mgr);
  }
  return nullptr;
}



IRValue *OpNode::IRGen(list<Instruction*> & code, CodeGenMgr & mgr) {
  string true_label, false_label;
  IRValue * op1, *op2;
  Instruction *inst = nullptr;
 
  switch (opcode_) {
  case OpNode::AND:
  {
    mgr.SaveBranchTarget();
    true_label = mgr.NewLabel();
    mgr.true_target = true_label;
    GenBranchCode(arg_[0], code, mgr);
    mgr.RestoreBranchTarget();
    LabelInst * true_label_inst = new LabelInst(true_label);
    code.push_back(true_label_inst);
    GenBranchCode(arg_[1], code, mgr);
    break;
  }

  case OpNode::OR:
  {
    mgr.SaveBranchTarget();
    false_label = mgr.NewLabel();
    mgr.false_target = false_label;
    GenBranchCode(arg_[0], code, mgr);
    mgr.RestoreBranchTarget();
    LabelInst * false_label_inst = new LabelInst(false_label);
    code.push_back(false_label_inst);
    GenBranchCode(arg_[1], code, mgr);
    break;
  }
  case OpNode::NOT:
  {
    true_label = mgr.true_target;
    mgr.true_target = mgr.false_target;
    mgr.false_target = true_label;
    GenBranchCode(arg_[0], code, mgr);
    break;
  }
  case OpNode::GT: case OpNode::GE: case OpNode::LT: 
  case OpNode::LE: case OpNode::EQ: case OpNode::NE: 
  { 
    op1 = arg_[0]->IRGen(code, mgr);
    op2 = arg_[1]->IRGen(code, mgr);
    BranchInst * branch = new BranchInst(opcode_, op1, op2, mgr.true_target, mgr.false_target);
    code.push_back(branch);
    break;
  }

  case OpNode::UMINUS:
  {
    op1 = arg_[0]->IRGen(code, mgr);
    inst = new ArithmeticInst(opcode_, op1);
    code.push_back(inst);
    break;
  }

  case OpNode::INVALID:
    internalErr("[IRGEN:] Operation Invalid");
    break;
  
  default: // binary ops
  {
    op1 = arg_[0]->IRGen(code, mgr);
    op2 = arg_[1]->IRGen(code, mgr);
    inst = new ArithmeticInst(opcode_, op1, op2);
    code.push_back(inst);
    break;
  }

  };
  return inst;
}

IRValue *AssignmentNode::IRGen(list<Instruction*> & code, CodeGenMgr & mgr) {
  IRValue * exp = expr_->IRGen(code, mgr);
  Memory *mem = new Memory(arg_);
  StoreInst * store = new StoreInst(exp, mem);
  code.push_back(store);
  return nullptr;
}

IRValue *PermNode::IRGen(list<Instruction*> & code, CodeGenMgr & mgr) {
  auto *rval = new Immediate<PermType>(type_);
  ReturnInst * ret = new ReturnInst(rval);
  code.push_back(ret);
  return nullptr;
}

IRValue *RefExprNode::IRGen(list<Instruction*> & code, CodeGenMgr & mgr) {
  return new Memory(name_);
}

IRValue *ValueNode::IRGen(list<Instruction*> & code, CodeGenMgr & mgr) {
  return new Immediate<int>(GetValue()->ival());
}

IRValue * InvocationNode::IRGen(list<Instruction*> & code, CodeGenMgr & mgr) {
  CallInst *call_inst = new CallInst(func_name_, params_);
  code.push_back(call_inst);
  return call_inst;
}



