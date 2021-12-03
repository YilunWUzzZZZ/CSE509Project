#ifndef AST_H
#define AST_H

#include "value.h"
#include "program_elem.h"

#include <set>
#include <map>
#include <bitset>
#include <assert.h>
#include <sstream>
#include <vector>
#include <list>
#define INDENT_BASE 2
using namespace std;

class Instruction;
class CodeGenMgr;
class IRValue;


/*****************************************************************************
   Here is the class hierarchy:
                                      ProgramElem
                                           |
                                        AstNode
                   +-----------------+---------------------------+
                   |                                             | 
                  ExprNode                                   StmtNode
                   |                                             |
                   |                                             |
          +---------+----------+-----------+                     |
          |         |          |           |                     |
      RefExprNode  OpNode  ValueNode  InvocationNode             |
                                        +------------------------------------+------+
                                        |                 |            |            |
                                AssignmentStmt         PermStmt      IfNode        CompoundStmt
     
     
******************************************************************************/

enum AstNodeType  {
   EXPR_NODE,
   STMT_NODE,
};


class AstNode: public ProgramElem {
 private: 
  AstNodeType node_type_;
  const AstNode* operator=(const AstNode& other); /* disable asg */

 public: 
  AstNode(AstNodeType nt, int line=0, int column=0, string file="");
  virtual ~AstNode() {};


  AstNodeType NodeType() const { return node_type_;}

  virtual void Print(ostream& os, int indent=0) const=0;
  virtual IRValue *IRGen(list<Instruction*> & code, CodeGenMgr & mgr) { return nullptr; }

};

// inline ostream& operator<<(ostream& os, const AstNode& an) {
//   an.Print(os);
//   return os;
// };

enum ExprNodeType {
   REF_EXPR_NODE, 
   OP_NODE,
   VALUE_NODE, 
   INV_NODE
};

/****************************************************************/

class ExprNode: public AstNode {
 public:
 private:
  ExprNodeType expr_type_;
  Value *val_; 

 public:
  ExprNode(ExprNodeType et, Value* val=0, int line=0, int col=0,
       string file="") // val is saved, but not copied
    : AstNode(EXPR_NODE, line,col, file), expr_type_(et) { val_ = val; };
  virtual ~ExprNode() {};

  ExprNodeType GetExprNodeType() const { return expr_type_;};
  void SetExprNodeType(ExprNodeType t) { expr_type_ = t; };
  void SetValue(Value * v) { val_ = v; }
  Value * GetValue() const { return val_;}
  // Type GetType() const { return val_->GetType(); }

  void Print(ostream& os, int indent=0) const {};
  virtual IRValue *IRGen(list<Instruction*> & code, CodeGenMgr & mgr) { return nullptr; }
};


/****************************************************************/
class RefExprNode: public ExprNode {
 private:
  string name_;
 
 public:
  RefExprNode(string name, int line=0, int column=0, string file=""): 
    ExprNode(REF_EXPR_NODE, 0, line, column, file), name_(name) {}
  ~RefExprNode() {};


  string GetName() const {
    return name_;
  };

  void Print(ostream& os, int indent=0) const;
  IRValue *IRGen(list<Instruction*> & code, CodeGenMgr & mgr);

};

/****************************************************************/

class OpNode: public ExprNode {
 public:
  enum OpCode {
    UMINUS, PLUS, MINUS, MULT, DIV, MOD, 
    EQ, NE, GT, LT, GE, LE,
    AND, OR, NOT, 
    BITNOT, BITAND, BITOR, BITXOR, 
    SHL, SHR,
    INVALID
  };

    
  enum OpPrintType {PREFIX, INFIX, POSTFIX};
  struct OpInfo {
    OpCode code_;
    const char* name_;
    int arity_;
    int need_paren_;
    OpPrintType prt_type_;
    Type arg_type_[2]; 
    Type out_type_;
    
  };

 private: 
  OpCode   opcode_;
  vector<ExprNode*> arg_;


 public:
  OpNode(OpCode op, ExprNode *l, ExprNode *r=nullptr,
   int line=0, int column=0, string file=""): ExprNode(OP_NODE, nullptr, line, column, file) {
     opcode_ = op;
     arg_.push_back(l);
     if (r) 
       arg_.push_back(r);
   }

  //OpNode(const OpNode&);
  ~OpNode() {};


  OpCode GetOpCode() const { return opcode_;};

  void SetOpCode(OpCode a) { opcode_ = a; };
  void Print(ostream& os, int indent=0) const; 
  IRValue *IRGen(list<Instruction*> & code, CodeGenMgr & mgr); 

};

extern const OpNode::OpInfo opInfo[];


/****************************************************************/

class ValueNode: public ExprNode {
 private:
  /* val_ field is already included in ExprNode, so no new data members */

