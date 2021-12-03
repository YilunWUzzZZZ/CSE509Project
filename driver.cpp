#include <unistd.h>
#include <iostream>
#include <cstdio>
#include "policy.h"
#include "ast.h"
#include "error.h"
#include "IR_gen.h"
#include "IR.h"
#include <cstring>

using namespace std;

#include "parser.hpp"



#ifdef DEBUG_LEXER
#ifdef LEXER_OUT
ofstream lexOutStr(LEXER_OUT);
ostream& lexOut = lexOutStr;
#else
ostream& lexOut = cout;
#endif
#endif

extern int yyparse();
extern int yylinenum;
extern const char* yyfilename;
extern YYSTYPE yylval;

void yyerror(const char *s)
{
  errMsg(s);
}

PolicyManager manager;
string inputFile;
string outputFile;

extern FILE* yyin;

void
printUsage(const char* cmd) {
  cerr << "Usage: " << cmd;
}

int 
parseOptions(int argc, char* argv[]) {
  opterr = 0; // Suppress printing of errors by getopt.

  while (1) {
  	if ((argc > 2) || (argc < 2)) {
  		cerr << "Please specify exactly a single input file\n";
  		return -1;
  	}
  	else {
  	  inputFile.assign(argv[1], argv[1] + strlen(argv[1]));
  	  return 0;
  	}
  }

  if (inputFile.length() == 0) 
    return -1;

  return 0;
}

int 
main(int argc, char *argv[], char *envp[]) {

  int optionsOK = parseOptions(argc, argv);
  if (optionsOK < 0)
	  return -1;


  if ((yyin = fopen(inputFile.c_str(), "r")) == NULL) {
    cerr << "Unexpected error in reading input file\n";
    return 1;
  }

#ifdef TEST_LEXER
  int token;

  while ((token = yylex()) != 0) {
    if (token == TOK_LEX_ERROR) {
      cout << yyfilename << ":" << yylinenum 
        << ": Error: Unrecognized token `" << yytext << "'\n";
    }
    else {
        cout << "Token: " << token << " ";
        switch(token)
        {
          case TOK_UINT:
            cout << "Attribute: (int) " << yylval.uVal;
            break;

          case TOK_ID:
            cout << "Attribute: (id) " << yylval.cVal;
            break;

          default:
            break;
        }
        cout << endl;
    }
  }
#else
  //   yydebug = 1;
  yyparse();
  if (errCount()) {
    return 0;
  }

  manager.Print(cout);
  CodeGenMgr mgr;
  const vector<Policy*> & ps = manager.GetAllPolicy();
  for (auto p : ps) {
    if (p->Type() == Policy::SYSCALL_CHECK) {
      SyscallCheck* ck = (SyscallCheck*)p;
      ck->IRGen(mgr);
      cerr << "\n";
      ck->PrintIR();
      cerr << "\n\n";
      ck->CodeGen(mgr);
    }
  }
#endif
}
