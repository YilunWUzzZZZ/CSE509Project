#ifndef TYPE_H
#define TYPE_H

#include <string>
#include <vector>
#include "Error.h"


class Type;
class Value;
class ProgramElem;
class SymTabEntry; 

class Type {
 public:
  // Dont change the order below --- it is necessary for isSubType()
  // logic to work correctly.
  enum BaseTypeTag  {
    ERROR, UNKNOWN, VOID, 
    STRING,
    BOOL, 
    BIT,
    UCHAR, CHAR, 
    USHORT, SHORT,
    ENUM, 
    UINT, INT,
    ULONG, LONG, 
    DOUBLE,
    VECTOR, SET, DICT, TUPLE, 
	FUNCTION,
    NUMERIC, /* BIT to DOUBLE */
    INTEGRAL, /* BIT to LONG */
    SIGNED, /* CHAR, SHORT, INT, LONG, DOUBLE */
    UNSIGNED, /* UCHAR, USHORT, UINT, ULONG */
    SCALAR, /* STRING to DOUBLE */
    PRIMITIVE, /* STRING to DOUBLE */
    DICTORSET, 
    CONTAINER, /* VECTOR, SET, DICT */
    CONTAINER_OR_STR, 
    NATIVE, /* Not STRUCT, CLASS or VOID */
    DATA, /* VOID to TUPLE */
    ANY   /* VOID to FUNCTION */
  };

  class TypeTag {
   private:
     BaseTypeTag tag_: 7;
     bool isConst_: 1;

   public:
     TypeTag() { tag_ = ERROR; isConst_ = 0; }
     TypeTag(BaseTypeTag t, bool isConst=false) 
        { tag_ = t; isConst_ = isConst; }

     BaseTypeTag tag() const { return tag_; };
     bool isConst() const { return isConst_; };
     void isConst(bool c) { isConst_ = c; };

     //operator BaseTypeTag() const { return tag_; };
     bool operator==(const TypeTag& t) const 
        { return (tag_ == t.tag_ && isConst_ == t.isConst_); }
     bool operator!=(const TypeTag& t) const { return !(*this == t);}

     bool operator==(BaseTypeTag t) const { return tag_ == t; }
     bool operator!=(BaseTypeTag t) const { return tag_ != t; }
     bool operator< (BaseTypeTag t) const { return tag_ <  t; }
     bool operator<=(BaseTypeTag t) const { return tag_ <= t; }
     bool operator> (BaseTypeTag t) const { return tag_ >  t; }
     bool operator>=(BaseTypeTag t) const { return tag_ >= t; }
  };

 public:
  static const int SIZE_UNDEF;

  static const string name(TypeTag t);
  static const string codeName(TypeTag t, bool inclConst=false);
  static int size(BaseTypeTag t);

  static const Type errorType, voidType, unkType;
  static const Type boolType;
  static const Type stringType, constStringType; 
  static const Type bitType, ucharType, charType, ushortType, 
    shortType, uintType, constUintType, intType, 
    ulongType, longType,
    doubleType;
  
  static const Type* type[];
  static bool isConst(TypeTag t) { return (t.isConst()); }
  static bool isTagOnly(TypeTag t) { return (t <= DOUBLE); };
  static bool isString(TypeTag t) { return (t==STRING);}
  static bool isNumeric(TypeTag t) { return ((t >= BIT) && (t <= DOUBLE));}
  static bool isBool(TypeTag t) { return (t == BOOL); }
  static bool isInt(TypeTag t) {return (t >= BIT && t <= INT); }
  static bool isLong(TypeTag t) {	return (t >= ULONG && t <= LONG);}
  static bool isIntegral(TypeTag t) { return ((t >= BIT) && (t <= LONG));}
  static bool isSigned(TypeTag t) {
    return ((t==CHAR) || (t==SHORT) || (t==INT) || (t==LONG) || (t==DOUBLE)); }
  static bool isUnsigned(TypeTag t) { return (isIntegral(t) && !isSigned(t));}
  static bool isDouble(TypeTag t) {return (t==DOUBLE);}
  static bool isPrimitive(TypeTag t) { return ((t >= STRING) && (t <= DOUBLE));}
  static bool isContainer(TypeTag t) { return (t==VECTOR || t==SET || t==DICT);}
  static bool isContainerOrStr(TypeTag t) {return (t==STRING || isContainer(t));}
  static bool isNative(TypeTag t) {
    return ((t > VOID)); }
  static bool isData(TypeTag t) { return ((t >= VOID) && (t <= TUPLE));}
  static bool isValid(TypeTag t) { return ((t >= VOID) && (t <= FUNCTION)); }
  static bool isScalar(TypeTag t) { return ((t >= STRING) && (t <= DOUBLE)); }
  static bool isTuple(TypeTag t) { return (t == TUPLE); }
  static bool isDict(TypeTag t) { return (t == DICT); }
  static bool isVector(TypeTag t) { return (t == VECTOR); }
  static bool isSet(TypeTag t) { return (t == SET); }
  static bool isVecOrSet(TypeTag t) { return (t == SET || t == VECTOR); }
  static bool isDictOrSet(TypeTag t) { return (t == SET || t == DICT); }