 public:
  ValueNode(Value* val=0, int line=0, int col=0, string file="")
    : ExprNode(VALUE_NODE, val, line, col, file) {
    
  }

  ~ValueNode() {};

  void Print(ostream& os, int indent=0) const;
  IRValue *IRGen(list<Instruction*> & code, CodeGenMgr & mgr);
};


/****************************************************************/

class InvocationNode: public ExprNode {
  // Used to represent function invocation, module instantiation and 
  // emitting of output events
 private:
  string func_name_;
  vector<string> * params_;
  

 public:
  InvocationNode(const string & fname, vector<string>* param=0, 
     int line=0, int column=0, string file="")
    :ExprNode(INV_NODE, 0, line, column, file), func_name_(fname), params_(param) {}
  ~InvocationNode() {};


  vector<string>* GetParams() const { return params_;};
  
  void SetParams(vector<string>* args){ params_ = args;};

  const string GetParam(unsigned int i) const {
    return (params_ != nullptr && i < params_->size())? (const string)((*params_)[i]) : ""; 
  };


  void SetParam(const string & arg, unsigned int i) 
    { if (params_ != nullptr && i < params_->size()) (*params_)[i] = arg;};


  void Print(ostream& os, int indent=0) const;
  virtual IRValue *IRGen(list<Instruction*> & code, CodeGenMgr & mgr); 

};


/****************************************************************/

class StmtNode: public AstNode {
 public:
  enum StmtNodeKind { ILLEGAL=-1, EMPTY, PERM, IF, ASSIGN, COMPOUND};

 private:
  StmtNodeKind skind_;

 public: 
  StmtNode(StmtNodeKind skm, int line=0, int col=0, string file=""):
    AstNode(STMT_NODE, line,col,file) { skind_ = skm; };
  StmtNode( int line=0, int column=0, string file=""):
    AstNode(STMT_NODE, line,column,file) { skind_ = EMPTY; };
  ~StmtNode() {};

  StmtNodeKind StmtNodeKind() const { return skind_;}

  void Print(ostream& os, int indent) const { os << ";" ;};

};

/****************************************************************/

class CompoundStmtNode: public StmtNode {
 private: 
  vector<StmtNode*> *stmts_;

 public: 
  CompoundStmtNode(vector<StmtNode*> *stmts, 
         int line=0, int column=0, string file=""):
    StmtNode(StmtNode::COMPOUND, line, column, file) { stmts_ = stmts;}
  ~CompoundStmtNode(){};

  void AddStmt(StmtNode* s) {
    if (!stmts_) {
      stmts_ = new vector<StmtNode*>(); 
    }
    stmts_->push_back(s);
  }

  inline vector<StmtNode*> *GetStmts() { return stmts_; }
  void Print(ostream& os, int indent) const;
  virtual IRValue *IRGen(list<Instruction*> & code, CodeGenMgr & mgr); 

};
/****************************************************************/

class IfNode: public StmtNode {
 private: 
  ExprNode *cond_;
  StmtNode *then_, *else_;

 public: 
  IfNode(ExprNode* cond, StmtNode* thenStmt, StmtNode* elseStmt=nullptr, 
         int line=0, int column=0, string file=""):
    StmtNode(StmtNode::IF, line, column, file) { cond_=cond; then_=thenStmt; else_=elseStmt;}
  ~IfNode(){};

  const ExprNode* GetCond() const {return cond_;}
  const StmtNode* GetElseStmt() const { return else_;};
  const StmtNode* GetThenStmt() const  { return then_;};

  ExprNode* SetCond() {return cond_;}      
  StmtNode* SetElseStmt() { return else_;};
  StmtNode* SetThenStmt() { return then_;};

  void Print(ostream& os, int indent) const;
  virtual IRValue *IRGen(list<Instruction*> & code, CodeGenMgr & mgr); 
};

/****************************************************************/

class AssignmentNode: public StmtNode {
 private: 
  ExprNode * expr_;
  string arg_;

 public: 
  AssignmentNode(ExprNode* val_expr, const string & arg, int line=0, int column=0, string file=""):
    StmtNode(StmtNode::ASSIGN, line, column, file) { expr_ = val_expr; arg_ = arg; }
  ~AssignmentNode(){};

  void Print(ostream& os, int indent) const; 
  virtual IRValue *IRGen(list<Instruction*> & code, CodeGenMgr & mgr); 
};

/****************************************************************/


class PermNode: public StmtNode {
 public: 
  enum PermType {
    ALLOW,
    DENY,
  };
  PermNode(PermType ptype, int line=0, int column=0, string file=""):
    StmtNode(StmtNode::PERM, line, column, file) { type_ = ptype; }
  ~PermNode(){};

  void Print(ostream& os, int indent) const;
  virtual IRValue *IRGen(list<Instruction*> & code, CodeGenMgr & mgr); 
 private: 
  PermType type_;
};

/****************************************************************/

#endif
