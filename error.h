#ifndef __ERROR__H__
#define __ERROR__H__

#include <string>

// Legacy Code from CSE504

#define MAX_ERROR_BEFORE_QUIT 10000

using namespace std;
extern int yylinenum, yycolumnno;
extern const char* yyfilename;

#define WARN_LEVEL 1
#define ERROR_LEVEL 2

#define errM1(x) errMsg(""); cerr << x << endl
#define errM2(x, y) do { errMsg("", y); cerr << x << endl; } while (0)
#define warnM2(x, y) do { warnMsg("", y); cerr << x << endl; } while (0)
#define prtMsg(level, ...) \
   do { \
      if (level==WARN_LEVEL) warnMsg(__VA_ARGS__); else errMsg(__VA_ARGS__); \
   } while (0)
#define prtM2(level, x, y) \
   do { \
      if (level==WARN_LEVEL) warnM2(x, y); else errM2(x, y); \
   } while (0)

#undef errMsg
#undef warnMsg

void errMsg(string, string, int=0,int=0,string="");
void errMsg(string, int=0, int=0, string="");
void warnMsg(string, string, int=0, int=0, string="");
void warnMsg(string, int=0,int=0, string="");

void prtBufMsgs();

void resetErrs() ;
void resetWarns();
int warnCount() ;
int errCount();

#define errMsgLn(s) errMsg(s, 0, 0, yyfilename)

#define internalErr(s) internalError(s, __LINE__, __FILE__)
void internalError(const string& s, int line, string file);

#ifdef debug
#define internalWarn(s) errMsg(s); internalErr(s);
#else
#define internalWarn(s)
#endif

#endif