 private:
  TypeTag tag_;
  Type* keyType_;         // Used by dict
  union {
    SymTabEntry* typeDesc_; // enum, fun
    Type* elemType_;        // Used by list, set and dict
    struct {                // For function and tuples
      Type* retType_;       // Applicable only for functions
      vector<Type*>* argTypes_;  // Applicable for tuples
    };
  };

 public:
  Type(TypeTag tag=VOID);                    // All elementary types
  Type(SymTabEntry* td, TypeTag t);          // Must be enum
  Type(Type* elemType, TypeTag t);           // For list and set types
  Type(Type* keyType, Type* elemType);       // For dict type         
  Type(vector<Type*>* tupleType, TypeTag t); // For tuples
  Type(vector<Type*>* argt, Type* rt);       // For functions
  //Type(const Type& t) {operator=(t);};
  ~Type() {};
  
  const Type& operator=(const Type&);
  // Assignment does a deep copy for all fields except typeDesc_, which is
  // is a SymTabEntry* and has a reference semantics. (Note SymTabEntry
  // doesn't even have a copy constructor or assignment.)

  bool isConst() const { return tag_.isConst(); }; 

  string name() const {return name(tag_); }
  string fullName() const;
  string codeName(bool inclConst=false) const 
     {return codeName(tag_, inclConst); }
  string fullCodeName(bool inclConst=false) const;

  BaseTypeTag tag() const 
   { return tag_.tag(); }

  const SymTabEntry* typeDesc() const {
    if (tag_ != ENUM) return nullptr;
    else return typeDesc_;
  };
  SymTabEntry* typeDesc() {
    if (tag_ != ENUM) return nullptr;
    else return typeDesc_;
  };

  const Type* keyType() const { 
    if (tag_ == VECTOR || tag_ == SET) 
       return elemType_; 
    if (tag_ != DICT) return nullptr;
    else return keyType_; 
  };

  Type* keyType() { 
    if (tag_ == VECTOR || tag_ == SET) 
       return elemType_; 
    if (tag_ != DICT) return nullptr;
    else return keyType_; 
  };

  const Type* elemType() const { 
    if (tag_ != VECTOR && tag_ != SET && tag_ != DICT) return nullptr;
    else return elemType_; 
  };

  Type* elemType() { 
    if (tag_ != VECTOR && tag_ != SET && tag_ != DICT) return nullptr;
    else return elemType_; 
  };

  int arity() const {
    if (((tag_ != TUPLE) && 
		(tag_ != FUNCTION))||(argTypes_ == nullptr)) return 0;
    else return argTypes_->size(); 
  };  

  const vector<const Type*>* argTypes() const { 
    if ((tag_ != TUPLE) && (tag_ != FUNCTION)) return nullptr;
    else return (vector<const Type*>*) argTypes_; 
  };
  
  vector<Type*>* argTypes() { 
    if ((tag_ != TUPLE) && (tag_ != FUNCTION)) return nullptr;
    else return argTypes_; 
  };
  
  const Type* retType() const {
    if (tag_ != FUNCTION) return nullptr;
    else return retType_;
  };

  void constType(bool b=true) { tag_.isConst(b); };
  void tag(TypeTag t) { tag_ = t; }
  void typeDesc(SymTabEntry* ste) {
    if ((tag_==ENUM)) typeDesc_ = ste; 
    //else errMsgLn("Type::typeDesc(SymTabEntry*) called when type = " + name()); 
  };

  void elemType(Type *t) { 
    if (tag_ == VECTOR || tag_ == SET || tag_ == DICT) elemType_ = t;
    //else errMsgLn("Type::elemType(Type*) called when type = " + name()); 
  };

  void argTypes(vector<Type*>* t) { 
    if ((tag_ == TUPLE) || (tag_ == FUNCTION)) argTypes_ = t;
    //else errMsgLn("Type::argTypes(vector<Type*> *) called when type = " + name()); 
  };
  void retType(Type* t) {
    if (tag_ == FUNCTION) retType_ = t;
    //else errMsgLn("Type::retType(Type *) called when type = " + name());
  };

  void print(ostream &os, int indent=0) const;

  bool isValid() const { return isValid(tag_); };
  bool isBool() const { return isBool(tag()); };
  bool isString() const { return isString(tag()); };
  bool isIntegral() const { return isIntegral(tag()); };
  bool isNumeric() const { return isNumeric(tag()); };
  bool isScalar() const { return isScalar(tag()); };
  bool isDouble() const {return isDouble(tag());}

  bool isAssignable(const Type& t) const ; /* initialization with literal*/
  bool isSubType(const Type& t, bool relaxed=0) const; // Is this a subtype of t?
  bool isSubType(TypeTag t) const;   // Is this a subtype of t?
  bool isEquiv(const Type& t) const      // Type equivalence
    { return (isSubType(t) && t.isSubType(*this)); };
   
  void unify(const Type* t, const ProgramElem* pe); // Updates this to the most 
  // general type that's a subtype of this and t. Doesn't handle type vars (yet).

  bool operator==(const Type& t) const;  // strict equality
  bool operator!=(const Type& t) const { return !operator==(t);};

  void printArgs(ostream& os, int indent=0)const;
};

inline ostream& operator<< (ostream& os, const Type& tt) {
  tt.print(os);
  return os;
};

#endif
