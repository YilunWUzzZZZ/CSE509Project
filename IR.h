#ifndef IR_H
#define IR_H 
/*
    Simple IR written in a cool fashion
*/
#include <string>
#include <vector>
#include <stack>
#include "ast.h"
using namespace std;

class OpNode;


enum class IRClassID {
  MEM,
  IMM,
  INST,
  LOAD,
  STORE,
  RETURN,
  CALL,
  ARITH,
  BRANCH,
  LABEL,
};

template<class To, class From>
bool isa(From *val) {
  return To::ClassOf(val);
};

class Instruction;
class BasicBlock {
 public:
  BasicBlock(const string & label): label_(label) {};
  ~BasicBlock();
  string GetLabel() { return label_; }
  void SetLabel(const string & label) { label_=label; }
  void AddInst(Instruction * inst) { insts_.push_back(inst); }
  Instruction *FirstInst() { return insts_.size()? insts_[0] : nullptr; }
  vector<Instruction*> & Insts() { return insts_; }
  void AddChild(BasicBlock * bb) { children_.push_back(bb); }
  vector<BasicBlock*> &GetChildren() { return children_; }
  void AddParent(BasicBlock * bb) { parents_.push_back(bb); }
  vector<BasicBlock*> &GetParents() { return parents_; }
 private:
  string label_;
  vector<Instruction*> insts_;
  vector<BasicBlock*> children_;
  vector<BasicBlock*> parents_;
};

class Function {
 public:
  Function(const string & name): name_(name) {}
  ~Function(){}
  void AddBasicBlock( BasicBlock * bb) { bbs_.push_back(bb); }
 private:
  string name_;
  vector<BasicBlock*> bbs_; 
};

class IRValue {
 public:
  IRValue(IRClassID id): class_id_(id) {}
  virtual ~IRValue() {}
  virtual string String()=0;
  virtual string ValueStr() { return "undefined"; };
  IRClassID ClassID() { return class_id_; }
 private:
  IRClassID class_id_;
};

class Memory: public IRValue {
 private:
  string name_;
 public:
  Memory(const string & name): IRValue(IRClassID::MEM), name_(name) {}
  ~Memory() {}
  string String() { return name_; }
  string ValueStr() { return name_; };
  static bool ClassOf(IRValue * v) { return v->ClassID() == IRClassID::MEM; } 
};


template<class T>
class Immediate: public IRValue {
 private:
  T imm_;
 public:
  Immediate(T imm): IRValue(IRClassID::IMM), imm_(imm) {}
  ~Immediate() {}

 string String() { return to_string(imm_); } 
 string ValueStr() { return to_string(imm_); } 
 T GetValue() { return imm_; }
 static bool ClassOf(IRValue * v) { return v->ClassID() == IRClassID::IMM; }
};



class Instruction: public IRValue {
 public:
  Instruction(IRClassID id): IRValue(id) {}
  Instruction(IRClassID id,  vector<IRValue*>* ops = nullptr, int ip=0,  BasicBlock * bb=nullptr): IRValue(id), oprands_(ops), ip_(ip),  bb_(bb) {}
  ~Instruction() {}
  inline int GetIP() const { return ip_; }
  inline void SetIP(int ip) { ip_ = ip; }
  inline void SetDstReg(const string & r) { dst_reg_ = r; }
  inline string GetDstReg() { return dst_reg_; }
  string ValueStr() { return "%" + GetDstReg(); } 
  inline IRValue *GetOperand(int n) const { return (size_t)n < oprands_->size()? (*oprands_)[n] : nullptr; }
  inline BasicBlock* GetBasicBlock() const { return bb_;} 
  inline void SetBasicBlock(BasicBlock * bb) { bb_ = bb; }
  string String() { return dst_reg_; }

  static bool ClassOf(IRValue * v) { return v->ClassID() > IRClassID::INST; }
  static int ip_count;
 private:
  vector<IRValue*> *oprands_;
  string dst_reg_;  // if applicable
  int ip_; // instruction pointer. i.e. index in the instr. list 0...n
  BasicBlock * bb_; // BasicBlock it blongs to
};


