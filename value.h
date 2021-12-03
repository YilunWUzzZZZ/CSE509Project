#ifndef VALUE_H
#define VALUE_H

#include <iostream>

using namespace std;

class Value;

enum Type {
  NONE,
  STRING,
  BOOLEAN,
  INT,
};

class Value {
 private:

  union {            // All components of this union shd be deep-copied
    char* cval_;
    int ival_;
    bool bval_;
  };

  Type type_;

 public:
 
  Value(int i) {ival_ = i; type_=Type::INT; }
  Value(char * s) {cval_ = s; type_=Type::STRING; }
  Value(bool b) { bval_ = b; type_=Type::BOOLEAN; }


  ~Value();

  Type GetType() const { return type_; } 
  void SetType(Type t) {type_ = t; } 

  
  bool IsIntegral() const { return type_ == Type::INT; }
  bool IsBoolean() const { return type_ == Type::BOOLEAN; }
  bool IsString() const { return type_ == Type::STRING; }


  char * cval() const {return cval_; }
  bool bval() const {return bval_; }
  int ival() const {return ival_; }

  void cval(char * c)  { cval_ = c; }
  void bval(bool b)  { bval_ = b; }
  void ival(int i) { ival_ = i; }

  void Print(ostream& os, int indent=0) const;

  // bool operator==(const Value&) const;
  // bool operator!=(const Value& v) const { return !operator==(v);};
  // bool operator<(const Value& v) const;
  // bool operator>(const Value& v) const { return v < *this; };
  // bool operator<=(const Value& v) const
  //     { return (*this < v) || (*this == v); }
  // bool operator>=(const Value& v) const {return v <= *this;};
  // const Value& operator=(const Value&);


};

inline ostream& operator<< (ostream& os, const Value& tt) {
  tt.Print(os);
  return os;
};

#endif
