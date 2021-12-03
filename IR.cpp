#include "IR.h"
#include <unordered_map>

string ArithmeticInst::String() {
    string s = "%" + GetDstReg() + " = ";
    IRValue * op1 = GetOperand(0);
    IRValue * op2 = GetOperand(1);
    OpNode::OpInfo info = opInfo[opcode_];
    if (info.prt_type_ == OpNode::PREFIX) {
        s += string(info.name_) + op1->ValueStr();
    }  else {
        s += op1->ValueStr() + " " + string(info.name_) + " " + op2->ValueStr();
    }
    return s;

}

static unordered_map<OpNode::OpCode, string> jmpcode_2_str = {{OpNode::GT, "JGT"}, {OpNode::GE, "JGE"}, {OpNode::LT, "JLT"},
                                                            {OpNode::LE, "JLE"}, {OpNode::EQ, "JEQ"}, {OpNode::NE, "JNE"}};

string CallInst::String() {
  string s;
  s += ValueStr() + " = call " + callee_ + "(";
  for (size_t i=0; i<args_->size(); i++) {
    s += (*args_)[i];
    if (i < args_->size() -1)
      s += ",";
  }
  s += ")";
  return s;
}

string BranchInst::String() {
    string s;

    IRValue * op1 = GetOperand(0);
    IRValue * op2 = GetOperand(1);
    if (opcode_ != OpNode::INVALID) {
        s += jmpcode_2_str[opcode_] + " ";
        s += op1->ValueStr() + " " + op2->ValueStr() + " " + true_target_ + " " + false_target_;
    } else {
        s = "JMP " + true_target_;
    }
    return s;

}

void TestISA() {
  Instruction * i = new LabelInst("xyz");
  Instruction * r = new ReturnInst(nullptr);
  LabelInst * l = (LabelInst*)i;
  bool x = isa<LabelInst>(i);
  if (x) {
    cout << "yes it is\n";
  }
  bool y = isa<ReturnInst>(r);
  if (y) {
    cout << "yes it is\n";
  }
  bool z = isa<LabelInst>(l);
  if (z) {
    cout << "yes it is\n";
  }
} 