// load [mem] reg
class LoadInst: public Instruction {
 public:
  LoadInst(IRValue * src, int ip=0, BasicBlock * bb=nullptr) 
    :Instruction(IRClassID::LOAD, new vector<IRValue*>{src}, ip, bb) {}
  ~LoadInst() {}
  string String() { return GetDstReg() + " =  load " + GetOperand(0)->String(); }

  static bool ClassOf(IRValue * v) { return v->ClassID() == IRClassID::LOAD; }
};

// store  reg [mem]   
class StoreInst: public Instruction {
 public:
  StoreInst(IRValue * src, IRValue *dst, int ip=0, BasicBlock * bb=nullptr) 
    :Instruction(IRClassID::STORE, new vector<IRValue*>{src, dst}, ip, bb) {}
  ~StoreInst() {}
 
  string String() { return "store " + GetOperand(0)->ValueStr()+ " " + GetOperand(1)->String(); }
  static bool ClassOf(IRValue * v) { return v->ClassID() == IRClassID::STORE; }
 private:
  
};

class ReturnInst: public Instruction {
 public:
  ReturnInst(IRValue * rval, int ip=0, BasicBlock * bb=nullptr)
    :Instruction(IRClassID::RETURN, new vector<IRValue*>{rval}, ip, bb) {}
  ~ReturnInst() {}
  string String() { return "return " + GetOperand(0)->String(); }
  static bool ClassOf(IRValue * v) { return v->ClassID() == IRClassID::RETURN; }
};


class ArithmeticInst: public Instruction {
 public:
  ArithmeticInst(OpNode::OpCode opcode, IRValue * op1, IRValue *op2=nullptr, int ip=0, BasicBlock * bb=nullptr) 
    :Instruction(IRClassID::ARITH, new vector<IRValue*>{op1, op2}, ip,  bb), opcode_(opcode) {}
  ~ArithmeticInst() {}
  string String();
  OpNode::OpCode GetOpcode() { return opcode_; }
  static bool ClassOf(IRValue * v) { return v->ClassID() == IRClassID::ARITH; }
 private:
  OpNode::OpCode opcode_;
};


// JXX op1 [oprator] op2 true_target false target
class BranchInst: public Instruction {
  public:
  BranchInst(OpNode::OpCode opcode, IRValue * op1, IRValue *op2, string true_tar, string false_tar, int ip=0, BasicBlock * bb=nullptr) 
    :Instruction(IRClassID::BRANCH, new vector<IRValue*>{op1, op2}, ip, bb), opcode_(opcode), true_target_(true_tar), false_target_(false_tar) {}
  ~BranchInst() {}
  string String();
  void SetTrueTarget(string t) { true_target_ = t;}
  void SetFalseTarget(string t) { false_target_ = t;}
  string GetTrueTarget() {  return true_target_; }
  string GetFalseTarget() { return false_target_; }
  bool IsUncondBranch() { return opcode_ == OpNode::INVALID; }
  OpNode::OpCode GetJmpType() { return opcode_; }
  static bool ClassOf(IRValue * v) { return v->ClassID() == IRClassID::BRANCH; }
 private:
  OpNode::OpCode opcode_;
  string true_target_;
  string false_target_;
};

class CallInst: public Instruction {
  public:
  CallInst(string callee, vector<string> *args = nullptr, int ip=0, BasicBlock * bb=nullptr) 
    :Instruction(IRClassID::CALL, new vector<IRValue*>(), ip, bb), callee_(callee), args_(args) {}
  ~CallInst() {}
  string String(); 
  static bool ClassOf(IRValue * v) { return v->ClassID() == IRClassID::CALL; }
  string GetCalleeName() { return callee_; }
  vector<string> *GetArgs() {return args_;}
 private:
  string callee_;
  vector<string> *args_;
};

// Nothing but placeholder in the code
class LabelInst: public Instruction {
  public:
  LabelInst(string label, int ip=0, BasicBlock * bb=nullptr) 
    :Instruction(IRClassID::LABEL, new vector<IRValue*>(), ip, bb), label_(label) {}
  ~LabelInst() {}
  string String() { return label_; }
  static bool ClassOf(IRValue * v) { return v->ClassID() == IRClassID::LABEL; }
 private:
  string label_;
};

void TestISA(); 

#endif