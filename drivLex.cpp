#include <libgen.h>
#include <unistd.h>
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
using namespace std;

#include "parser.hpp"


#define CPP_PROG_NAME "CPP" // Environment variable to specify location of cpp
#define DEFAULT_CPP_PROG_NAME "cpp"
#define CC_PROG_NAME "CC" // Environment variable to specify C compiler
#define DEFAULT_CC_PROG_NAME "gcc -g"
#define CC_PROG_OPTS "CFLAGS"

#define MAX_DEBUG_LEVEL 2

extern YYSTYPE yylval;

extern int yylinenum;
extern const char* yyfilename;
extern char *yytext;
extern int yylex();

//SymTabMgr stm;
string outputFile;
string inputFile = "";
string cppShellCmd, ccShellCmd;

int debugLevel;
bool genCCode;
bool genBinCode;
bool genSharedLib;
bool genDottyCode;
bool genPrologCode;
string outFileSuffix;  


extern FILE* yyin;

void errMsg(const char* s) {
  cerr << yyfilename << ":" << yylinenum << ":" << s << endl;
};

void yyerror(const char *s)
{
  errMsg(s);
}


int 
parseOptions(int argc, char* argv[]) {
  opterr = 0; // Suppress printing of errors by getopt.

  // if (getenv(CPP_PROG_NAME) == NULL)
	//  cppShellCmd = DEFAULT_CPP_PROG_NAME;  
  // else 
  //   cppShellCmd = getenv(CPP_PROG_NAME);
  // cppShellCmd += " ";

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

  string ccCmd;
  int optionsOK = parseOptions(argc, argv);
  if (optionsOK < 0)
	 return -1;

  yyfilename = inputFile.c_str();
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
  yyparse();
#endif

  return 0;
}
